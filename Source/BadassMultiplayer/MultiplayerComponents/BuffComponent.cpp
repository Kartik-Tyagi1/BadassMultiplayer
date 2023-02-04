#include "BuffComponent.h"
#include "BadassMultiplayer/Character/MultiplayerCharacter.h"

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

