#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"

class AMultiplayerCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BADASSMULTIPLAYER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();

	// Allows AMultiplayerCharacter to have full access to everything in the UCombatComponent (USE THIS VERY CAREFULLY)
	friend class AMultiplayerCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void HealCharacter(float HealAmount, float HealingTime);

	void ReplenishShield(float ShieldAmount, float ShieldReplenishTime);

	void BuffSpeed(float BuffedWalkSpeed, float BuffedCrouchSpeed, float BuffTime);

	void BuffJump(float JumpBuffVelocity, float BuffTime);


protected:
	virtual void BeginPlay() override;

	// Allows us to show the increase in the health bar instead of a sudden jump in health
	void HealRampUp(float DeltaTime);

	// Allows us to show the increase in the shield bar instead of a sudden jump in shield
	void ShieldRampUp(float DeltaTime);

private:
	UPROPERTY()
	AMultiplayerCharacter* Character;

	/*----------------------------------------------------- Health Buff -----------------------------------------------------------*/

	// Tracks if we are already healing, to not keep healing and increase the health bar
	bool bIsHealing = false;

	// Amount of health to increase per second (HealingAmount / HealingTime) -> Variables are passed from the health pickup
	float HealingRate = 0.f;

	// Tracks how much to heal, should not heal more after reaching this amount
	float AmountToHeal = 0.f;

	/*----------------------------------------------------- Health Buff -----------------------------------------------------------*/

	// Tracks if we are already healing, to not keep healing and increase the health bar
	bool bIsShieldReplenishing = false;

	// Amount of health to increase per second (HealingAmount / HealingTime) -> Variables are passed from the health pickup
	float ShieldReplenishingRate = 0.f;

	// Tracks how much to heal, should not heal more after reaching this amount
	float ShieldReplenishAmount = 0.f;

	/*----------------------------------------------------- Speed Buff ------------------------------------------------------------*/

	FTimerHandle SpeedBuffTimer;

	void EndSpeedBuffTimer();

	float BaseWalkSpeed;
	float BaseCrouchSpeed;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float WalkSpeed, float CrouchSpeed);

	/*----------------------------------------------------- Speed Buff ------------------------------------------------------------*/

	FTimerHandle JumpBuffTimer;

	void EndJumpBuffTimer();

	float BaseJumpVelocity;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float JumpVelocity);

public:	
	void SetBaseSpeeds(float WalkSpeedAmount, float CrouchSpeedAmount);
	void SetBaseJumpVelocity(float JumpVelocity);
	
};
