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
	virtual float GetServerTime(); // Synced with server world clock
	virtual void ReceivedPlayer() override; // Sync with server clock as soon as possible

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	void SetHUDTime();

	/* Sync Time between Client and Server */

	// Request Current Time on the Server passing in the current time of the client when the request is sent
	UFUNCTION(Server, Reliable)
		void ServerRequestServerTime(float TimeOfClientRequest);

	// Reports the current server time to the client in response to ServerRequestServerTime()
	UFUNCTION(Client, Reliable)
		void ClientReportServerTime(float TimeOfClientRequest, float TimeServerRecievedClientRequest);

	// Difference between Client and Server Times
	float ClientServerDelta = 0;

	// How often the server and client times should be re-synced
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;
	
	float TimeSyncRunningTime = 0.f;

	void CheckTimeSync(float DeltaTime);

private:
	UPROPERTY()
	ABadassHUD* BadassHUD;

	float MatchTime = 120.f;
	uint32 CountdownInt = 0;

};
