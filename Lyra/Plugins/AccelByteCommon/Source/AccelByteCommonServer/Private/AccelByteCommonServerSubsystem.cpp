// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "AccelByteCommonServerSubsystem.h"

#include "OnlineSessionInterfaceAccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "Core/AccelByteMultiRegistry.h"
#include "BlackBoxSDKModule.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAccelByteCommonServer, Log, All);
DEFINE_LOG_CATEGORY(LogAccelByteCommonServer);

void UAccelByteCommonServerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	FString GameVersion;
	GConfig->GetString(
		TEXT("/Script/EngineSettings.GeneralProjectSettings"),
		TEXT("ProjectVersion"),
		GameVersion,
		GGameIni
	);

	// Log out the game version, debugging purpose.
	UE_LOG(LogAccelByteCommonServer, Log, TEXT("GAME VERSION = %s"), *GameVersion);
}

void UAccelByteCommonServerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UAccelByteCommonServerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	TArray<UClass*> ChildClasses;
	GetDerivedClasses(GetClass(), ChildClasses, false);

	// Only create an instance if there is not a game-specific subclass
	return ChildClasses.Num() == 0;
}

bool UAccelByteCommonServerSubsystem::StartServerInitialization()
{
#if UE_SERVER
	if(ServerState < EServerState::ServerLogin)
	{
		ServerState = EServerState::ServerLogin;
		ServerLogin();
		return true;
	}
	else if(ServerState < EServerState::RegisterToDSM)
	{
		ServerState = EServerState::RegisterToDSM;
		RegisterServerToDSM();
		return true;
	}

	UE_LOG(LogAccelByteCommonServer, Warning, TEXT("Session registration already in process."));
#endif
	return false;
}

void UAccelByteCommonServerSubsystem::TryConstructSession()
{
#if UE_SERVER
	if(SessionData.Session_id.IsEmpty())
	{
		GetSessionIdDSM();
	}
	else
	{
		EServerSessionType SessionType = GetServerSessionType();
		if(SessionType == EServerSessionType::UNKNOWN)
		{
			UE_LOG(LogAccelByteCommonServer, Warning, TEXT("Session is UNKNOWN."));
			return;
		}
		else if(SessionType == EServerSessionType::CustomMatch)
		{
			UE_LOG(LogAccelByteCommonServer, Log, TEXT("Found dashes on the session ID, means this is custom match"));
			GetServerApi()->ServerSessionBrowser.GetGameSessionBySessionId(SessionData.Session_id,
			THandler<FAccelByteModelsSessionBrowserData>::CreateUObject(this, &UAccelByteCommonServerSubsystem::OnGetSessionBySessionIdSuccess),
				FErrorHandler::CreateUObject(this, &UAccelByteCommonServerSubsystem::OnQuerySessionStatusFailed)
			);
		}
		else
		{
			UE_LOG(LogAccelByteCommonServer, Log, TEXT("Not found dashes on the session ID, means this is matchmaking session"));
			GetServerApi()->ServerMatchmaking.QuerySessionStatus(SessionData.Session_id, 
			THandler<FAccelByteModelsMatchmakingResult>::CreateUObject(this, &UAccelByteCommonServerSubsystem::OnQuerySessionStatusSuccess),
				FErrorHandler::CreateUObject(this, &UAccelByteCommonServerSubsystem::OnQuerySessionStatusFailed)
			);
		}
	}
#endif
}

void UAccelByteCommonServerSubsystem::TryDestructSession()
{
#if UE_SERVER
	GetServerApi()->ServerDSM.SendShutdownToDSM(
		true,
		SessionData.Session_id,
		FVoidHandler::CreateLambda([this]()
		{
			UE_LOG(LogAccelByteCommonServer, Log, TEXT("Shutting Down the Server Success!"));
		}),
		FErrorHandler::CreateUObject(this, &UAccelByteCommonServerSubsystem::OnAccelByteCommonServerError));

	if(SessionData.Joinable && !SessionData.Match.Match_id.IsEmpty())
	{
		DequeueJoinable();
	}
#endif
}

