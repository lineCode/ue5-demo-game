// Copyright (c) 2022 AccelByte, inc. All rights reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineError.h"
#include "OnlineSubsystem.h"
#include "Core/AccelByteApiClient.h"
#include "Interfaces/OnlinePurchaseInterface.h"
#include "Interfaces/OnlineStoreInterfaceV2.h"
#include "AccelByteCommonCommerceSubsystem.generated.h"

UENUM(BlueprintType)
enum class EOnlineSubsystemType : uint8
{
	AccelByte = 0,
	Platform
};

USTRUCT(BlueprintType)
struct ACCELBYTECOMMONGAME_API FPurchasingOfferDisplay
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "AccelByte Subsystem | Commerce | Offer")
	FString Title {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "AccelByte Subsystem | Commerce | Offer")
	FString Description {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "AccelByte Subsystem | Commerce | Offer")
	int32 RegularPrice {0};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "AccelByte Subsystem | Commerce | Offer")
	int32 FinalPrice {0};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "AccelByte Subsystem | Commerce | Offer")
	FSlateBrush IconBrush;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "AccelByte Subsystem | Commerce | Offer")
	FString IconUrl {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "AccelByte Subsystem | Commerce | Offer")
	FString Id {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "AccelByte Subsystem | Commerce | Offer")
	FString Namespace {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "AccelByte Subsystem | Commerce | Offer")
	bool bIsConsumable {false};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "AccelByte Subsystem | Commerce | Offer")
	FString Category {};

	static FPurchasingOfferDisplay Create(FOnlineStoreOffer InOffer)
	{
		FPurchasingOfferDisplay Result;
		Result.Title = InOffer.Title.ToString();
		Result.Description = InOffer.Description.ToString();
		Result.RegularPrice = InOffer.RegularPrice;
		Result.FinalPrice = InOffer.NumericPrice;
		Result.Id = InOffer.OfferId;
		// not sure are we using this/?
		//Result.Namespace =
		FString* IsConsumable = InOffer.DynamicFields.Find(TEXT("IsConsumable"));
		if(IsConsumable)
		{
			Result.bIsConsumable = IsConsumable->ToBool();
		}
		FString* Category = InOffer.DynamicFields.Find(TEXT("Category"));
		if(Category)
		{
			Result.Category = *Category;
		}
		
		return Result;
	}
};

USTRUCT(BlueprintType)
struct ACCELBYTECOMMONGAME_API FPurchasingOfferCheckout
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "AccelByte Subsystem | Commerce | Offer")
	int32 RegularPrice {0};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "AccelByte Subsystem | Commerce | Offer")
	int32 FinalPrice {0};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "AccelByte Subsystem | Commerce | Offer")
	FString Id {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "AccelByte Subsystem | Commerce | Offer")
	FString Namespace {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "AccelByte Subsystem | Commerce | Offer")
	int32 Quantity {1};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "AccelByte Subsystem | Commerce | Offer")
	bool bIsConsumable {false};
};

USTRUCT(BlueprintType)
struct FUserEntitlement
{
	GENERATED_BODY()
	/** Unique Entitlement Id associated with this entitlement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "AccelByte Subsystem | Commerce | Entitlement")
	FString EntitlementId;
	/** Display name for the entitlement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "AccelByte Subsystem | Commerce | Entitlement")
	FString Name;
	/** Id for the item that this entitlement is associated with */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "AccelByte Subsystem | Commerce | Entitlement")
	FString ItemId;
	/** True if the entitlement is a consumable */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "AccelByte Subsystem | Commerce | Entitlement")
	bool bIsConsumable;
	/** Number of uses still available for a consumable */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "AccelByte Subsystem | Commerce | Entitlement")
	int32 RemainingCount;
	/** Current Status of the entitlement e.g. Active, Subscribe, Expire ... */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "AccelByte Subsystem | Commerce | Entitlement")
	FString Status;

	/**
	 * Equality operator
	 */
	bool operator==(const FUserEntitlement& Other) const
	{
		return Other.EntitlementId == EntitlementId;
	}

	static FUserEntitlement Create(TSharedRef<FOnlineEntitlement> OnlineEntitlement)
	{
		FUserEntitlement Result;
		Result.EntitlementId = OnlineEntitlement->Id;
		Result.Name = OnlineEntitlement->Name;
		Result.ItemId = OnlineEntitlement->ItemId;
		Result.bIsConsumable = OnlineEntitlement->bIsConsumable;
		Result.RemainingCount = OnlineEntitlement->RemainingCount;
		Result.Status = OnlineEntitlement->Status;
		return Result;
	}
};


