// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MultiplayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UWidgetComponent;
class AWeapon;
class UCombatComponent;

UCLASS()
class BADASSMULTIPLAYER_API AMultiplayerCharacter : public ACharacter
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

protected:
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);

	void EquipButtonPressed();

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

public:
	void SetOverlappingWeapon(AWeapon* Weapon);

};
