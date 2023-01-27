#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

class UParticleSystem;
class USoundCue;

UCLASS()
class BADASSMULTIPLAYER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	virtual void FireWeapon(const FVector& HitTarget) override;

protected:
	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHitResult);

protected:
	UPROPERTY(EditAnywhere, Category = "HitScan Weapon Properties")
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere, Category = "HitScan Weapon Properties")
	USoundCue* HitSound;

	UPROPERTY(EditAnywhere, Category = "HitScan Weapon Properties")
	float DamageAmount = 20.f;

private:
	UPROPERTY(EditAnywhere, Category = "HitScan Weapon Properties")
	UParticleSystem* SmokeBeamParticles;

	UPROPERTY(EditAnywhere, Category = "HitScan Weapon Properties")
	UParticleSystem* MuzzleFlashParticles;

	UPROPERTY(EditAnywhere, Category = "HitScan Weapon Properties")
	USoundCue* FireSound;

	/* 
	* Trace End With Scatter 
	* Line Trace will pick random points in a sphere to trace to which simulated weapon scatter
	*/

	UPROPERTY(EditAnywhere, Category = "HitScan Weapon Properties")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "HitScan Weapon Properties")
	float SphereRadius = 75.f;

	UPROPERTY(EditAnywhere, Category = "HitScan Weapon Properties")
	bool bUseScatter = false;
};
