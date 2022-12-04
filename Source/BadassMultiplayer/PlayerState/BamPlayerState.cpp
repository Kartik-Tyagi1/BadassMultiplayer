#include "BamPlayerState.h"
#include "BadassMultiplayer/Character/MultiplayerCharacter.h"
#include "BadassMultiplayer/PlayerController/MPPlayerController.h"
#include "Net/UnrealNetwork.h"


void ABamPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABamPlayerState, Defeats);
}

// Called on Server
void ABamPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	Character = Character == nullptr ? Cast<AMultiplayerCharacter>(GetPawn()) : Character;
	if (Character)
	{
		PlayerController = PlayerController == nullptr ? Cast<AMPPlayerController>(Character->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->SetHUDKillCount(GetScore());
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
			PlayerController->SetHUDKillCount(GetScore());
		}
	}
}

void ABamPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;
	Character = Character == nullptr ? Cast<AMultiplayerCharacter>(GetPawn()) : Character;
	if (Character)
	{
		PlayerController = PlayerController == nullptr ? Cast<AMPPlayerController>(Character->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->SetHUDDefeats(Defeats);
		}
	}
}

void ABamPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<AMultiplayerCharacter>(GetPawn()) : Character;
	if (Character)
	{
		PlayerController = PlayerController == nullptr ? Cast<AMPPlayerController>(Character->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->SetHUDDefeats(Defeats);
		}
	}
}







