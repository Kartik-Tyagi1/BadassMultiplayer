#include "PickupSpawnPoint.h"
#include "Pickup.h"


APickupSpawnPoint::APickupSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	StartSpawnPickupTimer((AActor*)nullptr);
}

void APickupSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APickupSpawnPoint::SpawnPickup()
{
	int32 NumPickupClasses = PickupClasses.Num();
	if (NumPickupClasses > 0)
	{
		int PickupClassIndex = FMath::RandRange(0, NumPickupClasses - 1);
		SpawnedPickup = GetWorld()->SpawnActor<APickup>(PickupClasses[PickupClassIndex], GetActorTransform());

		if (HasAuthority() && SpawnedPickup)
		{
			SpawnedPickup->OnDestroyed.AddDynamic(this, &APickupSpawnPoint::StartSpawnPickupTimer);
		}
	}
}

void APickupSpawnPoint::StartSpawnPickupTimer(AActor* DestroyedActor)
{
	const float SpawnPickupTime = FMath::RandRange(SpawnPickupMinTime, SpawnPickupMaxTime);

	GetWorldTimerManager().SetTimer(
		SpawnPickupTimer, 
		this, 
		&APickupSpawnPoint::EndSpawnPickupTimer, 
		SpawnPickupTime
	);
}

void APickupSpawnPoint::EndSpawnPickupTimer()
{
	if (HasAuthority())
	{
		SpawnPickup();
	}
}


