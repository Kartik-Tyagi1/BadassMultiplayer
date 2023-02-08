#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

class USphereComponent;
class USoundCue;
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class BADASSMULTIPLAYER_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	APickup();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	void RotatePickup(float DeltaTime);

protected:
	// Kinda like the mesh but uses Niagara Component for cool effects
	UPROPERTY(VisibleAnywhere)
	UNiagaraComponent* PickupEffectComponent;

	// Effect played after the Niagara Component is picked up
	UPROPERTY(EditAnywhere)
	UNiagaraSystem* PickupEffectSystem;

	UPROPERTY(EditAnywhere)
	float RaiseSystemAmount = 100.f;

	UPROPERTY(EditAnywhere)
	float BaseTurnRate = 45.f;

private:
	UPROPERTY(EditAnywhere)
	USphereComponent* OverlapSphere;

	UPROPERTY(EditAnywhere)
	USoundCue* PickupSound;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PickupMesh;

	// Delay The Overlap Bind for a bit so event does not occur is character is standing on pickup spawn point
	FTimerHandle BindOverlapTimer;
	float BindOverlapTime = 0.25f;
	void EndBindOverlapTimer();

public:	
	

};
