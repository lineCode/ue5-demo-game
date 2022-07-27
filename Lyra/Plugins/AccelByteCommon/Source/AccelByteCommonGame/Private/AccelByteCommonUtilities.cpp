// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "AccelByteCommonUtilities.h"

FString UAccelByteCommonUtilities::ProjectVersion = TEXT("");
FString UAccelByteCommonUtilities::GitHash = TEXT("");

FString UAccelByteCommonUtilities::GetProjectVersion()
{
	if(ProjectVersion.IsEmpty())
	{
		GConfig->GetString(
			TEXT("/Script/EngineSettings.GeneralProjectSettings"),
			TEXT("ProjectVersion"),
			ProjectVersion,
			GGameIni
		);
	}
	return ProjectVersion;
}

FString UAccelByteCommonUtilities::GetGitHash()
{
	if(GitHash.IsEmpty())
	{
		GConfig->GetString(
			TEXT("/Script/EngineSettings.GeneralProjectSettings"),
			TEXT("GitHash"),
			GitHash,
			GGameIni
		);
	}
	return GitHash;
}
