// Copyright (c) 2020 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "BlackBoxSettings.h"
namespace BlackboxSDK {
bool IsAPIKeyOverriden = true;
bool IsGameVersionIDOverriden = true;
bool IsNamespaceOverriden = true;
} // namespace BlackboxSDK
#if WITH_EDITOR

#    if (ENGINE_MAJOR_VERSION == 4) && (ENGINE_MINOR_VERSION < 25)
bool UBlackBoxSettings::CanEditChange(const UProperty* InProperty) const
#    else
bool UBlackBoxSettings::CanEditChange(const FProperty* InProperty) const
#    endif
{
#    if PLATFORM_WINDOWS
    if (InProperty->GetName() == FString("APIKey"))
        return !BlackboxSDK::IsAPIKeyOverriden;
    else if (InProperty->GetName() == FString("GameVersionID")) {
        return !BlackboxSDK::IsGameVersionIDOverriden;
    }
    else if (InProperty->GetName() == FString("Namespace")) {
        return !BlackboxSDK::IsNamespaceOverriden;
    }
    else
        return true;
#    else
    return false;
#    endif
}
#endif