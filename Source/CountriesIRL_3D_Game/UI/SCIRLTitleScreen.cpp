// CountriesIRL 3D Game

#include "UI/SCIRLTitleScreen.h"
#include "UI/SCIRLButton.h"
#include "UI/SCIRLSoundSettings.h"
#include "UI/CIRLUIStyle.h"
#include "Audio/CIRLAudioSubsystem.h"
#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"
#include "GeneralProjectSettings.h"
#include "Styling/SlateTypes.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "CIRLTitleScreen"

using namespace CIRLUIStyle;

namespace
{
	/** A painting that fills the whole screen, cropping the edges instead of stretching */
	TSharedRef<SWidget> MakeFullScreenPainting(const FSlateBrush* Painting)
	{
		return SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFill)
			[
				SNew(SImage).Image(Painting)
			];
	}

	/** Short true facts about 1455 England, shown while loading */
	FText LoadingTip(int32 Index)
	{
		const FText Tips[] =
		{
			LOCTEXT("Tip0", "The First Battle of St Albans, fought on 22 May 1455, is often counted as the start of the Wars of the Roses."),
			LOCTEXT("Tip1", "The white rose was a badge of the House of York. The name \"Wars of the Roses\" only became popular centuries later."),
			LOCTEXT("Tip2", "England used the Julian calendar in 1455, so every date in the game is a Julian date."),
			LOCTEXT("Tip3", "Most English soldiers of the time were archers and billmen, often in a padded jack and a kettle hat."),
			LOCTEXT("Tip4", "Richard Neville, Earl of Salisbury, held Middleham Castle in Yorkshire."),
			LOCTEXT("Tip5", "King Henry VI fell ill in 1453; Richard, Duke of York, ruled as Protector until early 1455."),
		};
		return Tips[Index % UE_ARRAY_COUNT(Tips)];
	}
}

void SCIRLTitleScreen::Construct(const FArguments& InArgs)
{
	OnNewGame = InArgs._OnNewGame;
	OnQuit = InArgs._OnQuit;
	Audio = InArgs._Audio;

	const UGeneralProjectSettings* Project = GetDefault<UGeneralProjectSettings>();

	TSharedRef<SWidget> Title =
		SNew(SOverlay)

		+ SOverlay::Slot()[MakeFullScreenPainting(TitleBackground())]

		// Darken the left side so the title and buttons read clearly over the painting
		+ SOverlay::Slot().HAlign(HAlign_Left)
		[
			SNew(SBox).WidthOverride(1150.f)
			[
				SNew(SImage).Image(GradientLeft()).ColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.88f))
			]
		]

		+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Center).Padding(FMargin(130.f, 0.f, 0.f, 40.f))
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(Project->ProjectName))
				.Font(Font(EFont::TitleSemiBold, 84.f))
				.ColorAndOpacity(GoldBright())
				.ShadowOffset(FVector2D(3.f, 3.f))
				.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.75f))
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(6.f, 0.f, 0.f, 0.f))
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Subtitle", "The Wars of the Roses  ·  England, 1455"))
				.Font(Font(EFont::BodyItalic, 32.f))
				.ColorAndOpacity(Text())
				.ShadowOffset(FVector2D(2.f, 2.f))
				.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.8f))
			]

			// Gold rule under the title
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(FMargin(6.f, 26.f, 0.f, 44.f))
			[
				SNew(SBox).WidthOverride(460.f).HeightOverride(1.5f)
				[
					SNew(SImage).Image(GoldFill())
				]
			]

			// The buttons, or the settings panel in their place
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SWidgetSwitcher)
				.WidgetIndex_Lambda([this]() { return bSettingsOpen ? 1 : 0; })

				+ SWidgetSwitcher::Slot()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						MakeTitleButton(LOCTEXT("NewGame", "New Game"), FText::GetEmpty(), true, [this]() { StartNewGame(); }, &NewGameButton)
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						MakeTitleButton(LOCTEXT("Continue", "Continue"), LOCTEXT("NoSave", "no saved game yet"), false, []() {})
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						MakeTitleButton(LOCTEXT("Settings", "Settings"), FText::GetEmpty(), true, [this]() { OpenSettings(); }, &SettingsButton)
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						MakeTitleButton(LOCTEXT("Quit", "Quit"), FText::GetEmpty(), true, [this]() { OnQuit.ExecuteIfBound(); })
					]
				]

				+ SWidgetSwitcher::Slot()
				[
					MakeSettingsPanel()
				]
			]
		]

		+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Bottom).Padding(FMargin(136.f, 0.f, 0.f, 40.f))
		[
			SNew(STextBlock)
			.Text(FText::Format(LOCTEXT("Version", "Version {0}  ·  pre-alpha test build"), FText::FromString(Project->ProjectVersion)))
			.Font(Font(EFont::BodyItalic, 19.f))
			.ColorAndOpacity(TextMuted())
		];

	ChildSlot
	[
		SNew(SOverlay)

		+ SOverlay::Slot()
		[
			SNew(SWidgetSwitcher)
			.WidgetIndex_Lambda([this]() { return bLoading ? 1 : 0; })
			+ SWidgetSwitcher::Slot()[Title]
			+ SWidgetSwitcher::Slot()[MakeLoadingScreen()]
		]

		// Fade in from black when the game starts
		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.Visibility(EVisibility::HitTestInvisible)
			.BorderImage(ScreenDim())
			.BorderBackgroundColor_Lambda([this]()
			{
				const float Alpha = 1.f - FMath::SmoothStep(0.f, 1.2f, Age);
				return FLinearColor(1.f, 1.f, 1.f, Alpha * 2.22f);	// ScreenDim is 45% black: x2.22 = fully black
			})
		]
	];

	RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateLambda([this](double, float DeltaTime)
	{
		Age += DeltaTime;
		return Age < 1.5f ? EActiveTimerReturnType::Continue : EActiveTimerReturnType::Stop;
	}));
	RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateLambda([this](double, float)
	{
		FocusFirstButton();
		return EActiveTimerReturnType::Stop;
	}));
}

