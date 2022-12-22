#include "MPPlayerController.h"
#include "BadassMultiplayer/HUD/BadassHUD.h"
#include "BadassMultiplayer/HUD/CharacterOverlay.h"
#include "BadassMultiplayer/HUD/Announcement.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "BadassMultiplayer/Character/MultiplayerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "BadassMultiplayer/GameModes/BamGameMode.h"
#include "Kismet/GameplayStatics.h"


void AMPPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BadassHUD = Cast<ABadassHUD>(GetHUD());
	ServerCheckMatchState();

}

void AMPPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMPPlayerController, MatchState);
}

void AMPPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInit();
}

void AMPPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void AMPPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
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
	else
	{
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
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
	else
	{
		bInitializeCharacterOverlay = true;
		HUDKills = Kills;
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
	else
	{
		bInitializeCharacterOverlay = true;
		HUDDeaths = Defeats;
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

void AMPPlayerController::SetHUDWeaponType(EWeaponType WeaponType)
{
	BadassHUD = BadassHUD == nullptr ? Cast<ABadassHUD>(GetHUD()) : BadassHUD;

	bool bIsHUDValid = BadassHUD && BadassHUD->CharacterOverlay &&
		BadassHUD->CharacterOverlay->WeaponType;

	if (bIsHUDValid)
	{
		FString WeaponTypeString;
		switch (WeaponType)
		{
		case EWeaponType::EWT_AssaultRifle:
			WeaponTypeString = FString("Assault Rifle");
			break;
		case EWeaponType::EWT_MAX:
			WeaponTypeString = FString("");
			break;
		}

		BadassHUD->CharacterOverlay->WeaponType->SetText(FText::FromString(WeaponTypeString));
	}
}

void AMPPlayerController::SetElimText(FString Text)
{
	BadassHUD = BadassHUD == nullptr ? Cast<ABadassHUD>(GetHUD()) : BadassHUD;

	bool bIsHUDValid = BadassHUD && BadassHUD->CharacterOverlay &&
		BadassHUD->CharacterOverlay->ElimText;

	if (bIsHUDValid)
	{
		if (Text.IsEmpty())
		{
			FString TextToDisplay = FString::Printf(TEXT("You killed yourself"));
			BadassHUD->CharacterOverlay->ElimText->SetText(FText::FromString(TextToDisplay));
		}
		else
		{
			FString TextToDisplay = FString::Printf(TEXT("You were killed by \r %s"), *Text);
			BadassHUD->CharacterOverlay->ElimText->SetText(FText::FromString(TextToDisplay));
		}
		BadassHUD->CharacterOverlay->ElimText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void AMPPlayerController::ClearElimText()
{
	BadassHUD = BadassHUD == nullptr ? Cast<ABadassHUD>(GetHUD()) : BadassHUD;

	bool bIsHUDValid = BadassHUD && BadassHUD->CharacterOverlay &&
		BadassHUD->CharacterOverlay->ElimText;

	if (bIsHUDValid)
	{
		BadassHUD->CharacterOverlay->ElimText->SetText(FText());
		BadassHUD->CharacterOverlay->ElimText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void AMPPlayerController::SetHUDMatchTimer(float CountdownTime)
{
	BadassHUD = BadassHUD == nullptr ? Cast<ABadassHUD>(GetHUD()) : BadassHUD;

	bool bIsHUDValid = BadassHUD && BadassHUD->CharacterOverlay &&
		BadassHUD->CharacterOverlay->MatchTimerText;

	if (bIsHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			BadassHUD->CharacterOverlay->MatchTimerText->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString MatchTimerString = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BadassHUD->CharacterOverlay->MatchTimerText->SetText(FText::FromString(MatchTimerString));
	}
}

void AMPPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	BadassHUD = BadassHUD == nullptr ? Cast<ABadassHUD>(GetHUD()) : BadassHUD;

	bool bIsHUDValid = BadassHUD && BadassHUD->Announcement &&
		BadassHUD->Announcement->WarmupTime;

	if (bIsHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			BadassHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownString = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BadassHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownString));
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

void AMPPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if (HasAuthority())
	{
		BamGameMode = BamGameMode == nullptr ? Cast<ABamGameMode>(UGameplayStatics::GetGameMode(this)) : BamGameMode;
		if (BamGameMode)
		{
			SecondsLeft = FMath::CeilToInt(BamGameMode->GetCountdownTime() + LevelStartingTime);
		}
	}

	// This function will be called each frame but we only want HUD updated each second so we make a check to see how much time has passed
	//   Once the seconds left is equal to the countdown then we can updated
	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchTimer(TimeLeft);
		}
	}
	
	CountdownInt = SecondsLeft;
}

void AMPPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerRecivedClientRequest = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerRecivedClientRequest);
}

void AMPPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerRecievedClientRequest)
{
	const float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	const float CurrentServerTime = TimeServerRecievedClientRequest + (RoundTripTime *  0.5f); // This can also be Time on Client Machine + RoundTrip Time

	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float AMPPlayerController::GetServerTime()
{
	if (HasAuthority())
	{
		return GetWorld()->GetTimeSeconds();
	}
	else
	{
		return GetWorld()->GetTimeSeconds() + ClientServerDelta;
	}
}

void AMPPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}

}

void AMPPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AMPPlayerController::HandleMatchHasStarted()
{
	BadassHUD = BadassHUD == nullptr ? Cast<ABadassHUD>(GetHUD()) : BadassHUD;
	if (BadassHUD)
	{
		BadassHUD->AddCharacterOverlay();
		if (BadassHUD->Announcement)
		{
			BadassHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AMPPlayerController::HandleCooldown()
{
	BadassHUD = BadassHUD == nullptr ? Cast<ABadassHUD>(GetHUD()) : BadassHUD;
	if (BadassHUD)
	{
		BadassHUD->CharacterOverlay->RemoveFromParent();
		bool bIsHUDValid = BadassHUD->Announcement && 
			BadassHUD->Announcement->AnnouncemetText && BadassHUD->Announcement->InfoText;

		if (bIsHUDValid)
		{
			BadassHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementString = FString::Printf(TEXT("Match Ended. New Match Starts In:"));
			BadassHUD->Announcement->AnnouncemetText->SetText(FText::FromString(AnnouncementString));
			BadassHUD->Announcement->InfoText->SetText(FText());
		}
	}
}

void AMPPlayerController::ServerCheckMatchState_Implementation()
{
	ABamGameMode* GameMode = Cast<ABamGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		CooldownTime = GameMode->CooldownTime;
		ClientJoinMidGame(MatchState, MatchTime, WarmupTime, LevelStartingTime, CooldownTime);
	}
}

void AMPPlayerController::ClientJoinMidGame_Implementation(FName StateOfMatch, float Match, float Warmup, float StartingTime, float Cooldown)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	CooldownTime = Cooldown;
	OnMatchStateSet(MatchState);

	if (BadassHUD && MatchState == MatchState::WaitingToStart)
	{
		BadassHUD->AddAnnouncement();
	}
}

void AMPPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (BadassHUD && BadassHUD->CharacterOverlay)
		{
			CharacterOverlay = BadassHUD->CharacterOverlay;
			SetHUDHealthStats(HUDHealth, HUDMaxHealth);
			SetHUDKillCount(HUDKills);
			SetHUDDefeats(HUDDeaths);
		}
	}
}



