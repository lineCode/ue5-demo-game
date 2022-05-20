// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.\nThis is licensed software from AccelByte Inc, for limitations\nand restrictions contact your company contract manager.


#include "AccelBytePartyMember.h"

UAccelBytePartyMember::UAccelBytePartyMember()
{
	MemberDataReplicator.EstablishRepDataInstance<FAccelBytePartyMemberRepData>(RepData);
}
