// Copyright (c) 2022 AccelByte, inc. All rights reserved.


#include "AccelByteCommonCommerceSubsystem.h"

#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "Utils/ImageUtils.h"

void UAccelByteCommonCommerceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	PlatformSubsystem = IOnlineSubsystem::GetByPlatform();
	DefaultSubsystem = Online::GetSubsystem(GetWorld());

	if(DefaultSubsystem)
	{
		DefaultSubsystem->GetIdentityInterface()->AddOnLoginStatusChangedDelegate_Handle(0,
			FOnLoginStatusChangedDelegate::CreateUObject(this, &UAccelByteCommonCommerceSubsystem::HandleOnLoginStatusChanged));
	}
	
	GenericErrorHandler = FErrorHandler::CreateWeakLambda(this, [](int32 Code, FString const& ErrMsg){});
}

void UAccelByteCommonCommerceSubsystem::GetUserEntitlements(TArray<FUserEntitlement>& OutEntitlements)
{
	AllEntitlements.GenerateValueArray(OutEntitlements);
}

void UAccelByteCommonCommerceSubsystem::CheckoutOrder(EOnlineSubsystemType SubsystemType, FPurchasingOfferCheckout Offer)
{
	IOnlinePurchasePtr PurchaseInterface;
	if(SubsystemType == EOnlineSubsystemType::AccelByte && DefaultSubsystem)
	{
		PurchaseInterface = DefaultSubsystem->GetPurchaseInterface();
	}
	else if(SubsystemType == EOnlineSubsystemType::Platform && PlatformSubsystem)
	{
		PurchaseInterface = PlatformSubsystem->GetPurchaseInterface();
	}
	if(!PurchaseInterface.IsValid())
	{
		//Debug Error here
		return;
	}
	
	FPurchaseCheckoutRequest Request;
	FPurchaseCheckoutRequest::FPurchaseOfferEntry Entry {
		Offer.Namespace, Offer.Id, Offer.Quantity, Offer.bIsConsumable
	};
	Request.PurchaseOffers.Add(Entry);
	PurchaseInterface->Checkout(*DefaultSubsystem->GetIdentityInterface()->GetUniquePlayerId(0), Request,
		SubsystemType == EOnlineSubsystemType::AccelByte ?
		FOnPurchaseCheckoutComplete::CreateUObject(this, &UAccelByteCommonCommerceSubsystem::HandleCheckoutDefaultComplete) :
		FOnPurchaseCheckoutComplete::CreateUObject(this, &UAccelByteCommonCommerceSubsystem::HandleCheckoutPlatformComplete) 
	);
}

void UAccelByteCommonCommerceSubsystem::CreateOrderRequest(FPurchasingOfferDisplay const& SelectedOffer, int32 Quantity, FPurchasingOfferCheckout& OutOrderRequest)
{
	OutOrderRequest = {SelectedOffer.RegularPrice, SelectedOffer.FinalPrice, SelectedOffer.Id, SelectedOffer.Namespace, Quantity, SelectedOffer.bIsConsumable};
}

void UAccelByteCommonCommerceSubsystem::GetAllOffersByCategory(EOnlineSubsystemType SubsystemType, FString Category, TArray<FPurchasingOfferDisplay>& OutOffers)
{
	FOfferMap* Map = AllOffers.Find(SubsystemType);
	if(Map)
	{
		TArray<FPurchasingOfferDisplay>* Result = Map->Find(Category);
		if(Result)
		{
			OutOffers = *Result;
		}
	}
}

void UAccelByteCommonCommerceSubsystem::GetAllCategories(TArray<FString>& OutCategories)
{
	OutCategories = AllCategories;
}

int32 UAccelByteCommonCommerceSubsystem::GetBalance() const
{
	return Balance;
}

void UAccelByteCommonCommerceSubsystem::HandleOnGetWalletInfoByCurrencyCode(const FAccelByteModelsWalletInfo& Result)
{
	Balance = Result.Balance;
	OnBalanceUpdated.Broadcast(Result);
	
	StartupTaskCounter-= 1;
	if(StartupTaskCounter == 0)
	{
		OnStartupComplete.Broadcast();
	}
}

