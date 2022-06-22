// Copyright (c) 2018 AccelByte, inc. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "GameplayTagContainer.h"

#include "LyraBlueprintUtilities.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class LYRAGAME_API ULyraBlueprintUtilities final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (ExpandBoolAsExecs = "ReturnValue"))
	static bool IsContentOnTopOnLayer(
		APlayerController* InOwningPlayer,
		FGameplayTag InLayerName,
		TSoftClassPtr<UCommonActivatableWidget> InWidgetClass);
};
