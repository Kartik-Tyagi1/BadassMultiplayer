#include "BamPlayerState.h"
#include "BadassMultiplayer/Character/MultiplayerCharacter.h"
#include "BadassMultiplayer/PlayerController/MPPlayerController.h"

// Called on Server
void ABamPlayerState::AddToScore(float ScoreAmount)
{
	Score += ScoreAmount;
	Character = Character == nullptr ? Cast<AMultiplayerCharacter>(GetPawn()) : Character;
	if (Character)
	{
		PlayerController = PlayerController == nullptr ? Cast<AMPPlayerController>(Character->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->SetHUDKillCount(Score);
		}
	}
}

// Called on Clients
void ABamPlayerState::OnRep_Score()
{
	Character = Character == nullptr ? Cast<AMultiplayerCharacter>(GetPawn()) : Character;
	if (Character)
	{
		PlayerController = PlayerController == nullptr ? Cast<AMPPlayerController>(Character->Controller) : PlayerController;
		if (PlayerController)
		{
			// Score is replicated variable on the APlayerState class
			PlayerController->SetHUDKillCount(Score);
		}
	}
}


