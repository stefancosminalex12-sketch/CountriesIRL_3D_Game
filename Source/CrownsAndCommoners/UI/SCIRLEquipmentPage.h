// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Items/EquipmentSlot.h"

class SCIRLButton;
class SVerticalBox;
class UCIRLInventoryComponent;
struct FCIRLItemRow;

/**
 *  The Equipment tab (Master file > UI & HUD > Slot layout): the character in the middle, 8 small boxes around it,
 *  each split into two slots, an item card on the right and a weight / protection / warmth line under the character.
 *  Every half is its own button, so mouse, WASD/arrows and the controller move straight from slot to slot;
 *  the card describes whichever slot was highlighted last: the item in it, or what goes there while it's empty.
 *  Click / Enter on a slot turns the card into the item picker (what you own that fits); F takes the item off.
 */
class SCIRLEquipmentPage : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SCIRLEquipmentPage) {}
		/** What's shown in the middle (the live 3D character); a placeholder if not set */
		SLATE_NAMED_SLOT(FArguments, CharacterView)
		/** Whose things these are */
		SLATE_ARGUMENT(TWeakObjectPtr<UCIRLInventoryComponent>, Inventory)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Where keyboard/controller focus starts: the helmet */
	TSharedPtr<SWidget> GetFirstFocus() const;

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

private:

	/** A box with a small label above it and two slots side by side */
	TSharedRef<SWidget> MakeBox(const FText& Label, ECIRLEquipSlot Left, ECIRLEquipSlot Right);
	TSharedRef<SWidget> MakeHalf(ECIRLEquipSlot Slot);
	TSharedRef<SWidget> MakeItemCard();
	TSharedRef<SWidget> MakeItemDetails();
	TSharedRef<SWidget> MakePicker();
	TSharedRef<SWidget> MakeStatsLine() const;

	/** The item in a slot, or nullptr. For a blocked off hand: the two-handed weapon using it, with bOutBlocked set */
	const FCIRLItemRow* ItemIn(ECIRLEquipSlot Slot, bool* bOutBlocked = nullptr) const;

	void OpenPicker(ECIRLEquipSlot Slot);
	void ClosePicker();
	void RebuildPickerList();
	void FocusLater(TWeakPtr<SWidget> Widget);

	static constexpr int32 SlotCount = static_cast<int32>(ECIRLEquipSlot::Count);

	TWeakObjectPtr<UCIRLInventoryComponent> Inventory;

	TSharedPtr<SCIRLButton> Halves[SlotCount];

	/** The slot the card describes: the last one hovered or focused */
	ECIRLEquipSlot Selected = ECIRLEquipSlot::Helmet;

	/** The card shows the list of items to choose from for the selected slot */
	bool bPickerOpen = false;
	TSharedPtr<SVerticalBox> PickerList;

	/** Slot under the mouse / with focus last frame, to notice when either moves */
	int32 LastHovered = INDEX_NONE;
	int32 LastFocused = INDEX_NONE;
};
