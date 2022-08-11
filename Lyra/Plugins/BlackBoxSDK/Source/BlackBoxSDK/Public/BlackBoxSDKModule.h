// Copyright (c) 2019 - 2020 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Modules/ModuleManager.h"

class BLACKBOXSDK_API IAccelByteBlackBoxSDKModuleInterface : public IModuleInterface {
public:
    /**
     * @brief Get the BlackBox SDK, this is the entry point of the SDK
     *
     * @return IAccelByteBlackBoxSDKModuleInterface&
     */
    static IAccelByteBlackBoxSDKModuleInterface& Get()
    {
        return FModuleManager::LoadModuleChecked<IAccelByteBlackBoxSDKModuleInterface>("BlackBoxSDK");
    }

    /**
     * @brief Check if the SDK module has been correctly loaded
     *
     * @return true
     * @return false
     */
    static bool IsAvailable() { return FModuleManager::Get().IsModuleLoaded("BlackBoxSDK"); }

    /**
     * @brief Tick the SDK
     *
     * @param DeltaTime
     */
    virtual void Tick(float DeltaTime) = 0;

    /**
     * @brief Call this at the start of the game
     *
     */
    virtual void Start() = 0;

    /**
     * @brief Call this at the end of the game
     *
     */
    virtual void Stop() = 0;

    /**
     * @brief Call this on every tick of the game to record the keypresses
     *
     * @param PlayerController
     */
    virtual void FeedKeyboardInput(APlayerController* PlayerController) = 0;

    /**
     * @brief Delete a field in additional info
     *
     * @param FieldName
     */
    virtual void DeleteAdditionalInfoField(FString FieldName) = 0;

    /**
     * @brief Empty the additional info
     *
     */
    virtual void EmptyAdditionalInfo() = 0;

    /**
     * @brief Update / register a new additional info
     *
     * @param FieldName
     * @param Value
     */
    virtual bool UpdateAdditionalInfo(FString FieldName, FString Value) = 0;

    /**
     * @brief Get a registered additional info
     *
     * @param FieldName
     */
    virtual FString GetAdditionalInfoValue(FString FieldName) = 0;

    /**
     * @brief Set true to enable logs from the SDK
     *
     * @param Enable
     */
    virtual void EnableLog(bool Enable) = 0;

    /**
     * @brief Set level of severity of returned logs
     *
     * @param LogSeverity
     */
    virtual void SetLogCallbackSeverity(uint8 MaxLogSeverity) = 0;
};
