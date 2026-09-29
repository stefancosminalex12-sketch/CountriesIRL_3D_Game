// CountriesIRL 3D Game

#include "UI/SCIRLGameMenu.h"
#include "UI/SCIRLButton.h"
#include "UI/CIRLUIStyle.h"
#include "UI/SCIRLEquipmentPage.h"
#include "UI/SCIRLCharacterPage.h"
#include "UI/SCIRLMapPage.h"
#include "UI/SCIRLPaperDollView.h"
#include "UI/SCIRLSettingsPanel.h"
#include "Audio/CIRLAudioSubsystem.h"
#include "GeneralProjectSettings.h"
#include "Styling/SlateTypes.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBackgroundBlur.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "CIRLGameMenu"

using namespace CIRLUIStyle;

namespace
{
	constexpr int32 TabCount = static_cast<int32>(ECIRLMenuTab::Count);

	FText TabName(ECIRLMenuTab Tab)
	{
		switch (Tab)
		{
		case ECIRLMenuTab::Map:			return LOCTEXT("TabMap", "Map");
		case ECIRLMenuTab::Quests:		return LOCTEXT("TabQuests", "Quests");
		case ECIRLMenuTab::Equipment:	return LOCTEXT("TabEquipment", "Equipment");
		case ECIRLMenuTab::Character:	return LOCTEXT("TabCharacter", "Character");
		case ECIRLMenuTab::Game:		return LOCTEXT("TabGame", "Game");
		default:						return FText::GetEmpty();
		}
	}

	/** Small gold diamond between the tabs */
	TSharedRef<SWidget> MakeDiamond()
	{
		return SNew(SBox)
			.WidthOverride(7.f)
			.HeightOverride(7.f)
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.Image(GoldFill())
				.RenderTransform(FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(45.f))))
				.RenderTransformPivot(FVector2D(0.5f, 0.5f))
			];
	}

	TSharedRef<SWidget> MakeKeyCap(const FText& Key)
	{
		return SNew(SBorder)
			.BorderImage(KeyCap())
			.Padding(FMargin(8.f, 1.f))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.MinDesiredWidth(14.f)
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Text(Key)
					.Font(Font(EFont::Title, 15.f))
					.ColorAndOpacity(Text())
				]
			];
	}
}

void SCIRLGameMenu::Construct(const FArguments& InArgs)
{
	OnCloseRequested = InArgs._OnCloseRequested;
	OnMainMenuRequested = InArgs._OnMainMenuRequested;
	Audio = InArgs._Audio;
	OnQuitRequested = InArgs._OnQuitRequested;

	SAssignNew(Pages, SWidgetSwitcher);
	for (int32 Index = 0; Index < TabCount; ++Index)
	{
		TSharedRef<SWidget> Page = SNullWidget::NullWidget;
		switch (static_cast<ECIRLMenuTab>(Index))
		{
		case ECIRLMenuTab::Map:
		{
			TSharedRef<SCIRLMapPage> MapPage = SNew(SCIRLMapPage).Player(InArgs._Player);
			TabFocus[Index] = MapPage;
			Page = MapPage;
			break;
		}
		case ECIRLMenuTab::Quests:
			Page = MakeComingSoon(LOCTEXT("QuestsSoonTitle", "No quests yet"),
				LOCTEXT("QuestsSoonNote", "The local lord has not sent for you... yet."));
			break;
		case ECIRLMenuTab::Equipment:
		{
			TSharedRef<SCIRLEquipmentPage> Equipment = SNew(SCIRLEquipmentPage)
				.CharacterView()
				[
					SNew(SCIRLPaperDollView).Stage(InArgs._PaperDollStage)
				];
			TabFocus[Index] = Equipment->GetFirstFocus();
			Page = Equipment;
			break;
		}
		case ECIRLMenuTab::Character:
		{
			TSharedRef<SCIRLCharacterPage> Character = SNew(SCIRLCharacterPage)
				.Stage(InArgs._PaperDollStage)
				.InitialArms(InArgs._CurrentArms)
				.OnArmsChosen(SCIRLCharacterPage::FOnArmsChosen::CreateLambda([Chosen = InArgs._OnArmsChosen](int32 ArmsIndex)
				{
					Chosen.ExecuteIfBound(ArmsIndex);
				}));
			TabFocus[Index] = Character->GetFirstFocus();
			Page = Character;
			break;
		}
		case ECIRLMenuTab::Game:
			Page = MakeGameTab();
			break;
		default:
			break;
		}
		Pages->AddSlot()[Page];
	}

	ChildSlot
	[
		SNew(SOverlay)

		// The paused game behind the menu: blurred and darkened
		+ SOverlay::Slot()
		[
			SNew(SBackgroundBlur)
			.BlurStrength(6.f)
			[
				SNew(SBorder).BorderImage(ScreenDim())
			]
		]

		// The panel keeps one size on every tab (1320 x 840 at 1080p) and shrinks to fit smaller or narrower screens
		+ SOverlay::Slot()
		.Padding(FMargin(60.f, 50.f))
		[
			SNew(SScaleBox)
			.Stretch(EStretch::ScaleToFit)
			.StretchDirection(EStretchDirection::DownOnly)
			[
			SNew(SBox)
			.WidthOverride(1320.f)
			.HeightOverride(840.f)
			[
				MakeOrnatePanel(
					SNew(SVerticalBox)

					+ SVerticalBox::Slot().AutoHeight()
					[
						MakeTabBar()
					]

					// Thin gold line under the tabs
					+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 8.f, 0.f, 18.f))
					[
						SNew(SBox).HeightOverride(1.f)
						[
							SNew(SImage).Image(GoldFill()).ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.5f))
						]
					]

					+ SVerticalBox::Slot().FillHeight(1.f)
					[
						Pages.ToSharedRef()
					]

					// Key hints, bottom right
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(FMargin(0.f, 14.f, 0.f, 0.f))
					[
						SAssignNew(KeyHints, SHorizontalBox)
					],
					FMargin(46.f, 18.f, 46.f, 20.f))
			]
			]
		]
	];

	SetTab(InArgs._InitialTab);

	// Focus the first button once the menu is on screen (focus can't move into widgets that aren't laid out yet)
	RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateLambda([this](double, float)
	{
		FocusCurrentTab();
		return EActiveTimerReturnType::Stop;
	}));
}

