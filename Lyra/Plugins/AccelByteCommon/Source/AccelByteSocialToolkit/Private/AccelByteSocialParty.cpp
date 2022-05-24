// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "AccelByteSocialParty.h"

#include "AccelBytePartyMember.h"


void FAccelBytePartyRepData::CompareAgainst(const FOnlinePartyRepDataBase& OldData) const
{
	FPartyRepData::CompareAgainst(OldData);
	
	const FAccelBytePartyRepData& TypedOldData = static_cast<const FAccelBytePartyRepData&>(OldData);

	CompareServerConnectInfo(TypedOldData);
}

UAccelByteSocialParty::UAccelByteSocialParty() : Super()
{
	PartyDataReplicator.EstablishRepDataInstance<FAccelBytePartyRepData>(RepData);
}


FPartyPrivacySettings UAccelByteSocialParty::GetDesiredPrivacySettings() const
{
	UAccelByteSocialParty *NonConstThis = const_cast<UAccelByteSocialParty*>(this);
	return GetPrivacySettingsForConfig(NonConstThis->GetCurrentConfiguration());
}

void UAccelByteSocialParty::SetServerConnectInfo(const FString &InConnectInfo)
{
	RepData.SetServerConnectInfo(InConnectInfo);
}

void UAccelByteSocialParty::HandleServerConnectInfoChanged(const FString &InConnectInfo)
{
	if (InConnectInfo.IsEmpty())
	{
		return;
	}

	if (GetWorld() != nullptr && (GetWorld()->GetNetDriver() == nullptr || GetWorld()->URL.GetHostPortString() != InConnectInfo))
	{
		GEngine->SetClientTravel(GetWorld(), *InConnectInfo, TRAVEL_Absolute);
	}
}

TSubclassOf<UPartyMember> UAccelByteSocialParty::GetDesiredMemberClass(bool bLocalPlayer) const
{
	return UAccelBytePartyMember::StaticClass();
}
