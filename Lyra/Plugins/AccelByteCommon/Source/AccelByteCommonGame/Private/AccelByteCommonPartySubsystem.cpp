// Copyright (c) 2018 AccelByte, inc. All rights reserved.


#include "AccelByteCommonPartySubsystem.h"

#include "Online.h"
#include "OnlinePartyInterfaceAccelByte.h"
#include "SocialManager.h"
#include "Party/PartyMember.h"
#include "Party/SocialParty.h"

UAccelByteCommonPartySubsystem::UAccelByteCommonPartySubsystem()
{
}

TArray<FABPartySubsystemPartyMember> UAccelByteCommonPartySubsystem::GetPartyMember(ULocalPlayer* LocalPlayer)
{
	TArray<FABPartySubsystemPartyMember> ABPartyMembers;
	if (const USocialToolkit* SocialToolkit = USocialToolkit::GetToolkitForPlayer(LocalPlayer))
	{
		const IOnlinePartyPtr PartyInf =
			SocialToolkit->GetSocialOss(ESocialSubsystem::Primary)->GetPartyInterface();
		check(PartyInf);

		const FUniqueNetId& LocalUserId = *SocialToolkit->GetLocalUser().GetUserId(ESocialSubsystem::Primary).GetUniqueNetId();

		const FOnlinePartyConstPtr OnlineParty = PartyInf->GetParty(
			LocalUserId,
			FOnlinePartySystemAccelByte::FOnlinePartySystemAccelByte::GetAccelBytePartyTypeId());

		TArray<FOnlinePartyMemberConstRef> PartyMembers;
		PartyInf->GetPartyMembers(
			LocalUserId,
			OnlineParty->PartyId.Get(),
			PartyMembers);
		for (const FOnlinePartyMemberConstRef PartyMember : PartyMembers)
		{
			ABPartyMembers.Add(BlueprintablePartyMemberData(PartyMember, LocalUserId.AsShared()));
		}
	}
	return ABPartyMembers;
}

FABPartySubsystemPartyMember UAccelByteCommonPartySubsystem::GetPartyLeader(ULocalPlayer* LocalPlayer)
{
	FABPartySubsystemPartyMember PartyLeader;
	if (const USocialToolkit* SocialToolkit = USocialToolkit::GetToolkitForPlayer(LocalPlayer))
	{
		const USocialParty* Party = SocialToolkit->GetSocialManager().GetParty(FOnlinePartySystemAccelByte::GetAccelBytePartyTypeId());
		check(Party);

		PartyLeader = BlueprintablePartyMemberData(
			Party->GetPartyLeader<UPartyMember>(),
			SocialToolkit->GetLocalUser().GetUserId(ESocialSubsystem::Primary)->AsShared());
	}
	return PartyLeader;
}

void UAccelByteCommonPartySubsystem::InviteToParty(ULocalPlayer* LocalPlayer, FUniqueNetIdRepl TargetUniqueId)
{
	if (const USocialToolkit* SocialToolkit = USocialToolkit::GetToolkitForPlayer(LocalPlayer))
	{
		const USocialParty* Party = SocialToolkit->GetSocialManager().GetParty(FOnlinePartySystemAccelByte::GetAccelBytePartyTypeId());
		check(Party);

		const IOnlineSubsystem* OSS = SocialToolkit->GetSocialOss(ESocialSubsystem::Primary);
		check(OSS);

		const IOnlinePartyPtr PartyInf = OSS->GetPartyInterface();

		const FUniqueNetIdRepl LocalUserId = SocialToolkit->GetLocalUser().GetUserId(ESocialSubsystem::Primary);

		PartyInf->SendInvitation(*LocalUserId.GetUniqueNetId(), Party->GetPartyId(),*TargetUniqueId.GetUniqueNetId());
	}
}

