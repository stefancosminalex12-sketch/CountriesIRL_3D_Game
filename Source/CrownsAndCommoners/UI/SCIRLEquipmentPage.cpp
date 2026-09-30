// Crowns & Commoners

#include "UI/SCIRLEquipmentPage.h"
#include "UI/SCIRLButton.h"
#include "UI/CIRLUIStyle.h"
#include "Items/CIRLInventoryComponent.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
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
	/** The off hand while a two-handed weapon uses it: that weapon, faded */
	const FLinearColor SlotBlockedTint(1.f, 1.f, 1.f, 0.32f);

	const FLinearColor Better(0.45f, 0.78f, 0.36f);
	const FLinearColor Worse(0.86f, 0.33f, 0.27f);

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

	FText Number(float Value, int32 Decimals = 0)
	{
		FNumberFormattingOptions Options;
		Options.MinimumFractionalDigits = Decimals;
		Options.MaximumFractionalDigits = Decimals;
		return FText::AsNumber(Value, &Options);
	}

	FText Dash()
	{
		return FText::FromString(TEXT("—"));
	}

	/** Pence as the money of 1455: pounds, shillings and pence (12 d = 1 s, 20 s = 1 pound) */
	FText Money(int32 Pence)
	{
		const int32 Pounds = Pence / 240;
		const int32 Shillings = (Pence % 240) / 12;
		const int32 Pennies = Pence % 12;
		TArray<FString> Parts;
		if (Pounds > 0)
		{
			Parts.Add(FString::Printf(TEXT("£%d"), Pounds));
		}
		if (Shillings > 0)
		{
			Parts.Add(FString::Printf(TEXT("%ds"), Shillings));
		}
		if (Pennies > 0 || Parts.Num() == 0)
		{
			Parts.Add(FString::Printf(TEXT("%dd"), Pennies));
		}
		return FText::FromString(FString::Join(Parts, TEXT(" ")));
	}

	/** The number that matters most for an item: a weapon's damage, else protection, else warmth */
	float MainStat(const FCIRLItemRow* Item)
	{
		if (!Item)
		{
			return 0.f;
		}
		return Item->IsWeapon() ? Item->Damage : (Item->Protection > 0.f ? Item->Protection : Item->Warmth);
	}

	FText MainStatText(const FCIRLItemRow& Item)
	{
		if (Item.IsWeapon())
		{
			return FText::Format(LOCTEXT("RowDamage", "Damage {0}"), Number(Item.Damage));
		}
		if (Item.Protection > 0.f)
		{
			return FText::Format(LOCTEXT("RowDefense", "Defense {0}"), Number(Item.Protection));
		}
		if (Item.Warmth > 0.f)
		{
			return FText::Format(LOCTEXT("RowWarmth", "Warmth {0}"), Number(Item.Warmth));
		}
		return FText::GetEmpty();
	}
}

