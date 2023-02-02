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
#include "Sound/SoundCue.h"
#include "BadassMultiplayer/Character/Multiplayer_AnimInstance.h"
#include "BadassMultiplayer/Weapon/Projectile.h"


UCombatComponent::UCombatComponent():
	AimWalkSpeed(450.f),
	BaseWalkSpeed(600.f)
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// We want this data to replicate everywhere so that the animation is shown on server and clients. So no need for condition
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bIsAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, Grenades);
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

		if (Character->HasAuthority())
		{
			InitializeCarriedAmmoMap();
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
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	DropEquippedWeapon();

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	AttachActorToRightHand(EquippedWeapon);

	// The Owner is a built in replicated variable. So when we change the owner, it will be replicated across clients
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();

	UpdateCarriedAmmo();
	PlayWeaponEquipSound();
	ReloadWeaponIfEmpty();

	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::DropEquippedWeapon()
{
	// Drop Weapon if picking another one up
	if (EquippedWeapon)
	{
		EquippedWeapon->DropWeapon();
		if (Controller)
		{
			Controller->SetHUDWeaponType(EWeaponType::EWT_MAX);
		}
	}
}

void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;

	const USkeletalMeshSocket* RightHandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (RightHandSocket)
	{
		RightHandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;

	const USkeletalMeshSocket* RightHandSocket = Character->GetMesh()->GetSocketByName(FName("LeftHandSocket"));
	if (RightHandSocket)
	{
		RightHandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr) return;

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Cast<AMPPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
		Controller->SetHUDWeaponType(EquippedWeapon->GetWeaponType());
	}
}

void UCombatComponent::PlayWeaponEquipSound()
{
	if (Character && EquippedWeapon && EquippedWeapon->WeaponEquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->WeaponEquipSound, Character->GetActorLocation());
	}
}

void UCombatComponent::ReloadWeaponIfEmpty()
{
	if (EquippedWeapon && EquippedWeapon->IsWeaponEmpty())
	{
		Reload();
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		Controller = Controller == nullptr ? Cast<AMPPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDWeaponType(EquippedWeapon->GetWeaponType());
		}

		// This needs to be done here in case the weapon state isn't set. Calling it here makes the variable replicate across clients before the weapon is attached
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon);

		PlayWeaponEquipSound();

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
	if (CanFire())
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
	ReloadWeaponIfEmpty();
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr) return false;
	
	// New Case for Shotgun to fire in between reload anim
	if (!EquippedWeapon->IsWeaponEmpty() && bCanFire && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun)
	{
		return true;
	}

	// Return true if weapon is NOT empty and if we CAN fire (based on auto fire variable) and that we are unoccupied
	return !EquippedWeapon->IsWeaponEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;

}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	NetMulticastFire(TraceHitTarget);
}

void UCombatComponent::NetMulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;

	if (Character && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun)
	{
		Character->PlayFireMontage(bIsAiming);
		EquippedWeapon->FireWeapon(TraceHitTarget);
		CombatState = ECombatState::ECS_Unoccupied;
		return;
	}

	if (Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bIsAiming);
		EquippedWeapon->FireWeapon(TraceHitTarget);
	}
}

void UCombatComponent::Reload()
{
	if (CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied && EquippedWeapon && !EquippedWeapon->IsWeaponFull())
	{
		ServerReload();
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	CombatState = ECombatState::ECS_Reloading; // OnRep 
	HandleReload();
}

void UCombatComponent::FinishReload()
{
	if (Character == nullptr) return;

	if (Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}
	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::ShotgunShellInsert()
{
	if (Character && Character->HasAuthority())
	{
		UpdateShotgunAmmoValues();
	}
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed)
		{
			Fire();
		}
		break;
	case ECombatState::ECS_ThrowingGrenade:
		// Plays Animation on all other clients
		if (Character && !Character->IsLocallyControlled())
		{
			Character->PlayThrowGrenadeMontage();
			AttachActorToLeftHand(EquippedWeapon);
			ShowAttachedGrenade(true);
		}
		break;
	}
}

void UCombatComponent::HandleReload()
{
	Character->PlayReloadMontage();
}

int32 UCombatComponent::CalculateReloadAmount()
{
	if (EquippedWeapon == nullptr) return 0;

	int32 EmptySpaceInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		// Determine if our carried ammo is less than the space in the mag cuase then that is the amount we have to reload 
		int32 Least = FMath::Min(AmountCarried, EmptySpaceInMag);
		return FMath::Clamp(EmptySpaceInMag, 0, Least);
	}

	return 0;
}

