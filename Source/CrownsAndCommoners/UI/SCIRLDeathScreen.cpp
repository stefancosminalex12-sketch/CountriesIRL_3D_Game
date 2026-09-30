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
	// The background darkens at the pace it had over the first seconds, and keeps going up to MaxDarkness (the world
	// always shows through a little)
	auto Darkness = [this]() { return FLinearColor(1.f, 1.f, 1.f, FMath::Min(Elapsed() / FadeSeconds * 0.45f, MaxDarkness)); };

	ChildSlot
	[
		SNew(SOverlay)
		// Darkness over the fallen body: black with the grim tomb-slab painting on it, see-through at first,
		// 90% after about 6 seconds
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(NoBrush())
			.Padding(0.f)
			.ColorAndOpacity_Lambda(Darkness)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SImage)
					.Image(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.ColorAndOpacity(FLinearColor::Black)
				]
				+ SOverlay::Slot()
				[
					SNew(SScaleBox)
					.Stretch(EStretch::ScaleToFill)
					.Visibility(Background ? EVisibility::HitTestInvisible : EVisibility::Collapsed)
					[
						SNew(SImage)
						.Image(Background)
					]
				]
			]
		]
		// A faint wash of blood red over everything
		+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.ColorAndOpacity_Lambda([Faded]() { return Faded(FLinearColor(0.55f, 0.02f, 0.01f, 0.16f)); })
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

float SCIRLDeathScreen::Elapsed() const
{
	return static_cast<float>(FSlateApplication::Get().GetCurrentTime() - ShownAt);
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
