#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BamPlayerState.generated.h"

/**
 * 
 */

class AMultiplayerCharacter;
class AMPPlayerController;

UCLASS()
class BADASSMULTIPLAYER_API ABamPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void OnRep_Score() override;
	void AddToScore(float ScoreAmount);

private:
	AMultiplayerCharacter* Character;
	AMPPlayerController* PlayerController;
	
};
