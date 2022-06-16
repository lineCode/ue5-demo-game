// Copyright (c) 2022 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "AccelByteEndgame.h"
#include "Json.h"
#if UE_EDITOR
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "ISettingsContainer.h"
#endif
#include "AccelByteEndgameSettings.h"
#define LOCTEXT_NAMESPACE "FAccelByteEndgameModule"

using namespace endgame;

namespace
{
	///////////////////////////////////////////////////////////////////////////////

	TArray<TSharedPtr<EndGameObject>> ConvertBodyToPlayer(FHttpResponsePtr response)
	{
		TSharedPtr<FJsonObject> responseObj;
		TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(response->GetContentAsString());
		TSharedPtr<FJsonObject> jsonPlayer;
		bool successful = FJsonSerializer::Deserialize(reader, jsonPlayer);

		int code = response->GetResponseCode();

		Player* playerPtr = new Player();;
		FGuid::Parse(jsonPlayer->GetStringField("player_id"), playerPtr->m_objectId);

		TArray<EndGameObjectPtr> players;
		players.Add(EndGameObjectPtr(playerPtr));

		return players;
	}

	///////////////////////////////////////////////////////////////////////////////

	TArray<TSharedPtr<EndGameObject>> ConvertBodyToPlayers(FHttpResponsePtr response)
	{
		TSharedPtr<FJsonObject> responseObj;
		TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(response->GetContentAsString());
		TArray< TSharedPtr<FJsonValue> > jsonArray;
		bool successful = FJsonSerializer::Deserialize(reader, jsonArray);

		int code = response->GetResponseCode();

		TArray<TSharedPtr<EndGameObject>> players;

		for (auto i : jsonArray)
		{
			Player* playerPtr = new Player();

			FGuid::Parse(i->AsObject()->GetStringField("player_id"), playerPtr->m_objectId);

			players.Add(EndGameObjectPtr(playerPtr));
		}

		return players;
	}
}

///////////////////////////////////////////////////////////////////////////////

void FAccelByteEndgameModule::StartupModule()
{
	RegisterSettings();

	//TestAwardToken("matt2",FGuid("d4503b6d-64ff-4f4d-85a7-72075283052a"));
}

///////////////////////////////////////////////////////////////////////////////

void FAccelByteEndgameModule::ShutdownModule()
{
	UnregisterSettings();
}

///////////////////////////////////////////////////////////////////////////////

void FAccelByteEndgameModule::RegisterSettings()
{
#if WITH_EDITOR
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings")) {
		SettingsModule->RegisterSettings(
			TEXT("Project"),
			TEXT("Plugins"),
			TEXT("AccelByte EndGame"),
			FText::FromName(TEXT("AccelByte EndGame SDK")),
			FText::FromName(TEXT("Setup your plugin.")),
			GetMutableDefault<UAccelbyteEndgameSettings>());
	}
	OnPropertyChangedDelegateHandle =
		FCoreUObjectDelegates::OnObjectPropertyChanged.AddRaw(this, &FAccelByteEndgameModule::OnPropertyChanged);
#endif

	m_baseAddress = GetDefault<UAccelbyteEndgameSettings>()->BaseAddress;
	m_apiKey = GetDefault<UAccelbyteEndgameSettings>()->APIKey;
	FGuid::Parse(GetDefault<UAccelbyteEndgameSettings>()->GameID, m_gameId);
	FGuid::Parse(GetDefault<UAccelbyteEndgameSettings>()->NamespaceID, m_namespaceId);
}

///////////////////////////////////////////////////////////////////////////////

void FAccelByteEndgameModule::UnregisterSettings()
{
#if WITH_EDITOR
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings")) {
		SettingsModule->UnregisterSettings(TEXT("Project"), TEXT("Plugins"), TEXT("AccelByte BlackBox SDK"));
	}
	FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(OnPropertyChangedDelegateHandle);
#endif
}

///////////////////////////////////////////////////////////////////////////////

void FAccelByteEndgameModule::OnPropertyChanged
(
	UObject* ModifiedObject, 
	FPropertyChangedEvent& PropertyChangedEvent
)
{
	if (UAccelbyteEndgameSettings* settings = Cast<UAccelbyteEndgameSettings>(ModifiedObject)) {
		m_baseAddress = settings->BaseAddress;
		m_apiKey = settings->APIKey;
		FGuid::Parse( settings->GameID, m_gameId );
		FGuid::Parse( settings->NamespaceID, m_namespaceId );
	}
}
	
