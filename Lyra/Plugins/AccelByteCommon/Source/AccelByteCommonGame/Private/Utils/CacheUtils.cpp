// Copyright (c) 2020 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "CacheUtils.h"
#include "JsonObjectConverter.h"

FUserCaches FCommonCacheUtils::UserCaches = FUserCaches();

void FCommonCacheUtils::LoadUserCaches()
{
	UserCaches.Caches.Empty();
	FString SavePath = GetCacheDir() / TEXT("UserCaches.json");

	FString CachesString;
	if (!FFileHelper::LoadFileToString(CachesString, *SavePath)) return;
	FJsonObjectConverter::JsonObjectStringToUStruct(CachesString, &UserCaches, 0, 0);
}

void FCommonCacheUtils::SaveUserCache(FUserCache UserCache)
{
	UserCaches.Caches.Add(UserCache);

	FString SavePath = GetCacheDir() / TEXT("UserCaches.json");
	FString CachesString;
	if (!FJsonObjectConverter::UStructToJsonObjectString(UserCaches, CachesString)) return;
	FFileHelper::SaveStringToFile(CachesString, *SavePath);
}

FUserCache FCommonCacheUtils::GetUserCache(FString UserId)
{
	int32 Index = UserCaches.Caches.IndexOfByPredicate([UserId](FUserCache UserCache)
	{
		return UserCache.UserId == UserId;
	});
	return UserCaches.Caches[Index];
}

FCacheBrush FCommonCacheUtils::GetImageCache(FString ImageId)
{
	TArray<uint8> ImageData;
	FString ImagePath = GetCacheDir() / ImageId + TEXT(".png");
	FCacheBrush ImageSlate;
	if (FFileHelper::LoadFileToArray(ImageData, *ImagePath))
	{
		ImageSlate = FCommonImageUtils::CreateBrush(TEXT("png"), FName(*ImagePath), ImageData);
	}
	else if(FFileHelper::LoadFileToArray(ImageData, *(FPaths::ProjectDir() / TEXT("Plugins/AccelByteCommon/Resources/Icon128.png"))))
	{
		// Load default image
		ImageSlate = FCommonImageUtils::CreateBrush(TEXT("png"), FName(*ImagePath), ImageData);
	}
	return ImageSlate;
}

bool FCommonCacheUtils::SaveImageCache(const FString& Filename, const TArray<uint8>& Binary)
{
	FString ImagePath = GetCacheDir() / Filename + TEXT(".png");
	
	if (Filename.IsEmpty() || Binary.Num() == 0) return false;
	if (!FFileHelper::SaveArrayToFile(Binary, *ImagePath)) return false;
	return true;
}

bool FCommonCacheUtils::IsImageCacheExist(FString ImageId)
{
	FString ImagePath = GetCacheDir() / ImageId + TEXT(".png");
	return IFileManager::Get().FileExists(*ImagePath);
}
