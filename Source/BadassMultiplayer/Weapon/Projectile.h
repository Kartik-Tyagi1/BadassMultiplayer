#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UBoxComponent;
class UProjectileMovementComponent;
class UParticleSystem;
class UParticleSystemComponent;
class USoundCue;
class UNiagaraSystem;
class UNiagaraComponent;

UCLASS()
class BADASSMULTIPLAYER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void SpawnTrailSystem();

	void StartDestroyTimer();

	void DestroyTimerFinished();

	void ApplyExplosionDamage();

protected:
	UPROPERTY(EditAnywhere)
	UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(EditAnywhere)
	USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* TrailSystem;

	UPROPERTY()
	UNiagaraComponent* TrailSystemComponent;

	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovementComp;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere, Category = Niagara)
	float DestroyTime = 3.f;

	UPROPERTY(EditAnywhere)
	float DamageAmount = 20.f;

	UPROPERTY(EditAnywhere, Category = Damage)
	float MinimumDamageAmount = 10.f;

	UPROPERTY(EditAnywhere, Category = Damage)
	float OuterRadius = 500.f;

	UPROPERTY(EditAnywhere, Category = Damage)
	float InnerRadius = 200.f;


private:
	UPROPERTY(EditAnywhere)
	UParticleSystem* ProjectileTracer;

	UPROPERTY()
	UParticleSystemComponent* ProjectileTracerComp;

public:	
	

};
