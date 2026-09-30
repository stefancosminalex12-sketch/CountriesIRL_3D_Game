// Crowns & Commoners

#include "UI/SCIRLDeathScreen.h"
#include "UI/CIRLUIStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Images/SImage.h"
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

	const FSlateBrush* Background = DeathBackground();
	const FSlateBrush* Emblem = DeathEmblem();
	auto Faded = [this](FLinearColor Color) { Color.A *= FadeIn(); return FSlateColor(Color); };

	ChildSlot
	[
		SNew(SOverlay)
		// Darkness, fading in over the fallen body: the world still shows through
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor_Lambda([Faded]() { return Faded(FLinearColor(0.f, 0.f, 0.f, 0.45f)); })
		]
		// The grim tomb-slab painting, once it has been made
		+ SOverlay::Slot()
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFill)
			.Visibility(Background ? EVisibility::HitTestInvisible : EVisibility::Collapsed)
			[
				SNew(SImage)
				.Image(Background)
				.ColorAndOpacity_Lambda([Faded]() { return Faded(FLinearColor(1.f, 1.f, 1.f, 0.55f)); })
			]
		]
		// Darker towards the edges
		+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(Vignette())
			.ColorAndOpacity_Lambda([Faded]() { return Faded(FLinearColor::White); })
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SVerticalBox)
			// Memento mori, once it has been made
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, 0.f, 0.f, 10.f)
			[
				SNew(SBox)
				.WidthOverride(280.f)
				.HeightOverride(280.f)
				.Visibility(Emblem ? EVisibility::HitTestInvisible : EVisibility::Collapsed)
				[
					SNew(SImage)
					.Image(Emblem)
					.ColorAndOpacity_Lambda([Faded]() { return Faded(FLinearColor::White); })
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, 0.f, 0.f, 48.f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Dead", "DEAD"))
				.Font(Font(EFont::TitleBold, 96.f))
				.ColorAndOpacity_Lambda([Faded]() { return Faded(FLinearColor(0.72f, 0.1f, 0.07f)); })
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, 0.f, 0.f, 14.f)
			[
				MakeMenuButton(LOCTEXT("Respawn", "RESPAWN"),
					FOnClicked::CreateLambda([OnRespawn]() { OnRespawn.ExecuteIfBound(); return FReply::Handled(); }), &RespawnButton)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				MakeMenuButton(LOCTEXT("MainMenu", "MAIN MENU"),
					FOnClicked::CreateLambda([OnMainMenu]() { OnMainMenu.ExecuteIfBound(); return FReply::Handled(); }))
			]
		]
	];
}

float SCIRLDeathScreen::FadeIn() const
{
	return FMath::Clamp(static_cast<float>(FSlateApplication::Get().GetCurrentTime() - ShownAt) / FadeSeconds, 0.f, 1.f);
}

TSharedPtr<SWidget> SCIRLDeathScreen::GetFocusTarget() const
{
	return RespawnButton;
}

#undef LOCTEXT_NAMESPACE