void SCIRLEquipmentPage::Construct(const FArguments& InArgs)
{
	using S = ECIRLEquipSlot;
	Inventory = InArgs._Inventory;

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

const FCIRLItemRow* SCIRLEquipmentPage::ItemIn(ECIRLEquipSlot Slot, bool* bOutBlocked) const
{
	const UCIRLInventoryComponent* Things = Inventory.Get();
	if (bOutBlocked)
	{
		*bOutBlocked = false;
	}
	if (!Things)
	{
		return nullptr;
	}
	FName BlockedBy;
	if (Things->IsSlotBlocked(Slot, &BlockedBy))
	{
		if (bOutBlocked)
		{
			*bOutBlocked = true;
		}
		return Things->FindItem(BlockedBy);
	}
	return Things->FindItem(Things->GetEquipped(Slot));
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
	.OnClicked_Lambda([this, Slot]() { OpenPicker(Slot); return FReply::Handled(); });

	TWeakPtr<SCIRLButton> Weak = Button;
	Button->SetContent(
		SNew(SBox)
		.WidthOverride(SlotSize)
		.HeightOverride(SlotSize)
		[
			SNew(SBorder)
			// Highlighted: gold glow. Described by the card, or holding an item: a thin gold outline
			.BorderImage_Lambda([this, Weak, Slot]()
			{
				const TSharedPtr<SCIRLButton> Pinned = Weak.Pin();
				if (Pinned.IsValid() && Pinned->IsHighlighted())
				{
					return Pinned->IsPressed() ? ButtonPressed() : ButtonHovered();
				}
				bool bBlocked = false;
				const bool bHolds = ItemIn(Slot, &bBlocked) != nullptr && !bBlocked;
				return (Selected == Slot || bHolds) ? SlotSelected() : ButtonNormal();
			})
			.Padding(FMargin(8.f))
			[
				// The item in the slot; while empty, a faint picture of what goes there
				SNew(SImage)
				.Image_Lambda([this, Slot]()
				{
					const FCIRLItemRow* Item = ItemIn(Slot);
					const FSlateBrush* Icon = ItemIcon(Item ? Item->Icon : CIRLEquipSlot::SilhouetteIcon(Slot));
					return Icon ? Icon : NoBrush();
				})
				.ColorAndOpacity_Lambda([this, Slot]()
				{
					bool bBlocked = false;
					const FCIRLItemRow* Item = ItemIn(Slot, &bBlocked);
					return FSlateColor(!Item ? SlotSilhouetteTint : (bBlocked ? SlotBlockedTint : FLinearColor::White));
				})
				.Visibility(EVisibility::HitTestInvisible)
			]
		]);

	return Button.ToSharedRef();
}

TSharedRef<SWidget> SCIRLEquipmentPage::MakeItemCard()
{
	return SNew(SBorder)
		.BorderImage(Card())
		.Padding(FMargin(22.f, 18.f))
		[
			// What's in the slot, or the list of things to put in it
			SNew(SWidgetSwitcher)
			.WidgetIndex_Lambda([this]() { return bPickerOpen ? 1 : 0; })
			+ SWidgetSwitcher::Slot()[MakeItemDetails()]
			+ SWidgetSwitcher::Slot()[MakePicker()]
		];
}

TSharedRef<SWidget> SCIRLEquipmentPage::MakeItemDetails()
{
	// One stat row: small gold diamond, name, value on the right (a dash when it doesn't apply)
	auto StatRow = [](TAttribute<FText> Label, TAttribute<FText> Value)
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(2.f, 0.f, 12.f, 0.f))[MakeStatDiamond(7.f)]
			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(Label).Font(Font(EFont::Body, 18.f)).ColorAndOpacity(Text())
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(Value).Font(Font(EFont::BodySemiBold, 18.f)).ColorAndOpacity(GoldBright())
			];
	};
	// A value from the selected slot's item, or a dash
	auto ItemValue = [this](TFunction<FText(const FCIRLItemRow&)> Get)
	{
		return TAttribute<FText>::CreateLambda([this, Get]()
		{
			bool bBlocked = false;
			const FCIRLItemRow* Item = ItemIn(Selected, &bBlocked);
			return Item && !bBlocked ? Get(*Item) : Dash();
		});
	};

	return SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text_Lambda([this]()
			{
				bool bBlocked = false;
				const FCIRLItemRow* Item = ItemIn(Selected, &bBlocked);
				return Item && !bBlocked ? Item->Name : CIRLEquipSlot::Name(Selected);
			})
			.Font(Font(EFont::Title, 24.f))
			.ColorAndOpacity(GoldBright())
			.AutoWrapText(true)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 0.f, 0.f, 10.f))
		[
			SNew(STextBlock)
			.Text_Lambda([this]()
			{
				bool bBlocked = false;
				const FCIRLItemRow* Item = ItemIn(Selected, &bBlocked);
				if (Item && bBlocked)
				{
					return FText::Format(LOCTEXT("UsedBy", "Used by the {0}"), Item->Name);
				}
				return Item ? Item->Kind : LOCTEXT("Empty", "Nothing equipped");
			})
			.Font(Font(EFont::BodyItalic, 19.f))
			.ColorAndOpacity(TextMuted())
		]
		+ SVerticalBox::Slot().AutoHeight()[MakeDivider(0.4f)]

		// The item; while the slot is empty, a big faint picture of what goes here
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(FMargin(0.f, 10.f))
		[
			SNew(SBox).WidthOverride(140.f).HeightOverride(140.f)
			[
				SNew(SImage)
				.Image_Lambda([this]()
				{
					const FCIRLItemRow* Item = ItemIn(Selected);
					const FSlateBrush* Icon = ItemIcon(Item ? Item->Icon : CIRLEquipSlot::SilhouetteIcon(Selected));
					return Icon ? Icon : NoBrush();
				})
				.ColorAndOpacity_Lambda([this]()
				{
					bool bBlocked = false;
					const FCIRLItemRow* Item = ItemIn(Selected, &bBlocked);
					return FSlateColor(!Item ? SlotSilhouetteTint * FLinearColor(1.f, 1.f, 1.f, 1.4f) : (bBlocked ? SlotBlockedTint : FLinearColor::White));
				})
			]
		]

		// The description takes the space that's left and scrolls if it's longer than that: it never runs over the stats
		+ SVerticalBox::Slot().FillHeight(1.f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					bool bBlocked = false;
					const FCIRLItemRow* Item = ItemIn(Selected, &bBlocked);
					if (Item && bBlocked)
					{
						return LOCTEXT("TwoHands", "A two-handed weapon needs both hands, so the off hand holds nothing.");
					}
					return Item ? Item->Description : CIRLEquipSlot::Holds(Selected);
				})
				.Font(Font(EFont::Body, 17.f))
				.ColorAndOpacity(Text())
				.AutoWrapText(true)
			]
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 10.f, 0.f, 6.f))[MakeDivider(0.25f)]
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 2.f))
		[
			// Weapons show what they do; everything else what it stops
			StatRow(TAttribute<FText>::CreateLambda([this]()
				{
					const FCIRLItemRow* Item = ItemIn(Selected);
					return Item && Item->IsWeapon() ? LOCTEXT("Damage", "Damage") : LOCTEXT("Defense", "Defense");
				}),
				ItemValue([](const FCIRLItemRow& Item) { return Number(Item.IsWeapon() ? Item.Damage : Item.Protection); }))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 2.f))
		[
			StatRow(LOCTEXT("ItemWarmth", "Warmth"), ItemValue([](const FCIRLItemRow& Item) { return Number(Item.Warmth); }))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 2.f))
		[
			StatRow(LOCTEXT("ItemWeight", "Weight"), ItemValue([](const FCIRLItemRow& Item)
			{
				return FText::Format(LOCTEXT("Kg", "{0} kg"), Number(Item.WeightKg, Item.WeightKg < 10.f ? 1 : 0));
			}))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 2.f))
		[
			StatRow(LOCTEXT("Durability", "Durability"), ItemValue([](const FCIRLItemRow& Item) { return Number(Item.Durability); }))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 2.f))
		[
			StatRow(LOCTEXT("Value", "Value"), ItemValue([](const FCIRLItemRow& Item) { return Money(Item.ValuePence); }))
		];
}

