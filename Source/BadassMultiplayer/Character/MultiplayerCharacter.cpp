// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "BadassMultiplayer/Weapon/Weapon.h"
#include "BadassMultiplayer/MultiplayerComponents/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"


AMultiplayerCharacter::AMultiplayerCharacter() :
	TurningState(ETurningState::ETIP_Still)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	// Attach to mesh so that when we crouch or change capsule size it won't effect spring arm elevation
	SpringArm->SetupAttachment(GetMesh());
	SpringArm->TargetArmLength = 600.f;
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	// No need to register components since they are special and get replicated themselves
	Combat->SetIsReplicated(true);
	
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	// How fast we want data to be sent per second across the network
	NetUpdateFrequency = 66.f;
	// The slowest we want the network to send data across the network
	MinNetUpdateFrequency = 33.f;

	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);


}

void AMultiplayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	/*
	* This line registers OverlappingWeapon to be replicated but it replicates on all clients when we only want it to happen on one clients screen not everyone.
	* Meaning we only want a widget to show on the client who overlaps. Other clients don't need to see the other's pickup widget
	*		DOREPLIFETIME(AMultiplayerCharacter, OverlappingWeapon);
	*/ 

	/*
	* To only have the OverlappingWeapon replicated on the client we have to specify a condition to only replicate there.
	* COND_OwnerOnly is the owner of the pawn on that owner's machine
	* Problem is this replicated to that client and to the server (this is a big issue if the game is using a listen server like this one)
	* To fix this we use RepNotifies (look at .h file)
	*/
	DOREPLIFETIME_CONDITION(AMultiplayerCharacter, OverlappingWeapon, COND_OwnerOnly);
}

void AMultiplayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	StartingAimRotation = GetBaseAimRotation();	
}

void AMultiplayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Set value of the character on the combat component asap so we don't crash
	if (Combat)
	{
		Combat->Character = this;
	}
}

void AMultiplayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CalculateAO(DeltaTime);
}

void AMultiplayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ThisClass::Jump);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ThisClass::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ThisClass::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ThisClass::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ThisClass::AimButtonReleased);

	PlayerInputComponent->BindAxis("MoveForward", this, &ThisClass::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ThisClass::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ThisClass::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ThisClass::LookUp);

}

void AMultiplayerCharacter::MoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation( 0.f, Controller->GetControlRotation().Yaw, 0.f );
		const FVector Direction( FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X) );
		AddMovementInput(Direction, Value);
	}
}

void AMultiplayerCharacter::MoveRight(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void AMultiplayerCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AMultiplayerCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void AMultiplayerCharacter::Jump()
{
	if (bIsCrouched)
	{
		// Make character stand up is space bar is pressed when crouching
		UnCrouch();
	}
	else
	{
		// Make character jump if space bar is pressed when standing
		Super::Jump();
	}
}


void AMultiplayerCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	/*
	* On the server when we end overlap we make the OverlappingWeapon nullptr but the widget does not go away since the rep notify is only called on client
	* So if the overalapping weapon is still valid on the end overlap we can check and then hide the widget
	* 
	* ALSO!!!!
	* When server overlaps with weapon, server can see widget PickUp, but if another client also overlaps the same weapon,
	* first condition in the code will hide pickup widget for  server. So when we check for OverlappingWeapon we also need to check IsLocallyControlled() is true
	* to make sure that the widget only is hidden if its the local server player that leaves
	*/
	if (IsLocallyControlled() && OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = Weapon;

	/*
	* IsLocallyControlled() is Called on the character that is locally controlled
	* Since SetOverlappingWeapon() is only called on the server from Weapon.cpp, this means it will only be called for the listen server player
	* OverlappingWeapon is not going to be replicated on the server since we set the condition to register its replication to only happen on Owners (line 52)
	*/
	if (IsLocallyControlled())
	{
		// Show pickup widget on server controlled character
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}

}


void AMultiplayerCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	// In this case the last value will be null since on the overlap this will be set to the overlapping weapon
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}

	/*
	* In this case the last value will be of the overlapping weapon since on the end of the overlap the OverlappingWeapon will be set to nullptr
	* But since this is happening in the Rep Notify it will only happen on clients and not on the server
	*/ 
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void AMultiplayerCharacter::EquipButtonPressed()
{
	// Weapon Equipping should be handled by server so that a proper record of the game is kept
	if (Combat)
	{
		if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			// Called for clients
			ServerEquipButtonPressed();
		}
	}
}

void AMultiplayerCharacter::CrouchButtonPressed()
{
	// Crouch() and UnCrouch()  are inherited from Character class and replicate the bIsCrouched variable
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void AMultiplayerCharacter::AimButtonPressed()
{
	if (Combat)
	{
		Combat->SetIsAiming(true);
	}
}

void AMultiplayerCharacter::AimButtonReleased()
{
	if (Combat)
	{
		Combat->SetIsAiming(false);
	}
}

void AMultiplayerCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}


bool AMultiplayerCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool AMultiplayerCharacter::GetIsAiming()
{
	return (Combat && Combat->bIsAiming);
}

AWeapon* AMultiplayerCharacter::GetEquippedWeapon()
{
	if(Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

void AMultiplayerCharacter::CalculateAO(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;

	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();

	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0 && !bIsInAir) // Not Moving or Jumping
	{
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		const FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaRotation.Yaw;

		if (TurningState == ETurningState::ETIP_Still)
		{
			Interp_AO_Yaw = AO_Yaw;
		}

		TurnInPlace(DeltaTime);
		bUseControllerRotationYaw = true;
	}
	if (Speed > 0.f || bIsInAir) // Moving or Jumping
	{
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningState = ETurningState::ETIP_Still;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;
	// Rotation data is compressed before it is sent across the network, and this causes any negative angles values
	// To become unsigned so we need to map them back to original values
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void AMultiplayerCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningState = ETurningState::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningState = ETurningState::ETIP_Left;
	}

	if (TurningState != ETurningState::ETIP_Still)
	{
		Interp_AO_Yaw = FMath::FInterpTo(Interp_AO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = Interp_AO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningState = ETurningState::ETIP_Still;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
		
	}
}






