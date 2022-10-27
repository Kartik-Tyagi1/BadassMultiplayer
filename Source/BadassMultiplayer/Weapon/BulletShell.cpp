#include "BulletShell.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundCue.h"

ABulletShell::ABulletShell()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	BulletShellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletShellMesh"));
	SetRootComponent(BulletShellMesh);

	BulletShellMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	BulletShellMesh->SetSimulatePhysics(true);
	BulletShellMesh->SetEnableGravity(true);
	BulletShellMesh->SetNotifyRigidBodyCollision(true);

}

void ABulletShell::BeginPlay()
{
	Super::BeginPlay();
	BulletShellMesh->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);

	FVector RandomShellOrientation = UKismetMathLibrary::RandomUnitVectorInConeInDegrees(GetActorForwardVector(), 20.f);
	BulletShellMesh->AddImpulse(RandomShellOrientation * FMath::RandRange(5,10));
	
	SetLifeSpan(3.f);
}

void ABulletShell::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (BulletShellHitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, BulletShellHitSound, GetActorLocation());
	}
	BulletShellMesh->SetNotifyRigidBodyCollision(false);
}