TSharedRef<SWidget> SCIRLEquipmentPage::MakePicker()
{
	return SNew(SVerticalBox)

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
			.Text(LOCTEXT("PickerHint", "Choose what to put here"))
			.Font(Font(EFont::BodyItalic, 19.f))
			.ColorAndOpacity(TextMuted())
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 0.f, 0.f, 8.f))[MakeDivider(0.4f)]

		+ SVerticalBox::Slot().FillHeight(1.f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SAssignNew(PickerList, SVerticalBox)
			]
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.f, 10.f, 0.f, 0.f))
		[
			SNew(STextBlock)
			.Text(LOCTEXT("PickerKeys", "Enter: put on   ·   Esc: back"))
			.Font(Font(EFont::BodyItalic, 17.f))
			.ColorAndOpacity(TextMuted())
		];
}

void SCIRLEquipmentPage::RebuildPickerList()
{
	PickerList->ClearChildren();
	UCIRLInventoryComponent* Things = Inventory.Get();
	if (!Things)
	{
		return;
	}

	// One row: picture, name, and the number that matters (green if better than what's worn, red if worse)
	auto MakeRow = [this](const FSlateBrush* Icon, const FText& Name, const FText& Stat, const FLinearColor& StatColor, const FText& Weight, TFunction<void()> OnChosen)
	{
		TSharedPtr<SCIRLButton> Button;
		SAssignNew(Button, SCIRLButton)
		.ButtonStyle(&PlainButtonStyle())
		.OnClicked_Lambda([OnChosen]() { OnChosen(); return FReply::Handled(); });

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
			.Padding(FMargin(8.f, 6.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(SBox).WidthOverride(44.f).HeightOverride(44.f)
					[
						SNew(SImage).Image(Icon ? Icon : NoBrush())
					]
				]
				+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(FMargin(10.f, 0.f, 0.f, 0.f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(STextBlock).Text(Name).Font(Font(EFont::BodySemiBold, 18.f)).ColorAndOpacity(Text()).AutoWrapText(true)
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth()
						[
							SNew(STextBlock).Text(Stat).Font(Font(EFont::Body, 16.f)).ColorAndOpacity(StatColor)
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(FMargin(Stat.IsEmpty() ? 0.f : 12.f, 0.f, 0.f, 0.f))
						[
							SNew(STextBlock).Text(Weight).Font(Font(EFont::Body, 16.f)).ColorAndOpacity(TextMuted())
						]
					]
				]
			]);
		PickerList->AddSlot().AutoHeight().Padding(FMargin(0.f, 0.f, 0.f, 5.f))[Button.ToSharedRef()];
	};

	const ECIRLEquipSlot Slot = Selected;
	const FName WornId = Things->GetEquipped(Slot);
	const FCIRLItemRow* Worn = Things->FindItem(WornId);
	TWeakObjectPtr<UCIRLInventoryComponent> WeakThings = Things;

	if (Worn)
	{
		MakeRow(ItemIcon(Worn->Icon), FText::Format(LOCTEXT("TakeOff", "Take off: {0}"), Worn->Name), FText::GetEmpty(), TextMuted(), FText::GetEmpty(),
			[this, WeakThings, Slot]()
			{
				if (WeakThings.IsValid())
				{
					WeakThings->Unequip(Slot);
				}
				ClosePicker();
			});
	}

	TArray<FName> Choices;
	Things->GetChoicesForSlot(Slot, Choices);
	int32 Rows = 0;
	for (const FName& ItemId : Choices)
	{
		const FCIRLItemRow* Item = Things->FindItem(ItemId);
		if (!Item || ItemId == WornId)
		{
			continue;
		}
		// Compare like with like: only against a worn item of the same sort (weapon against weapon)
		FLinearColor StatColor = Text();
		if (Worn && Worn->IsWeapon() == Item->IsWeapon())
		{
			const float Difference = MainStat(Item) - MainStat(Worn);
			StatColor = Difference > 0.f ? Better : (Difference < 0.f ? Worse : Text());
		}
		MakeRow(ItemIcon(Item->Icon), Item->Name, MainStatText(*Item), StatColor,
			FText::Format(LOCTEXT("Kg", "{0} kg"), Number(Item->WeightKg, Item->WeightKg < 10.f ? 1 : 0)),
			[this, WeakThings, ItemId, Slot]()
			{
				if (WeakThings.IsValid())
				{
					WeakThings->Equip(ItemId, Slot);
				}
				ClosePicker();
			});
		++Rows;
	}

	if (Rows == 0)
	{
		PickerList->AddSlot().AutoHeight().Padding(FMargin(0.f, 8.f))
		[
			SNew(STextBlock)
			.Text(Worn ? LOCTEXT("NothingElse", "You own nothing else that goes here.") : LOCTEXT("NothingFits", "You own nothing that goes here."))
			.Font(Font(EFont::BodyItalic, 19.f))
			.ColorAndOpacity(TextMuted())
			.AutoWrapText(true)
		];
	}
}

