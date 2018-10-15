
#include "CustomPlacementModeSettings.h"
#include "UObject/UnrealType.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "EditorStyleSet.h"
#include "Framework/Notifications/NotificationManager.h"
#include "ICustomPlacementModeModule.h"
#include "Misc/MessageDialog.h"

DEFINE_LOG_CATEGORY(CustomPlacementModeSettingLog);


UCustomPlacementModeSettings::UCustomPlacementModeSettings()
{
}


void UCustomPlacementModeSettings::PreEditChange(class FEditPropertyChain & PropertyAboutToChange)
{
	TempPlaceableCategoryItems.Empty();

	for (auto& Item : PlaceableCategoryItems)
	{
		TempPlaceableCategoryItems.Add(Item);
	}
	
	Super::PreEditChange(PropertyAboutToChange);
}

void UCustomPlacementModeSettings::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
		
	if (PropertyChangedEvent.Property != nullptr)
	{
		CheckTestItems(PropertyChangedEvent.Property);
	}
}


void UCustomPlacementModeSettings::CheckTestItems(UProperty* Property)
{
	FName PropertyName = (Property != nullptr) ? Property->GetFName() : NAME_None;


	FNotificationInfo DublicateSetCategoryName(NSLOCTEXT("CustomPlacementMode", "DublicateCategory", "Cannot add new element to the set while an element with the default value exists"));
	DublicateSetCategoryName.Image = FEditorStyle::GetBrush(TEXT("NotificationList.DefaultMessage"));
	DublicateSetCategoryName.FadeInDuration = 0.1f;
	DublicateSetCategoryName.FadeOutDuration = 0.5f;
	DublicateSetCategoryName.ExpireDuration = 1.5f;
	DublicateSetCategoryName.bUseThrobber = false;
	DublicateSetCategoryName.bUseSuccessFailIcons = true;
	DublicateSetCategoryName.bUseLargeFont = true;
	DublicateSetCategoryName.bFireAndForget = true;
	DublicateSetCategoryName.bAllowThrottleWhenFrameRateIsLow = false;

	if (PropertyName == "PlaceableCategoryItems")
	{
		FSettingPlaceableCategoryItem LastItem;

		for (auto& Item : PlaceableCategoryItems)
		{
			LastItem = Item;
		}

		for (int i = 0; i < TempPlaceableCategoryItems.Num(); i++)
		{
			if (TempPlaceableCategoryItems[i] == LastItem)
			{
				PlaceableCategoryItems.Remove(LastItem);
				if (!NotificationSetAddDublicateCategoryName)
				{
					NotificationSetAddDublicateCategoryName = FSlateNotificationManager::Get().AddNotification(DublicateSetCategoryName);
				}

				else
				{
					if (!NotificationSetAddDublicateCategoryName->IsParentValid())
					{
						NotificationSetAddDublicateCategoryName = FSlateNotificationManager::Get().AddNotification(DublicateSetCategoryName);
					}
				}

				break;
			}
		}
	}

	if (PropertyName == "CategoryName")
	{
		if (PlaceableCategoryItems.Num() == TempPlaceableCategoryItems.Num())
		{
			FSettingPlaceableCategoryItem ChangedItem;
			FSettingPlaceableCategoryItem ReturnedItem;

			int index = 0;

			for (auto& Item : PlaceableCategoryItems)
			{
				if (Item.CategoryName != TempPlaceableCategoryItems[index].CategoryName)
				{
					ChangedItem = Item;
					ReturnedItem = TempPlaceableCategoryItems[index];
				}

				index++;
			}

			FText ReturnDialogMessage;
			FString ReturnMessage;
			FFormatOrderedArguments ReturnArguments;
			ReturnArguments.Add(FText::FromString(ReturnedItem.CategoryName.ToString()));

			bool bReturnItems = false;

			for (int i = 0; i < TempPlaceableCategoryItems.Num(); i++)
			{
				if (ChangedItem.CategoryName == FCustomBuiltInPlacementCategories::RecentlyPlaced() ||
					ChangedItem.CategoryName == FCustomBuiltInPlacementCategories::Basic() ||
					ChangedItem.CategoryName == FCustomBuiltInPlacementCategories::Lights() ||
					ChangedItem.CategoryName == FCustomBuiltInPlacementCategories::Visual() ||
					ChangedItem.CategoryName == FCustomBuiltInPlacementCategories::Volumes() ||
					ChangedItem.CategoryName == FCustomBuiltInPlacementCategories::Custom() ||
					ChangedItem.CategoryName == FCustomBuiltInPlacementCategories::AllClasses()
					)
				{
					bReturnItems = true;
					ReturnDialogMessage = FText::Format(NSLOCTEXT("CustomPlacementMode","DialogValueChange","Default category names are not allowed.\nThe current category name will be returned to - {0}\n\nDefaultCategories:\n-RecentlyPlaced\n-Basic\n-Lights\n-Visual\n-Volumes\n-AllClasses\n-Custom"), ReturnArguments);
					break;
				}
				
				if (TempPlaceableCategoryItems[i].CategoryName == ChangedItem.CategoryName)
				{
					bReturnItems = true;
					ReturnDialogMessage = FText::Format(NSLOCTEXT("CustomPlacementMode","DialogValueChange","Dublicate category names are not allowed.\nThe current category name will be returned to {0}"), ReturnArguments);
					break;
				}
			}

			if (bReturnItems)
			{
				FNotificationInfo InfoDublicateCategoryName(FText::Format(NSLOCTEXT("CustomPlacementMode","ValueChange","The name of the current category has been returned to {0}"), ReturnArguments));
				InfoDublicateCategoryName.Image = FEditorStyle::GetBrush(TEXT("Cascade.AddLODBeforeCurrent"));
				InfoDublicateCategoryName.FadeInDuration = 0.1f;
				InfoDublicateCategoryName.FadeOutDuration = 0.5f;
				InfoDublicateCategoryName.ExpireDuration = 1.5f;
				InfoDublicateCategoryName.bUseThrobber = false;
				InfoDublicateCategoryName.bUseSuccessFailIcons = true;
				InfoDublicateCategoryName.bUseLargeFont = true;
				InfoDublicateCategoryName.bFireAndForget = false;
				InfoDublicateCategoryName.bAllowThrottleWhenFrameRateIsLow = false;
				TSharedPtr<SNotificationItem> NotificationDeblicateCategoryName;

				FMessageDialog* DialogMsg = new FMessageDialog();

				if (DialogMsg)
				{
					FText Title = FText::FromString(TEXT("Error"));

					DialogMsg->Open(
						EAppMsgType::Ok,
						ReturnDialogMessage,
						&Title
					);
					
					int i = 0;

					for (auto& Item : PlaceableCategoryItems)
					{
						Item.CategoryName = TempPlaceableCategoryItems[i].CategoryName;
						i++;
					}

					if (!NotificationDeblicateCategoryName)
					{
						NotificationDeblicateCategoryName = FSlateNotificationManager::Get().AddNotification(InfoDublicateCategoryName);
					}

					NotificationDeblicateCategoryName->SetCompletionState(SNotificationItem::CS_Success);
					NotificationDeblicateCategoryName->ExpireAndFadeout();
				}
			}
		}
	}
	
	if (PropertyName == "Items")
	{
		FNotificationInfo Info(NSLOCTEXT("CustomPlacementMode", "ItemsChange", "All problem items has been resolved"));
		Info.Image = FEditorStyle::GetBrush(TEXT("Cascade.AddLODBeforeCurrent"));
		Info.FadeInDuration = 0.1f;
		Info.FadeOutDuration = 0.5f;
		Info.ExpireDuration = 1.5f;
		Info.bUseThrobber = false;
		Info.bUseSuccessFailIcons = true;
		Info.bUseLargeFont = true;
		Info.bFireAndForget = false;
		Info.bAllowThrottleWhenFrameRateIsLow = false;
		TSharedPtr<SNotificationItem> NotificationCategoryItem;

		int i = 0;
		
		for (auto& PlacementCategoryItem : PlaceableCategoryItems)
		{
			FSettingPlaceableCategoryItem SettingCategoryItem = PlacementCategoryItem;

			if (SettingCategoryItem.Items.Num() > 0)
			{
				FString Message;
				FMessageDialog* DialogMsg = nullptr;
				TSet<TSubclassOf<AActor>> NeedForDelete;

				for (auto& Item : SettingCategoryItem.Items)
				{
					for (auto& SortedItem : SettingCategoryItem.Items)
					{
						if (Item != SortedItem)
						{
							if (Item->IsChildOf(SortedItem))
							{
								TArray<FStringFormatArg> Arguments;
								Arguments.Add(FStringFormatArg(Item->GetName()));
								Arguments.Add(FStringFormatArg(SortedItem->GetName()));

								Message += FString::Format(TEXT("{0} is a child of {1}\n"), Arguments);
								NeedForDelete.Add(Item);
							}
						}
					}
				}

				if (!Message.IsEmpty())
				{
					Message += TEXT("All child Actors will be deleted\n");
					
					if (!DialogMsg)
					{
						DialogMsg = new FMessageDialog();
						FText Title = FText::FromString(TEXT("Error"));
						FText MessageText = FText::FromString(Message);
						if(DialogMsg->Open(EAppMsgType::OkCancel, MessageText, &Title) == EAppReturnType::Ok)
						{
							for (auto& ActorItem : NeedForDelete)
							{
								PlacementCategoryItem.Items.Remove(ActorItem);
								//PlacementCategoryItem.Items.Shrink();

								if (!NotificationCategoryItem)
								{
									NotificationCategoryItem = FSlateNotificationManager::Get().AddNotification(Info);
								}

								NotificationCategoryItem->SetCompletionState(SNotificationItem::CS_Success);
								NotificationCategoryItem->ExpireAndFadeout();
							}
						}
						else
						{
							PlacementCategoryItem.Items = TempPlaceableCategoryItems[i].Items;
							
							if (!NotificationCategoryItem)
							{
								NotificationCategoryItem = FSlateNotificationManager::Get().AddNotification(Info);
							}

							NotificationCategoryItem->SetCompletionState(SNotificationItem::CS_Success);
							NotificationCategoryItem->ExpireAndFadeout();
						}
					}
				}
			}
			i++;
		}
	}
}