TSharedRef<SWidget> SCIRLGameMenu::MakeTabBar()
{
	TSharedRef<SHorizontalBox> Bar = SNew(SHorizontalBox);

	// Q at the far left, E at the far right, the tabs centred between them at their natural width
	Bar->AddSlot().AutoWidth().VAlign(VAlign_Center)[MakeKeyCap(LOCTEXT("KeyQ", "Q"))];
	Bar->AddSlot().FillWidth(1.f);
	for (int32 Index = 0; Index < TabCount; ++Index)
	{
		if (Index > 0)
		{
			Bar->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(14.f, 0.f))[MakeDiamond()];
		}
		Bar->AddSlot().AutoWidth().VAlign(VAlign_Center)[MakeTabButton(static_cast<ECIRLMenuTab>(Index))];
	}
	Bar->AddSlot().FillWidth(1.f);
	Bar->AddSlot().AutoWidth().VAlign(VAlign_Center)[MakeKeyCap(LOCTEXT("KeyE", "E"))];

	return Bar;
}

TSharedRef<SWidget> SCIRLGameMenu::MakeTabButton(ECIRLMenuTab Tab)
{
	TSharedPtr<SCIRLButton> Button;
	SAssignNew(Button, SCIRLButton)
	.ButtonStyle(&PlainButtonStyle())
	.IsFocusable(false)
	.OnClicked_Lambda([this, Tab]() { SetTab(Tab); FocusCurrentTab(); return FReply::Handled(); });

	// Open tab: bright gold with an underline; hovered: parchment; others: faded
	TWeakPtr<SCIRLButton> Weak = Button;
	// Open tab: bright gold text in a framed box; hovered: parchment; others: faded
	Button->SetContent(
		SNew(SBorder)
		.BorderImage_Lambda([this, Tab]() { return CurrentTab == Tab ? TabActive() : NoBrush(); })
		.Padding(FMargin(20.f, 6.f))
		[
			SNew(STextBlock)
			.Text(TabName(Tab))
			.Font(Font(EFont::Title, 21.f))
			.ColorAndOpacity_Lambda([this, Tab, Weak]()
			{
				if (CurrentTab == Tab)
				{
					return FSlateColor(GoldBright());
				}
				const TSharedPtr<SCIRLButton> Pinned = Weak.Pin();
				return FSlateColor(Pinned.IsValid() && Pinned->IsHovered() ? Text() : TextMuted());
			})
		]);

	return Button.ToSharedRef();
}

TSharedRef<SWidget> SCIRLGameMenu::MakeKeyHint(const FText& Key, const FText& Label) const
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[MakeKeyCap(Key)]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(8.f, 0.f, 26.f, 0.f))
		[
			SNew(STextBlock)
			.Text(Label)
			.Font(Font(EFont::Body, 18.f))
			.ColorAndOpacity(Text())
		];
}

