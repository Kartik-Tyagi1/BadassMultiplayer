#include "BuffComponent.h"
#include "BadassMultiplayer/Character/MultiplayerCharacter.h"
#include "BadassMultiplayer/MultiplayerComponents/CombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	HealRampUp(DeltaTime);
}

void UBuffComponent::HealCharacter(float HealAmount, float HealingTime)
{
	bIsHealing = true;
	HealingRate = HealAmount / HealingTime;
	AmountToHeal += HealAmount;
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (!bIsHealing || Character == nullptr || Character->GetIsEliminated()) return;

	const float AmountToHealThisFrame = HealingRate * DeltaTime;
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + AmountToHealThisFrame, 0.f, Character->GetMaxHealth()));
	Character->UpdateHUDHealth();

	AmountToHeal -= AmountToHealThisFrame;

	if (AmountToHeal <= 0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bIsHealing = false;
		AmountToHeal = 0.f;
	}
}

void UBuffComponent::BuffSpeed(float BuffedWalkSpeed, float BuffedCrouchSpeed, float BuffTime)
{
	if (Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(
		SpeedBuffTimer, 
		this, 
		&UBuffComponent::EndSpeedBuffTimer, 
		BuffTime
	);

	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BuffedWalkSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BuffedCrouchSpeed;
	}


	MulticastSpeedBuff(BuffedWalkSpeed, BuffedCrouchSpeed);
}

void UBuffComponent::EndSpeedBuffTimer()
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;

	Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BaseCrouchSpeed;
	MulticastSpeedBuff(BaseWalkSpeed, BaseCrouchSpeed);
}

void UBuffComponent::SetBaseSpeeds(float WalkSpeedAmount, float CrouchSpeedAmount)
{
	BaseWalkSpeed = WalkSpeedAmount; 
	BaseCrouchSpeed = CrouchSpeedAmount;
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float WalkSpeed, float CrouchSpeed)
{
	Character->GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;

	if (Character->GetCombatComponent())
	{
		// Increase base walk speed on combat component when buffed so aiming doesn't turn it off
		Character->GetCombatComponent()->SetSpeeds(WalkSpeed);
	}
}

