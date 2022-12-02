#include "CombatComponent.h"
#include "BadassMultiplayer/Character/MultiplayerCharacter.h"
#include "BadassMultiplayer/Weapon/Weapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "BadassMultiplayer/PlayerController/MPPlayerController.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"


UCombatComponent::UCombatComponent():
	BaseWalkSpeed(600.f),
	AimWalkSpeed(450.f)
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// We want this data to replicate everywhere so that the animation is shown on server and clients. So no need for condition
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bIsAiming);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		if (Character->GetCamera())
		{
			DefaultFOV = Character->GetCamera()->FieldOfView;
			CurrentFOV = Character->GetCamera()->FieldOfView;
		}
	}

}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		// These only need to be done on locally controlled people not simulated proxies -> EFFICIENCYYYYYYYYY
		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}

}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	const USkeletalMeshSocket* RightHandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (RightHandSocket)
	{
		RightHandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}

	// The Owner is a built in replicated variable. So when we change the owner, it will be replicated across clients
	EquippedWeapon->SetOwner(Character);
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		// This needs to be done here in case the weapon state isn't set. Calling it here makes the variable replicate across clients before the weapon is attached
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		const USkeletalMeshSocket* RightHandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (RightHandSocket)
		{
			RightHandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
		}

		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::FireButtonPressed(bool bFireIsPressed)
{
	bFireButtonPressed = bFireIsPressed;
	if (bFireButtonPressed && EquippedWeapon)
	{
		Fire();
	}
	
}

void UCombatComponent::Fire()
{
	if (bCanFire)
	{
		bCanFire = false;
		// Only trace when fire button is pressed then send the hit target (impact point) to the RPC's to do all the weapon firing across machines
		ServerFire(HitTarget);

		if (EquippedWeapon)
		{
			CrosshairShootingFactor = 1.f;
		}

		StartFireTimer();
	}
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(FireTimer, this, &ThisClass::EndFireTimer, EquippedWeapon->FireDelay);
}

void UCombatComponent::EndFireTimer()
{
	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bIsAutomatic)
	{
		Fire();
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	NetMulticastFire(TraceHitTarget);
}

void UCombatComponent::NetMulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;
	if (Character)
	{
		Character->PlayFireMontage(bIsAiming);
		EquippedWeapon->FireWeapon(TraceHitTarget);
	}
}

void UCombatComponent::SetIsAiming(bool bAiming)
{
	bIsAiming = bAiming;

	// When we call an RPC on a client machine then it gets excecuted on the server and on the client machine when the UPROPERTY has the Server argument
	// When the server calls this it will be set on the server anyway so its kinda redundant but not big deal
	ServerSetIsAiming(bAiming);
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::ServerSetIsAiming_Implementation(bool bAiming)
{
	bIsAiming = bAiming;

	// Server needs to know how fast we are moving so it can be replicated
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& HitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairCenter(ViewportSize.X / 2, ViewportSize.Y / 2);

	FVector CrosshairWorldPosition, CrosshairWorldDirection;
	UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairCenter,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	FVector Start = CrosshairWorldPosition;

	// Move trace start in front of the character so we don't target ourselves or players behind us
	if (Character)
	{
		float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
		Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
	}

	FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

	GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECollisionChannel::ECC_Visibility
	);	

	if (HitResult.GetActor() && HitResult.GetActor()->Implements<UCrosshairsInterface>())
	{
		Package.CrosshairColor = FLinearColor::Red;
	}
	else
	{
		Package.CrosshairColor = FLinearColor::White;
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;

	Controller = Controller == nullptr ? Cast<AMPPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<ABadassHUD>(Controller->GetHUD()) : HUD;
		if (HUD)
		{
			if (EquippedWeapon)
			{
				Package.CrosshairCenter = EquippedWeapon->CrosshairCenter;
				Package.CrosshairTop = EquippedWeapon->CrosshairTop;
				Package.CrosshairBottom = EquippedWeapon->CrosshairBottom;
				Package.CrosshairLeft = EquippedWeapon->CrosshairLeft;
				Package.CrosshairRight = EquippedWeapon->CrosshairRight;
			}
			else
			{
				Package.CrosshairCenter = nullptr;
				Package.CrosshairTop = nullptr;
				Package.CrosshairBottom = nullptr;
				Package.CrosshairLeft = nullptr;
				Package.CrosshairRight = nullptr;
			}

			// Calculate Crosshair Spread
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0;
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());
			
			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairJumpFactor = FMath::FInterpTo(CrosshairJumpFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairJumpFactor = FMath::FInterpTo(CrosshairJumpFactor, 0.f, DeltaTime, 30.f);
			}

			if (bIsAiming && EquippedWeapon)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, -0.58f, DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairJumpFactor, 0.f, DeltaTime, ZoomUninterpSpeed);
			}

			// When firing it is set to 0.8 but after it will go down to 0
			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);
			
			
			Package.CrosshairsSpread = 0.5f + CrosshairVelocityFactor + CrosshairJumpFactor + CrosshairAimFactor + CrosshairShootingFactor;

			HUD->SetHUDPackage(Package);
		}
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;

	if (bIsAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomUninterpSpeed); // Uninterp all weapons at same speed
	}

	if (Character && Character->GetCamera())
	{
		Character->GetCamera()->SetFieldOfView(CurrentFOV);
	}

}