///////////////////////////////////////////////////////////////////////////////

void FAccelByteEndgameModule::OnResponseReceived
(
	FHttpRequestPtr request, 
	FHttpResponsePtr response, 
	bool bConnectedSuccessfully
)
{
	UE_LOG(LogTemp, Display, TEXT("Response %s"), *response->GetContentAsString());
	if (bConnectedSuccessfully)
	{
		TSharedPtr<FJsonObject> responseObj;
		TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(response->GetContentAsString());
		FJsonSerializer::Deserialize(reader, responseObj);
		UE_LOG(LogTemp, Display, TEXT("Title %s"), *responseObj->GetStringField("title"));

	}
}

///////////////////////////////////////////////////////////////////////////////

FHttpRequestRef FAccelByteEndgameModule::CreateEndgameRequest
(
	FString path, 
	FString verb, 
	TMap<FString,FString> const& queryParams, 
	TMap<FString, FString> const& bodyParams, 
	std::function<void(FHttpRequestPtr request, FHttpResponsePtr response, bool bConnectedSuccessfully)> callback
) const
{
	FString requestBody;
	FHttpRequestRef request = FHttpModule::Get().CreateRequest();

	if (bodyParams.Num() > 0)
	{
		TSharedRef<FJsonObject> requestObj = MakeShared<FJsonObject>();

		for (auto& i : bodyParams)
		{
			requestObj->SetStringField(i.Key, i.Value);
		}

		TSharedRef<TJsonWriter<>> writer = TJsonWriterFactory<>::Create(&requestBody);
		FJsonSerializer::Serialize(requestObj, writer);
		request->SetHeader("Content-Type", "application/json");
		request->SetContentAsString(requestBody);
	}

	request->SetHeader("X-API-KEY", m_apiKey);

	if (queryParams.Num() > 0)
	{
		path += "?";

		for (auto& i : queryParams)
		{
			path += i.Key;
			path += "=";
			path += i.Value;
			path += "&";
		}
	}

	path = "localhost:8080/" + path;

	UE_LOG(LogTemp, Display, TEXT("Endgame request %s"), *path);

	request->OnProcessRequestComplete().BindLambda(callback);
	request->SetURL(path);
	request->SetVerb(verb);
	request->ProcessRequest();

	return request;
}

///////////////////////////////////////////////////////////////////////////////

HandlerResult FAccelByteEndgameModule::GetCommonResultInfo
(
	FHttpResponsePtr response, 
	bool bConnectedSuccessfully
) const
{
	HandlerResult resultInfo;

	auto responseCode = response->GetResponseCode();

	if (bConnectedSuccessfully && responseCode == 200)
	{
		resultInfo.error = ErrorType::None;
	}
	else if (response->GetResponseCode() == 400)
	{
		resultInfo.error = ErrorType::InvalidRequest;
	}
	else if (response->GetResponseCode() == 401)
	{
		resultInfo.error = ErrorType::AuthenticationError;
	}

	return resultInfo;
}

///////////////////////////////////////////////////////////////////////////////

void FAccelByteEndgameModule::GetPlayerByUniqueId
(
	FString uniquePlayerId, 
	FGuid gameId, 
	HandlerPtr handler 
) const
{
	TMap<FString, FString> bodyParams;
	TMap<FString, FString> queryParams;

	queryParams.Add("player_game_data_unique_id", uniquePlayerId);

	FString path = FString("v1/games/") + gameId.ToString() + "/players";
	
	handler->request = CreateEndgameRequest(path, "GET", queryParams, bodyParams, [handler,this](FHttpRequestPtr request, FHttpResponsePtr response, bool bConnectedSuccessfully)
	{
		HandlerResult resultInfo = GetCommonResultInfo(response,bConnectedSuccessfully);
		resultInfo.responseObjects = ConvertBodyToPlayers(response);

		if (handler->alive)
		{
			handler->callback(resultInfo);
		}
	});
}

///////////////////////////////////////////////////////////////////////////////

void FAccelByteEndgameModule::GetPlayerInGame
(
	FGuid playerId, 
	FGuid gameId, 
	HandlerPtr handler
) const
{
	TMap<FString, FString> bodyParams;
	TMap<FString, FString> queryParams;

	queryParams.Add("player_id", playerId.ToString(EGuidFormats::DigitsWithHyphensLower));

	FString path = FString("v1/games/") + m_gameId.ToString(EGuidFormats::DigitsWithHyphensLower) + "/players";

	handler->request = CreateEndgameRequest(path, "GET", queryParams, bodyParams, [handler,this](FHttpRequestPtr request, FHttpResponsePtr response, bool bConnectedSuccessfully)
	{
		HandlerResult resultInfo = GetCommonResultInfo(response, bConnectedSuccessfully);

		if (handler->alive)
		{
			resultInfo.responseObjects = ConvertBodyToPlayers(response);

			handler->callback(resultInfo);
		}
	});
}

