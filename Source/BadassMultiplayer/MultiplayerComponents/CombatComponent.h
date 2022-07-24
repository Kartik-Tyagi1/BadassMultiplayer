// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class AWeapon;
class AMultiplayerCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BADASSMULTIPLAYER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void EquipWeapon(AWeapon* WeaponToEquip);

	// Allows AMultiplayerCharacter to have full access to everything in the UCombatComponent (USE THIS VERY CAREFULLY)
	friend class AMultiplayerCharacter;

protected:
	virtual void BeginPlay() override;


private:
	AMultiplayerCharacter* Character;
	AWeapon* EquippedWeapon;


public:	
	

		
};