// Key is category
using FOfferMap = TMap<FString, TArray<FPurchasingOfferDisplay>>;
/**
 * @brief Provide user entitlement, wallet and purchasing stuffs.
 */
UCLASS()
class ACCELBYTECOMMONGAME_API UAccelByteCommonCommerceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
public:
	/** Get User entitlements by Entitlement Id */
	UFUNCTION(BlueprintCallable, Category = "AccelByte Subsystem | Commerce")
	virtual void GetUserEntitlements(TArray<FUserEntitlement>& OutEntitlements);

	// All startup Async task is complete
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStartupComplete);
	
	UPROPERTY(BlueprintAssignable, Category = "AccelByte Subsystem | Commerce")
	FOnStartupComplete OnStartupComplete;

	UFUNCTION(BlueprintCallable, Category = "AccelByte Subsystem | Commerce")
	FORCEINLINE bool IsStartupTaskComplete() const { return StartupTaskCounter == 0; }

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCheckoutSuccess, bool, bWasSuccess, FString const&, ErrMsg, FString const&, OfferId);

	UPROPERTY(BlueprintAssignable, Category = "AccelByte Subsystem | Commerce")
	FOnCheckoutSuccess OnCheckoutSuccess;
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBalanceUpdated, const FAccelByteModelsWalletInfo&, Result);

	UPROPERTY(BlueprintAssignable, Category = "AccelByte Subsystem | Commerce")
	FOnBalanceUpdated OnBalanceUpdated;

	/** Create an order, pass the context (Platform will be purchasing via platform like steam, ps, or something else, AccelByte is buying item from justice) */
	UFUNCTION(BlueprintCallable, Category = "AccelByte Subsystem | Commerce")
	virtual void CheckoutOrder(EOnlineSubsystemType SubsystemType, FPurchasingOfferCheckout Checkout);
	
	UFUNCTION(BlueprintCallable, Category = "AccelByte Subsystem | Commerce")
	virtual void CreateOrderRequest(FPurchasingOfferDisplay const& SelectedOffer, int32 Quantity, FPurchasingOfferCheckout& OutOrderRequest);

	UFUNCTION(BlueprintCallable, Category = "AccelByte Subsystem | Commerce")
	virtual void GetAllOffersByCategory(EOnlineSubsystemType SubsystemType, FString Category, TArray<FPurchasingOfferDisplay>& OutOffers);

    /** @brief Currently only works on accelbyte categories*/
	UFUNCTION(BlueprintCallable, Category = "AccelByte Subsystem | Commerce")
	virtual void GetAllCategories(TArray<FString>& OutCategories);

	UFUNCTION(BlueprintCallable, Category = "AccelByte Subsystem | Commerce")
	virtual int32 GetBalance() const;

protected:
	void HandleOnGetWalletInfoByCurrencyCode(const FAccelByteModelsWalletInfo& Result);
	void HandleOnQueryCategoriesCompleted(bool bWasSuccessful, const FString& ErrorText);
	void HandleOnLoginStatusChanged(int UserIndex, ELoginStatus::Type OldStatus, ELoginStatus::Type NewStatus, const FUniqueNetId& UniqueNetId);
	void HandleOnQueryOffersPlatformCompleted(bool bWasSuccessful, const TArray<FUniqueOfferId>& OfferIds, const FString& Error);
	void HandleOnQueryOffersDefaultCompleted(bool bWasSuccessful, const TArray<FUniqueOfferId>& OfferIds, const FString& Error);
	void HandleCheckoutDefaultComplete(const FOnlineError& OnlineError, const TSharedRef<FPurchaseReceipt, ESPMode::ThreadSafe>& Receipt);
	void HandleCheckoutPlatformComplete(const FOnlineError& OnlineError, const TSharedRef<FPurchaseReceipt, ESPMode::ThreadSafe>& Receipt);
	void HandleOnQueryEntitlementsComplete(bool bSuccess, const FUniqueNetId& UniqueNetId, const FString& Namespaces, const FString& Error);

	void CacheImage(FPurchasingOfferDisplay& Purchasing);
	
	IOnlineSubsystem* PlatformSubsystem;
	IOnlineSubsystem* DefaultSubsystem;
	FApiClientPtr ApiClientPtr;

	TMap<EOnlineSubsystemType, FOfferMap> AllOffers;
	TArray<FString> AllCategories;
	TMap<FString, FUserEntitlement> AllEntitlements;
	int32 Balance;

	int32 StartupTaskCounter = -1;
	FCriticalSection CommerceImageLock;
private:
	FErrorHandler GenericErrorHandler;
};
