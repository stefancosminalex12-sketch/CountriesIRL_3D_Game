// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Items/EquipmentSlot.h"

class SCIRLButton;

/**
 *  The Equipment tab (Master file > UI & HUD > Slot layout): the character in the middle, 8 small boxes around it,
 *  each split into two slots, an item card on the right and a weight / protection / warmth line under the character.
 *  Every half is its own button, so mouse, WASD/arrows and the controller move straight from slot to slot;
 *  the card describes whichever slot was highlighted last. Slots show a faint picture of what goes there while empty.
 */
class SCIRLEquipmentPage : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SCIRLEquipmentPage) {}
		/** What's shown in the middle (the live 3D character); a placeholder if not set */
		SLATE_NAMED_SLOT(FArguments, CharacterView)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Where keyboard/controller focus starts: the helmet */
	TSharedPtr<SWidget> GetFirstFocus() const;

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:

	/** A box with a small label above it and two slots side by side */
	TSharedRef<SWidget> MakeBox(const FText& Label, ECIRLEquipSlot Left, ECIRLEquipSlot Right);
	TSharedRef<SWidget> MakeHalf(ECIRLEquipSlot Slot);
	TSharedRef<SWidget> MakeItemCard();
	TSharedRef<SWidget> MakeStatsLine() const;

	static constexpr int32 SlotCount = static_cast<int32>(ECIRLEquipSlot::Count);

	TSharedPtr<SCIRLButton> Halves[SlotCount];

	/** The slot the card describes: the last one hovered or focused */
	ECIRLEquipSlot Selected = ECIRLEquipSlot::Helmet;

	/** Slot under the mouse / with focus last frame, to notice when either moves */
	int32 LastHovered = INDEX_NONE;
	int32 LastFocused = INDEX_NONE;
};
