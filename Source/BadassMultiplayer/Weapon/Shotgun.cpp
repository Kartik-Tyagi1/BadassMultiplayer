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
		for (uint32 i = 0; i < NumOfPellets; i++)
		{
			FVector End = TraceEndWithScatter(Start, HitTarget);

		}
	}
}
