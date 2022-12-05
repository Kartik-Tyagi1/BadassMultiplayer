#include "MPPlayerController.h"
#include "BadassMultiplayer/HUD/BadassHUD.h"
#include "BadassMultiplayer/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "BadassMultiplayer/Character/MultiplayerCharacter.h"

void AMPPlayerController::BeginPlay()
{
	Super::BeginPlay();
	BadassHUD = Cast<ABadassHUD>(GetHUD());
}

void AMPPlayerController::SetHUDHealthStats(float Health, float MaxHealth)
{
	BadassHUD = BadassHUD == nullptr ? Cast<ABadassHUD>(GetHUD()) : BadassHUD;

	bool bIsHUDValid = BadassHUD && BadassHUD->CharacterOverlay &&
		BadassHUD->CharacterOverlay->HealthBar && BadassHUD->CharacterOverlay->HealthText;

	if (bIsHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		BadassHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthTextString = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		BadassHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthTextString));
	}
}

void AMPPlayerController::SetHUDKillCount(float Kills)
{
	BadassHUD = BadassHUD == nullptr ? Cast<ABadassHUD>(GetHUD()) : BadassHUD;

	bool bIsHUDValid = BadassHUD && BadassHUD->CharacterOverlay &&
		BadassHUD->CharacterOverlay->KillCount;

	if (bIsHUDValid)
	{
		FString KillString = FString::Printf(TEXT("%d"), FMath::FloorToInt(Kills));
		BadassHUD->CharacterOverlay->KillCount->SetText(FText::FromString(KillString));
	}

}

void AMPPlayerController::SetHUDDefeats(int32 Defeats)
{
	BadassHUD = BadassHUD == nullptr ? Cast<ABadassHUD>(GetHUD()) : BadassHUD;

	bool bIsHUDValid = BadassHUD && BadassHUD->CharacterOverlay &&
		BadassHUD->CharacterOverlay->DefeatsAmount;

	if (bIsHUDValid)
	{
		FString DefeatsString = FString::Printf(TEXT("%d"), Defeats);
		BadassHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsString));
	}
}

void AMPPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	BadassHUD = BadassHUD == nullptr ? Cast<ABadassHUD>(GetHUD()) : BadassHUD;

	bool bIsHUDValid = BadassHUD && BadassHUD->CharacterOverlay &&
		BadassHUD->CharacterOverlay->WeaponAmmoAmount;

	if (bIsHUDValid)
	{
		FString AmmoString = FString::Printf(TEXT("%d"), Ammo);
		BadassHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoString));
	}
}

void AMPPlayerController::SetHUDCarriedAmmo(int32 CarriedAmmo)
{
	BadassHUD = BadassHUD == nullptr ? Cast<ABadassHUD>(GetHUD()) : BadassHUD;

	bool bIsHUDValid = BadassHUD && BadassHUD->CharacterOverlay &&
		BadassHUD->CharacterOverlay->CarriedAmmoAmount;

	if (bIsHUDValid)
	{
		FString CarriedAmmoString = FString::Printf(TEXT("%d"), CarriedAmmo);
		BadassHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(CarriedAmmoString));
	}
}

void AMPPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	AMultiplayerCharacter* MPCharacter = Cast<AMultiplayerCharacter>(InPawn);
	if (MPCharacter)
	{
		SetHUDHealthStats(MPCharacter->GetMaxHealth(), MPCharacter->GetMaxHealth());
	}
}
