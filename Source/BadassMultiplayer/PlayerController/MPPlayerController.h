#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BadassMultiplayer/Weapon/WeaponTypes.h"
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
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo); // Ammo In the Weapon 
	void SetHUDCarriedAmmo(int32 CarriedAmmo); // Ammo In the Weapon 
	void SetHUDWeaponType(EWeaponType WeaponType);
	void SetHUDMatchTimer(float CountdownTime);
	void SetElimText(FString Text);
	void ClearElimText();
	virtual void OnPossess(APawn* InPawn) override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	void SetHUDTime();

private:
	UPROPERTY()
	ABadassHUD* BadassHUD;

	float MatchTime = 120.f;
	uint32 CountdownInt = 0;
};
