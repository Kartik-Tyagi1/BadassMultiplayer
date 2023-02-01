// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BadassMultiplayer/HUD/BadassHUD.h"
#include "BadassMultiplayer/Weapon/WeaponTypes.h"
#include "BadassMultiplayer/Types/CombatState.h"
#include "CombatComponent.generated.h"


class AWeapon;
class AMultiplayerCharacter;
class AMPPlayerController;
class ABadassHUD;
class AProjectile;

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

	// Called from MultiplayerCharacter.cpp 
	void FireButtonPressed(bool bFireIsPressed);

protected:
	virtual void BeginPlay() override;
	void SetIsAiming(bool bAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetIsAiming(bool bAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

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

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();

	int32 CalculateReloadAmount();

	void ThrowGrenade();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	void DropEquippedWeapon();
	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToLeftHand(AActor* ActorToAttach);
	void UpdateCarriedAmmo();
	void PlayWeaponEquipSound();
	void ReloadWeaponIfEmpty();

	void ShowAttachedGrenade(bool bShowGrenade);

protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> GrenadeClass;

private:
	/********************************** COMPONENTS ***************************************/
	UPROPERTY()
	AMultiplayerCharacter* Character;

	UPROPERTY()
	AMPPlayerController* Controller;

	UPROPERTY()
	ABadassHUD* HUD;

	/********************************** Weapon and Firing ***************************************/

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bIsAiming;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	/* Automatic Fire */
	FTimerHandle FireTimer;
	bool bCanFire = true;
	void StartFireTimer();
	void EndFireTimer();

	bool CanFire();

	FVector HitTarget;

	/********************************** Crosshair Caluclation Variables ***************************************/

	float CrosshairVelocityFactor;
	float CrosshairJumpFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;
	FHUDPackage Package;

	/********************************** FOV ***************************************/

	float DefaultFOV;
	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = Aiming)
	float ZoomUninterpSpeed = 20.f;

	/********************************** Combat State ***************************************/

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	/********************************** CARRIED AMMO AND RELOADING ***************************************/
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

	UPROPERTY(EditAnywhere, Category = Ammo)
	int32 StartingRocketLauncherAmmo = 0;

	UPROPERTY(EditAnywhere, Category = Ammo)
	int32 StartingPistolAmmo = 10;

	UPROPERTY(EditAnywhere, Category = Ammo)
	int32 StartingSMGAmmo = 40;

	UPROPERTY(EditAnywhere, Category = Ammo)
	int32 StartingShotgunAmmo = 5;

	UPROPERTY(EditAnywhere, Category = Ammo)
	int32 StartingSniperRifleAmmo = 10;

	UPROPERTY(EditAnywhere, Category = Ammo)
	int32 StartingGrenadeLauncherAmmo = 5;

	void UpdateAmmoValues();

	void UpdateShotgunAmmoValues();



public:	
	void Reload();

	UFUNCTION(BlueprintCallable)
	void FinishReload();

	UFUNCTION(BlueprintCallable)
	void ShotgunShellInsert();

	void JumpToShotgunReloadEnd();

	UFUNCTION(BlueprintCallable)
	void FinishThrowingGrenade();

	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();

	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);
	
};