void UAccelByteCommonPartySubsystem::KickFromParty(ULocalPlayer* LocalPlayer, FUniqueNetIdRepl TargetUniqueId)
{
	if (const USocialToolkit* SocialToolkit = USocialToolkit::GetToolkitForPlayer(LocalPlayer))
	{
		const USocialParty* Party = SocialToolkit->GetSocialManager().GetParty(FOnlinePartySystemAccelByte::GetAccelBytePartyTypeId());
		check(Party);

		const IOnlineSubsystem* OSS = SocialToolkit->GetSocialOss(ESocialSubsystem::Primary);
		check(OSS);

		const IOnlinePartyPtr PartyInf = OSS->GetPartyInterface();
		check(PartyInf);

		const FUniqueNetIdRepl LocalUserId = SocialToolkit->GetLocalUser().GetUserId(ESocialSubsystem::Primary);
		check(TargetUniqueId.GetUniqueNetId());

		PartyInf->KickMember(*LocalUserId.GetUniqueNetId(), Party->GetPartyId(),*TargetUniqueId.GetUniqueNetId());
	}
}

void UAccelByteCommonPartySubsystem::PromoteAsLeader(ULocalPlayer* LocalPlayer, FUniqueNetIdRepl TargetUniqueId)
{
	if (const USocialToolkit* SocialToolkit = USocialToolkit::GetToolkitForPlayer(LocalPlayer))
	{
		const USocialParty* Party = SocialToolkit->GetSocialManager().GetParty(FOnlinePartySystemAccelByte::GetAccelBytePartyTypeId());
		check(Party);

		const IOnlineSubsystem* OSS = SocialToolkit->GetSocialOss(ESocialSubsystem::Primary);
		check(OSS);

		const IOnlinePartyPtr PartyInf = OSS->GetPartyInterface();

		const FUniqueNetIdRepl LocalUserId = SocialToolkit->GetLocalUser().GetUserId(ESocialSubsystem::Primary);

		PartyInf->PromoteMember(*LocalUserId.GetUniqueNetId(), Party->GetPartyId(),*TargetUniqueId.GetUniqueNetId());
	}
}

void UAccelByteCommonPartySubsystem::LeaveParty(ULocalPlayer* LocalPlayer)
{
	if (const USocialToolkit* SocialToolkit = USocialToolkit::GetToolkitForPlayer(LocalPlayer))
	{
		USocialParty* Party = SocialToolkit->GetSocialManager().GetParty(FOnlinePartySystemAccelByte::GetAccelBytePartyTypeId());

		Party->LeaveParty();
	}
}

void UAccelByteCommonPartySubsystem::SetOnPartyInviteRequestReceivedDelegate(ULocalPlayer* LocalPlayer, FPartyInviteReceived OnInvitationReceived)
{
	if (const USocialToolkit* SocialToolkit = USocialToolkit::GetToolkitForPlayer(LocalPlayer))
	{
		const IOnlineSubsystem* OSS = SocialToolkit->GetSocialOss(ESocialSubsystem::Primary);
		check(OSS);

		const IOnlinePartyPtr PartyInf = OSS->GetPartyInterface();

		const FUniqueNetIdRef LocalUserId = SocialToolkit->GetLocalUser().GetUserId(ESocialSubsystem::Primary)->AsShared();

		CreatePartyIfNoPartyExist(SocialToolkit, TDelegate<void()>::CreateWeakLambda(this, [SocialToolkit, PartyInf, OnInvitationReceived, this]()
		{
			const USocialParty* Party = SocialToolkit->GetSocialManager().GetParty(
				FOnlinePartySystemAccelByte::GetAccelBytePartyTypeId());
			check(Party);

			PartyInf->OnPartyInviteReceivedDelegates.AddWeakLambda(this,
				[SocialToolkit, OnInvitationReceived](
					const FUniqueNetId& LocalUserId,
					const FOnlinePartyId& PartyId,
					const FUniqueNetId& SenderId)
				{
					FABPartySubsystemPartyMember Sender(
						FUniqueNetIdRepl(SenderId),
						SocialToolkit->FindUser(SenderId.AsShared())->GetDisplayName(),
						&PartyId);
					OnInvitationReceived.ExecuteIfBound(Sender);
				});
		}));
	}
}

