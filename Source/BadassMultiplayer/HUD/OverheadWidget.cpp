// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent();
	Super::NativeDestruct();
}


void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	/*
	* LOCAL NET ROLE
	* ENetRole::Authority -------> Actual version of the player on the server
	* ENetRole::AutonomousProxy -> Client version of the player on their own machine
	* ENetRole::SimulatedProxy---> Other characters of the player on another player's machine
	* 
	* REMOTE NET ROLE
	* ENetRole::Authority -------> Actual version of the player on their own machine
	* ENetRole::AutonomousProxy -> Client version of the player on the server
	* ENetRole::SimulatedProxy---> Other characters of the player on the server
	* 
	* LOCAL and REMOTE NET ROLES are reversed
	*/

	ENetRole LocalNetRole = InPawn->GetLocalRole();
	FString Role;

	switch (LocalNetRole)
	{
		case ENetRole::ROLE_None:
			Role = FString("None");
			break;
		case ENetRole::ROLE_SimulatedProxy:
			Role = FString("Simulated Proxy");
			break;
		case ENetRole::ROLE_AutonomousProxy:
			Role = FString("Autonomous Proxy");
			break;
		case ENetRole::ROLE_Authority:
			Role = FString("Authority");
			break;
		default:
			break;
	}

	FString LocalRoleString = FString::Printf(TEXT("Local Role is: %s"), *Role);
	SetDisplayText(LocalRoleString);
}

void UOverheadWidget::ShowPlayerName(APawn* InPawn)
{
	APlayerState* PlayerState = InPawn->GetPlayerState<APlayerState>();
	if (PlayerState)
	{
		FString PlayerNameString = FString::Printf(TEXT("Player Name Is is: %s"), *PlayerState->GetPlayerName());
		SetDisplayText(PlayerNameString);
	}

}