void UAccelByteCommonServerSubsystem::AddUserToSession(const FUniqueNetIdRepl& UniqueNetId)
{
	// This means that server is not claimed yet and need to start from beginning.
	if(SessionData.Session_id.IsEmpty())
	{
		TryConstructSession();
		return;
	}
	
	// Decode from Base64
	FString JsonString;
	FBase64::Decode(UniqueNetId.ToString(), JsonString);
	FAccelByteUniqueIdComposite AccelByteUniqueIdComposite;
	FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &AccelByteUniqueIdComposite);
	
	UE_LOG(LogAccelByteCommonServer, Log, TEXT("Registering player to session : %s"), *AccelByteUniqueIdComposite.Id);
	const EServerSessionType SessionType = GetServerSessionType();
	if(SessionType == EServerSessionType::CustomMatch)
	{
		GetServerApi()->ServerSessionBrowser.RegisterPlayer(
			SessionData.Session_id,
			AccelByteUniqueIdComposite.Id,
			false,
			THandler<FAccelByteModelsSessionBrowserAddPlayerResponse>::CreateUObject(this, &UAccelByteCommonServerSubsystem::OnAddPlayerFromSession),
			FErrorHandler::CreateUObject(this, &UAccelByteCommonServerSubsystem::OnAccelByteCommonServerError)
		);
	}
	else if(SessionType == EServerSessionType::Matchmaking)
	{
		// No need to register player. It automatically registered by matchmaking service
		UE_LOG(LogAccelByteCommonServer, Log, TEXT("Registering player to session canceled. Automatically registered by Matchmaking service"), *UniqueNetId.ToString());
	}
}

void UAccelByteCommonServerSubsystem::RemoveUserFromSession(const FUniqueNetIdRepl& UniqueNetId)
{
	// Decode from Base64
	FString JsonString;
	FBase64::Decode(UniqueNetId.ToString(), JsonString);
	FAccelByteUniqueIdComposite AccelByteUniqueIdComposite;
	FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &AccelByteUniqueIdComposite);
	
	UE_LOG(LogAccelByteCommonServer, Log, TEXT("Unregistering player to session : %s"), *AccelByteUniqueIdComposite.Id);
	
	const EServerSessionType SessionType = GetServerSessionType();
	if(SessionType == EServerSessionType::CustomMatch)
	{
		GetServerApi()->ServerSessionBrowser.UnregisterPlayer(
			SessionData.Session_id,
			AccelByteUniqueIdComposite.Id,
			THandler<FAccelByteModelsSessionBrowserAddPlayerResponse>::CreateUObject(this, &UAccelByteCommonServerSubsystem::OnRemovePlayerFromSession),
			FErrorHandler::CreateUObject(this, &UAccelByteCommonServerSubsystem::OnAccelByteCommonServerError)
		);
	}
	else if(SessionType == EServerSessionType::Matchmaking)
	{
		TArray<FString> Channel;
		SessionData.Match.Channel.ParseIntoArray(Channel, TEXT(":"), true);
		GetServerApi()->ServerMatchmaking.RemoveUserFromSession(
			Channel.Last(),
			SessionData.Match.Match_id,
			AccelByteUniqueIdComposite.Id,
			FVoidHandler::CreateUObject(this, &UAccelByteCommonServerSubsystem::TryConstructSession), // reconstruct again
			FErrorHandler::CreateUObject(this, &UAccelByteCommonServerSubsystem::OnAccelByteCommonServerError)
		);
	}
}

void UAccelByteCommonServerSubsystem::ServerLogin()
{
#if UE_SERVER
	GetServerApi()->ServerOauth2.LoginWithClientCredentials(
		FVoidHandler::CreateUObject(this, &UAccelByteCommonServerSubsystem::OnServerLoginSuccess),
		FErrorHandler::CreateUObject(this, &UAccelByteCommonServerSubsystem::OnServerLoginFailed)
	);
#endif
}