void UAccelByteCommonPartySubsystem::AcceptPartyInvite(ULocalPlayer* LocalPlayer, FUniqueNetIdRepl SenderUniqueId)
{
	if (const USocialToolkit* SocialToolkit = USocialToolkit::GetToolkitForPlayer(LocalPlayer))
	{
		USocialParty* Party = SocialToolkit->GetSocialManager().GetParty(FOnlinePartySystemAccelByte::GetAccelBytePartyTypeId());

		const IOnlineSubsystem* OSS = SocialToolkit->GetSocialOss(ESocialSubsystem::Primary);
		check(OSS);

		const IOnlinePartyPtr PartyInf = OSS->GetPartyInterface();
		check(PartyInf.IsValid());

		const FUniqueNetIdPtr LocalUserId =
			SocialToolkit->GetLocalUser().GetUserId(ESocialSubsystem::Primary).GetUniqueNetId();

		if (Party)
		{
			Party->LeaveParty(USocialParty::FOnLeavePartyAttemptComplete::CreateWeakLambda(this,
				[PartyInf, LocalUserId, SenderUniqueId, this](ELeavePartyCompletionResult Result)
				{
					AcceptInviteRequest(PartyInf, LocalUserId, SenderUniqueId.GetUniqueNetId());
				}));
		}
		else
		{
			AcceptInviteRequest(PartyInf, LocalUserId, SenderUniqueId.GetUniqueNetId());
		}
	}
}

void UAccelByteCommonPartySubsystem::RejectPartyInvite(ULocalPlayer* LocalPlayer, FUniqueNetIdRepl SenderUniqueId)
{
	if (const USocialToolkit* SocialToolkit = USocialToolkit::GetToolkitForPlayer(LocalPlayer))
	{
		const IOnlineSubsystem* OSS = SocialToolkit->GetSocialOss(ESocialSubsystem::Primary);
		check(OSS);

		const IOnlinePartyPtr PartyInf = OSS->GetPartyInterface();
		check(PartyInf.IsValid())

		const FUniqueNetIdRepl LocalUserId = SocialToolkit->GetLocalUser().GetUserId(ESocialSubsystem::Primary);

		PartyInf->RejectInvitation(*LocalUserId.GetUniqueNetId(), *SenderUniqueId.GetUniqueNetId());
	}
}

void UAccelByteCommonPartySubsystem::SetOnPartyDataChangeDelegate(ULocalPlayer* LocalPlayer, FPartyVoidDelegate OnChange)
{
	if (const USocialToolkit* SocialToolkit = USocialToolkit::GetToolkitForPlayer(LocalPlayer))
	{
		const USocialParty* Party = SocialToolkit->GetSocialManager().GetParty(FOnlinePartySystemAccelByte::GetAccelBytePartyTypeId());
		check(Party);

		const IOnlineSubsystem* OSS = SocialToolkit->GetSocialOss(ESocialSubsystem::Primary);
		check(OSS);

		const IOnlinePartyPtr PartyInf = OSS->GetPartyInterface();
		IOnlineIdentityPtr IdentityInf = OSS->GetIdentityInterface();

		// Called when invite received
		PartyInf->OnPartyMemberJoinedDelegates.AddWeakLambda(this,
			[OnChange](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId)
			{
				OnChange.ExecuteIfBound();
			});

		// testing purpose
		PartyInf->OnPartyDataReceivedDelegates.AddWeakLambda(this,
			[](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FName& Namespace, const FOnlinePartyData& PartyData)
			{
				UE_LOG(LogTemp, Warning, TEXT("DATA RECEIVED"));
			});

		PartyInf->OnPartyJoinedDelegates.AddWeakLambda(this,
			[](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId)
			{
				UE_LOG(LogTemp, Warning, TEXT("PARTY JOINED"));
			});

		PartyInf->OnPartyMemberDataReceivedDelegates.AddWeakLambda(this,
			[](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId, const FName& Namespace, const FOnlinePartyData& PartyData)
			{
				UE_LOG(LogTemp, Warning, TEXT("MEMBER DATA RECEIVED"));
			});

		PartyInf->OnPartyExitedDelegates.AddWeakLambda(this,
			[](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId)
			{
				UE_LOG(LogTemp, Warning, TEXT("LEFT PARTY"));
			});
	}
}

