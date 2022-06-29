#include "AccelByteCustomCommands.h"


#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "HAL/IConsoleManager.h"
/*
#include "Online.h"
#include "OnlineSubsystemUtils.h"
#include "HAL/IConsoleManager.h"
*/
static void OnSyncComplete()
{
	UE_LOG(LogTemp, Log, TEXT("Sync COMPLETE"))
}
static void OnSyncError(int32 Code, FString const& Msg)
{
	UE_LOG(LogTemp, Log, TEXT("Sync Error: %d, %s"), Code, *Msg);
}

static void OnPurchaseComplete(const FOnlineError& Result, const TSharedRef<FPurchaseReceipt>& Receipt)
{
	UE_LOG(LogTemp, Log, TEXT("Purchase Complete, try to sync platform purchase"))
	IOnlineIdentityPtr Identity = Online::GetIdentityInterface();
	check(Identity);
	FOnlineIdentityAccelBytePtr IdentityAccelByte = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Identity);
	check(IdentityAccelByte);
	IdentityAccelByte->GetApiClient(0)->Entitlement.SyncPlatformPurchase(
		EAccelBytePlatformSync::STEAM,
		AccelByte::FVoidHandler::CreateStatic(&OnSyncComplete),
		FErrorHandler::CreateStatic(&OnSyncError)
	);
}

static FAutoConsoleCommandWithWorldAndArgs ShopPurchaseItem(
	TEXT("Shop.PurchaseItem"),
	TEXT("ItemDefIds"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		UE_LOG(LogTemp, Log, TEXT("ShopPurchaseItem %d args"), Args.Num());
		if(Args.Num() == 0)
		{
			UE_LOG(LogTemp, Log, TEXT("please pass args item def id"), Args.Num());
			return;
		}
		
		IOnlineSubsystem* Subsystem = IOnlineSubsystem::GetByPlatform();
		if(Subsystem->GetSubsystemName().IsEqual(TEXT("Steam")))
		check(Subsystem);
		{
			IOnlinePurchasePtr PurchaseInterface = Subsystem->GetPurchaseInterface();
			FPurchaseCheckoutRequest Request;
			Request.AddPurchaseOffer(TEXT(""), Args[0], 1);
			if(PurchaseInterface)
			{
				PurchaseInterface->Checkout(*FUniqueNetIdString::EmptyId(), Request, FOnPurchaseCheckoutComplete::CreateStatic(&OnPurchaseComplete));
			}
		}
	})
);

static void HandleQueryOnlineStoreOffersComplete(bool bWasSuccessful, const TArray<FUniqueOfferId>& OfferIds, const FString& Error)
{
	FString ItemIds;
	for(const FUniqueOfferId& Offer : OfferIds)
	{
		ItemIds = FString::Printf(TEXT("%s, %s"), *ItemIds, *Offer);
	}
	UE_LOG(LogTemp, Log, TEXT("HandleQueryOnlineStoreOffersComplete with %s. Num: %d, %s"), bWasSuccessful ? TEXT("Successful") : TEXT("Failure"), OfferIds.Num(), *ItemIds);
}

static FAutoConsoleCommandWithWorldAndArgs ShopRequestPrice(
	TEXT("Shop.RequestPrices"), TEXT("Requesting prices to steam"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		IOnlineSubsystem* Subsystem = IOnlineSubsystem::GetByPlatform();
		if(Subsystem->GetSubsystemName().IsEqual(TEXT("Steam")))
		{
			IOnlineStoreV2Ptr Store = Subsystem->GetStoreV2Interface();
			if(Store.IsValid())
			{
				IOnlineIdentityPtr Identity = Online::GetIdentityInterface();
				check(Identity);
				Store->QueryOffersByFilter(*Identity->GetUniquePlayerId(0), FOnlineStoreFilter(), FOnQueryOnlineStoreOffersComplete::CreateStatic(&HandleQueryOnlineStoreOffersComplete));
			}
		}
	}), ECVF_Cheat
);

static FAutoConsoleCommandWithWorldAndArgs GetOffers(
	TEXT("Shop.GetOffers"), TEXT("Get items with prices."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		IOnlineSubsystem* Subsystem = IOnlineSubsystem::GetByPlatform();
		if(Subsystem->GetSubsystemName().IsEqual(TEXT("Steam")))
		{
			TArray<FOnlineStoreOfferRef> OutOffers;
			IOnlineStoreV2Ptr Store = Subsystem->GetStoreV2Interface();
			if(Store.IsValid())
			{
				Store->GetOffers(OutOffers);
			}

			for(const FOnlineStoreOfferRef& Offer : OutOffers)
			{
				UE_LOG(LogTemp, Log, TEXT("GetOffers. Id: %s\nTitle: %s\nCurrency: %s\nPrice: %d\nBasePrice: %d"),
					*Offer->OfferId, *Offer->Title.ToString(), *Offer->CurrencyCode, Offer->NumericPrice, Offer->RegularPrice);
			}
		}
	}), ECVF_Cheat
);
