// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

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
	void ShowPickupWidget(bool bShowWidget);

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
	class USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "WeaponProperties")
	class USphereComponent* AreaSphere;

	UPROPERTY(VisibleAnywhere, Category = "WeaponProperties")
	class UWidgetComponent* PickupWidget;

	UPROPERTY(VisibleAnywhere, Category = "WeaponProperties")
	EWeaponState WeaponState;


public:	

	

};
