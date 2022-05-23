// Copyright (c) 2021 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#if WITH_EDITOR
#    include "BlackBoxSettingsCustomization.h"
#    include "DetailLayoutBuilder.h"
#    include "DetailWidgetRow.h"
#    include "IDetailPropertyRow.h"
#    include "DetailCategoryBuilder.h"
#    include "Widgets/Layout/SBorder.h"
#    include "Widgets/Text/STextBlock.h"

TSharedRef<IDetailCustomization> FBlackBoxSettingsCustomization::MakeInstance()
{
    return MakeShareable(new FBlackBoxSettingsCustomization);
}

FBlackBoxSettingsCustomization::FBlackBoxSettingsCustomization()
{
}

void FBlackBoxSettingsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
    IDetailCategoryBuilder& SettingCategory = DetailLayout.EditCategory(TEXT("Settings"));
    SettingCategory.AddCustomRow(FText::FromString("Test"), false)
        .WholeRowWidget[SNew(SBorder).Padding(1)
                            [SNew(SHorizontalBox) +
                             SHorizontalBox::Slot()
                                 .Padding(FMargin(10, 10, 10, 10))
                                 .FillWidth(1.0f)[SNew(STextBlock)
                                                      .Text(FText::FromString(
                                                          "Settings input disabled. BlackBox is currently using "
                                                          "settings value from Config/BlackBox.ini file."))
                                                      .AutoWrapText(true)]]];
}
#endif