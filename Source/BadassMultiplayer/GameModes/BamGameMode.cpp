// Fill out your copyright notice in the Description page of Project Settings.


#include "BamGameMode.h"
#include "BadassMultiplayer/Character/MultiplayerCharacter.h"
#include "BadassMultiplayer/PlayerController/MPPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "BadassMultiplayer/PlayerState/BamPlayerState.h"

void ABamGameMode::PlayerEliminated(AMultiplayerCharacter* EliminatedCharacter, AMPPlayerController* VictimController, AMPPlayerController* AttackerController)
{
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;

	ABamPlayerState* AttackerPlayerState = AttackerController ? Cast<ABamPlayerState>(AttackerController->PlayerState) : nullptr;
	ABamPlayerState* VictimPlayerState = VictimController ? Cast<ABamPlayerState>(VictimController->PlayerState) : nullptr;

	// Check if player didnt kill themselves
	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}

	if (EliminatedCharacter)
	{
		EliminatedCharacter->Eliminated();
	}
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
