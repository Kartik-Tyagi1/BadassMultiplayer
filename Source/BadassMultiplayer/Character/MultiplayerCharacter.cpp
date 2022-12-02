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
#include "Multiplayer_AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "BadassMultiplayer/BadassMultiplayer.h"
#include "BadassMultiplayer/PlayerController/MPPlayerController.h"
#include "BadassMultiplayer/GameModes/BamGameMode.h"
#include "TimerManager.h"


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
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	// How fast we want data to be sent per second across the network
	NetUpdateFrequency = 66.f;
	// The slowest we want the network to send data across the network
	MinNetUpdateFrequency = 33.f;

	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));


}

void AMultiplayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	/*
	* This line registers OverlappingWeapon to be replicated but it replicates on all clients when we only want it to happen on one clients screen not everyone.
	* Meaning we only want a widget to show on the client who overlaps. Other clients don't need to see the other's pickup widget
	*		DOREPLIFETIME(AMultiplayerCharacter, OverlappingWeapon);
	*/ 
	DOREPLIFETIME(AMultiplayerCharacter, Health);
	
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

	if (HasAuthority())
	{
		// Only handle damage events on the server
		OnTakeAnyDamage.AddDynamic(this, &ThisClass::RecieveDamage);
	}

	UpdateHUDHealth();
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

	// Greater than compares the enum values
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		// This happens on the locally controlled player
		CalculateAO(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementRep += DeltaTime;
		if (TimeSinceLastMovementRep > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAOPitch();
	}

	HideCamera();
}

void AMultiplayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ThisClass::Jump);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ThisClass::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ThisClass::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ThisClass::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ThisClass::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ThisClass::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ThisClass::FireButtonReleased);

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

void AMultiplayerCharacter::FireButtonPressed()
{
	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void AMultiplayerCharacter::FireButtonReleased()
{
	if (Combat)
	{
		Combat->FireButtonPressed(false);
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

void AMultiplayerCharacter::OnRep_ReplicatedMovement()
{
	// Since this is replicated when sim proxy movement is changed then we dont need to call this in tick
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurnInPlace();
	TimeSinceLastMovementRep = 0.f;
}

void AMultiplayerCharacter::CalculateAO(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;

	float Speed = CalculateSpeed();

	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0 && !bIsInAir) // Not Moving or Jumping
	{
		bRotateRootBone = true;
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
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningState = ETurningState::ETIP_Still;
	}

	CalculateAOPitch();
	
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

void AMultiplayerCharacter::CalculateAOPitch()
{
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

void AMultiplayerCharacter::SimProxiesTurnInPlace()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	if(Speed > 0.f)
	{
		TurningState = ETurningState::ETIP_Still;
		return;
	}
	CalculateAOPitch();

	// Turn in Place
	SimProxyRotationLastFrame = SimProxyRotationCurrent;
	SimProxyRotationCurrent = GetActorRotation();
	SimYawDelta = UKismetMathLibrary::NormalizedDeltaRotator(SimProxyRotationCurrent, SimProxyRotationLastFrame).Yaw;
	if (FMath::Abs(SimYawDelta) > TurnThreshold)
	{
		if (SimYawDelta > TurnThreshold)
		{
			TurningState = ETurningState::ETIP_Right;
		}
		else if (SimYawDelta < -TurnThreshold)
		{
			TurningState = ETurningState::ETIP_Left;
		}
		else
		{
			TurningState = ETurningState::ETIP_Still;
		}
		return;
	}

	TurningState = ETurningState::ETIP_Still;
}

void AMultiplayerCharacter::PlayFireMontage(bool bIsAiming)
{
	// Dont play any animation if the character doesn't have a weapon
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName FireSectionName = bIsAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(FireSectionName);
	}
}

FVector AMultiplayerCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();

	return Combat->HitTarget;
}

void AMultiplayerCharacter::PlayHitReactMontage()
{
	// Dont play any animation if the character doesn't have a weapon
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("HitFromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AMultiplayerCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void AMultiplayerCharacter::RecieveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	// On Server
	UpdateHUDHealth();
	PlayHitReactMontage(); 

	// Elimination will be handled in the GameMode
	if (Health == 0.f)
	{
		ABamGameMode* BamGameMode = GetWorld()->GetAuthGameMode<ABamGameMode>();
		if (BamGameMode)
		{
			MPPlayerController = MPPlayerController == nullptr ? Cast<AMPPlayerController>(Controller) : MPPlayerController;
			AMPPlayerController* AttackerPlayerController = Cast< AMPPlayerController>(InstigatorController);
			BamGameMode->PlayerEliminated(this, MPPlayerController, AttackerPlayerController);
		}
	}
}

void AMultiplayerCharacter::UpdateHUDHealth()
{
	MPPlayerController = Cast<AMPPlayerController>(Controller);
	if (MPPlayerController)
	{
		MPPlayerController->SetHUDHealthStats(Health, MaxHealth);
	}
}

void AMultiplayerCharacter::Eliminated()
{
	if (Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->DropWeapon();
	}

	// Call Elimination on server and clients
	MulticastEliminated();

	// Start the Respawn Timer
	GetWorldTimerManager().SetTimer(RespawnTimer, this, &ThisClass::EndRespawnTimer, RespawnDelay);
}

void AMultiplayerCharacter::MulticastEliminated_Implementation()
{
	bIsEliminated = true;
	PlayElimMontage();

	// Dissolve Effect
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 300.f);
	}
	StartDissolve();

	// Stop Player Input/Movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if (MPPlayerController)
	{
		DisableInput(MPPlayerController);
	}
	// Stop Collisions
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMultiplayerCharacter::EndRespawnTimer()
{
	ABamGameMode* BamGameMode = GetWorld()->GetAuthGameMode<ABamGameMode>();
	if (BamGameMode)
	{
		BamGameMode->RequestPlayerRespawn(this, Controller);
	}
}


void AMultiplayerCharacter::HideCamera()
{
	if (!IsLocallyControlled()) return;

	if ((Camera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

float AMultiplayerCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void AMultiplayerCharacter::OnRep_Health()
{
	// Since Health is set up to replicate, we can use it instead of an RPC to play the hitReacts
	// On Clients
	UpdateHUDHealth();
	PlayHitReactMontage(); 
}

void AMultiplayerCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void AMultiplayerCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &AMultiplayerCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}









