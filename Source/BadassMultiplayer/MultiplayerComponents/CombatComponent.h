// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BadassMultiplayer/HUD/BadassHUD.h"
#include "BadassMultiplayer/Weapon/WeaponTypes.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000.f

class AWeapon;
class AMultiplayerCharacter;
class AMPPlayerController;
class ABadassHUD;

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
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	// This will be called in server fire (so it gets called on the server, and when a net multicast RPC gets called on the server
	//		it gets executed on the server and on all client machines
	// FVector_NetQuantize is a child class of FVector that is more efficent when sending data across the network
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs(FHitResult& HitResult);

	void SetHUDCrosshairs(float DeltaTime);

	void InterpFOV(float DeltaTime); 

	void Fire();

private:
	UPROPERTY()
	AMultiplayerCharacter* Character;

	UPROPERTY()
	AMPPlayerController* Controller;

	UPROPERTY()
	ABadassHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bIsAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	/* HUD and Crosshair Caluclation Variables */
	float CrosshairVelocityFactor;
	float CrosshairJumpFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;
	FHUDPackage Package;

	FVector HitTarget;

	/* Aiming and FOV */
	float DefaultFOV;
	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = Aiming)
	float ZoomUninterpSpeed = 20.f;

	/* Automatic Fire */
	FTimerHandle FireTimer;
	bool bCanFire = true;
	void StartFireTimer();
	void EndFireTimer();

	bool CanFire();

	/********************************** CARRIED AMMO ***************************************/
	// Carried Ammo for the currently equipped Weapon
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	// Ammo for all the weapons equipped
	// This cannot be replicated becuase TMap types cannot be replicated so that is why we have a carried ammo variable
	TMap<EWeaponType, int32> CarriedAmmoMap;
	void InitializeCarriedAmmoMap();

	UPROPERTY(EditAnywhere, Category = Ammo)
	int32 StartingAssaultRifleAmmo = 30;

public:	
		
};
