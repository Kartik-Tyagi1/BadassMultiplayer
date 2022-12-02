// Fill out your copyright notice in the Description page of Project Settings.


#include "BamGameMode.h"
#include "BadassMultiplayer/Character/MultiplayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

void ABamGameMode::PlayerEliminated(AMultiplayerCharacter* EliminatedCharacter, AMPPlayerController* EliminatedController, AMPPlayerController* AttackerController)
{
	EliminatedCharacter->Eliminated();
}

void ABamGameMode::RequestPlayerRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController)
{
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Reset(); // Detach the Controller
		EliminatedCharacter->Destroy(); // Destroy the Character
	}

	if (EliminatedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);

		// Create a new character and attach the controller to it
		RestartPlayerAtPlayerStart(EliminatedController, PlayerStarts[Selection]);
	}
}
