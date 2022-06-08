// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Http.h"
#include "Misc/Guid.h"
#include <functional>

namespace endgame {

    enum class ErrorType
    {
        None,
        ServerDetailsNotSet,
        AuthenticationError,
        InvalidRequest
    };

    struct EndGameObject
    {
        FGuid m_objectId;
    };

    struct Player : public EndGameObject
    {
    public:
        FString m_uniqueId;
    };

    typedef TSharedPtr<EndGameObject> EndGameObjectPtr;

    struct HandlerResult
    {
        ErrorType error = ErrorType::None;
        TArray<EndGameObjectPtr> responseObjects;
    };    

    typedef std::function<void(HandlerResult const&)> HandlerCallback;

    struct Handler
    {
        HandlerCallback callback;
        FGuid id;
        FHttpRequestPtr request;
        bool alive = true;
        TArray<TSharedPtr<Handler>> dependentHandlers;

        virtual ~Handler()
        {
            Cancel();
        }

        void AddDependentHandler(TSharedPtr<Handler> dependentHandler)
        {
            dependentHandlers.Add(dependentHandler);
        }

        void Cancel()
        {
            request->CancelRequest();
            callback = HandlerCallback();
            dependentHandlers.Reset();
            alive = false;
        }
    };

    typedef TSharedPtr<Handler> HandlerPtr;

    HandlerPtr CreateHandler(HandlerCallback callback)
    {
        HandlerPtr handler = HandlerPtr(new Handler());
        
        handler->callback = callback;
        handler->id = FGuid::NewGuid();
        handler->alive = true;
        return handler;
    }
}

class FAccelByteEndgameModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

    void GetOrCreatePlayer(FString uniquePlayerId, FGuid gameId, endgame::HandlerPtr getOrCreatePlayerHandler) const;
    void GetPlayerInGame(FGuid playerId, FGuid gameId, endgame::HandlerPtr handler) const;
    void GetPlayerByUniqueId(FString uniquePlayerId, FGuid gameId, endgame::HandlerPtr handler) const;
    void EnsurePlayerAddedToGame(endgame::Player player, FGuid gameId, FString uniquePlayerId, endgame::HandlerPtr ensurePlayerInGameHandler) const;
    void AwardPlayerItem(FGuid const& playerId, FGuid const& itemId, uint32 amount, endgame::HandlerPtr result) const;
    void AddPlayerToGame(FGuid playerId, FGuid gameId, FString playerGameDataJson, endgame::HandlerPtr handler) const;
    void CreateNewPlayer(FString playerData, endgame::HandlerPtr handler) const;

private:

    void OnResponseReceived(FHttpRequestPtr request, FHttpResponsePtr response, bool bConnectedSuccessfully);
    void RegisterSettings();
    void UnregisterSettings();
    void OnPropertyChanged(UObject* ModifiedObject, FPropertyChangedEvent& PropertyChangedEvent);

    FHttpRequestRef CreateEndgameRequest
    (
        FString path, 
        FString verb, 
        TMap<FString, FString> const& queryParams, 
        TMap<FString, FString> const& bodyParams, 
        std::function<void(FHttpRequestPtr request, FHttpResponsePtr response, bool bConnectedSuccessfully)> callback
    ) const;


    endgame::HandlerResult GetCommonResultInfo(FHttpResponsePtr response, bool bConnectedSuccessfully) const;
    
    FString m_baseAddress;
    FDelegateHandle OnPropertyChangedDelegateHandle;
    TMap<FGuid, endgame::Handler> m_activeHandlers;
    FGuid m_gameId;
    FGuid m_namespaceId;
    FString m_apiKey;

    /////////////// TEST //////////////////////////

    void TestAwardToken(FString userName, FGuid itemId);
    endgame::HandlerPtr m_testHandler;
};