void UAccelByteCommonServerSubsystem::RegisterServerToDSM()
{
#if UE_SERVER
	if(FString(FCommandLine::Get()).Contains(TEXT("-localds")))
	{
		// TODO: Not complete yet to make local ds
		GetServerApi()->ServerDSM.RegisterLocalServerToDSM(TEXT("127.0.0.1"), 7777, TEXT("localds"), 
			FVoidHandler::CreateUObject(this, &UAccelByteCommonServerSubsystem::OnServerRegisterToDSMSuccess),
			FErrorHandler::CreateUObject(this, &UAccelByteCommonServerSubsystem::OnServerRegisterToDSMFailed));
		return;
	}
	
	GetServerApi()->ServerDSM.RegisterServerToDSM(7777,
		FVoidHandler::CreateUObject(this, &UAccelByteCommonServerSubsystem::OnServerRegisterToDSMSuccess),
		FErrorHandler::CreateUObject(this, &UAccelByteCommonServerSubsystem::OnServerRegisterToDSMFailed)
	);
#endif
}

void UAccelByteCommonServerSubsystem::GetSessionIdDSM()
{
#if UE_SERVER
	GetServerApi()->ServerDSM.GetSessionId(
		THandler<FAccelByteModelsServerSessionResponse>::CreateUObject(this, &UAccelByteCommonServerSubsystem::OnGetSessionIdSuccess),
		FErrorHandler::CreateUObject(this, &UAccelByteCommonServerSubsystem::OnGetSessionIdFailed)
	);
#endif
}

void UAccelByteCommonServerSubsystem::EnqueueJoinable()
{
	GetServerApi()->ServerMatchmaking.EnqueueJoinableSession(SessionData.Match,
		FVoidHandler::CreateUObject(this, &UAccelByteCommonServerSubsystem::OnEnqueueJoinableSuccess),
		FErrorHandler::CreateUObject(this, &UAccelByteCommonServerSubsystem::OnEnqueueJoinableFailed)
	);
}

void UAccelByteCommonServerSubsystem::DequeueJoinable()
{
	GetServerApi()->ServerMatchmaking.DequeueJoinableSession(SessionData.Match.Match_id,
		FVoidHandler(),
		FErrorHandler::CreateUObject(this, &UAccelByteCommonServerSubsystem::OnQuerySessionStatusFailed)
	);
}

AccelByte::FServerApiClientPtr UAccelByteCommonServerSubsystem::GetServerApi()
{
	return FMultiRegistry::GetServerApiClient();
}

EServerSessionType UAccelByteCommonServerSubsystem::GetServerSessionType() const
{
	if(SessionData.Session_id.IsEmpty())
	{
		UE_LOG(LogAccelByteCommonServer, Log, TEXT("sessionId is empty!"));
		return EServerSessionType::UNKNOWN;
	}
	// NOTE(damar): Not sure if this true or not. SessionId without dash (-) char is session matchmaking
	// if with dash (-) is custom match.
	else if(SessionData.Session_id.Contains(TEXT("-")))
	{
		UE_LOG(LogAccelByteCommonServer, Log, TEXT("Found dashes on the session ID, means this is custom match"));
		return EServerSessionType::CustomMatch;
	}
	else
	{
		UE_LOG(LogAccelByteCommonServer, Log, TEXT("dashes not found on the session ID, means this is matchmaking session"));
		return EServerSessionType::Matchmaking;
	}
}

void UAccelByteCommonServerSubsystem::OnServerLoginSuccess()
{
	UE_LOG(LogAccelByteCommonServer, Log, TEXT("Server Login Success!"));
	
	if(OnServerLoginCompleteDelegate.IsBound())
	{
		OnServerLoginCompleteDelegate.Broadcast(true, 0, FText());
	}
	if(ServerState == EServerState::ServerLogin)
	{
		ServerState = EServerState::RegisterToDSM;
		RegisterServerToDSM();
	}
}

void UAccelByteCommonServerSubsystem::OnServerLoginFailed(int32 ErrCode, FString const& ErrStr)
{
	UE_LOG(LogAccelByteCommonServer, Error, TEXT("Server Login Failed!"));

	if(ServerState == EServerState::ServerLogin)
	{
		ServerState = EServerState::Failed;
	}
	
	if(OnServerLoginCompleteDelegate.IsBound())
	{
		OnServerLoginCompleteDelegate.Broadcast(false, ErrCode, FText::FromString(ErrStr));
	}
	OnAccelByteCommonServerError(ErrCode, ErrStr);
}

