// Copyright (c) 2018 AccelByte, inc. All rights reserved.


#include "ImageUtilsBlueprintFunctions.h"

#include "CacheUtils.h"

bool UImageUtilsBlueprintFunctions::GetCacheImage(const FString& ImageId, FSlateBrush& OutBrush)
{
	if(FCommonCacheUtils::IsImageCacheExist(ImageId))
	{
		OutBrush = *FCommonCacheUtils::GetImageCache(ImageId).Get();
		return true;
	}
	return false;
}
