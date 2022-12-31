#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;

UCLASS()
class BADASSMULTIPLAYER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectileRocket();

	virtual void BeginPlay() override;

	virtual void Destroyed() override;

protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	void DestroyTimerFinished();

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* RocketTrailSystem;

	UPROPERTY()
	UNiagaraComponent* RocketTrailSystemComponent;

	UPROPERTY(EditAnywhere)
	USoundCue* RocketLoopSound;

	UPROPERTY()
	UAudioComponent* RocketLoopComponent;

	UPROPERTY(EditAnywhere)
	USoundAttenuation* RocketLoopSoundAttenuation;

private:
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* RocketMesh;

	UPROPERTY(EditAnywhere, Category = Damage)
	float MinimumDamageAmount = 10.f;

	UPROPERTY(EditAnywhere, Category = Damage)
	float OuterRadius = 500.f;

	UPROPERTY(EditAnywhere, Category = Damage)
	float InnerRadius = 200.f;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere, Category = Niagara)
	float DestroyTime = 3.f;



};
