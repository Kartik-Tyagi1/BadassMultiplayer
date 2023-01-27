#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "ProjectileWeapon.generated.h"

class AProjectile;

/**
 * 
 */
UCLASS()
class BADASSMULTIPLAYER_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	virtual void FireWeapon(const FVector& HitTarget) override;

private:
	UPROPERTY(EditAnywhere, Category = "Projectile Weapon Properties")
	TSubclassOf<AProjectile> ProjectileClass;
	
};