// Called Locally On Client
void UCombatComponent::ThrowGrenade()
{
	if (Grenades == 0) return;
	if (CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon == nullptr) return;

	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}

	if (Character && !Character->HasAuthority())
	{
		ServerThrowGrenade();
	}

	if (Character && Character->HasAuthority())
	{
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
		UpdateGrenadeHUD();
	}
}

// Plays Anim on Server Instance
void UCombatComponent::ServerThrowGrenade_Implementation()
{
	if (Grenades == 0) return;

	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}

	Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
	UpdateGrenadeHUD();
}

void UCombatComponent::OnRep_Grenades()
{
	UpdateGrenadeHUD();
}

void UCombatComponent::UpdateGrenadeHUD()
{
	Controller = Controller == nullptr ? Cast<AMPPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDGrenades(Grenades);
	}
}

void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if (Character && Character->GetGrenadeMesh())
	{
		Character->GetGrenadeMesh()->SetVisibility(bShowGrenade);
	}
}

void UCombatComponent::FinishThrowingGrenade()
{
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
}

void UCombatComponent::LaunchGrenade()
{
	ShowAttachedGrenade(false);
	if (Character && Character->IsLocallyControlled())
	{
		ServerLaunchGrenade(HitTarget);
	}
}

void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
	if (Character && Character->GetGrenadeMesh() && GrenadeClass)
	{
		const FVector StartingGrenadeLocation = Character->GetGrenadeMesh()->GetComponentLocation();
		FVector ToTarget = Target - StartingGrenadeLocation;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		GetWorld()->SpawnActor<AProjectile>(GrenadeClass, StartingGrenadeLocation, ToTarget.Rotation(), SpawnParams);
	}
}

void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{
	if (CarriedAmmoMap.Contains(WeaponType))
	{
		CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + AmmoAmount, 0, MaxCarriedAmmo);
		UpdateCarriedAmmo();
	}

	if (EquippedWeapon && EquippedWeapon->IsWeaponEmpty() && EquippedWeapon->GetWeaponType() == WeaponType)
	{
		Reload();
	}
}

void UCombatComponent::UpdateAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	// Calculate Amount to Reload and update the ammo map
	int32 AmountToReload = CalculateReloadAmount();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= AmountToReload;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	// Update the amount of ammo in the mag of the weapon
	EquippedWeapon->AddAmmo(AmountToReload);

	// Update the HUD for the new carried ammo amount
	Controller = Controller == nullptr ? Cast<AMPPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::UpdateShotgunAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	// Update the amount of ammo in the mag of the weapon
	EquippedWeapon->AddAmmo(1);

	// Update the HUD for the new carried ammo amount
	Controller = Controller == nullptr ? Cast<AMPPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}

	bCanFire = true;
		
	// End the shotgun reload notifies if the shotgun is full and jump to finishreloading notify
	if (EquippedWeapon->IsWeaponFull() || CarriedAmmo == 0)
	{
		JumpToShotgunReloadEnd();
	}
}

void UCombatComponent::JumpToShotgunReloadEnd()
{
	// Dont play any animation if the character doesn't have a weapon
	if (EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (AnimInstance && Character->GetReloadMontage())
	{
		AnimInstance->Montage_Play(Character->GetReloadMontage());
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
}

void UCombatComponent::SetIsAiming(bool bAiming)
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	bIsAiming = bAiming;

	// When we call an RPC on a client machine then it gets excecuted on the server and on the client machine when the UPROPERTY has the Server argument
	// When the server calls this it will be set on the server anyway so its kinda redundant but not big deal
	ServerSetIsAiming(bAiming);
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bAiming ? AimWalkSpeed : BaseWalkSpeed;
	}

	if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		Character->ShowSniperScopeWidget(bAiming);
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

void UCombatComponent::OnRep_CarriedAmmo()
{
	Controller = Controller == nullptr ? Cast<AMPPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}

	bool bJumpToShotgunEnd = CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon != nullptr &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun &&
		CarriedAmmo == 0;

	if (bJumpToShotgunEnd)
	{
		JumpToShotgunReloadEnd();
	}
}

void UCombatComponent::InitializeCarriedAmmoMap()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingAssaultRifleAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketLauncherAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SMG, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingSniperRifleAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeLauncherAmmo);
}









