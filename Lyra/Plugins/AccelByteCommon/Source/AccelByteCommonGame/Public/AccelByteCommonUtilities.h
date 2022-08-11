// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"

#include "AccelByteCommonUtilities.generated.h"

/**
 * 
 */
UCLASS()
class UAccelByteCommonUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "Project Version")
    static FString GetProjectVersion();
	UFUNCTION(BlueprintPure, Category = "Project Version")
    static FString GetGitHash();

private:
	static FString ProjectVersion;
	static FString GitHash;
};
