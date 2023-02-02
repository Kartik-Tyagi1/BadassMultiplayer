#include "AmmoPickup.h"
#include "BadassMultiplayer/Character/MultiplayerCharacter.h"
#include "BadassMultiplayer/MultiplayerComponents/CombatComponent.h"

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AMultiplayerCharacter* Character = Cast<AMultiplayerCharacter>(OtherActor);
	if (Character && Character->GetCombatComponent())
	{
		Character->GetCombatComponent()->PickupAmmo(WeaponType, AmmoAmount);
	}

	Destroy();
}
