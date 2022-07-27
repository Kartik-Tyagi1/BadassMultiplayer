// Fill out your copyright notice in the Description page of Project Settings.


#include "Multiplayer_AnimInstance.h"
#include "MultiplayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

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

	bIsCrouched = MC->bIsCrouched;

}