void UAccelByteCommonCommerceSubsystem::HandleOnQueryCategoriesCompleted(bool bWasSuccessful, const FString& ErrorText)
{
	TArray<FOnlineStoreCategory> OutCategories;
	DefaultSubsystem->GetStoreV2Interface()->GetCategories(OutCategories);
	for(const FOnlineStoreCategory& Category : OutCategories)
	{
		AllCategories.Emplace(Category.Id);
	}
	StartupTaskCounter-= 1;
	if(StartupTaskCounter == 0)
	{
		OnStartupComplete.Broadcast();
	}
}

void UAccelByteCommonCommerceSubsystem::HandleOnLoginStatusChanged(int UserIndex, ELoginStatus::Type OldStatus,	ELoginStatus::Type NewStatus, const FUniqueNetId& UniqueNetId)
{
	if(NewStatus != ELoginStatus::LoggedIn)
	{
		return;
	}
	
	StartupTaskCounter = 0;

	if(DefaultSubsystem)
	{
		DefaultSubsystem->GetEntitlementsInterface()->AddOnQueryEntitlementsCompleteDelegate_Handle(
			FOnQueryEntitlementsCompleteDelegate::CreateUObject(this,  &UAccelByteCommonCommerceSubsystem::HandleOnQueryEntitlementsComplete));
		
		StartupTaskCounter += 4;
		// AccelByte Store used to buy all the items using virtual currency
		DefaultSubsystem->GetStoreV2Interface()->QueryOffersByFilter(UniqueNetId, FOnlineStoreFilter(),
			FOnQueryOnlineStoreOffersComplete::CreateUObject(this, &UAccelByteCommonCommerceSubsystem::HandleOnQueryOffersDefaultCompleted));
		DefaultSubsystem->GetStoreV2Interface()->QueryCategories(UniqueNetId, FOnQueryOnlineStoreCategoriesComplete::CreateUObject(this, &UAccelByteCommonCommerceSubsystem::HandleOnQueryCategoriesCompleted));
		DefaultSubsystem->GetEntitlementsInterface()->QueryEntitlements(UniqueNetId, TEXT(""));
		// TODO: Replace this with OSS interface
		ApiClientPtr = StaticCastSharedPtr<FOnlineIdentityAccelByte>(DefaultSubsystem->GetIdentityInterface())->GetApiClient(UniqueNetId);
		ApiClientPtr->Wallet.GetWalletInfoByCurrencyCode(TEXT("JC"),
			THandler<FAccelByteModelsWalletInfo>::CreateUObject(this, &UAccelByteCommonCommerceSubsystem::HandleOnGetWalletInfoByCurrencyCode),
			GenericErrorHandler);
	}
	if(PlatformSubsystem)
	{
		StartupTaskCounter += 1;
		// Platform store used to buy the currency
		PlatformSubsystem->GetStoreV2Interface()->QueryOffersByFilter(FUniqueNetIdString::EmptyId().Get(), FOnlineStoreFilter(),
			FOnQueryOnlineStoreOffersComplete::CreateUObject(this, &UAccelByteCommonCommerceSubsystem::HandleOnQueryOffersPlatformCompleted));
	}
}

void UAccelByteCommonCommerceSubsystem::HandleOnQueryOffersPlatformCompleted(bool bWasSuccessful, const TArray<FUniqueOfferId>& OfferIds, const FString& Error)
{
	if(bWasSuccessful)
	{
		// I assume offer from platform service can only be bought using real currency and the item that can be bought is virtual currency
		for(FUniqueOfferId const& OfferId : OfferIds)
		{
			TSharedPtr<FOnlineStoreOffer> Offer = PlatformSubsystem->GetStoreV2Interface()->GetOffer(OfferId);
			if(Offer.IsValid())
			{
				TArray<FPurchasingOfferDisplay>& OfferDisplays = AllOffers.FindOrAdd(EOnlineSubsystemType::Platform).FindOrAdd(TEXT(""));
				OfferDisplays.Add(FPurchasingOfferDisplay::Create(*Offer));
			}
		}
	}
	
	StartupTaskCounter-= 1;
	if(StartupTaskCounter == 0)
	{
		OnStartupComplete.Broadcast();
	}
}