TSharedRef<SWidget> SCIRLTitleScreen::MakeTitleButton(const FText& Label, const FText& DisabledNote, bool bEnabled, TFunction<void()> OnClicked, TSharedPtr<SCIRLButton>* OutButton)
{
	TSharedPtr<SCIRLButton> Button;
	SAssignNew(Button, SCIRLButton)
	.ButtonStyle(&PlainButtonStyle())
	.IsEnabled(bEnabled)
	.OnClicked_Lambda([OnClicked]() { OnClicked(); return FReply::Handled(); });

	TWeakPtr<SCIRLButton> Weak = Button;
	auto IsLit = [Weak]()
	{
		const TSharedPtr<SCIRLButton> Pinned = Weak.Pin();
		return Pinned.IsValid() && Pinned->IsEnabled() && Pinned->IsHighlighted();
	};

	Button->SetContent(
		SNew(SHorizontalBox)

		// Gold diamond marking the highlighted button
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(0.f, 0.f, 18.f, 0.f))
		[
			SNew(SBox).WidthOverride(10.f).HeightOverride(10.f)
			[
				SNew(SImage)
				.Image(GoldFill())
				.ColorAndOpacity_Lambda([IsLit]() { return FLinearColor(1.f, 1.f, 1.f, IsLit() ? 1.f : 0.f); })
				.RenderTransform(FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(45.f))))
				.RenderTransformPivot(FVector2D(0.5f, 0.5f))
			]
		]

		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(0.f, 7.f))
		[
			SNew(STextBlock)
			.Text(Label)
			.Font(Font(EFont::Title, 36.f))
			.ShadowOffset(FVector2D(2.f, 2.f))
			.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.8f))
			.ColorAndOpacity_Lambda([IsLit, bEnabled]()
			{
				if (!bEnabled)
				{
					return FSlateColor(Text() * FLinearColor(1.f, 1.f, 1.f, 0.45f));
				}
				return FSlateColor(IsLit() ? GoldBright() : Text());
			})
		]

		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(16.f, 6.f, 0.f, 0.f))
		[
			SNew(STextBlock)
			.Text(DisabledNote)
			.Font(Font(EFont::BodyItalic, 20.f))
			.ColorAndOpacity(Text() * FLinearColor(1.f, 1.f, 1.f, 0.6f))
			.ShadowOffset(FVector2D(1.f, 1.f))
			.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.8f))
			.Visibility(bEnabled || DisabledNote.IsEmpty() ? EVisibility::Collapsed : EVisibility::HitTestInvisible)
		]);

	if (OutButton)
	{
		*OutButton = Button;
	}
	return Button.ToSharedRef();
}

