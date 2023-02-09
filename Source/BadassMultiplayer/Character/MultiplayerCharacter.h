// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BadassMultiplayer/Types/TurningInPlace.h"
#include "BadassMultiplayer/Interfaces/CrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "BadassMultiplayer/Types/CombatState.h"
#include "MultiplayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UWidgetComponent;
class AWeapon;
class UCombatComponent;
class UAnimMontage;
class AMPPlayerController;
class USoundCue;
class ABamPlayerState;
class UBuffComponent;

UCLASS()
class BADASSMULTIPLAYER_API AMultiplayerCharacter : public ACharacter, public ICrosshairsInterface
{
	GENERATED_BODY()

public:
	AMultiplayerCharacter();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/* Registers which variables are to be replicated across the game instances */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/* Earliest place that allow us to access components attached to this character and set values */
	virtual void PostInitializeComponents() override;

	/* Runs when player movement gets replicated */
	virtual void OnRep_ReplicatedMovement() override;

protected:
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	virtual void Jump() override;

	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void FireButtonPressed();
	void FireButtonReleased();
	void ReloadButtonPressed();
	void GrenadeButtonPressed();

	void CalculateAO(float DeltaTime);
	void TurnInPlace(float DeltaTime);
	void CalculateAOPitch();
	void SimProxiesTurnInPlace();

	void PlayHitReactMontage();
	void PlayElimMontage();

	UFUNCTION()
	void RecieveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	virtual void Destroyed() override;

	// Poll for creation of classes and initalize them
	void PollInit();

	void RotateInPlace(float DeltaTime);

	void DropOrDestroyWeapon(AWeapon* Weapon);
	void DropOrDestroyWeapons();
		
private:

	/*********************** COMPONENTS *************************/
	UPROPERTY(VisibleAnywhere, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArm;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* OverheadWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCombatComponent* Combat;

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	UBuffComponent* Buff;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* AttachedGrenade;

	/* PlayerController */
	UPROPERTY()
	AMPPlayerController* MPPlayerController;

	/* Player State */
	UPROPERTY()
	ABamPlayerState* BamPlayerState;


	/*********************** Overlapping Weapons. Used with Equip Weapon Functions *************************/

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;

	/*********************** Aim offset yaw | pitch calculations | Turn in Place *************************/
	float AO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	// Used for turn in place anims
	float Interp_AO_Yaw;
	ETurningState TurningState;

	/*********************** ANIMATION MONTAGES *************************/

	// Section to determine which fire weapon animation to use depending on aim state
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* FireWeaponMontage;

	// Section to determine which react animation to use depending on which side the hit came from
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ElimMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ThrowGrenadeMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	float CameraThreshold = 200.f;

	/*********************** Camera *************************/
	void HideCamera();

	/*********************** Simulated Proxies Rotation and Turn in Place *************************/

	// Determine if the root bone should rotate. Should only happen for locally controlled players
	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator SimProxyRotationLastFrame;
	FRotator SimProxyRotationCurrent;
	float SimYawDelta;
	float TimeSinceLastMovementRep;
	float CalculateSpeed();

	/*********************** PLAYER HEALTH *************************/
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;
	
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;
	
	UFUNCTION()
	void OnRep_Health(float PreviousHealth);

	/*********************** PLAYER SHIELD *************************/
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxShield = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere, Category = "Player Stats")
	float Shield = 0.f;

	UFUNCTION()
	void OnRep_Shield(float PreviousShield);

	/*********************** ELIMINATION AND RESPAWN *************************/
	bool bIsEliminated = false;

	FTimerHandle RespawnTimer;

	UPROPERTY(EditDefaultsOnly)
	float RespawnDelay = 3.f;

	void EndRespawnTimer();

	/*********************** DISSOLVE EFFECT *************************/
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;

	FOnTimelineFloat DissolveTrack;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);

	void StartDissolve();

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	// Created At Runtime
	UPROPERTY(VisibleAnywhere, Category = Eliminated)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	// Assigned in Blueprint and used with Dynamic Instance
	UPROPERTY(EditAnywhere, Category = Eliminated)
	UMaterialInstance* DissolveMaterialInstance;

	/*********************** ELIMINATION BOT *************************/
	UPROPERTY(EditAnywhere, Category = Eliminated)
	UParticleSystem* ElimBotEffect;

	UPROPERTY(VisibleAnywhere, Category = Eliminated)
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere, Category = Eliminated)
	USoundCue* ElimSound;


// Getters
public:
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE ETurningState GetTurningState() const { return TurningState; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	FORCEINLINE UCombatComponent* GetCombatComponent() const { return Combat; }
	FORCEINLINE UBuffComponent* GetBuffComponent() const { return Buff; }
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
	FORCEINLINE UStaticMeshComponent* GetGrenadeMesh() const { return AttachedGrenade; }
	FVector GetHitTarget() const;
	UCameraComponent* GetCamera() { return Camera; }
	AWeapon* GetEquippedWeapon();
	bool GetIsAiming();
	bool GetIsEliminated() const { return bIsEliminated; }
	ECombatState GetCombatState() const;
	bool IsWeaponEquipped();

// Setters
public:
	FORCEINLINE void SetHealth(float Amount) { Health = Amount; }
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; }
	void SetOverlappingWeapon(AWeapon* Weapon);

public:
	void PlayFireMontage(bool bIsAiming);
	void PlayReloadMontage();
	void PlayThrowGrenadeMontage();
	void UpdateHUDHealth();
	void UpdateHUDShield();
	void UpdateHUDAmmo();

	// Server Eliminination Stuff
	void Eliminated(APlayerController* AttackerController);

	// Server and Client Elimination Stuff
	UFUNCTION(NetMulticast, Reliable)
	void MulticastEliminated(const FString& AttackerName);

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

	void SpawnDefaultWeapon();
};
