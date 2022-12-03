#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MPPlayerController.generated.h"

/**
 * 
 */

class ABadassHUD;

UCLASS()
class BADASSMULTIPLAYER_API AMPPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void SetHUDHealthStats(float Health, float MaxHealth);
	void SetHUDKillCount(float Kills);
	virtual void OnPossess(APawn* InPawn) override;

protected:
	virtual void BeginPlay() override;

private:
	ABadassHUD* BadassHUD;
};