TSharedRef<SWidget> SCIRLTitleScreen::MakeLoadingScreen()
{
	const int32 Pick = FMath::RandHelper(LoadingPaintingCount());

	return SNew(SOverlay)

		+ SOverlay::Slot()[MakeFullScreenPainting(LoadingPainting(Pick))]

		// Darken the bottom for the text
		+ SOverlay::Slot().VAlign(VAlign_Bottom)
		[
			SNew(SBox).HeightOverride(360.f)
			[
				SNew(SImage).Image(GradientBottom()).ColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.85f))
			]
		]

		+ SOverlay::Slot().VAlign(VAlign_Bottom).Padding(FMargin(130.f, 0.f, 130.f, 56.f))
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Bottom).Padding(FMargin(0.f, 0.f, 80.f, 0.f))
			[
				SNew(STextBlock)
				.Text(LoadingTip(FMath::RandHelper(1000)))
				.Font(Font(EFont::BodyItalic, 26.f))
				.ColorAndOpacity(Text())
				.AutoWrapText(true)
				.ShadowOffset(FVector2D(2.f, 2.f))
				.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.8f))
			]

			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Bottom)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Loading", "Loading..."))
				.Font(Font(EFont::Title, 34.f))
				.ColorAndOpacity(GoldBright())
				.ShadowOffset(FVector2D(2.f, 2.f))
				.ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.8f))
			]
		];
}

void SCIRLTitleScreen::StartNewGame()
{
	if (bLoading)
	{
		return;
	}
	bLoading = true;

	// Loading the world blocks the game for a moment; wait until the loading painting has been drawn
	// so that's what stays on screen meanwhile
	RegisterActiveTimer(0.15f, FWidgetActiveTimerDelegate::CreateLambda([this](double, float)
	{
		OnNewGame.ExecuteIfBound();
		return EActiveTimerReturnType::Stop;
	}));
}

void SCIRLTitleScreen::FocusFirstButton()
{
	if (NewGameButton.IsValid())
	{
		FSlateApplication::Get().SetAllUserFocus(NewGameButton, EFocusCause::SetDirectly);
	}
}

TSharedRef<SWidget> SCIRLTitleScreen::MakeSettingsPanel()
{
	return SNew(SBorder)
		.BorderImage(Card())
		.Padding(FMargin(36.f, 26.f, 36.f, 22.f))
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SettingsTitle", "Settings"))
				.Font(Font(EFont::TitleSemiBold, 40.f))
				.ColorAndOpacity(GoldBright())
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 18.f, 0.f, 8.f))
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SoundHeading", "Sound"))
				.Font(Font(EFont::BodyItalic, 24.f))
				.ColorAndOpacity(TextMuted())
			]

			+ SVerticalBox::Slot().AutoHeight()
			[
				SAssignNew(SoundSettings, SCIRLSoundSettings).Audio(Audio)
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 22.f, 0.f, 0.f))
			[
				MakeTitleButton(LOCTEXT("Back", "Back"), FText::GetEmpty(), true, [this]() { CloseSettings(); })
			]
		];
}

void SCIRLTitleScreen::OpenSettings()
{
	bSettingsOpen = true;
	if (SoundSettings.IsValid() && SoundSettings->GetFirstFocus().IsValid())
	{
		FSlateApplication::Get().SetAllUserFocus(SoundSettings->GetFirstFocus(), EFocusCause::SetDirectly);
	}
}

void SCIRLTitleScreen::CloseSettings()
{
	if (!bSettingsOpen)
	{
		return;
	}
	bSettingsOpen = false;
	if (Audio.IsValid())
	{
		Audio->SaveVolumes();
	}
	if (SettingsButton.IsValid())
	{
		FSlateApplication::Get().SetAllUserFocus(SettingsButton, EFocusCause::SetDirectly);
	}
}

FReply SCIRLTitleScreen::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	// Esc / controller B leaves the settings panel
	if (bSettingsOpen && (InKeyEvent.GetKey() == EKeys::Escape || InKeyEvent.GetKey() == EKeys::Gamepad_FaceButton_Right))
	{
		CloseSettings();
		return FReply::Handled();
	}
	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

#undef LOCTEXT_NAMESPACE
