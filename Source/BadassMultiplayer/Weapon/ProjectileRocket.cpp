#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "RocketMovementComponent.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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

	if (RocketTrailSystem)
	{
		RocketTrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			RocketTrailSystem, 
			GetRootComponent(), 
			FName(), 
			GetActorLocation(), 
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition, 
			false);
	}

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

	// Get the Pawn who fired the rocket (owner is set in ProjectileWeapon.h Fire() function)
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn && HasAuthority())
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,
				DamageAmount,
				MinimumDamageAmount,
				GetActorLocation(),
				InnerRadius,
				OuterRadius,
				1.f, // Linear Damage Falloff
				UDamageType::StaticClass(),
				TArray<AActor*>(),
				this,
				FiringController
				);
		}
	}

	GetWorldTimerManager().SetTimer(DestroyTimer, this, &AProjectileRocket::DestroyTimerFinished, DestroyTime);

	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}

	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	if (RocketMesh)
	{
		RocketMesh->SetVisibility(false);
	}
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (RocketTrailSystemComponent && RocketTrailSystemComponent->GetSystemInstanceController())
	{
		RocketTrailSystemComponent->GetSystemInstanceController()->Deactivate();
	}
	if (RocketLoopComponent && RocketLoopComponent->IsPlaying())
	{
		RocketLoopComponent->Stop();
	}

	
	// Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AProjectileRocket::DestroyTimerFinished()
{
	Destroy();
}

void AProjectileRocket::Destroyed()
{
	// The point of overriding all these functions is to prevent the smoke trail from disappearing right when the rocket is destroyed
	// If we delay the destruction of the rocket then the niagara system can dissapate naturally
}
