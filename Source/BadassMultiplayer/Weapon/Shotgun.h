#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class BADASSMULTIPLAYER_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()

public:
	virtual void FireWeapon(const FVector& HitTarget) override;

private:
	UPROPERTY(EditAnywhere)
	uint32 NumOfPellets = 10;
	
};
