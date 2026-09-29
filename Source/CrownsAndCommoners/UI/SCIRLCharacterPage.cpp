// CountriesIRL 3D Game

#include "UI/SCIRLCharacterPage.h"
#include "UI/SCIRLButton.h"
#include "UI/SCIRLPaperDollView.h"
#include "UI/CIRLUIStyle.h"
#include "Characters/Heraldry.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "CIRLCharacterPage"

using namespace CIRLUIStyle;

void SCIRLCharacterPage::Construct(const FArguments& InArgs)
{
	OnArmsChosen = InArgs._OnArmsChosen;
	Chosen = FMath::Clamp(InArgs._InitialArms, 0, CIRLHeraldry::All().Num() - 1);

	TSharedRef<SVerticalBox> List = SNew(SVerticalBox);
	for (int32 Index = 0; Index < CIRLHeraldry::All().Num(); ++Index)
	{
		List->AddSlot().AutoHeight().Padding(FMargin(0.f, 0.f, 0.f, 6.f))[MakeHouseButton(Index)];
	}

	ChildSlot
	[
		SNew(SHorizontalBox)

		// Houses
		+ SHorizontalBox::Slot().AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(300.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 0.f, 0.f, 12.f))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("CoatOfArms", "Coat of Arms"))
					.Font(Font(EFont::Title, 24.f))
					.ColorAndOpacity(GoldBright())
				]
				+ SVerticalBox::Slot().FillHeight(1.f)
				[
					SNew(SScrollBox)
					+ SScrollBox::Slot()[List]
				]
			]
		]

		// The ball wearing the arms
		+ SHorizontalBox::Slot().FillWidth(1.f).Padding(FMargin(20.f, 0.f))
		[
			SNew(SOverlay)
			+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
			[
				SNew(SBox).WidthOverride(470.f).HeightOverride(470.f)
				[
					SNew(SImage).Image(DollRing())
				]
			]
			+ SOverlay::Slot()
			[
				SNew(SCIRLPaperDollView).Stage(InArgs._Stage)
			]
		]

		// About the house
		+ SHorizontalBox::Slot().AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(330.f)
			[
				MakeHouseCard()
			]
		]
	];
}

TSharedRef<SWidget> SCIRLCharacterPage::MakeHouseButton(int32 Index)
{
	const FCIRLArms& Arms = CIRLHeraldry::All()[Index];

	TSharedPtr<SCIRLButton> Button;
	SAssignNew(Button, SCIRLButton)
	.ButtonStyle(&PlainButtonStyle())
	.OnClicked_Lambda([this, Index]() { Choose(Index); return FReply::Handled(); });
	Buttons.Add(Button);

	TWeakPtr<SCIRLButton> Weak = Button;
	Button->SetContent(
		SNew(SBorder)
		.BorderImage_Lambda([this, Weak, Index]()
		{
			const TSharedPtr<SCIRLButton> Pinned = Weak.Pin();
			if (Pinned.IsValid() && Pinned->IsHighlighted())
			{
				return ButtonHovered();
			}
			return Chosen == Index ? SlotSelected() : ButtonNormal();
		})
		.Padding(FMargin(14.f, 7.f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(Arms.House)
				.Font(Font(EFont::Title, 19.f))
				.ColorAndOpacity_Lambda([this, Index]() { return FSlateColor(Chosen == Index ? GoldBright() : Text()); })
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(Arms.Side)
				.Font(Font(EFont::BodyItalic, 16.f))
				.ColorAndOpacity(TextMuted())
			]
		]);

	return Button.ToSharedRef();
}

TSharedRef<SWidget> SCIRLCharacterPage::MakeHouseCard()
{
	auto Current = [this]() -> const FCIRLArms& { return CIRLHeraldry::All()[Chosen]; };
	auto Heading = [](const FText& Label)
	{
		return SNew(STextBlock).Text(Label).Font(Font(EFont::Title, 16.f)).ColorAndOpacity(Text());
	};

	return SNew(SBorder)
		.BorderImage(Card())
		.Padding(FMargin(22.f, 18.f))
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text_Lambda([Current]() { return Current().House; })
				.Font(Font(EFont::Title, 28.f))
				.ColorAndOpacity(GoldBright())
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 0.f, 0.f, 12.f))
			[
				SNew(STextBlock)
				.Text_Lambda([Current]() { return Current().Side; })
				.Font(Font(EFont::BodyItalic, 19.f))
				.ColorAndOpacity(TextMuted())
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 0.f, 0.f, 14.f))
			[
				SNew(SBox).HeightOverride(1.f)
				[
					SNew(SImage).Image(GoldFill()).ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.4f))
				]
			]

			+ SVerticalBox::Slot().AutoHeight()[Heading(LOCTEXT("Arms", "Arms"))]
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 4.f, 0.f, 14.f))
			[
				SNew(STextBlock)
				.Text_Lambda([Current]() { return Current().Blazon; })
				.Font(Font(EFont::Body, 19.f))
				.ColorAndOpacity(Text())
				.AutoWrapText(true)
			]

			+ SVerticalBox::Slot().AutoHeight()[Heading(LOCTEXT("BorneBy", "Borne by"))]
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 4.f, 0.f, 0.f))
			[
				SNew(STextBlock)
				.Text_Lambda([Current]() { return Current().Holder; })
				.Font(Font(EFont::Body, 19.f))
				.ColorAndOpacity(Text())
				.AutoWrapText(true)
			]

			+ SVerticalBox::Slot().FillHeight(1.f)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("TryOn", "For now any ball can wear any arms, to try them out. Later your arms come from your house and your lord."))
				.Font(Font(EFont::BodyItalic, 16.f))
				.ColorAndOpacity(TextMuted())
				.AutoWrapText(true)
			]
		];
}

void SCIRLCharacterPage::Choose(int32 Index)
{
	Chosen = Index;
	OnArmsChosen.ExecuteIfBound(Index);
}

TSharedPtr<SWidget> SCIRLCharacterPage::GetFirstFocus() const
{
	return Buttons.IsValidIndex(Chosen) ? Buttons[Chosen] : nullptr;
}

#undef LOCTEXT_NAMESPACE
