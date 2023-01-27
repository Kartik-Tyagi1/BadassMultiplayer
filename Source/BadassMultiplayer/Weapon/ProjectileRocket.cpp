#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "RocketMovementComponent.h"

AProjectileRocket::AProjectileRocket()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RocketMovementComp = CreateDefaultSubobject<URocketMovementComponent>(TEXT("ProjectileMovementComponent"));
	RocketMovementComp->bRotationFollowsVelocity = true;
	RocketMovementComp->SetIsReplicated(true);
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();


	if (!HasAuthority())
	{
		// Server is bound in Projectile.h now we can bind client hit events here too
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);
	}

	SpawnTrailSystem();

	if (RocketLoopSound && RocketLoopSoundAttenuation)
	{
		RocketLoopComponent = UGameplayStatics::SpawnSoundAttached(
			RocketLoopSound,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false,
			1.f,
			1.f,
			0.f,
			RocketLoopSoundAttenuation,
			(USoundConcurrency*)nullptr,
			false
		);
	}

}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == GetOwner())
	{
		return; // Don't continue if the rocket was fired and the character ran into it
	}

	ApplyExplosionDamage();

	GetWorldTimerManager().SetTimer(DestroyTimer, this, &AProjectileRocket::DestroyTimerFinished, DestroyTime);

	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}

	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	if (ProjectileMesh)
	{
		ProjectileMesh->SetVisibility(false);
	}
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (TrailSystemComponent && TrailSystemComponent->GetSystemInstanceController())
	{
		TrailSystemComponent->GetSystemInstanceController()->Deactivate();
	}
	if (RocketLoopComponent && RocketLoopComponent->IsPlaying())
	{
		RocketLoopComponent->Stop();
	}

	
	// Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}



void AProjectileRocket::Destroyed()
{
	// The point of overriding all these functions is to prevent the smoke trail from disappearing right when the rocket is destroyed
	// If we delay the destruction of the rocket then the niagara system can dissapate naturally
}
