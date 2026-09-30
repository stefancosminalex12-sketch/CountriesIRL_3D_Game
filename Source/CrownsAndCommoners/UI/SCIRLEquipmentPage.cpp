// Crowns & Commoners

#include "UI/SCIRLEquipmentPage.h"
#include "UI/SCIRLButton.h"
#include "UI/CIRLUIStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "CIRLEquipmentPage"

using namespace CIRLUIStyle;

namespace
{
	/** Size of one slot (half a box) in 1080p pixels */
	constexpr float SlotSize = 72.f;
	/** Gap between the two halves of a box, and between the boxes in a column */
	constexpr float SlotHalfGap = 6.f;
	constexpr float SlotBoxSpacing = 30.f;
	/** Width of the item card on the right */
	constexpr float ItemCardWidth = 330.f;
	/** The faint ring drawn behind the character */
	constexpr float DollRingSize = 470.f;

	/** Empty slots show a faint, dark version of what goes there */
	const FLinearColor SlotSilhouetteTint(0.62f, 0.56f, 0.46f, 0.30f);

	TSharedRef<SWidget> MakeStatDiamond(float Size)
	{
		return SNew(SBox)
			.WidthOverride(Size)
			.HeightOverride(Size)
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.Image(GoldFill())
				.RenderTransform(FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(45.f))))
				.RenderTransformPivot(FVector2D(0.5f, 0.5f))
			];
	}

	TSharedRef<SWidget> MakeDivider(float Opacity)
	{
		return SNew(SBox).HeightOverride(1.f)
			[
				SNew(SImage).Image(GoldFill()).ColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, Opacity))
			];
	}
}

void SCIRLEquipmentPage::Construct(const FArguments& InArgs)
{
	using S = ECIRLEquipSlot;

	TSharedRef<SWidget> Character = InArgs._CharacterView.Widget != SNullWidget::NullWidget
		? InArgs._CharacterView.Widget
		: StaticCastSharedRef<SWidget>(SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("CharacterPlaceholder", "Your character"))
				.Font(Font(EFont::BodyItalic, 22.f))
				.ColorAndOpacity(TextMuted())
			]);

	auto Column = [this](const FText& A, S A1, S A2, const FText& B, S B1, S B2, const FText& C, S C1, S C2)
	{
		return SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()[MakeBox(A, A1, A2)]
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, SlotBoxSpacing))[MakeBox(B, B1, B2)]
			+ SVerticalBox::Slot().AutoHeight()[MakeBox(C, C1, C2)];
	};

	ChildSlot
	[
		SNew(SHorizontalBox)

		// Paper doll: the character in a faint ring, the boxes gathered closely around it
		+ SHorizontalBox::Slot().FillWidth(1.f)
		[
			SNew(SOverlay)

			+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(FMargin(0.f, 0.f, 0.f, 40.f))
			[
				SNew(SBox).WidthOverride(DollRingSize).HeightOverride(DollRingSize)
				[
					SNew(SImage).Image(DollRing())
				]
			]

			+ SOverlay::Slot().HAlign(HAlign_Center)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(0.f, 0.f, 0.f, 40.f))
				[
					Column(LOCTEXT("Weapons", "Weapons"), S::WeaponMain, S::WeaponOff,
						LOCTEXT("BackCloak", "Back / Cloak"), S::Back, S::Cloak,
						LOCTEXT("Belt", "Belt"), S::Belt1, S::Belt2)
				]

				+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(14.f, 0.f))
				[
					SNew(SBox).WidthOverride(430.f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
						[
							MakeBox(LOCTEXT("HelmetCoif", "Helmet / Coif"), S::Helmet, S::Coif)
						]
						+ SVerticalBox::Slot().FillHeight(1.f).Padding(FMargin(0.f, 4.f))
						[
							Character
						]
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
						[
							MakeBox(LOCTEXT("GlovesBoots", "Gloves / Boots"), S::Gloves, S::Boots)
						]
						+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(FMargin(0.f, 16.f, 0.f, 0.f))
						[
							MakeStatsLine()
						]
					]
				]

				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(0.f, 0.f, 0.f, 40.f))
				[
					Column(LOCTEXT("Clothing", "Clothing"), S::Gambeson, S::Tunic,
						LOCTEXT("Armour", "Armour"), S::Mail, S::Plate,
						LOCTEXT("Jewellery", "Jewellery"), S::Ring, S::Necklace)
				]
			]
		]

		// Item card
		+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(26.f, 0.f, 0.f, 0.f))
		[
			SNew(SBox)
			.WidthOverride(ItemCardWidth)
			[
				MakeItemCard()
			]
		]
	];
}

TSharedRef<SWidget> SCIRLEquipmentPage::MakeBox(const FText& Label, ECIRLEquipSlot Left, ECIRLEquipSlot Right)
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(FMargin(0.f, 0.f, 0.f, 6.f))
		[
			SNew(STextBlock)
			.Text(Label)
			.Font(Font(EFont::Title, 16.f))
			.ColorAndOpacity(Text())
		]

		// Two separate square slots side by side
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()[MakeHalf(Left)]
			+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(SlotHalfGap, 0.f, 0.f, 0.f))[MakeHalf(Right)]
		];
}

