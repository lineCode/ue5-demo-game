// Copyright (c) 2022 AccelByte, inc. All rights reserved.


#include "AccelByteCommonCommerceSubsystem.h"

#include "IImageWrapper.h"
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
		IOnlineIdentityPtr IdentityInterface = DefaultSubsystem->GetIdentityInterface();
		if(IdentityInterface.IsValid())
		{
			IdentityInterface->AddOnLoginStatusChangedDelegate_Handle(0,
				FOnLoginStatusChangedDelegate::CreateUObject(this, &UAccelByteCommonCommerceSubsystem::HandleOnLoginStatusChanged));
		}

		FOnlineSubsystemAccelByte* SubsystemAccelByte = static_cast<FOnlineSubsystemAccelByte*>(DefaultSubsystem);
		if(SubsystemAccelByte)
		{
			// TODO: Uncomment this after OSS commerce is release
			//SubsystemAccelByte->SetLanguage(TEXT("en-US"));
		}
	}
	
	GenericErrorHandler = FErrorHandler::CreateWeakLambda(this, [](int32 Code, FString const& ErrMsg)
	{
		UE_LOG(LogAccelByteCommonCommerce, Error, TEXT("UAccelByteCommonCommerceSubsystem Error: %d, %s"), Code, *ErrMsg);
	});
}

void UAccelByteCommonCommerceSubsystem::QueryEntitlements()
{
	if(DefaultSubsystem != nullptr)
	{
		IOnlineEntitlementsPtr EntitlementInterface = DefaultSubsystem->GetEntitlementsInterface();
		if(EntitlementInterface.IsValid())
		{
			EntitlementInterface->QueryEntitlements(*DefaultSubsystem->GetIdentityInterface()->GetUniquePlayerId(0), TEXT(""));
		}
	}
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
		UE_LOG(LogAccelByteCommonCommerce, Error, TEXT("Purchase Interface not found for %s subsystem"), SubsystemType == EOnlineSubsystemType::Platform ? TEXT("Platform") : TEXT("AccelByte"));
		return;
	}
	
	OnCheckoutStarted.Broadcast(SubsystemType, Offer.Id);
	
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

void UAccelByteCommonCommerceSubsystem::QueryBalance()
{
	// TODO: Replace this with OSS interface
	ApiClientPtr = StaticCastSharedPtr<FOnlineIdentityAccelByte>(DefaultSubsystem->GetIdentityInterface())->GetApiClient(0);
	ApiClientPtr->Wallet.GetWalletInfoByCurrencyCode(TEXT("JC"),
		THandler<FAccelByteModelsWalletInfo>::CreateUObject(this, &UAccelByteCommonCommerceSubsystem::HandleOnGetWalletInfoByCurrencyCode),
		GenericErrorHandler);
}

void UAccelByteCommonCommerceSubsystem::HandleOnGetWalletInfoByCurrencyCode(const FAccelByteModelsWalletInfo& Result)
{
	Balance = Result.Balance;
	OnBalanceUpdated.Broadcast(Result);
	
	DecrementStartupTask();
}

void UAccelByteCommonCommerceSubsystem::HandleOnQueryCategoriesCompleted(bool bWasSuccessful, const FString& ErrorText)
{
	TArray<FOnlineStoreCategory> OutCategories;
	DefaultSubsystem->GetStoreV2Interface()->GetCategories(OutCategories);
	for(const FOnlineStoreCategory& Category : OutCategories)
	{
		AllCategories.Emplace(Category.Id);
	}
	DecrementStartupTask();
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
		IOnlineEntitlementsPtr EntitlementInterface = DefaultSubsystem->GetEntitlementsInterface();
		if(EntitlementInterface.IsValid())
		{
			EntitlementInterface->AddOnQueryEntitlementsCompleteDelegate_Handle(
				FOnQueryEntitlementsCompleteDelegate::CreateUObject(this,  &UAccelByteCommonCommerceSubsystem::HandleOnQueryEntitlementsComplete));
		}
		
		StartupTaskCounter += 4;
		// AccelByte Store used to buy all the items using virtual currency
		IOnlineStoreV2Ptr StoreV2Interface = DefaultSubsystem->GetStoreV2Interface();
		if(StoreV2Interface.IsValid())
		{
			StoreV2Interface->QueryOffersByFilter(UniqueNetId, FOnlineStoreFilter(),
				FOnQueryOnlineStoreOffersComplete::CreateUObject(this, &UAccelByteCommonCommerceSubsystem::HandleOnQueryOffersDefaultCompleted));
			StoreV2Interface->QueryCategories(UniqueNetId, FOnQueryOnlineStoreCategoriesComplete::CreateUObject(this, &UAccelByteCommonCommerceSubsystem::HandleOnQueryCategoriesCompleted));
		}
		QueryEntitlements();
		QueryBalance();
	}
	if(PlatformSubsystem)
	{
		if(PlatformSubsystem->GetStoreV2Interface())
		{
			StartupTaskCounter += 1;
			// Platform store used to buy the currency
			PlatformSubsystem->GetStoreV2Interface()->QueryOffersByFilter(FUniqueNetIdString::EmptyId().Get(), FOnlineStoreFilter(),
				FOnQueryOnlineStoreOffersComplete::CreateUObject(this, &UAccelByteCommonCommerceSubsystem::HandleOnQueryOffersPlatformCompleted));
		}
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

	DecrementStartupTask();
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
					if(!OfferDisplay.IconUrl.IsEmpty())
					{
						CacheImage(OfferDisplay);
					}
				}
			}
		}
	}
	DecrementStartupTask();
}

