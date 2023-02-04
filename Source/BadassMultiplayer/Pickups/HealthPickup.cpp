#include "HealthPickup.h"
#include "BadassMultiplayer/Character/MultiplayerCharacter.h"
#include "BadassMultiplayer/MultiplayerComponents/BuffComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"


AHealthPickup::AHealthPickup()
{
	bReplicates = true;

	PickupEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupEffect"));
	PickupEffectComponent->SetupAttachment(RootComponent);
}

void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AMultiplayerCharacter* Character = Cast<AMultiplayerCharacter>(OtherActor);
	if (Character && Character->GetBuffComponent())
	{
		Character->GetBuffComponent()->HealCharacter(HealAmount, HealingTime);
	}

	Destroy();
}

void AHealthPickup::Destroyed()
{
	if (PickupEffectSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this, 
			PickupEffectSystem, 
			FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + RaiseSystemAmount),
			GetActorRotation()
		);
	}

	Super::Destroyed();
}