void UAccelByteCommonServerSubsystem::OnServerRegisterToDSMSuccess()
{
	UE_LOG(LogAccelByteCommonServer, Log, TEXT("Server Register to DSM Success!"));
	
	if(ServerState == EServerState::RegisterToDSM)
	{
		ServerState = EServerState::GetSessionId;
		GetSessionIdDSM();
	}
	if(OnServerRegisterToDSMCompleteDelegate.IsBound())
	{
		OnServerRegisterToDSMCompleteDelegate.Broadcast(true, 0, FText());
	}
	
	GetServerApi()->ServerDSM.ConfigureAutoShutdown(5, 120);
}

void UAccelByteCommonServerSubsystem::OnServerRegisterToDSMFailed(int32 ErrCode, FString const& ErrStr)
{
	UE_LOG(LogAccelByteCommonServer, Error, TEXT("Server Register To DSM Failed!"));
	
	if(OnServerRegisterToDSMCompleteDelegate.IsBound())
	{
		OnServerRegisterToDSMCompleteDelegate.Broadcast(false, ErrCode, FText::FromString(ErrStr));
	}
	
	GetServerApi()->ServerDSM.ConfigureAutoShutdown(5, 15);
	OnAccelByteCommonServerError(ErrCode, ErrStr);
}

void UAccelByteCommonServerSubsystem::OnGetSessionIdSuccess(const FAccelByteModelsServerSessionResponse& Response)
{
	UE_LOG(LogAccelByteCommonServer, Log, TEXT("Server GetSessionId Success!"));

	SessionData.Session_id = Response.Session_id;
	if(SessionData.Session_id.IsEmpty())
	{
		ServerState = EServerState::NotClaimed;
		return;
	}
	
	if(ServerState <= EServerState::NotClaimed)
	{
		ServerState = EServerState::GetSessionStatus;
		TryConstructSession();
	}

	// Set Armada information to crash via BlackBox
	IAccelByteBlackBoxSDKModuleInterface::Get().UpdateAdditionalInfo(TEXT("ARMADA_SESSION_ID"), Response.Session_id);
}

void UAccelByteCommonServerSubsystem::OnGetSessionIdFailed(int32 ErrCode, FString const& ErrStr)
{
	if(OnServerRegisterToDSMCompleteDelegate.IsBound())
	{
		OnServerRegisterToDSMCompleteDelegate.Broadcast(false, ErrCode, FText::FromString(ErrStr));
	}
	OnAccelByteCommonServerError(ErrCode, ErrStr);
}

void UAccelByteCommonServerSubsystem::OnGetSessionBySessionIdSuccess(const FAccelByteModelsSessionBrowserData& Response)
{
	UE_LOG(LogAccelByteCommonServer, Log, TEXT("Server OnGetSessionBySessionIdSuccess Success!"));
	if(ServerState == EServerState::GetSessionStatus)
	{
		SessionData = Response;
		SimpleSessionInfo = {};

		// this is matchmaking session
		if(!Response.Match.Match_id.IsEmpty())
		{
			OnQuerySessionStatusSuccess(Response.Match);
			return;
		}
		
		SimpleSessionInfo.SessionMode = EServerSessionType::CustomMatch;
		SimpleSessionInfo.SessionId = Response.Session_id;
		SimpleSessionInfo.SessionType = Response.Session_type;
		SimpleSessionInfo.AllPlayers = Response.All_players;
		SimpleSessionInfo.Players = Response.Players;
		SimpleSessionInfo.Spectators = Response.Spectators;
		SimpleSessionInfo.NumBots = Response.Game_session_setting.Num_bot;
		SimpleSessionInfo.SessionSetting.AllowJoinInProgress = Response.Joinable;
		SimpleSessionInfo.SessionSetting.MapName = Response.Game_session_setting.Map_name;
		SimpleSessionInfo.SessionSetting.Password = Response.Game_session_setting.Password;
		
		UE_LOG(LogAccelByteCommonServer, Warning, TEXT("This is Custom Match, no need to enqueue Joinable!"))
		ServerState = EServerState::Completed;

		OnSessionInfoReceivedDelegate.Broadcast(SimpleSessionInfo);
		return;
	}
	UE_LOG(LogAccelByteCommonServer, Warning, TEXT("ServerState already Enqueue. Skipping!"))
}

