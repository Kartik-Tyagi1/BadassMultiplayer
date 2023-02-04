#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "HealthPickup.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class BADASSMULTIPLAYER_API AHealthPickup : public APickup
{
	GENERATED_BODY()

public:
	AHealthPickup();
	virtual void Destroyed() override;

protected:
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	) override;

private:
	UPROPERTY(EditAnywhere)
	float HealAmount = 100.f;

	UPROPERTY(EditAnywhere)
	float RaiseSystemAmount = 100.f;

	UPROPERTY(EditAnywhere)
	float HealingTime = 5.f;

	// Kinda like the mesh for this class
	UPROPERTY(VisibleAnywhere)
	UNiagaraComponent* PickupEffectComponent;

	// Effect played after the health buff is picked up
	UPROPERTY(EditAnywhere)
	UNiagaraSystem* PickupEffectSystem;
	
};
