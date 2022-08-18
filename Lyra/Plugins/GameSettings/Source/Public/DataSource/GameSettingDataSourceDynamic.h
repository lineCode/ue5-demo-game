// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "GameSettingDataSource.h"
#include "PropertyPathHelpers.h"

//--------------------------------------
// FGameSettingDataSourceDynamic
//--------------------------------------

class GAMESETTINGS_API FGameSettingDataSourceDynamic : public FGameSettingDataSource
{
public:
	FGameSettingDataSourceDynamic(const TArray<FString>& InDynamicPath);

	virtual bool Resolve(ULocalPlayer* InLocalPlayer) override;

	virtual FString GetValueAsString(ULocalPlayer* InLocalPlayer) const override;

	virtual void SetValue(ULocalPlayer* InLocalPlayer, const FString& Value) override;

	virtual FString ToString() const override;

private:
	FCachedPropertyPath DynamicPath;
};