///////////////////////////////////////////////////////////////////////////////

void FAccelByteEndgameModule::AddPlayerToGame
(
	FGuid playerId, 
	FGuid gameId, 
	FString playerGameDataJson, 
	HandlerPtr handler
) const
{
	TMap<FString, FString> bodyParams;
	TMap<FString, FString> queryParams;

	bodyParams.Add("player_id", playerId.ToString(EGuidFormats::DigitsWithHyphensLower));
	bodyParams.Add("player_game_data", playerGameDataJson);

	FString path = FString("v1/games/") + m_gameId.ToString(EGuidFormats::DigitsWithHyphensLower) + "/players";

	handler->request = CreateEndgameRequest(path, "POST", queryParams, bodyParams, [handler, this](FHttpRequestPtr request, FHttpResponsePtr response, bool bConnectedSuccessfully)
	{
		HandlerResult resultInfo = GetCommonResultInfo(response, bConnectedSuccessfully);

		if (handler->alive)
		{
			handler->callback(resultInfo);
		}
	});
}

///////////////////////////////////////////////////////////////////////////////

void FAccelByteEndgameModule::AwardPlayerItem
(
	FGuid const& playerId, 
	FGuid const& itemId, 
	uint32 amount, 
	HandlerPtr handler
) const
{
	TMap<FString, FString> bodyParams;
	TMap<FString, FString> queryParams;

	bodyParams.Add("amount", FString::FromInt(amount));


	FString path = FString("v1/players/") + playerId.ToString(EGuidFormats::DigitsWithHyphensLower) + "/items/" + itemId.ToString(EGuidFormats::DigitsWithHyphensLower);

	handler->request = CreateEndgameRequest(path, "POST", queryParams, bodyParams, [handler, this](FHttpRequestPtr request, FHttpResponsePtr response, bool bConnectedSuccessfully)
	{
		HandlerResult resultInfo = GetCommonResultInfo(response, bConnectedSuccessfully);

		if (handler->alive)
		{
			handler->callback(resultInfo);
		}
	});
}

///////////////////////////////////////////////////////////////////////////////

void FAccelByteEndgameModule::CreateNewPlayer
(
	FString playerData, 
	HandlerPtr handler
) const
{
	TMap<FString, FString> bodyParams;
	TMap<FString, FString> queryParams;

	bodyParams.Add("player_data", playerData);

	FString path = FString("v1/namespaces/") + m_namespaceId.ToString(EGuidFormats::DigitsWithHyphensLower) + "/players";

	handler->request = CreateEndgameRequest(path, "POST", queryParams, bodyParams, [handler, this](FHttpRequestPtr request, FHttpResponsePtr response, bool bConnectedSuccessfully)
	{
		HandlerResult resultInfo = GetCommonResultInfo(response, bConnectedSuccessfully);

		resultInfo.responseObjects = ConvertBodyToPlayer(response);

		if (handler->alive)
		{
			handler->callback(resultInfo);
		}
	});
}

///////////////////////////////////////////////////////////////////////////////

void FAccelByteEndgameModule::EnsurePlayerAddedToGame
(
	Player player, 
	FGuid gameId, 
	FString uniquePlayerId, 
	endgame::HandlerPtr ensurePlayerInGameHandler
) const
{
	UE_LOG(LogTemp, Display, TEXT("EnsurePlayerAddedToGame %s"), *player.m_objectId.ToString());

	auto getPlayerInGameHander = CreateHandler([ensurePlayerInGameHandler, this, gameId, player, uniquePlayerId](HandlerResult const& existingPlayerResult)
	{
		if (existingPlayerResult.error == ErrorType::None)
		{
			TArray<EndGameObjectPtr> const& players = existingPlayerResult.responseObjects;

			if (players.Num() == 1)
			{
				HandlerResult result = existingPlayerResult;
				ensurePlayerInGameHandler->callback(result);
			}
			else
			{
				HandlerPtr addPlayerToGameHandler = CreateHandler([ensurePlayerInGameHandler, player](HandlerResult const& addPlayerToGameResult)
				{
					if (addPlayerToGameResult.error == ErrorType::None)
					{
						HandlerResult result = addPlayerToGameResult;
						ensurePlayerInGameHandler->callback(result);
					}
					else
					{
						ensurePlayerInGameHandler->callback(addPlayerToGameResult);
					}
				});

				ensurePlayerInGameHandler->AddDependentHandler(addPlayerToGameHandler);

				FString playerGameJson = FString("{\"unique_id\":\"") + uniquePlayerId + "\"}";

				AddPlayerToGame(player.m_objectId, gameId, playerGameJson, addPlayerToGameHandler);
			}
		}
		else
		{
			ensurePlayerInGameHandler->callback(existingPlayerResult);
		}
	});

	GetPlayerInGame(player.m_objectId, m_gameId, getPlayerInGameHander);
}

