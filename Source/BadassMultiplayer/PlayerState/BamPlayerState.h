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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Score() override;

	UFUNCTION()
	virtual void OnRep_Defeats();

	void AddToScore(float ScoreAmount);
	void AddToDefeats(int32 DefeatsAmount);

private:
	UPROPERTY()
	AMultiplayerCharacter* Character;

	UPROPERTY()
	AMPPlayerController* PlayerController;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;
	
};
