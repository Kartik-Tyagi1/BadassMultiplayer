#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "BadassMultiplayer/Character/MultiplayerCharacter.h"
#include "Kismet/GameplayStatics.h"

void AHitScanWeapon::FireWeapon(const FVector& HitTarget)
{
	Super::FireWeapon(HitTarget);

	// The pawn that fired the weapon
	APawn* PawnInstigator = Cast<APawn>(GetOwner());
	if (PawnInstigator == nullptr) return;

	AController* PawnController = PawnInstigator->GetController();
	if (PawnController == nullptr) return;

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket)
	{
		const FTransform MuzzleFlashSocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = MuzzleFlashSocketTransform.GetLocation();
		FVector End = Start + (HitTarget - Start) * 1.25;

		FHitResult HitResult;
		GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility);

		if (HitResult.bBlockingHit)
		{
			AMultiplayerCharacter* HitCharacter = Cast< AMultiplayerCharacter>(HitResult.GetActor());
			if (HitCharacter)
			{
				if (HasAuthority())
				{
					UGameplayStatics::ApplyDamage(
						HitCharacter,
						DamageAmount,
						PawnController,
						this,
						UDamageType::StaticClass()
					);
				}
			}

			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					HitResult.ImpactPoint,
					HitResult.ImpactNormal.Rotation()
				);
			}
		}
	}
}
