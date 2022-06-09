// Copyright (c) 2018 AccelByte, inc. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AccelByteCommonFriendSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SocialToolkit.h"
#include "Chaos/AABB.h"
#include "Chaos/AABB.h"
#include "Chaos/AABB.h"
#include "Chaos/AABB.h"
#include "Chaos/AABB.h"
#include "Chaos/AABB.h"
#include "Chaos/AABB.h"
#include "Chaos/AABB.h"
#include "Chaos/AABB.h"
#include "Chaos/AABB.h"
#include "AccelByteCommonPartySubsystem.generated.h"

#pragma region Blueprintable structs

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

#pragma endregion

DECLARE_DYNAMIC_DELEGATE_OneParam(FPartyInviteReceived, FABPartySubsystemPartyMember, Inviter);
DECLARE_DYNAMIC_DELEGATE(FPartyVoidDelegate);

/**
 * 
 */
UCLASS()
class ACCELBYTECOMMONGAME_API UAccelByteCommonPartySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	UAccelByteCommonPartySubsystem();

public:

	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party")
	TArray<FABPartySubsystemPartyMember> GetPartyMember(ULocalPlayer* LocalPlayer);

	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party")
	FABPartySubsystemPartyMember GetPartyLeader(ULocalPlayer* LocalPlayer);

	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party")
	void InviteToParty(ULocalPlayer* LocalPlayer, FUniqueNetIdRepl TargetUniqueId);

	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party")
	void KickFromParty(ULocalPlayer* LocalPlayer, FUniqueNetIdRepl TargetUniqueId);

	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party")
	void PromoteAsLeader(ULocalPlayer* LocalPlayer, FUniqueNetIdRepl TargetUniqueId);

	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party")
	void LeaveParty(ULocalPlayer* LocalPlayer);

	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party")
	void SetOnPartyInviteRequestReceivedDelegate(ULocalPlayer* LocalPlayer, FPartyInviteReceived OnInvitationReceived);

	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party")
	void AcceptPartyInvite(ULocalPlayer* LocalPlayer, FUniqueNetIdRepl SenderUniqueId);

	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party")
	void RejectPartyInvite(ULocalPlayer* LocalPlayer, FUniqueNetIdRepl SenderUniqueId);

	UFUNCTION(BlueprintCallable, Category = "AccelByte | Common | Party")
	void SetOnPartyDataChangeDelegate(ULocalPlayer* LocalPlayer, FPartyVoidDelegate OnChange);

protected:
	void CreatePartyIfNoPartyExist(const USocialToolkit* SocialToolkit, TDelegate<void()> OnComplete);

	static FABPartySubsystemPartyMember BlueprintablePartyMemberData(const UPartyMember* PartyMember, FUniqueNetIdRef);

	static FABPartySubsystemPartyMember BlueprintablePartyMemberData(const FOnlinePartyMemberConstRef PartyMember, FUniqueNetIdRef);

	void AcceptInviteRequest(IOnlinePartyPtr, FUniqueNetIdPtr, FUniqueNetIdPtr);
};
