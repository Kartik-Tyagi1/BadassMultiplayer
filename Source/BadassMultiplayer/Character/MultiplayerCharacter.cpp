// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "BadassMultiplayer/Weapon/Weapon.h"
#include "BadassMultiplayer/MultiplayerComponents/CombatComponent.h"


AMultiplayerCharacter::AMultiplayerCharacter()
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

}

void AMultiplayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ThisClass::EquipButtonPressed);

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
	if (Combat && HasAuthority())
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}






