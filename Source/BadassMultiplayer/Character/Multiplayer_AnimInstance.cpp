// Fill out your copyright notice in the Description page of Project Settings.


#include "Multiplayer_AnimInstance.h"
#include "MultiplayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "BadassMultiplayer/Weapon/Weapon.h"
#include "BadassMultiplayer/Types/CombatState.h"

void UMultiplayer_AnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	MC = Cast<AMultiplayerCharacter>(TryGetPawnOwner());
}

void UMultiplayer_AnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (MC == nullptr)
	{
		MC = Cast<AMultiplayerCharacter>(TryGetPawnOwner());
	}

	if (MC == nullptr) return;

	FVector Velocity = MC->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = MC->GetCharacterMovement()->IsFalling();

	bIsMoving = MC->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;

	bIsWeaponEquipped = MC->IsWeaponEquipped();

	EquippedWeapon = MC->GetEquippedWeapon();

	bIsCrouched = MC->bIsCrouched;

	bIsAiming = MC->GetIsAiming();

	TurningState = MC->GetTurningState();

	bRotateRootBone = MC->ShouldRotateRootBone();

	bIsEliminated = MC->GetIsEliminated();

	// This is the global rotation of the camera as we move the mouse (so if we face the character's back, aim rotation yaw is 0, )
	// left is negative values (0 -> -180), right is postive values (0 -> +180)
	FRotator AimRotation = MC->GetBaseAimRotation();

	// This is the rotation of the character based on its movement in the world direction 
	// i.e. moving in the world +x direction gives a yaw value of 0
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(MC->GetVelocity());

	// Getting the delta will tell us what direction to move in for the blendspace
	const FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;


	// Lean if found between the delta between the Current Rotation and the Rotation Last Frame 
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = MC->GetActorRotation(); // Rotation of the root/capsule component
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = MC->GetAO_Yaw();
	AO_Pitch = MC->GetAO_Pitch();

	if (bIsWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && MC->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		MC->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));


		if (MC->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = MC->GetMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
			RightHandRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - MC->GetHitTarget()));
		}
	}

	bUseFabrik = MC->GetCombatState() != ECombatState::ECS_Reloading;
	bUseAimOffsets = MC->GetCombatState() != ECombatState::ECS_Reloading;
	bTransformRightHand = MC->GetCombatState() != ECombatState::ECS_Reloading;
}
