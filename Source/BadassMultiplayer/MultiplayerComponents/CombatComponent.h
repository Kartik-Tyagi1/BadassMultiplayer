// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000.f

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

	/* Registers which variables are to be replicated across the game instances */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	void SetIsAiming(bool bAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetIsAiming(bool bAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	// Called from MultiplayerCharacter.cpp 
	void FireButtonPressed(bool bFireIsPressed);

	// This excutes the weapon fire on the server
	UFUNCTION(Server, Reliable)
	void ServerFire();

	// This will be called in server fire (so it gets called on the server, and when a net multicast RPC gets called on the server
	// it gets executed on the server and on all client machines
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticastFire();

	void TraceUnderCrosshairs(FHitResult& HitResult);


private:
	AMultiplayerCharacter* Character;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bIsAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	FVector HitTarget;


public:	
		
};
