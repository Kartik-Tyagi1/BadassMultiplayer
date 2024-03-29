#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BadassMultiplayer/Weapon/WeaponTypes.h"
#include "MPPlayerController.generated.h"

/**
 * 
 */

class ABadassHUD;
class UCharacterOverlay;
class ABamGameMode;

UCLASS()
class BADASSMULTIPLAYER_API AMPPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	void SetHUDHealthStats(float Health, float MaxHealth);
	void SetHUDShieldStats(float Shield, float MaxShield);
	void SetHUDKillCount(float Kills);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo); // Ammo In the Weapon 
	void SetHUDCarriedAmmo(int32 CarriedAmmo); // Ammo In the Weapon 
	void SetHUDWeaponType(EWeaponType WeaponType);
	void SetHUDMatchTimer(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);

	void SetElimText(FString Text);
	void ClearElimText();
	void SetHUDGrenades(int32 Grenades);
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

	void PollInit();

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(FName StateOfMatch, float Match, float Warmup, float StartingTime, float Cooldown);

private:
	UPROPERTY()
	ABadassHUD* BadassHUD;

	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;

	UPROPERTY()
	ABamGameMode* BamGameMode;

	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float LevelStartingTime = 0.f;
	float CooldownTime = 0.f;
	uint32 CountdownInt = 0;

	UPROPERTY(Replicated, ReplicatedUsing=OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();
	
	/* Booleans to determine if the values need to be inititalized when the character overlay is valid */
	bool bInitializeHealth = false;
	bool bInitializeShield = false;
	bool bInitializeKills = false;
	bool bInitializeDeaths = false;
	bool bInitializeGrenades = false;
	bool bInitializeWeaponAmmo = false;
	bool bInitializeCarriedAmmo = false;

	/* Cached values that will be used to initialize the HUD */
	float HUDHealth;
	float HUDMaxHealth;

	float HUDShield;
	float HUDMaxShield;

	float HUDKills;
	int32 HUDDeaths;

	int32 HUDGrenades;

	int32 HUDWeaponAmmo;
	int32 HUDCarriedAmmo;


public: 
	void OnMatchStateSet(FName State);
	void HandleMatchHasStarted();
	void HandleCooldown();

};
