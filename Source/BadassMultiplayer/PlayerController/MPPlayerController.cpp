#include "MPPlayerController.h"
#include "BadassMultiplayer/HUD/BadassHUD.h"
#include "BadassMultiplayer/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

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
