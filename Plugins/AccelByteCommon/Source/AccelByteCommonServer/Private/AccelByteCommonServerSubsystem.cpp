// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "AccelByteCommonServerSubsystem.h"

#include "Core/AccelByteMultiRegistry.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAccelByteCommonServer, Log, All);
DEFINE_LOG_CATEGORY(LogAccelByteCommonServer);

UAccelByteCommonServerSubsystem* UAccelByteCommonServerTask::GetSubsystem() const
{
	return Cast<UAccelByteCommonServerSubsystem>(GetOuter());
}

void UAccelByteCommonServerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
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

void UAccelByteCommonServerSubsystem::StartServerInitialization()
{
#if UE_SERVER
	ServerState = EServerState::ServerLogin;
	if(!IsServerLoggedIn())
	{
		ServerLogin();
	}
	else
	{
		RegisterServerToDSM();
	}
#endif
}

void UAccelByteCommonServerSubsystem::ContinueServerInitialization()
{
#if UE_SERVER
	switch(ServerState)
	{
		case(EServerState::NotStarted):
			StartServerInitialization();
			break;
		case(EServerState::GetSessionId):
		case(EServerState::NotClaimed):
			GetSessionIdDSM();
			break;
		default:
		break;
	}
#endif
}

void UAccelByteCommonServerSubsystem::ServerLogin()
{
#if UE_SERVER
	if(IsServerLoggedIn())
	{
		return;
	}

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
		// TODO : support for local DS (not in our milestone)
		//GetServerApi()->ServerDSM.RegisterLocalServerToDSM()
		//return;
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

void UAccelByteCommonServerSubsystem::SendShutdownToDSM()
{
#if UE_SERVER
	GetServerApi()->ServerDSM.SendShutdownToDSM(
		true,
		SessionId,
		FVoidHandler::CreateLambda([](){}),
		FErrorHandler::CreateUObject(this, &UAccelByteCommonServerSubsystem::OnAccelByteCommonServerError));
#endif
}

AccelByte::FServerApiClientPtr UAccelByteCommonServerSubsystem::GetServerApi()
{
	return FMultiRegistry::GetServerApiClient();
}

void UAccelByteCommonServerSubsystem::OnServerLoginSuccess()
{
	UE_LOG(LogAccelByteCommonServer, Log, TEXT("Server Login Success!"));
	
	bIsLoggedIn = true;
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
	bIsLoggedIn = false;
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

	SessionId = Response.Session_id;
	if(SessionId.IsEmpty())
	{
		ServerState = EServerState::NotClaimed;
		return;
	}

	if(ServerState <= EServerState::NotClaimed)
	{
		ServerState = EServerState::GetSessionStatus;
		GetServerApi()->ServerMatchmaking.QuerySessionStatus(SessionId,
			THandler<FAccelByteModelsMatchmakingResult>::CreateUObject(this, &UAccelByteCommonServerSubsystem::OnQuerySessionStatusSuccess),
			FErrorHandler::CreateUObject(this, &UAccelByteCommonServerSubsystem::OnQuerySessionStatusFailed)
		);
	}
}

void UAccelByteCommonServerSubsystem::OnGetSessionIdFailed(int32 ErrCode, FString const& ErrStr)
{
	if(OnServerRegisterToDSMCompleteDelegate.IsBound())
	{
		OnServerRegisterToDSMCompleteDelegate.Broadcast(false, ErrCode, FText::FromString(ErrStr));
	}
	OnAccelByteCommonServerError(ErrCode, ErrStr);
}

void UAccelByteCommonServerSubsystem::OnQuerySessionStatusSuccess(const FAccelByteModelsMatchmakingResult& Response)
{
	UE_LOG(LogAccelByteCommonServer, Log, TEXT("Server QuerySessionStatus Success!"));
	if(ServerState == EServerState::GetSessionStatus)
	{
		ServerState = EServerState::EnqueueJoinable;
		GetServerApi()->ServerMatchmaking.EnqueueJoinableSession(Response,
			FVoidHandler::CreateUObject(this, &UAccelByteCommonServerSubsystem::OnEnqueueJoinableSuccess),
			FErrorHandler::CreateUObject(this, &UAccelByteCommonServerSubsystem::OnEnqueueJoinableFailed)
		);
		return;
	}
	UE_LOG(LogAccelByteCommonServer, Warning, TEXT("ServerState already Enqueue. Skipping!"))
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

void UAccelByteCommonServerSubsystem::OnAccelByteCommonServerError(int32 ErrCode, FString const& ErrStr)
{
	UE_LOG(LogAccelByteCommonServer, Error, TEXT("Code: %d. %s"), ErrCode, *ErrStr);
}