void UAccelByteCommonPartySubsystem::CreatePartyIfNoPartyExist(const USocialToolkit* SocialToolkit, TDelegate<void()> OnComplete)
{
	if (SocialToolkit->GetSocialManager().GetParty(FOnlinePartySystemAccelByte::GetAccelBytePartyTypeId()))
	{
		OnComplete.ExecuteIfBound();
	}
	else
	{
		FPartyConfiguration PartyConfiguration;
		PartyConfiguration.bIsAcceptingMembers = true;
		SocialToolkit->GetSocialManager().CreateParty(
			FOnlinePartyTypeId(0),
			FOnlinePartySystemAccelByte::GetAccelBytePartyTypeId(),
			PartyConfiguration,
			USocialManager::FOnCreatePartyAttemptComplete::CreateWeakLambda(this,
				[OnComplete](ECreatePartyCompletionResult Result)
				{
					const bool bIsFailed =
						Result == ECreatePartyCompletionResult::FailedToCreateMucRoom ||
						Result == ECreatePartyCompletionResult::UnknownInternalFailure ||
						Result == ECreatePartyCompletionResult::UnknownClientFailure;
					if (!bIsFailed)
					{
						OnComplete.ExecuteIfBound();
					}
				}));
	}
}

FABPartySubsystemPartyMember UAccelByteCommonPartySubsystem::BlueprintablePartyMemberData(
	const UPartyMember* PartyMember, FUniqueNetIdRef LocalUserId)
{
	FABPartySubsystemPartyMember ABPartyMember = FABPartySubsystemPartyMember(
		PartyMember->GetSocialUser().GetUserId(ESocialSubsystem::Primary),
		PartyMember->GetDisplayName(),
		PartyMember->IsPartyLeader());
	ABPartyMember.bIsLocalUser = ABPartyMember.UserInfo.UserId == LocalUserId;

	return ABPartyMember;
}

FABPartySubsystemPartyMember UAccelByteCommonPartySubsystem::BlueprintablePartyMemberData(
	const FOnlinePartyMemberConstRef PartyMember, FUniqueNetIdRef LocalUserId)
{
	FABPartySubsystemPartyMember ABPartyMember = FABPartySubsystemPartyMember(
		PartyMember->GetUserId(),
		PartyMember->GetDisplayName(),
		false);
	ABPartyMember.bIsLocalUser = ABPartyMember.UserInfo.UserId == LocalUserId;

	return ABPartyMember;
}

void UAccelByteCommonPartySubsystem::AcceptInviteRequest(IOnlinePartyPtr PartyInf, FUniqueNetIdPtr LocalUserUniqueId, FUniqueNetIdPtr SenderUniqueId)
{
	IOnlinePartyJoinInfoConstPtr TargetPartyJoinInfo;
	TArray<IOnlinePartyJoinInfoConstRef> PartyJoinInfos;
	PartyInf->GetPendingInvites(
		*LocalUserUniqueId,
		PartyJoinInfos);
	for (const IOnlinePartyJoinInfoConstRef PartyJoinInfo : PartyJoinInfos)
	{
		if (PartyJoinInfo->GetSourceUserId() == SenderUniqueId)
		{
			TargetPartyJoinInfo = PartyJoinInfo;
		}
	}
	PartyInf->JoinParty(*LocalUserUniqueId, *TargetPartyJoinInfo);
}