TSharedRef<SWidget> SCIRLGameMenu::MakeComingSoon(const FText& Title, const FText& Note) const
{
	return SNew(SBox)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(Title)
				.Font(Font(EFont::Title, 30.f))
				.ColorAndOpacity(GoldBright())
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(FMargin(0.f, 12.f, 0.f, 0.f))
			[
				SNew(STextBlock)
				.Text(Note)
				.Font(Font(EFont::BodyItalic, 21.f))
				.ColorAndOpacity(TextMuted())
			]
		];
}

TSharedRef<SWidget> SCIRLGameMenu::MakeMenuButton(const FText& Label, FOnClicked OnClicked, TSharedPtr<SButton>* OutButton)
{
	TSharedPtr<SCIRLButton> Button;
	SAssignNew(Button, SCIRLButton)
	.ButtonStyle(&PlainButtonStyle())
	.OnClicked(OnClicked);

	TWeakPtr<SCIRLButton> Weak = Button;
	Button->SetContent(
		SNew(SBorder)
		.BorderImage_Lambda([Weak]()
		{
			const TSharedPtr<SCIRLButton> Pinned = Weak.Pin();
			if (!Pinned.IsValid() || !Pinned->IsHighlighted())
			{
				return ButtonNormal();
			}
			return Pinned->IsPressed() ? ButtonPressed() : ButtonHovered();
		})
		.Padding(FMargin(24.f, 10.f))
		[
			SNew(STextBlock)
			.Text(Label)
			.Font(Font(EFont::Title, 21.f))
			.ColorAndOpacity_Lambda([Weak]()
			{
				const TSharedPtr<SCIRLButton> Pinned = Weak.Pin();
				return FSlateColor(Pinned.IsValid() && Pinned->IsHighlighted() ? GoldBright() : Text());
			})
		]);

	if (OutButton)
	{
		*OutButton = Button;
	}
	return SNew(SBox).WidthOverride(360.f)[Button.ToSharedRef()];
}

TSharedRef<SWidget> SCIRLGameMenu::MakeGameTab()
{
	const UGeneralProjectSettings* Project = GetDefault<UGeneralProjectSettings>();
	const FText Version = FText::Format(LOCTEXT("Version", "Version {0}  ·  pre-alpha test build"), FText::FromString(Project->ProjectVersion));

	TSharedPtr<SButton> ResumeButton;
	TSharedRef<SWidget> Page =
		SNew(SHorizontalBox)

		// Left: title and the main buttons
		+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(20.f, 10.f, 60.f, 0.f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(Project->ProjectName))
				.Font(Font(EFont::TitleSemiBold, 44.f))
				.ColorAndOpacity(GoldBright())
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 2.f, 0.f, 30.f))
			[
				SNew(STextBlock)
				.Text(Version)
				.Font(Font(EFont::BodyItalic, 18.f))
				.ColorAndOpacity(TextMuted())
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 0.f, 0.f, 12.f))
			[
				MakeMenuButton(LOCTEXT("Resume", "Resume"),
					FOnClicked::CreateLambda([this]() { OnCloseRequested.ExecuteIfBound(); return FReply::Handled(); }), &ResumeButton)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 0.f, 0.f, 12.f))
			[
				MakeMenuButton(LOCTEXT("MainMenu", "Main Menu"),
					FOnClicked::CreateLambda([this]() { OnMainMenuRequested.ExecuteIfBound(); return FReply::Handled(); }))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 0.f, 0.f, 12.f))
			[
				MakeMenuButton(LOCTEXT("Settings", "Settings"),
					FOnClicked::CreateLambda([this]() { OpenSettings(); return FReply::Handled(); }), &SettingsButton)
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				MakeMenuButton(LOCTEXT("Quit", "Quit to Desktop"),
					FOnClicked::CreateLambda([this]() { OnQuitRequested.ExecuteIfBound(); return FReply::Handled(); }))
			]

		]

		// Right: the controls
		+ SHorizontalBox::Slot().FillWidth(1.f).Padding(FMargin(0.f, 10.f, 20.f, 0.f))
		[
			SNew(SBorder)
			.BorderImage(Card())
			.Padding(FMargin(28.f, 18.f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 0.f, 0.f, 14.f))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ControlsTitle", "Controls"))
					.Font(Font(EFont::Title, 26.f))
					.ColorAndOpacity(GoldBright())
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					MakeControlsList()
				]
			]
		];

	TabFocus[static_cast<int32>(ECIRLMenuTab::Game)] = ResumeButton;

	// The buttons, or the settings in their place
	return SNew(SWidgetSwitcher)
		.WidgetIndex_Lambda([this]() { return bSettingsOpen ? 1 : 0; })
		+ SWidgetSwitcher::Slot()[Page]
		+ SWidgetSwitcher::Slot()[MakeSettingsPage()];
}