void SCIRLEquipmentPage::OpenPicker(ECIRLEquipSlot Slot)
{
	Selected = Slot;
	const UCIRLInventoryComponent* Things = Inventory.Get();
	// The off hand can't take anything while a two-handed weapon uses it: the card says so instead
	if (!Things || Things->IsSlotBlocked(Slot))
	{
		return;
	}
	bPickerOpen = true;
	RebuildPickerList();
	if (PickerList->NumSlots() > 0)
	{
		FocusLater(PickerList->GetSlot(0).GetWidget());
	}
}

void SCIRLEquipmentPage::ClosePicker()
{
	if (!bPickerOpen)
	{
		return;
	}
	bPickerOpen = false;
	FocusLater(Halves[static_cast<int32>(Selected)]);
}

void SCIRLEquipmentPage::FocusLater(TWeakPtr<SWidget> Widget)
{
	// Focus can't go to a widget that isn't laid out yet (the list was just built, or the card just switched back)
	RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateLambda([Widget](double, float)
	{
		if (const TSharedPtr<SWidget> Pinned = Widget.Pin())
		{
			FSlateApplication::Get().SetAllUserFocus(Pinned, EFocusCause::SetDirectly);
		}
		return EActiveTimerReturnType::Stop;
	}));
}

FReply SCIRLEquipmentPage::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (bPickerOpen)
	{
		// Back to the slots (Esc would otherwise close the whole menu)
		if (Key == EKeys::Escape || Key == EKeys::Gamepad_FaceButton_Right)
		{
			ClosePicker();
			return FReply::Handled();
		}
	}
	else if (Key == EKeys::F || Key == EKeys::Gamepad_FaceButton_Left)
	{
		if (UCIRLInventoryComponent* Things = Inventory.Get())
		{
			Things->Unequip(Selected);
		}
		return FReply::Handled();
	}
	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

