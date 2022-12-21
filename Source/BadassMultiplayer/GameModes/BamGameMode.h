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
	ABamGameMode();
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(AMultiplayerCharacter* EliminatedCharacter, AMPPlayerController* VictimController, AMPPlayerController* AttackerController);
	virtual void RequestPlayerRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;
};
