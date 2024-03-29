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

		FHitResult FireHitResult;
		WeaponTraceHit(Start, HitTarget, FireHitResult);

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

		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHitResult.ImpactPoint);
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

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHitResult)
{
	FVector TraceEnd = bUseScatter ? TraceEndWithScatter(TraceStart, HitTarget) : TraceStart + (HitTarget - TraceStart) * 1.25;
	GetWorld()->LineTraceSingleByChannel(OutHitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility);

	FVector SmokeBeamEnd = TraceEnd;
	if (OutHitResult.bBlockingHit)
	{
		SmokeBeamEnd = OutHitResult.ImpactPoint;
	}

	if (SmokeBeamParticles)
	{
		UParticleSystemComponent* SmokeBeamParticleComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			SmokeBeamParticles,
			TraceStart,
			FRotator::ZeroRotator,
			true
		);

		if (SmokeBeamParticleComponent)
		{
			SmokeBeamParticleComponent->SetVectorParameter(FName("Target"), SmokeBeamEnd);
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

	/*DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	DrawDebugSphere(GetWorld(), EndLocation, 4.f, 12, FColor::Green, true);
	DrawDebugLine(GetWorld(),
		TraceStart, 
		TraceStart + DirectionToEndLocation * TRACE_LENGTH / DirectionToEndLocation.Size(),
		FColor::Orange,
		true);*/

	return FVector(TraceStart + DirectionToEndLocation * TRACE_LENGTH / DirectionToEndLocation.Size());
}


