#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::FireWeapon(const FVector& HitTarget)
{
	Super::FireWeapon(HitTarget);

	APawn* ProjectileInstigator = Cast<APawn>(GetOwner());

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket)
	{
		const FTransform MuzzleFlashSocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());

		// Direction of the vector between the hit location and the socket location
		FVector ProjectileDirection = HitTarget - MuzzleFlashSocketTransform.GetLocation();
		FRotator ProjectileRotaion = ProjectileDirection.Rotation();

		if (ProjectileClass && ProjectileInstigator)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = ProjectileInstigator;
			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<AProjectile>(ProjectileClass, MuzzleFlashSocketTransform.GetLocation(), ProjectileRotaion, SpawnParams);
			}
		}
	}

}