void UAccelByteCommonCommerceSubsystem::HandleCheckoutDefaultComplete(const FOnlineError& OnlineError, const TSharedRef<FPurchaseReceipt, ESPMode::ThreadSafe>& Receipt)
{
	OnCheckoutSuccess.Broadcast(OnlineError.bSucceeded, OnlineError.ErrorMessage.ToString(), Receipt->ReceiptOffers.Num() > 0 ? Receipt->ReceiptOffers[0].OfferId : TEXT(""));
	QueryBalance();
	QueryEntitlements();
}

void UAccelByteCommonCommerceSubsystem::HandleCheckoutPlatformComplete(const FOnlineError& OnlineError, const TSharedRef<FPurchaseReceipt, ESPMode::ThreadSafe>& Receipt)
{
	// Currently steam purchase doesn't know if purchase success or not.
	const FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(DefaultSubsystem->GetIdentityInterface());
	IdentityInterface->GetApiClient(0)->Entitlement.SyncPlatformPurchase(EAccelBytePlatformSync::STEAM,
		FVoidHandler::CreateWeakLambda(this, [this, Receipt]()
		{
			QueryBalance();
			OnCheckoutSuccess.Broadcast(true, TEXT(""),
				Receipt->ReceiptOffers.Num() > 0 ? Receipt->ReceiptOffers[0].OfferId : TEXT(""));
		}),
		FErrorHandler::CreateWeakLambda(this, [this, Receipt](int32 Code, FString const& ErrMsg)
		{
			OnCheckoutSuccess.Broadcast(false, FString::Printf(TEXT("Purchase Failed, %d: %s"), Code, *ErrMsg),
				Receipt->ReceiptOffers.Num() > 0 ? Receipt->ReceiptOffers[0].OfferId : TEXT(""));
		})
	);
}

void UAccelByteCommonCommerceSubsystem::HandleOnQueryEntitlementsComplete(bool bSuccess, const FUniqueNetId& UniqueNetId, const FString& Namespaces, const FString& Error)
{
	TArray<TSharedRef<FOnlineEntitlement>> OutEntitlements;
	if(DefaultSubsystem->GetEntitlementsInterface())
	{
		DefaultSubsystem->GetEntitlementsInterface()->GetAllEntitlements(UniqueNetId, Namespaces, OutEntitlements);
	}

	for(TSharedRef<FOnlineEntitlement> const& Ent : OutEntitlements)
	{
		AllEntitlements.Emplace(Ent->Id, FUserEntitlement::Create(Ent));
	}
	
	OnQueryEntitlementsComplete.Broadcast();
	
	DecrementStartupTask();
}

void UAccelByteCommonCommerceSubsystem::CacheImage(FPurchasingOfferDisplay& Purchasing)
{
	StartupTaskCounter += 1;
	FCommonImageUtils::GetImage(Purchasing.IconUrl, FOnImageReceived::CreateWeakLambda(this, [this](FCacheBrush Brush)
	{
		DecrementStartupTask();
	}), Purchasing.Id);
}

void UAccelByteCommonCommerceSubsystem::DecrementStartupTask()
{
	bool bIsStartupCompleted = IsStartupTaskComplete();
	if(!bIsStartupCompleted)
	{
		StartupTaskCounter-= 1;
		if(StartupTaskCounter == 0)
		{
			OnStartupComplete.Broadcast();
		}
	}
}