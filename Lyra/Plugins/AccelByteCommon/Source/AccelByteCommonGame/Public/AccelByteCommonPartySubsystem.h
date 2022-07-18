// Copyright (c) 2018 AccelByte, inc. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AccelByteCommonFriendSubsystem.h"
#include "OnlinePartyInterfaceAccelByte.h"
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

UENUM(BlueprintType)
enum class EPartyMatchType : uint8
{
	QuickMatch = 0,
	CustomSession
};

UENUM(BlueprintType)
enum class ECustomSessionTeam : uint8
{
	NoTeam = 0,
	Team1 = 1,
	Team2 = 2,
	Queue = 3,
	Max = 4
};

#pragma endregion

DECLARE_LOG_CATEGORY_CLASS(LogAccelByteCommonParty, Log, All);

DECLARE_DYNAMIC_DELEGATE_OneParam(FPartyInviteReceived, FABPartySubsystemPartyMember, Inviter);
DECLARE_DYNAMIC_DELEGATE(FPartyVoidDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPartyMultiCastDelegate);

/**
 * 
 */
UCLASS()
class ACCELBYTECOMMONGAME_API UAccelByteCommonPartySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

public:
	inline static const FOnlinePartyTypeId PartyTypeId = FOnlinePartySystemAccelByte::GetAccelBytePartyTypeId();

#pragma region Project specific party attributes

	static inline FString PartyAttrValue_MatchType_QuickMatch = "QuickMatch";
	static inline FString PartyAttrValue_MatchType_CustomSession = "CustomSession";
	static inline FString PartyAttrName_CustomSession_Team1 = "Team1";
	static inline FString PartyAttrName_CustomSession_Team2 = "Team2";
	static inline FString PartyAttrName_CustomSession_Queue = "Queue";

	UPROPERTY(BlueprintReadOnly)
	TMap<EPartyMatchType, FString> PartyAttr_MatchType = {
		{EPartyMatchType::QuickMatch, PartyAttrValue_MatchType_QuickMatch},
		{EPartyMatchType::CustomSession, PartyAttrValue_MatchType_CustomSession}
	};

	UPROPERTY(BlueprintReadOnly)
	TMap<ECustomSessionTeam, FString> PartyAttr_CustomSession_Team = {
		{ECustomSessionTeam::Team1, PartyAttrName_CustomSession_Team1},
		{ECustomSessionTeam::Team2, PartyAttrName_CustomSession_Team2},
		{ECustomSessionTeam::Queue, PartyAttrName_CustomSession_Queue}
	};

	UPROPERTY(BlueprintReadOnly)
	FString PartyAttrName_MatchType = "MatchType";

	UPROPERTY(BlueprintReadOnly)
	FString PartyAttrName_CustomSession_Config_Bots = "BotsConfig";

	UPROPERTY(BlueprintReadOnly)
	FString PartyAttrName_CustomSession_Config_Network = "NetworkConfig";

	UPROPERTY(BlueprintReadOnly)
	FString PartyAttrName_CustomSession_Config_Map = "MapConfig";

	UPROPERTY(BlueprintReadOnly)
	FString PartyAttrName_CustomSession_AssignedTeam = "AssignedTeam";

	UPROPERTY(BlueprintReadOnly)
	FString PartyAttrName_CustomSession_AssignedBotsNum_Team1 = "Team1Bots";

	UPROPERTY(BlueprintReadOnly)
	FString PartyAttrName_CustomSession_Creating = "Creating";

	UPROPERTY(BlueprintReadOnly)
	FString PartyAttrName_CustomSession_Owner = "OwnerId";

