// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"

class USkeletalMeshComponent;
class USphereComponent;
class UWidgetComponent;
class UAnimationAsset;
class ABulletShell;
class UTexture2D;
class AMPPlayerController;
class AMultiplayerCharacter;
class USoundCue;

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial		UMETA(DisplayName = "Inital State"),
	EWS_Equipped	UMETA(DisplayName = "Equipped State"),
	EWS_Dropped		UMETA(DisplayName = "Dropped State"),

	EWS_MAX			UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class BADASSMULTIPLAYER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;

	/* Registers which variables are to be replicated across the game instances */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnRep_Owner() override;

	void ShowPickupWidget(bool bShowWidget);

	virtual void FireWeapon(const FVector& HitTarget);

	void DropWeapon();

	void SetHUDAmmo();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnAreaSphereOverlap( 
		UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, 
		bool bFromSweep, 
		const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnAreaSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

private:
	UPROPERTY(VisibleAnywhere, Category = "WeaponProperties")
		USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "WeaponProperties")
		USphereComponent* AreaSphere;

	UPROPERTY(VisibleAnywhere, Category = "WeaponProperties")
		UWidgetComponent* PickupWidget;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "WeaponProperties")
		EWeaponState WeaponState;

	UFUNCTION()
		void OnRep_WeaponState();

	UPROPERTY(EditAnywhere, Category = "WeaponProperties")
		UAnimationAsset* FireWeaponAnim;

	UPROPERTY(EditAnywhere)
		TSubclassOf<ABulletShell> BulletShellClass;

	/* Aiming Zoom Parameters */
	UPROPERTY(EditAnywhere, Category = Aiming)
		float ZoomFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = Aiming)
		float ZoomInterpSpeed = 20.f;

	// Amount of ammo in the mag
	UPROPERTY(ReplicatedUsing = OnRep_Ammo, EditAnywhere, Category = Ammo)
		int32 Ammo;

	UFUNCTION()
		void OnRep_Ammo();

	// Decrements ammo when firing weapon
	void SpendRound();

	UPROPERTY(EditAnywhere, Category = Ammo)
		int32 MagCapacity;

	UPROPERTY()
		AMPPlayerController* OwnerController;

	UPROPERTY()
		AMultiplayerCharacter* OwnerCharacter;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		EWeaponType WeaponType;

public:	
	void SetWeaponState(EWeaponState State);
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetZoomFOV() const { return ZoomFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE bool IsWeaponEmpty() const { return Ammo <= 0; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	void AddAmmo(int32 AmmoToAdd);

	/* Textures for the weapon crosshairs. These are individual so we can make them dynamic */
	UPROPERTY(EditAnywhere, Category = Crosshairs)
		UTexture2D* CrosshairCenter;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
		UTexture2D* CrosshairTop;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
		UTexture2D* CrosshairBottom;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
		UTexture2D* CrosshairLeft;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
		UTexture2D* CrosshairRight;

	// Delay for automatic fire
	UPROPERTY(EditAnywhere, Category = Combat)
		float FireDelay = 0.15f;

	// True is weapon is an automatic weapon
	UPROPERTY(EditAnywhere, Category = Combat)
		bool bIsAutomatic = true;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		USoundCue* WeaponEquipSound;



};
