// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BadassMultiplayer/Types/TurningInPlace.h"
#include "BadassMultiplayer/Interfaces/CrosshairsInterface.h"
#include "MultiplayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UWidgetComponent;
class AWeapon;
class UCombatComponent;
class UAnimMontage;
class AMPPlayerController;

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

	void CalculateAO(float DeltaTime);
	void TurnInPlace(float DeltaTime);
	void CalculateAOPitch();
	void SimProxiesTurnInPlace();

	void PlayHitReactMontage();
	void PlayElimMontage();

	UFUNCTION()
	void RecieveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
	void UpdateHUDHealth();


private:
	UPROPERTY(VisibleAnywhere, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArm;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* OverheadWidget;

	/* 
	* The UPROPERTY Designates Overlapping Weapon to be an actor that gets relpicated across all instances (clients) of the game.
	* 
	* NOTE: Replication only happens one way (Server ----> Client)
	* 
	* When the value of OverlappingWeapon changes on the server
	*	- only then it will replicate ( aka set on all clients of the MultiplayerCharacter)
	*	- only then will OnRep_OverlappingWeapon be called
	*/
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;

	/* 
	* Called automatically when the value of the OverlappingWeapon Changes
	* ----- WILL NOT BE CALLED ON THE SERVER. SO IT WON'T BE REPLICATED THERE -------
	* Notifies can have an input parameter of the type that is it notifying about and that parameter stores the last value of that variable it before is changed
	*/
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UPROPERTY(VisibleAnywhere)
	UCombatComponent* Combat;

	/*
	*	EquipButtonPressed only allows the server controller to equip a weapon but if clients want to then they need to send a request to the server
	*	We will use a Remote Procedure Call. This is a function that sends infomation from the client to the server to let it request information
	*	To do this we make a function with the UFUNCTION(Server, Reliable) Macro
	*	Server -> This designates this function to be called only from clients to the server
	*	Reliable -> Client is guaranteed to recieve confirmation after request is sent to the server (can also be unreliable meaning request can be dropped) 
	*/
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	// Used for Aim offset yaw and pitch calculations
	float AO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	// Used for turn in place anims
	float Interp_AO_Yaw;
	ETurningState TurningState;

	// Section to determine which fire weapon animation to use depending on aim state
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* FireWeaponMontage;

	// Section to determine which react animation to use depending on which side the hit came from
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ElimMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	float CameraThreshold = 200.f;

	void HideCamera();

	/* Variables for Simulated Proxies Rotation and Turn in Place */
	// Determine if the root bone should rotate. Should only happen for locally controlled players
	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator SimProxyRotationLastFrame;
	FRotator SimProxyRotationCurrent;
	float SimYawDelta;
	float TimeSinceLastMovementRep;
	float CalculateSpeed();

	/* Player Health */
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;
	UFUNCTION()
	void OnRep_Health();

	/* PlayerController */
	AMPPlayerController* MPPlayerController;

	/*********************** ELIMINATION AND RESPAWN *************************/
	bool bIsEliminated = false;
	FTimerHandle RespawnTimer;
	UPROPERTY(EditDefaultsOnly)
	float RespawnDelay = 3.f;
	void EndRespawnTimer();

// INLINES
public:
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE ETurningState GetTurningState() const { return TurningState; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }

public:
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool GetIsAiming();
	AWeapon* GetEquippedWeapon();
	void PlayFireMontage(bool bIsAiming);
	FVector GetHitTarget() const;
	UCameraComponent* GetCamera() { return Camera; }

	// Server Eliminination Stuff
	void Eliminated();

	// Server and Client Elimination Stuff
	UFUNCTION(NetMulticast, Reliable)
	void MulticastEliminated();

	bool GetIsEliminated() const { return bIsEliminated; }
};
