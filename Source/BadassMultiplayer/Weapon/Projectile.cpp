#include "Projectile.h"
#include "Components/BoxComponent.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic); // This will be flying through the air so it should be dynamic
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block); // Block Visible Objects (aka not the camera)
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block); // Static meshes like walls and stuff
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

