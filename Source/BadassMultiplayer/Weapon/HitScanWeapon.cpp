#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "BadassMultiplayer/Character/MultiplayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"

void AHitScanWeapon::FireWeapon(const FVector& HitTarget)
{
	Super::FireWeapon(HitTarget);

	// The pawn that fired the weapon
	APawn* PawnInstigator = Cast<APawn>(GetOwner());
	if (PawnInstigator == nullptr) return;

	AController* PawnController = PawnInstigator->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket)
	{
		const FTransform MuzzleFlashSocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = MuzzleFlashSocketTransform.GetLocation();
		FVector End = Start + (HitTarget - Start) * 1.25;

		FHitResult FireHitResult;
		GetWorld()->LineTraceSingleByChannel(FireHitResult, Start, End, ECollisionChannel::ECC_Visibility);

		FVector SmokeBeamEnd = End;

		if (FireHitResult.bBlockingHit)
		{
			SmokeBeamEnd = FireHitResult.ImpactPoint;

			AMultiplayerCharacter* HitCharacter = Cast< AMultiplayerCharacter>(FireHitResult.GetActor());
			if (HitCharacter && HasAuthority() && PawnController)
			{
				UGameplayStatics::ApplyDamage(
					HitCharacter,
					DamageAmount,
					PawnController,
					this,
					UDamageType::StaticClass()
				);
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

			if (SmokeBeamParticles)
			{
				UParticleSystemComponent* SmokeBeamParticleComponent = UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					SmokeBeamParticles,
					MuzzleFlashSocketTransform
				);

				if (SmokeBeamParticleComponent)
				{
					SmokeBeamParticleComponent->SetVectorParameter(FName("Target"), SmokeBeamEnd);
				}
			}
		}
	}
}
