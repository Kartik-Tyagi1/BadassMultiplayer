// Fill out your copyright notice in the Description page of Project Settings.


#include "BamGameMode.h"
#include "BadassMultiplayer/Character/MultiplayerCharacter.h"
#include "BadassMultiplayer/PlayerController/MPPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "BadassMultiplayer/PlayerState/BamPlayerState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown"); 
}

ABamGameMode::ABamGameMode()
{
	bDelayedStart = true;
}

void ABamGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Time since when the multiplayer map is loaded and not when the game loads (aka the "host/join" menu screen)
	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void ABamGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
	}
}

void ABamGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AMPPlayerController* Player = Cast<AMPPlayerController>(*It);
		if (Player)
		{
			Player->OnMatchStateSet(MatchState);
		}
	}
}


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
		EliminatedCharacter->Eliminated(AttackerController);
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