TSharedRef<SWidget> SCIRLGameMenu::MakeSettingsPage()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(20.f, 10.f, 0.f, 26.f))
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SettingsTitle", "Settings"))
			.Font(Font(EFont::TitleSemiBold, 44.f))
			.ColorAndOpacity(GoldBright())
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(20.f, 0.f, 0.f, 0.f))
		[
			SAssignNew(SettingsPanel, SCIRLSettingsPanel)
			.Audio(Audio)
			.FontSize(22.f)
			.SliderWidth(300.f)
			.LabelWidth(200.f)
		]
		+ SVerticalBox::Slot().FillHeight(1.f)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(FMargin(20.f, 0.f, 0.f, 10.f))
		[
			MakeMenuButton(LOCTEXT("Back", "Back"), FOnClicked::CreateLambda([this]() { CloseSettings(); return FReply::Handled(); }))
		];
}

void SCIRLGameMenu::OpenSettings()
{
	bSettingsOpen = true;
	RebuildKeyHints();
	FocusCurrentTab();
}

void SCIRLGameMenu::CloseSettings()
{
	bSettingsOpen = false;
	RebuildKeyHints();
	if (SettingsButton.IsValid())
	{
		FSlateApplication::Get().SetAllUserFocus(SettingsButton, EFocusCause::SetDirectly);
	}
}

TSharedRef<SWidget> SCIRLGameMenu::MakeControlsList() const
{
	struct FControl { const TCHAR* Key; FText Action; };
	const FControl Controls[] =
	{
		{ TEXT("W A S D"),		LOCTEXT("CtrlMove", "Move (on a horse: steer where you look)") },
		{ TEXT("Mouse"),		LOCTEXT("CtrlLook", "Look around") },
		{ TEXT("Shift"),		LOCTEXT("CtrlRun", "Hold to run (uses stamina)") },
		{ TEXT("On a horse"),	LOCTEXT("CtrlRide", "W walks · Shift: tap trot, hold canter, double-tap + hold gallop") },
		{ TEXT("Space"),		LOCTEXT("CtrlJump", "Jump (the horse jumps too)") },
		{ TEXT("Left click"),	LOCTEXT("CtrlPunch", "Punch") },
		{ TEXT("Right click"),	LOCTEXT("CtrlGuard", "Hold to raise your guard") },
		{ TEXT("E"),			LOCTEXT("CtrlInteract", "Get on / off a horse") },
		{ TEXT("V"),			LOCTEXT("CtrlView", "First-person / third-person view") },
		{ TEXT("T"),			LOCTEXT("CtrlEmotion", "Change your eyes' emotion") },
		{ TEXT("M"),			LOCTEXT("CtrlMap", "Map") },
		{ TEXT("N"),			LOCTEXT("CtrlNextSong", "Next song") },
		{ TEXT("Tab  /  I"),	LOCTEXT("CtrlEquipment", "Equipment") },
		{ TEXT("Esc"),			LOCTEXT("CtrlMenu", "This menu") },
	};

	TSharedRef<SGridPanel> Grid = SNew(SGridPanel).FillColumn(1, 1.f);
	for (int32 Row = 0; Row < UE_ARRAY_COUNT(Controls); ++Row)
	{
		Grid->AddSlot(0, Row).Padding(FMargin(0.f, 4.f, 30.f, 4.f))
		[
			SNew(STextBlock)
			.Text(FText::FromString(Controls[Row].Key))
			.Font(Font(EFont::Title, 18.f))
			.ColorAndOpacity(GoldBright())
		];
		Grid->AddSlot(1, Row).Padding(FMargin(0.f, 4.f))
		[
			SNew(STextBlock)
			.Text(Controls[Row].Action)
			.Font(Font(EFont::Body, 19.f))
			.ColorAndOpacity(Text())
			// Long entries wrap onto a second line instead of being cut off at the card's edge
			.AutoWrapText(true)
		];
	}
	return Grid;
}

void SCIRLGameMenu::SetTab(ECIRLMenuTab NewTab)
{
	// Leaving the Game tab closes its settings, so it opens on its buttons next time
	if (NewTab != CurrentTab)
	{
		bSettingsOpen = false;
	}
	CurrentTab = NewTab;
	Pages->SetActiveWidgetIndex(static_cast<int32>(NewTab));
	RebuildKeyHints();
}