TSharedRef<SWidget> SCIRLEquipmentPage::MakeStatsLine() const
{
	// Totals: everything carried against what a person can carry, and what the worn gear adds up to
	auto Stat = [](const FText& Label, TAttribute<FText> Value, TAttribute<FSlateColor> Color)
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(Label).Font(Font(EFont::Body, 19.f)).ColorAndOpacity(Text())
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(8.f, 0.f, 0.f, 0.f))
			[
				SNew(STextBlock).Text(Value).Font(Font(EFont::BodySemiBold, 19.f)).ColorAndOpacity(Color)
			];
	};
	TWeakObjectPtr<UCIRLInventoryComponent> Things = Inventory;
	const TAttribute<FSlateColor> GoldValue = FSlateColor(GoldBright());

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()
		[
			Stat(LOCTEXT("Weight", "Weight"),
				TAttribute<FText>::CreateLambda([Things]()
				{
					return FText::Format(LOCTEXT("WeightValue", "{0} / {1} kg"),
						Number(Things.IsValid() ? Things->GetCarriedWeight() : 0.f, 1), Number(Things.IsValid() ? Things->GetMaxCarryWeight() : 30.f));
				}),
				// Red once you carry more than a person comfortably can
				TAttribute<FSlateColor>::CreateLambda([Things]()
				{
					return FSlateColor(Things.IsValid() && Things->GetCarriedWeight() > Things->GetMaxCarryWeight() ? Worse : GoldBright());
				}))
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(16.f, 0.f))[MakeStatDiamond(6.f)]
		+ SHorizontalBox::Slot().AutoWidth()
		[
			Stat(LOCTEXT("Protection", "Protection"),
				TAttribute<FText>::CreateLambda([Things]() { return Number(Things.IsValid() ? Things->GetProtection() : 0.f); }), GoldValue)
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(16.f, 0.f))[MakeStatDiamond(6.f)]
		+ SHorizontalBox::Slot().AutoWidth()
		[
			Stat(LOCTEXT("Warmth", "Warmth"),
				TAttribute<FText>::CreateLambda([Things]() { return Number(Things.IsValid() ? Things->GetWarmth() : 0.f); }), GoldValue)
		];
}

TSharedPtr<SWidget> SCIRLEquipmentPage::GetFirstFocus() const
{
	return Halves[static_cast<int32>(ECIRLEquipSlot::Helmet)];
}

void SCIRLEquipmentPage::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// While choosing an item the card stays on that slot
	if (bPickerOpen)
	{
		return;
	}

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
