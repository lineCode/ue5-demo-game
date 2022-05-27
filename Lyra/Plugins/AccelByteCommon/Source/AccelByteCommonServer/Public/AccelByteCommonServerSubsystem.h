// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AccelByteServerApiClient.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AccelByteCommonServerSubsystem.generated.h"

UENUM()
enum EServerState
{
	Failed				= -1,
	NotStarted			= 0,
	ServerLogin			= 1,
	RegisterToDSM		= 2,
	GetSessionId		= 3,		//check if claimed or not
	NotClaimed			= 4,		//ds still idle (ready)
	GetSessionStatus	= 5,		// ds is claimed
	EnqueueJoinable		= 6,
	Completed			= 7
};

UENUM()
enum EServerSessionType
{
	UNKNOWN	= 0,
	Matchmaking,
	CustomMatch
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAccelByteSessionInfoReceivedDelegate, const FAccelByteModelsSessionBrowserData&, Response);

/** Generic Delegates when succeed or fail */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAccelByteCommonServerGenericDelegate, bool, bSuccess, int32, ErrCode, FText, Error);
/**
 * AccelByte service implementation of the Server Functionality
 */
UCLASS()
class ACCELBYTECOMMONSERVER_API UAccelByteCommonServerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UAccelByteCommonServerSubsystem() {};
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	UFUNCTION(BlueprintCallable, Category= "AB Common Server Subsystem")
	virtual void StartServerInitialization();
	
	/**
	 * @brief Trying to construct session.
	 * Flow : Getting Session Id, Getting Session Info. If Joinable, register joinable
	 */
	UFUNCTION(BlueprintCallable, Category= "AB Common Server Subsystem")
	virtual void TryConstructSession();
	/**
	 * @brief Trying to destruct session.
	 * Flow : Sending Shutdown to DSM, Dequeue From Joinable
	 */
	UFUNCTION(BlueprintCallable, Category= "AB Common Server Subsystem")
	virtual void TryDestructSession();

	virtual void AddUserToSession(const FUniqueNetIdRepl& UniqueNetId);

	virtual void RemoveUserFromSession(const FUniqueNetIdRepl& UniqueNetId);

	UFUNCTION(BlueprintCallable, Category= "AB Common Server Subsystem")
	virtual bool IsClaimed() const { return ServerState > EServerState::NotClaimed; }
	
	virtual FString GetSessionId() const { return SessionData.Session_id; };

	AccelByte::FServerApiClientPtr GetServerApi();
	
	EServerSessionType GetServerSessionType() const;

	FAccelByteCommonServerGenericDelegate OnServerLoginCompleteDelegate;
	FAccelByteCommonServerGenericDelegate OnServerRegisterToDSMCompleteDelegate;
	FAccelByteSessionInfoReceivedDelegate OnSessionInfoReceivedDelegate;
	
protected:
	virtual void ServerLogin();
	virtual void RegisterServerToDSM();
	virtual void GetSessionIdDSM();
	virtual void EnqueueJoinable();
	virtual void DequeueJoinable();
	
	UFUNCTION()
	void OnServerLoginSuccess();
	UFUNCTION()
	void OnServerLoginFailed(int32 ErrCode, FString const& ErrStr);

	UFUNCTION()
	void OnServerRegisterToDSMSuccess();
	UFUNCTION()
	void OnServerRegisterToDSMFailed(int32 ErrCode, FString const& ErrStr);
	
	UFUNCTION()
	void OnGetSessionIdSuccess(const FAccelByteModelsServerSessionResponse& Response);
	UFUNCTION()
	void OnGetSessionIdFailed(int32 ErrCode, FString const& ErrStr);
	
	UFUNCTION()
	void OnGetSessionBySessionIdSuccess(const FAccelByteModelsSessionBrowserData& Response);
	UFUNCTION()
	void OnQuerySessionStatusSuccess(const FAccelByteModelsMatchmakingResult& Response);
	UFUNCTION()
	void OnQuerySessionStatusFailed(int32 ErrCode, FString const& ErrStr);
	
	UFUNCTION()
	void OnEnqueueJoinableSuccess();
	UFUNCTION()
	void OnEnqueueJoinableFailed(int32 ErrCode, FString const& ErrStr);

	UFUNCTION()
	void OnAddPlayerFromSession(FAccelByteModelsSessionBrowserAddPlayerResponse const& Response);
	UFUNCTION()
	void OnRemovePlayerFromSession(FAccelByteModelsSessionBrowserAddPlayerResponse const& Response);
	
	UFUNCTION()
	void OnAccelByteCommonServerError(int32 ErrCode, FString const& ErrStr);
private:
	FAccelByteModelsSessionBrowserData SessionData;
	EServerState ServerState { EServerState::NotStarted };
};
