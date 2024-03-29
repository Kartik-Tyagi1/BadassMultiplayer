#include "JumpPickup.h"
#include "BadassMultiplayer/Character/MultiplayerCharacter.h"
#include "BadassMultiplayer/MultiplayerComponents/BuffComponent.h"

void AJumpPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AMultiplayerCharacter* Character = Cast<AMultiplayerCharacter>(OtherActor);
	if (Character && Character->GetBuffComponent())
	{
		Character->GetBuffComponent()->BuffJump(JumpZVelocityBuff, JumpBuffTime);
	}

	Destroy();
}
