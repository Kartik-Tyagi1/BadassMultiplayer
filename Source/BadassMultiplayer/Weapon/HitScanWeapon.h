#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

class UParticleSystem;

UCLASS()
class BADASSMULTIPLAYER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	virtual void FireWeapon(const FVector& HitTarget) override;

private:
	UPROPERTY(EditAnywhere)
	float DamageAmount = 20.f;

	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	UParticleSystem* SmokeBeamParticles;
};
