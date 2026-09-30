// Crowns & Commoners

#include "UI/SCIRLDeathScreen.h"
#include "UI/CIRLUIStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "CIRLDeathScreen"

using namespace CIRLUIStyle;

void SCIRLDeathScreen::Construct(const FArguments& InArgs)
{
	ShownAt = FSlateApplication::Get().GetCurrentTime();
	const FOnChosen OnRespawn = InArgs._OnRespawn;
	const FOnChosen OnMainMenu = InArgs._OnMainMenu;

	ChildSlot
	[
		SNew(SOverlay)
		// The world darkens, with a touch of red
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor_Lambda([this]() { return FSlateColor(FLinearColor(0.10f, 0.008f, 0.004f, 0.72f * FadeIn())); })
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("YouDied", "You died"))
				.Font(Font(EFont::TitleBold, 78.f))
				.ColorAndOpacity_Lambda([this]() { return FSlateColor(FLinearColor(0.85f, 0.16f, 0.1f, FadeIn())); })
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, 6.f, 0.f, 44.f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("BodyLies", "Your body lies where it fell."))
				.Font(Font(EFont::BodyItalic, 24.f))
				.ColorAndOpacity_Lambda([this]()
				{
					FLinearColor Color = TextMuted();
					Color.A *= FadeIn();
					return FSlateColor(Color);
				})
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, 0.f, 0.f, 14.f)
			[
				MakeMenuButton(LOCTEXT("Respawn", "Respawn"),
					FOnClicked::CreateLambda([OnRespawn]() { OnRespawn.ExecuteIfBound(); return FReply::Handled(); }), &RespawnButton)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				MakeMenuButton(LOCTEXT("MainMenu", "Main Menu"),
					FOnClicked::CreateLambda([OnMainMenu]() { OnMainMenu.ExecuteIfBound(); return FReply::Handled(); }))
			]
		]
	];
}

float SCIRLDeathScreen::FadeIn() const
{
	return FMath::Clamp(static_cast<float>(FSlateApplication::Get().GetCurrentTime() - ShownAt) / 1.2f, 0.f, 1.f);
}

TSharedPtr<SWidget> SCIRLDeathScreen::GetFocusTarget() const
{
	return RespawnButton;
}

#undef LOCTEXT_NAMESPACE
