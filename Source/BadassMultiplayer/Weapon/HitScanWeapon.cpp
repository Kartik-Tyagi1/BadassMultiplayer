#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "BadassMultiplayer/Character/MultiplayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"
#include "WeaponTypes.h"

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


			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHitResult.ImpactPoint);
			}
		}

		if (MuzzleFlashParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlashParticles, MuzzleFlashSocketTransform);
		}

		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}

	}
}

FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	FVector DirectionToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	FVector SphereCenter = TraceStart + DirectionToTargetNormalized * DistanceToSphere;
	FVector Rand = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	FVector EndLocation = SphereCenter + Rand;
	FVector DirectionToEndLocation = EndLocation - TraceStart;

	DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	DrawDebugSphere(GetWorld(), EndLocation, 4.f, 12, FColor::Green, true);
	DrawDebugLine(GetWorld(),
		TraceStart, 
		TraceStart + DirectionToEndLocation * TRACE_LENGTH / DirectionToEndLocation.Size(),
		FColor::Orange,
		true);

	return FVector(TraceStart + DirectionToEndLocation * TRACE_LENGTH / DirectionToEndLocation.Size());
}
