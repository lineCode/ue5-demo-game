// Copyright (c) 2018 AccelByte, inc. All rights reserved.


#include "ULyraBlueprintUtilities.h"

#include "PrimaryGameLayout.h"

bool ULyraBlueprintUtilities::IsContentOnTopOnLayer(
	APlayerController* InOwningPlayer,
	FGameplayTag InLayerName,
	TSoftClassPtr<UCommonActivatableWidget> InWidgetClass)
{
	if (UPrimaryGameLayout* RootLayout = UPrimaryGameLayout::GetPrimaryGameLayout(InOwningPlayer))
	{
		if (const UCommonActivatableWidgetContainerBase* WidgetContainer = RootLayout->GetLayerWidget(InLayerName))
		{
			const TArray<UCommonActivatableWidget*>& Contents = WidgetContainer->GetWidgetList();

			if (Contents.Num() < 1)
			{
				return false;
			}

			const FString LastContentName = Contents[Contents.Num() - 1]->GetName();
			if (LastContentName.Contains(InWidgetClass.GetAssetName()))
			{
				return true;
			}
		}
	}
	return false;
}
