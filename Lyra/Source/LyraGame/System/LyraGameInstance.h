// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CommonGameInstance.h"
#include "LyraGameInstance.generated.h"

class ULyraSocialManager;
class ULyraSocialParty;
class USocialManager;
class ALyraPlayerController;

UCLASS(Config = Game)
class LYRAGAME_API ULyraGameInstance : public UCommonGameInstance
{
	GENERATED_BODY()

public:

	ULyraGameInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	ALyraPlayerController* GetPrimaryPlayerController() const;

	USocialManager* GetSocialManager() const { return SocialManager; }
protected:

	virtual void Init() override;
	virtual void Shutdown() override;

private:
	UPROPERTY(Transient)
	USocialManager* SocialManager;
};
