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

protected:
	virtual void BeginPlay() override;

	// Allows us to show the increase in the health bar instead of a sudden jump in health
	void HealRampUp(float DeltaTime);

private:
	UPROPERTY()
	AMultiplayerCharacter* Character;

	// Tracks if we are already healing, to not keep healing and increase the health bar
	bool bIsHealing = false;

	// Amount of health to increase per second (HealingAmount / HealingTime) -> Variables are passed from the health pickup
	float HealingRate = 0.f;

	// Tracks how much to heal, should not heal more after reaching this amount
	float AmountToHeal = 0.f;

public:	
	

		
};
