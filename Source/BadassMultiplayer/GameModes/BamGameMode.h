// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BamGameMode.generated.h"

/**
 * 
 */

class AMultiplayerCharacter;
class AMPPlayerController;

UCLASS()
class BADASSMULTIPLAYER_API ABamGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PlayerEliminated(AMultiplayerCharacter* EliminatedCharacter, AMPPlayerController* EliminatedController, AMPPlayerController* AttackerController);
	virtual void RequestPlayerRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController);
};
