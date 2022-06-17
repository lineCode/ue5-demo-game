// Copyright (c) 2018 AccelByte, inc. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AccelByteCommonFriendSubsystem.h"
#include "OnlinePartyInterfaceAccelByte.h"
#include "SocialToolkit.h"
#include "Messaging/CommonMessagingSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AccelByteCommonPartySubsystem.generated.h"

#pragma region Blueprintable structs and enums

USTRUCT(BlueprintType)
struct FABPartySubsystemPartyMember
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FABFriendSubsystemOnlineUser UserInfo;

	UPROPERTY(BlueprintReadOnly)
	bool bIsLeader = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsLocalUser = false;

	const FOnlinePartyId* PartyId;

	FABPartySubsystemPartyMember(): PartyId(nullptr)
	{
	}

	FABPartySubsystemPartyMember(
		const FUniqueNetIdRepl UserId,
		const FString& DisplayName,
		bool bIsLeader) :
		UserInfo(UserId, DisplayName),
		bIsLeader(bIsLeader),
		PartyId(nullptr)
	{
	}

	FABPartySubsystemPartyMember(
		const FUniqueNetIdRepl UserId,
		const FString& DisplayName,
		const FOnlinePartyId* PartyId) :
	UserInfo(UserId, DisplayName),
	PartyId(PartyId)
	{
	}
};

UENUM(BlueprintType)
enum class EPartyStatus : uint8
{
	NoParty,
	PartyDataLoading,
	PartyValid
};

#pragma endregion

DECLARE_LOG_CATEGORY_CLASS(LogAccelByteCommonParty, Log, All);

DECLARE_DYNAMIC_DELEGATE_OneParam(FPartyInviteReceived, FABPartySubsystemPartyMember, Inviter);
DECLARE_DYNAMIC_DELEGATE(FPartyVoidDelegate);

/**
 * 
 */
UCLASS()
class ACCELBYTECOMMONGAME_API UAccelByteCommonPartySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

public:

	/**
	 * Get current party member list
	 *
	 * @param LocalPlayerIndex Local player index
	 * @param ABPartyMembers Party member list output
	 * @param PartyStatus Outputs NoParty if local player does not have party, Party Data Loading if async task not yet finished, PartyValid if everything is okay
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party", meta = (ExpandEnumAsExecs = "PartyStatus"))
	void GetPartyMember(
		int32 LocalPlayerIndex,
		TArray<FABPartySubsystemPartyMember>& ABPartyMembers,
		EPartyStatus& PartyStatus);

	/**
	 * Get Party Leader Unique Id
	 *
	 * @param bIsPartyExist Outputs true if party exist, false otherwise
	 * @param LocalPlayerIndex Local player index
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party", meta = (ExpandBoolAsExecs = "bIsPartyExist"))
	FUniqueNetIdRepl GetPartyLeaderIdIfPartyExist(bool& bIsPartyExist, int32 LocalPlayerIndex = 0);

	/**
	 * Invite other user to party
	 *
	 * @param TargetUniqueId Target Unique Id to be invited
	 * @param bIsPartyExist Outputs true if party exist, false otherwise
	 * @param LocalPlayerIndex Local player index
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party", meta = (ExpandBoolAsExecs = "bIsPartyExist"))
	void InviteToPartyIfPartyExist(FUniqueNetIdRepl TargetUniqueId, bool& bIsPartyExist, const int32 LocalPlayerIndex = 0);

	/**
	 * Kick user from party
	 *
	 * @param TargetUniqueId Target Unique Id to be kicked
	 * @param bIsPartyExist Outputs true if party exist, false otherwise
	 * @param LocalPlayerIndex Local player index
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party", meta = (ExpandBoolAsExecs = "bIsPartyExist"))
	void KickFromPartyIfPartyExist(FUniqueNetIdRepl TargetUniqueId, bool& bIsPartyExist, const int32 LocalPlayerIndex = 0);

	/**
	 * Promote user as Party Leader
	 *
	 * @param TargetUniqueId Target Unique Id to be promoted
	 * @param bIsPartyExist Outputs true if party exist, false otherwise
	 * @param LocalPlayerIndex Local player index
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party", meta = (ExpandBoolAsExecs = "bIsPartyExist"))
	void PromoteAsLeaderIfPartyExist(FUniqueNetIdRepl TargetUniqueId, bool& bIsPartyExist, const int32 LocalPlayerIndex = 0);

	/**
	 * Leave current party
	 *
	 * @param bWasInParty Outputs true if user was in party, false otherwise
	 * @param LocalPlayerIndex Local player index
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party", meta = (ExpandBoolAsExecs = "bWasInParty"))
	void LeavePartyIfInParty(bool& bWasInParty, int32 LocalPlayerIndex = 0);

	/**
	 * Leave current party
	 *
	 * @param bWasNotInParty Outputs true if user was NOT in party, false otherwise
	 * @param LocalPlayerIndex Local player index
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party", meta = (ExpandBoolAsExecs = "bWasNotInParty"))
	void CreatePartyIfNotExist(bool& bWasNotInParty, int32 LocalPlayerIndex = 0);

	/**
	 * Set delegate that will be called upon receiving party invitation
	 *
	 * @param OnInvitationReceived Delegate that will be executed
	 * @param LocalPlayerIndex Local player index
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party")
	void SetOnPartyInviteRequestReceivedDelegate(int32 LocalPlayerIndex = 0);

	/**
	 * Set delegate that will be called upon multiple cases
	 *
	 * @param OnChange Delegate that will be executed
	 * @param LocalPlayerIndex Local player index
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party")
	void SetOnPartyDataChangeDelegate(FPartyVoidDelegate OnChange, int32 LocalPlayerIndex = 0);

	/**
	 * Accept party invitation
	 *
	 * @param SenderUniqueId Party invitation sender Unique Id
	 * @param LocalPlayerIndex Local player index
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party")
	void AcceptPartyInvite(FUniqueNetIdRepl SenderUniqueId, int32 LocalPlayerIndex = 0);

	/**
	 * Reject party invitation
	 *
	 * @param SenderUniqueId Party invitation sender Unique Id
	 * @param LocalPlayerIndex Local player index
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party")
	void RejectPartyInvite(FUniqueNetIdRepl SenderUniqueId, int32 LocalPlayerIndex = 0);

	/**
	 * Get whether local player is current party leader or not
	 *
	 * @param LocalPlayerIndex Local player index
	 *
	 * @return whether local player is current party leader or not
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party")
	bool IsLocalUserLeader(int32 LocalPlayerIndex = 0);

	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party", meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool ShouldAutoCreateParty();

protected:
	void CreateParty(int32 LocalPlayerIndex, TDelegate<void()> OnComplete = TDelegate<void()>());

	void ShowReceivedInvitePopup(UObject* WorldContextObject, FABPartySubsystemPartyMember Sender, int32 LocalPlayerIndex);

	static FABPartySubsystemPartyMember BlueprintablePartyMember(const FOnlinePartyMemberConstRef PartyMember);

	static TArray<FABPartySubsystemPartyMember> BlueprintablePartyMembers(
		const TArray<FOnlinePartyMemberConstRef>& PartyMembers,
		FUniqueNetIdRef LocalUserId,
		FUniqueNetIdRef LeaderUserId);

	static void AcceptInviteRequest(
		IOnlinePartyPtr PartyPtr,
		FUniqueNetIdPtr LocalUserUniqueId,
		FUniqueNetIdPtr SenderUniqueId);

	const FOnlinePartyTypeId PartyTypeId = FOnlinePartySystemAccelByte::GetAccelBytePartyTypeId();

	IOnlineSubsystem* OSS;
};