void UAccelByteCommonServerSubsystem::OnQuerySessionStatusSuccess(const FAccelByteModelsMatchmakingResult& Response)
{
	UE_LOG(LogAccelByteCommonServer, Log, TEXT("Server QuerySessionStatus Success!"));
	
	IAccelByteBlackBoxSDKModuleInterface::Get().UpdateAdditionalInfo(TEXT("ARMADA_MATCH_ID"), Response.Match_id);
	IAccelByteBlackBoxSDKModuleInterface::Get().UpdateAdditionalInfo(TEXT("ARMADA_CHANNEL"), Response.Channel);
	IAccelByteBlackBoxSDKModuleInterface::Get().UpdateAdditionalInfo(TEXT("ARMADA_GAMEMODE"), Response.Game_mode);
	IAccelByteBlackBoxSDKModuleInterface::Get().UpdateAdditionalInfo(TEXT("ARMADA_REGION"), Response.Region);
	IAccelByteBlackBoxSDKModuleInterface::Get().UpdateAdditionalInfo(TEXT("ARMADA_SERVER_NAME"), Response.Server_name);

	SessionData.Match = Response;
	// this is matchmaking session
	if(!Response.Match_id.IsEmpty())
	{
		ServerState = EServerState::EnqueueJoinable;
		
		SimpleSessionInfo = {};
		SimpleSessionInfo.SessionMode = EServerSessionType::Matchmaking;
		SimpleSessionInfo.SessionId = Response.Match_id;
		SimpleSessionInfo.SessionType = TEXT("dedicated");
		SimpleSessionInfo.SessionSetting.AllowJoinInProgress = Response.Joinable;
		if(Response.Party_attributes.JsonObject->HasField(SETTING_MAPNAME.ToString()))
		{
			SimpleSessionInfo.SessionSetting.MapName = Response.Party_attributes.JsonObject->GetStringField(SETTING_MAPNAME.ToString());
		}
		if(Response.Party_attributes.JsonObject->HasField(SETTING_NUMBOTS.ToString()))
		{
			SimpleSessionInfo.NumBots = FCString::Atoi(*Response.Party_attributes.JsonObject->GetStringField(SETTING_NUMBOTS.ToString()));
		}
			
		EnqueueJoinable();
	}

	OnSessionInfoReceivedDelegate.Broadcast(SimpleSessionInfo);
}

void UAccelByteCommonServerSubsystem::OnQuerySessionStatusFailed(int32 ErrCode, FString const& ErrStr)
{
	OnAccelByteCommonServerError(ErrCode, ErrStr);
}

void UAccelByteCommonServerSubsystem::OnEnqueueJoinableSuccess()
{
	UE_LOG(LogAccelByteCommonServer, Log, TEXT("Server EnqueueJoinable Success!"));
	ServerState = EServerState::Completed;
}

void UAccelByteCommonServerSubsystem::OnEnqueueJoinableFailed(int32 ErrCode, FString const& ErrStr)
{
	ServerState = EServerState::Failed;
	OnAccelByteCommonServerError(ErrCode, ErrStr);
}

void UAccelByteCommonServerSubsystem::OnAddPlayerFromSession(
	FAccelByteModelsSessionBrowserAddPlayerResponse const& Response)
{
	if(SessionData.Players.Num() >= SessionData.Game_session_setting.Max_player && !SessionData.Match.Match_id.IsEmpty())
	{
		DequeueJoinable();
	}
}

void UAccelByteCommonServerSubsystem::OnRemovePlayerFromSession(
	FAccelByteModelsSessionBrowserAddPlayerResponse const& Response)
{
	if(SessionData.Players.Num() < SessionData.Game_session_setting.Max_player && !SessionData.Match.Match_id.IsEmpty())
	{
		EnqueueJoinable();
	}
}

void UAccelByteCommonServerSubsystem::OnAccelByteCommonServerError(int32 ErrCode, FString const& ErrStr)
{
	UE_LOG(LogAccelByteCommonServer, Error, TEXT("Code: %d. %s"), ErrCode, *ErrStr);
}
