// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager."


#include "AccelByteCommonGameSubsystem.h"

#include "SocialManager.h"
#include "SocialToolkit.h"


void UAccelByteCommonGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UCommonUserSubsystem* UserSubsystem = GetGameInstance()->GetSubsystem<UCommonUserSubsystem>();
	if(UserSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("Register User Initialize to Create party"));
		UserSubsystem->OnUserInitializeComplete.AddUniqueDynamic(this, &UAccelByteCommonGameSubsystem::HandleUserInitialized);
	}
}

void UAccelByteCommonGameSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UAccelByteCommonGameSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	TArray<UClass*> ChildClasses;
	GetDerivedClasses(GetClass(), ChildClasses, false);

	// Only create an instance if there is not a game-specific subclass
	return ChildClasses.Num() == 0;
}

void UAccelByteCommonGameSubsystem::HandleUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error,
                                                  ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext)
{
	ULocalPlayer* LocalPlayer = GetGameInstance()->GetLocalPlayerByIndex(UserInfo->LocalPlayerIndex);
	if(LocalPlayer)
	{
		USocialToolkit* SocialToolkit = USocialToolkit::GetToolkitForPlayer<USocialToolkit>(LocalPlayer);
		if(SocialToolkit)
		{
			SocialToolkit->GetSocialManager().CreatePersistentParty();
		}
	}
}