void SCIRLGameMenu::FocusCurrentTab()
{
	TSharedPtr<SWidget> Target = TabFocus[static_cast<int32>(CurrentTab)];
	if (CurrentTab == ECIRLMenuTab::Game && bSettingsOpen && SettingsPanel.IsValid())
	{
		Target = SettingsPanel->GetFirstFocus();
	}
	FSlateApplication::Get().SetAllUserFocus(Target.IsValid() ? Target : SharedThis(this), EFocusCause::SetDirectly);
}

void SCIRLGameMenu::RebuildKeyHints()
{
	KeyHints->ClearChildren();
	KeyHints->AddSlot().AutoWidth()[MakeKeyHint(LOCTEXT("HintQE", "Q / E"), LOCTEXT("HintTabs", "Switch Tab"))];
	if (CurrentTab == ECIRLMenuTab::Game)
	{
		KeyHints->AddSlot().AutoWidth()[MakeKeyHint(LOCTEXT("HintEnter", "Enter"), LOCTEXT("HintSelect", "Select"))];
	}
	else if (CurrentTab == ECIRLMenuTab::Map)
	{
		KeyHints->AddSlot().AutoWidth()[MakeKeyHint(LOCTEXT("HintDrag", "Drag / WASD"), LOCTEXT("HintMove", "Move"))];
		KeyHints->AddSlot().AutoWidth()[MakeKeyHint(LOCTEXT("HintWheel", "Wheel / + -"), LOCTEXT("HintZoom", "Zoom"))];
		KeyHints->AddSlot().AutoWidth()[MakeKeyHint(LOCTEXT("HintSpace", "Space"), LOCTEXT("HintFindMe", "Find Me"))];
	}
	else if (CurrentTab == ECIRLMenuTab::Equipment)
	{
		KeyHints->AddSlot().AutoWidth()[MakeKeyHint(LOCTEXT("HintEnterSlot", "Enter"), LOCTEXT("HintChoose", "Choose Item"))];
		KeyHints->AddSlot().AutoWidth()[MakeKeyHint(LOCTEXT("HintF", "F"), LOCTEXT("HintUnequip", "Unequip"))];
		KeyHints->AddSlot().AutoWidth()[MakeKeyHint(LOCTEXT("HintR", "R"), LOCTEXT("HintInspect", "Inspect"))];
	}
	const bool bBack = CurrentTab == ECIRLMenuTab::Game && bSettingsOpen;
	KeyHints->AddSlot().AutoWidth()[MakeKeyHint(LOCTEXT("HintEsc", "Esc"), bBack ? LOCTEXT("HintBack", "Back") : LOCTEXT("HintResume", "Resume"))];
}

FReply SCIRLGameMenu::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	const int32 Tab = static_cast<int32>(CurrentTab);

	if (Key == EKeys::Q || Key == EKeys::Gamepad_LeftShoulder)
	{
		SetTab(static_cast<ECIRLMenuTab>((Tab + TabCount - 1) % TabCount));
		FocusCurrentTab();
		return FReply::Handled();
	}
	if (Key == EKeys::E || Key == EKeys::Gamepad_RightShoulder)
	{
		SetTab(static_cast<ECIRLMenuTab>((Tab + 1) % TabCount));
		FocusCurrentTab();
		return FReply::Handled();
	}
	// Esc or controller B in the settings: back to the Game tab's buttons
	if (bSettingsOpen && CurrentTab == ECIRLMenuTab::Game && (Key == EKeys::Escape || Key == EKeys::Gamepad_FaceButton_Right))
	{
		CloseSettings();
		return FReply::Handled();
	}
	// Esc, controller B/Start, or a tab's own key again while on that tab (Tab/I Equipment, M Map): back to the game
	const bool bEquipmentKey = Key == EKeys::Tab || Key == EKeys::I || Key == EKeys::Gamepad_Special_Left;
	const bool bMapKey = Key == EKeys::M || Key == EKeys::Gamepad_DPad_Right;
	if (Key == EKeys::Escape || Key == EKeys::Gamepad_Special_Right || Key == EKeys::Gamepad_FaceButton_Right
		|| (bEquipmentKey && CurrentTab == ECIRLMenuTab::Equipment) || (bMapKey && CurrentTab == ECIRLMenuTab::Map))
	{
		OnCloseRequested.ExecuteIfBound();
		return FReply::Handled();
	}
	if (bEquipmentKey || bMapKey)
	{
		SetTab(bMapKey ? ECIRLMenuTab::Map : ECIRLMenuTab::Equipment);
		FocusCurrentTab();
		return FReply::Handled();
	}
	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

#undef LOCTEXT_NAMESPACE