TSharedRef<SWidget> SCIRLEquipmentPage::MakeHalf(ECIRLEquipSlot Slot)
{
	TSharedPtr<SCIRLButton>& Button = Halves[static_cast<int32>(Slot)];
	SAssignNew(Button, SCIRLButton)
	.ButtonStyle(&PlainButtonStyle())
	.OnClicked_Lambda([this, Slot]() { Selected = Slot; return FReply::Handled(); });

	TWeakPtr<SCIRLButton> Weak = Button;
	Button->SetContent(
		SNew(SBox)
		.WidthOverride(SlotSize)
		.HeightOverride(SlotSize)
		[
			SNew(SBorder)
			// Highlighted: gold glow. Described by the card but not highlighted: a thin gold outline
			.BorderImage_Lambda([this, Weak, Slot]()
			{
				const TSharedPtr<SCIRLButton> Pinned = Weak.Pin();
				if (Pinned.IsValid() && Pinned->IsHighlighted())
				{
					return Pinned->IsPressed() ? ButtonPressed() : ButtonHovered();
				}
				return Selected == Slot ? SlotSelected() : ButtonNormal();
			})
			.Padding(FMargin(8.f))
			[
				SNew(SImage)
				.Image(ItemIcon(CIRLEquipSlot::SilhouetteIcon(Slot)))
				.ColorAndOpacity(SlotSilhouetteTint)
				.Visibility(ItemIcon(CIRLEquipSlot::SilhouetteIcon(Slot)) ? EVisibility::HitTestInvisible : EVisibility::Hidden)
			]
		]);

	return Button.ToSharedRef();
}

TSharedRef<SWidget> SCIRLEquipmentPage::MakeItemCard()
{
	// One stat row: small gold diamond, name, value on the right (a dash until items exist)
	auto StatRow = [](const FText& Label)
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(2.f, 0.f, 12.f, 0.f))[MakeStatDiamond(7.f)]
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(Label).Font(Font(EFont::Body, 19.f)).ColorAndOpacity(Text())
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(FText::FromString(TEXT("—"))).Font(Font(EFont::BodySemiBold, 19.f)).ColorAndOpacity(TextMuted())
			];
	};

	return SNew(SBorder)
		.BorderImage(Card())
		.Padding(FMargin(22.f, 18.f))
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text_Lambda([this]() { return CIRLEquipSlot::Name(Selected); })
				.Font(Font(EFont::Title, 28.f))
				.ColorAndOpacity(GoldBright())
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 0.f, 0.f, 10.f))
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Empty", "Nothing equipped"))
				.Font(Font(EFont::BodyItalic, 19.f))
				.ColorAndOpacity(TextMuted())
			]
			+ SVerticalBox::Slot().AutoHeight()[MakeDivider(0.4f)]

			// Big faint picture of what goes here
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(FMargin(0.f, 16.f))
			[
				SNew(SBox).WidthOverride(180.f).HeightOverride(180.f)
				[
					SNew(SImage)
					.Image_Lambda([this]() { return ItemIcon(CIRLEquipSlot::SilhouetteIcon(Selected)); })
					.ColorAndOpacity(SlotSilhouetteTint * FLinearColor(1.f, 1.f, 1.f, 1.4f))
				]
			]

			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text_Lambda([this]() { return CIRLEquipSlot::Holds(Selected); })
				.Font(Font(EFont::Body, 19.f))
				.ColorAndOpacity(Text())
				.AutoWrapText(true)
			]

			+ SVerticalBox::Slot().FillHeight(1.f)

			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 14.f, 0.f, 8.f))[MakeDivider(0.25f)]
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 4.f))[StatRow(LOCTEXT("Defense", "Defense"))]
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 4.f))[StatRow(LOCTEXT("ItemWeight", "Weight"))]
			+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 4.f))[StatRow(LOCTEXT("Durability", "Durability"))]
		];
}

TSharedRef<SWidget> SCIRLEquipmentPage::MakeStatsLine() const
{
	// Totals of what's worn; all zero until the item system exists (roadmap step 6)
	auto Stat = [](const FText& Label, const FText& Value)
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(Label).Font(Font(EFont::Body, 19.f)).ColorAndOpacity(Text())
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(8.f, 0.f, 0.f, 0.f))
			[
				SNew(STextBlock).Text(Value).Font(Font(EFont::BodySemiBold, 19.f)).ColorAndOpacity(GoldBright())
			];
	};

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()[Stat(LOCTEXT("Weight", "Weight"), LOCTEXT("WeightValue", "0 / 30 kg"))]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(16.f, 0.f))[MakeStatDiamond(6.f)]
		+ SHorizontalBox::Slot().AutoWidth()[Stat(LOCTEXT("Protection", "Protection"), LOCTEXT("ProtectionValue", "0"))]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(16.f, 0.f))[MakeStatDiamond(6.f)]
		+ SHorizontalBox::Slot().AutoWidth()[Stat(LOCTEXT("Warmth", "Warmth"), LOCTEXT("WarmthValue", "0"))];
}

TSharedPtr<SWidget> SCIRLEquipmentPage::GetFirstFocus() const
{
	return Halves[static_cast<int32>(ECIRLEquipSlot::Helmet)];
}

void SCIRLEquipmentPage::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// The card follows whatever changed last: the mouse moving onto a slot, or focus moving with the keys/controller
	int32 Hovered = INDEX_NONE;
	int32 Focused = INDEX_NONE;
	for (int32 Index = 0; Index < SlotCount; ++Index)
	{
		const TSharedPtr<SCIRLButton>& Half = Halves[Index];
		if (Half.IsValid())
		{
			Hovered = Half->IsHovered() ? Index : Hovered;
			Focused = Half->HasKeyboardFocus() ? Index : Focused;
		}
	}
	if (Hovered != INDEX_NONE && Hovered != LastHovered)
	{
		Selected = static_cast<ECIRLEquipSlot>(Hovered);
	}
	if (Focused != INDEX_NONE && Focused != LastFocused)
	{
		Selected = static_cast<ECIRLEquipSlot>(Focused);
	}
	LastHovered = Hovered;
	LastFocused = Focused;
}

#undef LOCTEXT_NAMESPACE
