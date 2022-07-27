#pragma once

#include "Delegates/Delegate.h"
#include "SlateBasics.h"
#include "Runtime/Core/Public/Containers/LruCache.h"
#include "Runtime/ImageWrapper/Public/IImageWrapper.h"

typedef TSharedPtr<const FSlateBrush> FCacheBrush;

DECLARE_DELEGATE_OneParam(FOnImageReceived, FCacheBrush);

class FCommonImageUtils
{
public:
	//Static class doesn't have constructors or destructor
	FCommonImageUtils() = delete;
	FCommonImageUtils(const FCommonImageUtils& other) = delete;
	FCommonImageUtils& operator=(const FCommonImageUtils& other) = delete;
	FCommonImageUtils(FCommonImageUtils&& other) = delete;
	FCommonImageUtils& operator=(FCommonImageUtils&& other) = delete;
	~FCommonImageUtils() = delete;

	static void GetImage(const FString& Url, const FOnImageReceived& OnReceived, const FString& Filename = TEXT(""));

	static FSlateDynamicImageBrush* CreateBrush(const FName& ResourceName, const TArray<uint8>& ImageData, const EImageFormat InFormat);
	static TSharedPtr<FSlateDynamicImageBrush> CreateBrush(FString ContentType, FName ResourceName, TArray<uint8> ImageData);

	static FString MD5HashArray(const TArray<uint8>& Array);
};