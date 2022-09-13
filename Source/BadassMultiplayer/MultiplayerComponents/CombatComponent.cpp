#include "CombatComponent.h"
#include "BadassMultiplayer/Character/MultiplayerCharacter.h"
#include "BadassMultiplayer/Weapon/Weapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

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
	}

}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FHitResult HitResult;
	TraceUnderCrosshairs(HitResult);

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

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::FireButtonPressed(bool bFireIsPressed)
{
	bFireButtonPressed = bFireIsPressed;
	if (bFireButtonPressed)
	{
		ServerFire();
	}
	
}


void UCombatComponent::ServerFire_Implementation()
{
	NetMulticastFire();
}

void UCombatComponent::NetMulticastFire_Implementation()
{
	if (EquippedWeapon == nullptr) return;
	if (Character)
	{
		Character->PlayFireMontage(bIsAiming);
		EquippedWeapon->FireWeapon(HitTarget);
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
	FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

	GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECollisionChannel::ECC_Visibility
	);

	if (!HitResult.bBlockingHit)
	{
		HitResult.ImpactPoint = End;
		HitTarget = End;
	}
	else
	{
		HitTarget = HitResult.ImpactPoint;
		DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 12.f, 12, FColor::Red);
	}
		
}



