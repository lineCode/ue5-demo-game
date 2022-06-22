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

	/**
	 * Return whether the specified widget is on top on a layer or not.
	 * Will work with widget that was pushed using Push Content To Layer For Player function.
	 *
	 * @param InOwningPlayer Owning player controller
	 * @param InLayerName Specified layer name
	 * @param InWidgetClass Specified widget class name
	 */
	UFUNCTION(BlueprintCallable, meta = (ExpandBoolAsExecs = "ReturnValue"))
	static bool IsContentOnTopOnLayer(
		APlayerController* InOwningPlayer,
		FGameplayTag InLayerName,
		TSoftClassPtr<UCommonActivatableWidget> InWidgetClass);
};
