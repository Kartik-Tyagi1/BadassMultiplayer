#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "BadassMultiplayer/Character/MultiplayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AShotgun::FireWeapon(const FVector& HitTarget)
{
	AWeapon::FireWeapon(HitTarget);

	// The pawn that fired the weapon
	APawn* PawnInstigator = Cast<APawn>(GetOwner());
	if (PawnInstigator == nullptr) return;

	AController* PawnController = PawnInstigator->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket)
	{
		const FTransform MuzzleFlashSocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = MuzzleFlashSocketTransform.GetLocation();

		TMap<AMultiplayerCharacter*, uint32> HitMap; // Used to check if pellets spread enough to hit mulitple characters

		for (uint32 i = 0; i < NumOfPellets; i++)
		{
			FHitResult FireHitResult;
			WeaponTraceHit(Start, HitTarget, FireHitResult);

			AMultiplayerCharacter* HitCharacter = Cast< AMultiplayerCharacter>(FireHitResult.GetActor());
			if (HitCharacter && HasAuthority() && PawnController)
			{
				if (HitMap.Contains(HitCharacter))
				{
					HitMap[HitCharacter]++;
				}
				else
				{
					HitMap.Emplace(HitCharacter, 1);
				}
			}

			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					FireHitResult.ImpactPoint,
					FireHitResult.ImpactNormal.Rotation()
				);
			}

			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(
					this,
					HitSound,
					FireHitResult.ImpactPoint,
					0.5f,
					FMath::FRandRange(-0.5f, 0.5f));
			}
		}

		for (auto HitPair : HitMap)
		{
			if (HitPair.Key && HasAuthority() && PawnController)
			{
				UGameplayStatics::ApplyDamage(
					HitPair.Key,
					DamageAmount * HitPair.Value,
					PawnController,
					this,
					UDamageType::StaticClass()
				);
			}
		}
	}
}
