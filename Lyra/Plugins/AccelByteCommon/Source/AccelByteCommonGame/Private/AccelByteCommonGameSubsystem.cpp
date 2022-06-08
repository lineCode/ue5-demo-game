// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager."


#include "AccelByteCommonGameSubsystem.h"

#include "SocialManager.h"
#include "SocialToolkit.h"


void UAccelByteCommonGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
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
