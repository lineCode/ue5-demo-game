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

UCLASS(BlueprintType)
class ACCELBYTECOMMONSERVER_API UAccelByteCommonServerTask : public UObject
{
	GENERATED_BODY()
public:

	class UAccelByteCommonServerSubsystem* GetSubsystem() const;

private:
	EServerState ServerState {EServerState::NotStarted};
};

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
	
	UFUNCTION(BlueprintCallable, Category= "AB Common Server Subsystem")
	virtual void ContinueServerInitialization();
	
	UFUNCTION(BlueprintCallable, Category= "AB Common Server Subsystem")
	virtual void ServerLogin();

	UFUNCTION(BlueprintCallable, Category= "AB Common Server Subsystem")
	virtual void RegisterServerToDSM();

	UFUNCTION(BlueprintCallable, Category= "AB Common Server Subsystem")
	virtual void GetSessionIdDSM();

	virtual void SendShutdownToDSM();
	
	UFUNCTION(BlueprintCallable, Category= "AB Common Server Subsystem")
	virtual bool IsServerLoggedIn() const { return bIsLoggedIn; }

	UFUNCTION(BlueprintCallable, Category= "AB Common Server Subsystem")
	virtual bool IsClaimed() const { return ServerState > EServerState::NotClaimed; }
	
	UFUNCTION(BlueprintCallable, Category= "AB Common Server Subsystem")
	virtual FString GetSessionId() const { return SessionId; };

	AccelByte::FServerApiClientPtr GetServerApi();

	FAccelByteCommonServerGenericDelegate OnServerLoginCompleteDelegate;
	FAccelByteCommonServerGenericDelegate OnServerRegisterToDSMCompleteDelegate;
	
protected:
	
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
	void OnQuerySessionStatusSuccess(const FAccelByteModelsMatchmakingResult& Response);
	UFUNCTION()
	void OnQuerySessionStatusFailed(int32 ErrCode, FString const& ErrStr);
	
	UFUNCTION()
	void OnEnqueueJoinableSuccess();
	UFUNCTION()
	void OnEnqueueJoinableFailed(int32 ErrCode, FString const& ErrStr);
	
	
	void OnAccelByteCommonServerError(int32 ErrCode, FString const& ErrStr);
private:
	bool bIsLoggedIn {false};
	bool bIsRegisteredToDSM {false};
	bool bIsJoinable {false};
	FString SessionId;
	EServerState ServerState { EServerState::NotStarted };
};