#pragma endregion

	/**
	 * Get current party member list
	 *
	 * @param LocalPlayerIndex Local player index
	 * @param OutABPartyMembers Party member list output
	 * @param OutPartyStatus Outputs NoParty if local player does not have party, Party Data Loading if async task not yet finished, PartyValid if everything is okay
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party", meta = (ExpandEnumAsExecs = "OutPartyStatus"))
	void GetPartyMember(
		int32 LocalPlayerIndex,
		TArray<FABPartySubsystemPartyMember>& OutABPartyMembers,
		EPartyStatus& OutPartyStatus);

	/**
	 * Get party member object by Unique Net Id String
	 *
	 * @param AccelByteIdString Unique Net Id to be searched
	 * @param LocalPlayerIndex Local player index
	 *
	 * @return Party member object
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party")
	FABPartySubsystemPartyMember GetPartyMemberByAccelByteIdString(FString AccelByteIdString, int32 LocalPlayerIndex = 0);

	/**
	 * Get Local Player Unique Id String
	 *
	 * @param LocalPlayerIndex Local player index
	 *
	 * @return Local Player Unique Id String
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party")
	FString GetLocalPlayerAccelByteIdString(const int32 LocalPlayerIndex = 0) const;

	/**
	 * Get Party Leader Unique Id
	 *
	 * @param OutbIsPartyExist Outputs true if party exist, false otherwise
	 * @param LocalPlayerIndex Local player index
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party", meta = (ExpandBoolAsExecs = "OutbIsPartyExist"))
	FUniqueNetIdRepl GetPartyLeaderIdIfPartyExist(bool& OutbIsPartyExist, int32 LocalPlayerIndex = 0);

	/**
	 * Invite other user to party
	 *
	 * @param TargetUniqueId Target Unique Id to be invited
	 * @param OutbIsPartyExist Outputs true if party exist, false otherwise
	 * @param LocalPlayerIndex Local player index
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party", meta = (ExpandBoolAsExecs = "OutbIsPartyExist"))
	void InviteToPartyIfPartyExist(FUniqueNetIdRepl TargetUniqueId, bool& OutbIsPartyExist, const int32 LocalPlayerIndex = 0);

	/**
	 * Kick user from party
	 *
	 * @param TargetUniqueId Target Unique Id to be kicked
	 * @param OutbIsPartyExist Outputs true if party exist, false otherwise
	 * @param LocalPlayerIndex Local player index
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party", meta = (ExpandBoolAsExecs = "OutbIsPartyExist"))
	void KickFromPartyIfPartyExist(FUniqueNetIdRepl TargetUniqueId, bool& OutbIsPartyExist, const int32 LocalPlayerIndex = 0);

	/**
	 * Promote user as Party Leader
	 *
	 * @param TargetUniqueId Target Unique Id to be promoted
	 * @param OutbIsPartyExist Outputs true if party exist, false otherwise
	 * @param LocalPlayerIndex Local player index
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party", meta = (ExpandBoolAsExecs = "OutbIsPartyExist"))
	void PromoteAsLeaderIfPartyExist(FUniqueNetIdRepl TargetUniqueId, bool& OutbIsPartyExist, const int32 LocalPlayerIndex = 0);

	/**
	 * Leave current party. Automatically create party if bAutoCreateParty in DefaultEngine.ini is set to true
	 *
	 * @param OutbWasInParty Outputs true if user was in party, false otherwise
	 * @param OnComplete Executes on completion
	 * @param LocalPlayerIndex Local player index
	 * @param NewPartyMemberLimit Party member limit for the new party that will be created
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party", meta = (ExpandBoolAsExecs = "OutbWasInParty", AutoCreateRefTerm = "OnComplete"))
	void LeavePartyIfInParty(bool& OutbWasInParty, const FPartyVoidDelegate& OnComplete, int32 LocalPlayerIndex = 0, int32 NewPartyMemberLimit = 2);

	/**
	 * Create party for local user
	 *
	 * @param OutbWasNotInParty Outputs true if user was NOT in party, false otherwise
	 * @param LocalPlayerIndex Local player index
	 * @param NewPartyMemberLimit Party member limit for the new party that will be created
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party", meta = (ExpandBoolAsExecs = "OutbWasNotInParty"))
	void CreatePartyIfNotExist(bool& OutbWasNotInParty, int32 LocalPlayerIndex = 0, int32 NewPartyMemberLimit = 2);

	/**
	 * Set delegate that will be called upon local player left party.
	 * Automatically create party if bAutoCreateParty in DefaultEngine.ini is set to true
	 *
	 * @param LocalPlayerIndex Local player index
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party")
	void SetPartyNotifDelegates(int32 LocalPlayerIndex = 0);

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
	bool IsLocalUserLeader(int32 LocalPlayerIndex = 0) const;

	/**
	 * Return bAutoCreateParty value from DefaultEngine.ini
	 *
	 * @return value of bAutoCreateParty
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party", meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool ShouldAutoCreateParty();

	/**
	 * Return Configured Max Party Member. If 0, return value configured from DefaultEngine.ini
	 *
	 * @param LocalPlayerIndex Local player index
	 * @param PartyMatchType Party match type. Quick match or Custom Session
	 *
	 * @return Configured max party member
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party")
	int32 GetPartyMemberMax(const int32 LocalPlayerIndex, EPartyMatchType PartyMatchType = EPartyMatchType::QuickMatch);

	/**
	 * Return preset party member limit for each match type
	 *
	 * @param PartyMatchType Party match type. Quick match or Custom Session
	 *
	 * @return Preset party member limit
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party")
	int32 GetPartyMemberLimitPreset(EPartyMatchType PartyMatchType = EPartyMatchType::QuickMatch) const;

	/**
	 * Set party data from string. Will replace the attribute if already exist.
	 *
	 * @param LocalPlayerIndex Local Player Index
	 * @param PartyAttrName The name of the attribute that will be created / replaced
	 * @param PartyAttrValue The attribute value
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party")
	void SetPartyDataString(int32 LocalPlayerIndex, FString PartyAttrName, FString PartyAttrValue);

	/**
	 * Set party data by Map
	 *
	 * @param LocalPlayerIndex Local player index
	 * @param Datas The data to be set
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party")
	void SetPartyData(int32 LocalPlayerIndex, TMap<FString, FString> Datas) const;

	/**
	 * Set party data from array of string. Will replace or append the attribute if already exist
	 *
	 * @param LocalPlayerIndex Local Player Index
	 * @param PartyAttrName The name of the attribute that will be created / replaced
	 * @param PartyAttrValues The attribute value
	 * @param bAppend Should the attribute value be replaced or appended
	 * @param bUpdateImmediately Should immediately update the party data or not
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party")
	FString SetPartyDataArrayOfString(
		int32 LocalPlayerIndex, FString PartyAttrName, TArray<FString> PartyAttrValues, bool bAppend = true,
		const bool bUpdateImmediately = true);

	/**
	 * Remove array from party data array of string
	 *
	 * @param LocalPlayerIndex Local player index
	 * @param PartyAttrName Party attribute name that the value will be remove
	 * @param PartyAttrValue Value to be remove
	 * @param bUpdateImmediately Should immediately update the party data or not
	 *
	 * @return Removed value result in the form of comma seperated string
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party")
	FString RemoveStringFromPartyDataArrayOfString(
		int32 LocalPlayerIndex,
		FString PartyAttrName,
		FString PartyAttrValue,
		bool bUpdateImmediately = true);

#pragma region Project specific functions

	/**
	 * Get local player team from party attribute
	 *
	 * @param LocalPlayerIndex Local player index
	 *
	 * @return Local player team
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party")
	ECustomSessionTeam GetLocalPlayerTeam(int32 LocalPlayerIndex) const;

	/**
	 * Cycle local player from one team to another
	 *
	 * @param CycleNext The distance local player will cycle team. Set 1 to cycle to next, set -1 to cycle to previous.
	 * @param LocalPlayerIndex Local player index
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party")
	void CycleLocalPlayerTeam(bool CycleNext = true, int32 LocalPlayerIndex = 0);

	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party")
	int32 GetTeamMembersNum(ECustomSessionTeam Team, const int32 LocalPlayerIndex = 0) const;

#pragma endregion

	/**
	 * Get cached party data
	 *
	 * @param LocalPlayerIndex Local player index
	 * @param PartyAttrName Party data attribute name to be retrieved
	 *
	 * @return party attribute value in string
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party")
	FString GetCachedPartyDataString(int32 LocalPlayerIndex, FString PartyAttrName) const;

	/**
	 * Get cached party data
	 *
	 * @param LocalPlayerIndex Local player index
	 * @param PartyAttrName Party data attribute name to be retrieved
	 *
	 * @return party attribute value in array of strings
	 */
	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party")
	TArray<FString> GetCachedPartyDataArrayOfString(int32 LocalPlayerIndex, FString PartyAttrName) const;

	/**
	 * Delegate that will be called upon accept party invitation success
	 */
	UPROPERTY(BlueprintAssignable)
	FPartyMultiCastDelegate OnAcceptPartyInvitationDelegate;

	/**
	 * Delegate that will be called upon local player joined a party
	 */
	UPROPERTY(BlueprintAssignable)
	FPartyMultiCastDelegate OnPartyJoinedDelegate;

	/**
	 * Delegate that will be called upon party's member's info changed
	 */
	UPROPERTY(BlueprintAssignable)
	FPartyMultiCastDelegate OnPartyInfoChangedDelegate;

	/**
	 * Delegate that will be called upon party's data changed
	 */
	UPROPERTY(BlueprintAssignable)
	FPartyMultiCastDelegate OnPartyDataChangedDelegate;

protected:
	void CreateParty(int32 LocalPlayerIndex, TDelegate<void()> OnComplete = TDelegate<void()>(), int32 NewPartyMemberLimit = 2);

	void ShowReceivedInvitePopup(const UObject* WorldContextObject, FABPartySubsystemPartyMember Sender, int32 LocalPlayerIndex);

	static FABPartySubsystemPartyMember BlueprintablePartyMember(const FOnlinePartyMemberConstRef PartyMember);

	static TArray<FABPartySubsystemPartyMember> BlueprintablePartyMembers(
		const TArray<FOnlinePartyMemberConstRef>& PartyMembers,
		FUniqueNetIdRef LocalUserId,
		FUniqueNetIdRef LeaderUserId);

	static void AcceptInviteRequest(
		IOnlinePartyPtr PartyPtr,
		FUniqueNetIdPtr LocalUserUniqueId,
		FUniqueNetIdPtr SenderUniqueId);

	IOnlineSubsystem* OSS;

private:
	static FString SetPartyDataArrayOfString_Helper(TArray<FString> InArray);

	static int32 PositiveModulo(const int32 i, const int32 n);
};
