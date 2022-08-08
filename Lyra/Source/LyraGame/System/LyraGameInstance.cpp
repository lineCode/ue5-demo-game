// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraGameInstance.h"

#include "SocialManager.h"
#include "Player/LyraPlayerController.h"

ULyraGameInstance::ULyraGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ULyraGameInstance::Init()
{
	Super::Init();
#if !UE_SERVER
	SocialManager = NewObject<USocialManager>(this);
	SocialManager->InitSocialManager();
#endif
}

void ULyraGameInstance::Shutdown()
{
	if (SocialManager != nullptr)
	{
		SocialManager->ShutdownSocialManager();
	}
	Super::Shutdown();
}

ALyraPlayerController* ULyraGameInstance::GetPrimaryPlayerController() const
{
	return Cast<ALyraPlayerController>(Super::GetPrimaryPlayerController(false));
}