void UAccelByteCommonCommerceSubsystem::HandleOnQueryOffersDefaultCompleted(bool bWasSuccessful, const TArray<FUniqueOfferId>& OfferIds, const FString& Error)
{
	if(bWasSuccessful)
	{
		// I assume offer from platform service can only be bought using real currency and the item that can be bought is virtual currency
		for(FUniqueOfferId const& OfferId : OfferIds)
		{
			TSharedPtr<FOnlineStoreOffer> Offer = DefaultSubsystem->GetStoreV2Interface()->GetOffer(OfferId);
			if(Offer.IsValid())
			{
				FString* Category = Offer->DynamicFields.Find(TEXT("Category"));
				if(Category)
				{
					TArray<FPurchasingOfferDisplay>& OfferDisplays = AllOffers.FindOrAdd(EOnlineSubsystemType::AccelByte).FindOrAdd(*Category);
					FPurchasingOfferDisplay OfferDisplay = FPurchasingOfferDisplay::Create(*Offer);
					OfferDisplays.Add(OfferDisplay);
					CacheImage(OfferDisplay);
				}
			}
		}
	}
	StartupTaskCounter-= 1;
	if(StartupTaskCounter == 0)
	{
		OnStartupComplete.Broadcast();
	}
}

void UAccelByteCommonCommerceSubsystem::HandleCheckoutDefaultComplete(const FOnlineError& OnlineError, const TSharedRef<FPurchaseReceipt, ESPMode::ThreadSafe>& Receipt)
{
	OnCheckoutSuccess.Broadcast(OnlineError.bSucceeded, OnlineError.ErrorMessage.ToString(), Receipt->ReceiptOffers.Num() > 0 ? Receipt->ReceiptOffers[0].OfferId : TEXT(""));
	ApiClientPtr->Wallet.GetWalletInfoByCurrencyCode(TEXT("JC"),
		THandler<FAccelByteModelsWalletInfo>::CreateUObject(this, &UAccelByteCommonCommerceSubsystem::HandleOnGetWalletInfoByCurrencyCode),
		GenericErrorHandler);
}

void UAccelByteCommonCommerceSubsystem::HandleCheckoutPlatformComplete(const FOnlineError& OnlineError, const TSharedRef<FPurchaseReceipt, ESPMode::ThreadSafe>& Receipt)
{
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(DefaultSubsystem->GetIdentityInterface());
	FVoidHandler OnSyncSuccess;
	// need to determine what platform are we in.
	IdentityInterface->GetApiClient(0)->Entitlement.SyncPlatformPurchase(EAccelBytePlatformSync::STEAM,
		FVoidHandler::CreateWeakLambda(this, [this, &OnlineError, Receipt]()
		{
			OnCheckoutSuccess.Broadcast(OnlineError.bSucceeded, OnlineError.ErrorMessage.ToString(),
				// need to update on Steam OSS, might be no need?
				Receipt->ReceiptOffers.Num() > 0 ? Receipt->ReceiptOffers[0].OfferId : TEXT(""));
		}),
		// TODO: add proper error handler
		FErrorHandler()
	);
}

void UAccelByteCommonCommerceSubsystem::HandleOnQueryEntitlementsComplete(bool bSuccess, const FUniqueNetId& UniqueNetId, const FString& Namespaces, const FString& Error)
{
	TArray<TSharedRef<FOnlineEntitlement>> OutEntitlements;
	DefaultSubsystem->GetEntitlementsInterface()->GetAllEntitlements(UniqueNetId, Namespaces, OutEntitlements);

	for(TSharedRef<FOnlineEntitlement> const& Ent : OutEntitlements)
	{
		AllEntitlements.Emplace(Ent->Id, FUserEntitlement::Create(Ent));
	}
	
	StartupTaskCounter-= 1;
	if(StartupTaskCounter == 0)
	{
		OnStartupComplete.Broadcast();
	}
}

void UAccelByteCommonCommerceSubsystem::CacheImage(FPurchasingOfferDisplay& Purchasing)
{
	StartupTaskCounter += 1;
	FCommonImageUtils::GetImage(Purchasing.IconUrl, FOnImageReceived::CreateWeakLambda(this, [this, Purchasing](FCacheBrush Brush)
	{
//		Purchasing.IconBrush = *Brush.Get();
		StartupTaskCounter-= 1;
		if(StartupTaskCounter == 0)
		{
			OnStartupComplete.Broadcast();
		}
	}), Purchasing.Id);
}