///////////////////////////////////////////////////////////////////////////////

void FAccelByteEndgameModule::GetOrCreatePlayer
(
	FString uniquePlayerId, 
	FGuid gameId, 
	endgame::HandlerPtr getOrCreatePlayerHandler
) const
{
	UE_LOG(LogTemp, Display, TEXT("uniquePlayerId %s"), *uniquePlayerId);

	GetPlayerByUniqueId(uniquePlayerId, gameId, CreateHandler([uniquePlayerId, getOrCreatePlayerHandler,this](HandlerResult const& existingPlayerResult)
	{
		auto ensurePlayerAddedToGame = [&,this](Player player)
		{
			auto ensurePlayerAddedToGameHandler = CreateHandler([existingPlayerResult, getOrCreatePlayerHandler, player](HandlerResult const& addedToGameResult)
			{
				if (addedToGameResult.error == ErrorType::None)
				{
					getOrCreatePlayerHandler->callback(addedToGameResult);
				}
			});

			getOrCreatePlayerHandler->AddDependentHandler(ensurePlayerAddedToGameHandler);

			EnsurePlayerAddedToGame(player, m_gameId, uniquePlayerId, ensurePlayerAddedToGameHandler );
			return;
		};

		if (existingPlayerResult.error == ErrorType::AuthenticationError)
		{
			getOrCreatePlayerHandler->callback(existingPlayerResult);
			return;
		}

		if (existingPlayerResult.error == ErrorType::None)
		{
			if (existingPlayerResult.responseObjects.Num() > 0)
			{
				ensurePlayerAddedToGame(*static_cast<Player*>( existingPlayerResult.responseObjects[0].Get() ));
				return;
			}
		}
		
		auto createPlayerHandler = CreateHandler([ensurePlayerAddedToGame, getOrCreatePlayerHandler](HandlerResult const& createPlayerResult)
		{
			if (createPlayerResult.error == ErrorType::None && createPlayerResult.responseObjects.Num() == 1)
			{
				ensurePlayerAddedToGame(*static_cast<Player*>(createPlayerResult.responseObjects[0].Get()));
			}
			else
			{
				getOrCreatePlayerHandler->callback(createPlayerResult);
			}
		});

		getOrCreatePlayerHandler->AddDependentHandler(createPlayerHandler);

		CreateNewPlayer("{}", createPlayerHandler);
	}));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FAccelByteEndgameModule::TestAwardToken(FString userName, FGuid itemId)
{
	auto awardItemHandler = CreateHandler([](HandlerResult const& result)
	{
		UE_LOG(LogTemp, Display, TEXT("Awarded Item Handler Hit"));

		if (result.responseObjects.Num() > 0)
		{
			Player* player = static_cast<Player*>(result.responseObjects[0].Get());

			UE_LOG(LogTemp, Display, TEXT("PlayerId %s"), *player->m_objectId.ToString());
		}
	});

	m_testHandler = CreateHandler([awardItemHandler, itemId, this](HandlerResult const& result)
	{
		UE_LOG(LogTemp, Display, TEXT("GetPlayerByUniqueId matt Handler hit"));

		if (result.responseObjects.Num() > 0)
		{
			auto& players = result.responseObjects;

			if (players.Num() == 1)
			{
				UE_LOG(LogTemp, Display, TEXT("PlayerId %s"), *players[0]->m_objectId.ToString());

				FAccelByteEndgameModule::AwardPlayerItem(players[0]->m_objectId, itemId, 1, awardItemHandler);
			}
		}
	});

	m_testHandler->AddDependentHandler(awardItemHandler);

	FAccelByteEndgameModule::GetOrCreatePlayer(userName, m_gameId, m_testHandler);
}

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_MODULE(FAccelByteEndgameModule, AccelByteEndgame)
