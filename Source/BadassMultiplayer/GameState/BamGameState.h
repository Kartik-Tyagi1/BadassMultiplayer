#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BamGameState.generated.h"

/**
 * 
 */

class ABamPlayerState;

UCLASS()
class BADASSMULTIPLAYER_API ABamGameState : public AGameState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void UpdateTopScore(ABamPlayerState* ScoringPlayer);

	UPROPERTY(Replicated)
	TArray<ABamPlayerState*> TopScoringPlayers;

private:
	float TopScore = 0;
};
