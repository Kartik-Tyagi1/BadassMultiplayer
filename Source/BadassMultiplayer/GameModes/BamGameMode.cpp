// Fill out your copyright notice in the Description page of Project Settings.


#include "BamGameMode.h"
#include "BadassMultiplayer/Character/MultiplayerCharacter.h"

void ABamGameMode::PlayerEliminated(AMultiplayerCharacter* EliminatedCharacter, AMPPlayerController* EliminatedController, AMPPlayerController* AttackerController)
{
	EliminatedCharacter->Eliminated();
}
