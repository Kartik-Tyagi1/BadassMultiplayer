// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem():
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
	StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{
	// Create the Online Subsystem (aka steam online services)
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		// Use Online subsystem to create a session Interface (use steam online service to create an session interface which can be used to create online sessions)
		// Since we are using the online subsystem we dont need direct steam code. It is abstracted by the online susbsystem pointer and the the session interface pointer and more ...
		OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();
	}
}

void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{
	// Exit if the Online Session Interface is null
	if (!OnlineSessionInterface.IsValid())
	{
		return;
	}

	// Destory the existing game session is one already exists
	auto ExistingSession = OnlineSessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
		/*
		*	When a player who is hosting quits the game is shuts down the server. If another player tries to host after that then the old session will be destoryed here.
		*	But it won't create a session right away afterwards. To allow that to happen we need to create the session from the callback of destroy session
		*/
		MultiplayerOnDestroySessionComplete.Broadcast(false);
		bCreateSessionOnDestroy = true;
		LastNumPublicConnections = NumPublicConnections;
		LastMatchType = MatchType;
		DestroySession();
	}

	// Add the delegate the the OnlineSessionInterface Delegate List and store it so if needed we can remove it from the OnlineSessionInterface Delegate list
	CreateSessionCompleteDelegateHandle = OnlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	// Create and configure the session settings 
	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	LastSessionSettings->NumPublicConnections = NumPublicConnections;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->bUseLobbiesIfAvailable = true;
	LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	LastSessionSettings->BuildUniqueId = 1;

	// Get the net ID from the local player
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();

	// Create the session
	if (!OnlineSessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings))
	{
		// If the session was not created remove the delegate from the delegate list
		OnlineSessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

		// Broadcast the delegate so that menu knows the session was NOT created
		MultiplayerOnCreateSessionComplete.Broadcast(false);
	}

}

void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
	// Exit if the Online Session Interface is null
	if (!OnlineSessionInterface.IsValid())
	{
		return;
	}

	// Add the delegate the the OnlineSessionInterface Delegate List
	FindSessionsCompleteDelegateHandle = OnlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	// Create and configure the session search settings 
	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;;
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	// Get the net ID from the local player
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();

	// Find the sessions the correspond to the match settings 
	if (!OnlineSessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))
	{
		// Remove Delegate from list if failed to find 
		OnlineSessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		// Send empty array and failed status
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
	// Exit if the Online Session Interface is null and send info to menu
	if (!OnlineSessionInterface.IsValid())
	{
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	// Add the delegate the the OnlineSessionInterface Delegate List
	JoinSessionCompleteDelegateHandle = OnlineSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	// Get the net ID from the local player
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();

	// Join the session
	if (!OnlineSessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult))
	{
		// Clear Delegate handle and send info of failure
		OnlineSessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
	// Exit if the Online Session Interface is null and send info to menu
	if (!OnlineSessionInterface.IsValid())
	{
		MultiplayerOnDestroySessionComplete.Broadcast(false);
		return;
	}

	// Add the delegate the the OnlineSessionInterface Delegate List
	DestroySessionCompleteDelegateHandle = OnlineSessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	if (!OnlineSessionInterface->DestroySession(NAME_GameSession))
	{
		OnlineSessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		MultiplayerOnDestroySessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::StartSession()
{
	if (!OnlineSessionInterface.IsValid())
	{
		MultiplayerOnStartSessionComplete.Broadcast(false);
		return;
	}

	StartSessionCompleteDelegateHandle = OnlineSessionInterface->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);

	if (!OnlineSessionInterface->StartSession(NAME_GameSession))
	{
		OnlineSessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
		MultiplayerOnStartSessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (OnlineSessionInterface)
	{
		// Remove the delegate from the delegate list when session is created
		OnlineSessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	// Broadcast information to menu class that session was created
	MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (OnlineSessionInterface)
	{
		// Remove the delegate from the delegate list when sessions are found
		OnlineSessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}
	
	// Check to see if result array is empty even though function went through as a success
	if (LastSessionSearch->SearchResults.Num() <= 0)
	{
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}

	// Broadcast found sessions to menu
	MultiplayerOnFindSessionsComplete.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (OnlineSessionInterface)
	{
		OnlineSessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}
	
	MultiplayerOnJoinSessionComplete.Broadcast(Result);
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (OnlineSessionInterface)
	{
		OnlineSessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}

	if (bWasSuccessful && bCreateSessionOnDestroy)
	{
		bCreateSessionOnDestroy = false;
		CreateSession(LastNumPublicConnections, LastMatchType);
	}

	MultiplayerOnDestroySessionComplete.Broadcast(true);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (OnlineSessionInterface)
	{
		OnlineSessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
	}

	MultiplayerOnStartSessionComplete.Broadcast(bWasSuccessful);
}
