// Copyright (c) 2018 AccelByte, inc. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "ImageUtilsBlueprintFunctions.generated.h"

/**
 * 
 */
UCLASS()
class ACCELBYTECOMMONGAME_API UImageUtilsBlueprintFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	UFUNCTION(BlueprintPure, Category= "AccelByte | Image | Utils")
	static bool GetCacheImage(const FString& ImageId, FSlateBrush& OutBrush);
};
