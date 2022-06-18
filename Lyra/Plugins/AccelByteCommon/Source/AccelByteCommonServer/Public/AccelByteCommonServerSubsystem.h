// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AccelByteServerApiClient.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AccelByteCommonServerSubsystem.generated.h"

UENUM()
enum class EServerState : uint8
{
	Failed = 0,
	NotStarted,
	ServerLogin,
	RegisterToDSM,
	GetSessionId,		//check if claimed or not
	NotClaimed,		    //ds still idle (ready)
	GetSessionStatus,	// ds is claimed
	EnqueueJoinable,
	Completed
};

UENUM(BlueprintType)
enum class EServerSessionType : uint8
{
	UNKNOWN	= 0,
	Matchmaking,
	CustomMatch
};

USTRUCT(BlueprintType)
struct ACCELBYTECOMMONSERVER_API FSessionSetting
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AccelByte | Server Subsystem")
	FString Mode{};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AccelByte | Server Subsystem")
	FString MapName{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AccelByte | Server Subsystem")
	bool AllowJoinInProgress{};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AccelByte | Server Subsystem")
	FString Password{};
};

USTRUCT(BlueprintType)
struct ACCELBYTECOMMONSERVER_API FDedicatedServerInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AccelByte | Server Subsystem")
	EServerSessionType SessionMode{EServerSessionType::UNKNOWN}; // matchmaking or sessionbrowser

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AccelByte | Server Subsystem")
	FString SessionId{};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AccelByte | Server Subsystem")
	FString SessionType{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AccelByte | Server Subsystem")
	FSessionSetting SessionSetting{};

	/** @brief all players including player that leave the session. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AccelByte | Server Subsystem")
	TArray<FString> AllPlayers{};

	/** @brief Current active players */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AccelByte | Server Subsystem")
	TArray<FString> Players{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AccelByte | Server Subsystem")
	TArray<FString> Spectators{};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AccelByte | Server Subsystem")
	int32 NumBots{0};
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAccelByteSessionInfoReceivedDelegate, const FDedicatedServerInfo&, Response);

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
	virtual bool StartServerInitialization();
	
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

	UFUNCTION(BlueprintCallable, Category="AB Common Server Subsystem")
	FString GetProjectVersion() const { return ProjectVersion; }
	UFUNCTION(BlueprintCallable, Category="AB Common Server Subsystem")
	FString GetGitHash() const { return GitHash; }

	FAccelByteCommonServerGenericDelegate OnServerLoginCompleteDelegate;
	FAccelByteCommonServerGenericDelegate OnServerRegisterToDSMCompleteDelegate;
	FAccelByteSessionInfoReceivedDelegate OnSessionInfoReceivedDelegate;
	
protected:
	virtual void ServerLogin();
	virtual void RegisterServerToDSM();
	virtual void GetSessionIdDSM();
	virtual void EnqueueJoinable();
	virtual void DequeueJoinable();
	virtual void DestructSession();
	
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
	FDedicatedServerInfo SimpleSessionInfo;
	FAccelByteModelsSessionBrowserData SessionData;
	EServerState ServerState { EServerState::NotStarted };

	// temporary put the version in this class, might be better create a new class
	FString ProjectVersion;
	FString GitHash;

	FTimerHandle DestructDSHandle;
};
