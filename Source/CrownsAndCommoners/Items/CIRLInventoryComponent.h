// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/ItemTypes.h"
#include "CIRLInventoryComponent.generated.h"

/** Some number of one kind of item */
USTRUCT()
struct FCIRLItemStack
{
	GENERATED_BODY()

	UPROPERTY()
	FName ItemId;

	UPROPERTY()
	int32 Count = 1;
};

/**
 *  What a character owns and what it has on. Every ball has one (the player, and later villagers, soldiers and
 *  bandits get their gear the same way). Worn items stay in the owned list; a slot just names which one is in it.
 *  The Equipment screen reads and changes this; combat, weather and trade will read the totals.
 */
UCLASS()
class UCIRLInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	DECLARE_MULTICAST_DELEGATE(FOnChanged);

	/** Something was added, removed, put on or taken off */
	FOnChanged OnChanged;

	/** The item's data (name, stats...), or nullptr for an unknown id */
	const FCIRLItemRow* FindItem(FName ItemId) const;

	// --- Owning ---

	void AddItem(FName ItemId, int32 Count = 1);

	/** Removes up to Count; takes the item off if no copy is left to wear. Returns how many were removed */
	int32 RemoveItem(FName ItemId, int32 Count = 1);

	int32 CountOf(FName ItemId) const;
	const TArray<FCIRLItemStack>& GetItems() const { return Items; }

	// --- Wearing ---

	/** What's in a slot (NAME_None if empty) */
	FName GetEquipped(ECIRLEquipSlot Slot) const { return Equipped[static_cast<int32>(Slot)]; }

	/** The off hand is taken while the main hand holds a two-handed weapon. OutBy = that weapon */
	bool IsSlotBlocked(ECIRLEquipSlot Slot, FName* OutBy = nullptr) const;

	/** Owns a copy that isn't in another slot, it fits the slot, and the slot isn't blocked */
	bool CanEquip(FName ItemId, ECIRLEquipSlot Slot) const;

	/** Puts the item in the slot (whatever was there goes back to the pack). A two-handed weapon empties the off hand */
	bool Equip(FName ItemId, ECIRLEquipSlot Slot);

	void Unequip(ECIRLEquipSlot Slot);

	/** Puts the item in the first free slot it fits; false if there is none */
	bool EquipInFreeSlot(FName ItemId);

	/** Owned items that could go in this slot right now (not counting copies worn elsewhere), in pack order */
	void GetChoicesForSlot(ECIRLEquipSlot Slot, TArray<FName>& OutItems) const;

	/** Owns and wears exactly what Other does (the Equipment screen's character copies the player) */
	void CopyFrom(const UCIRLInventoryComponent& Other);

	// --- Totals ---

	/** Everything owned, worn or not (kg) */
	float GetCarriedWeight() const;
	float GetMaxCarryWeight() const;

	/** Carried weight against what a person can carry: 0 = nothing, 1 = a full load, more = overloaded */
	float GetLoadRatio() const;

	/** Sums over what's worn */
	float GetProtection() const;
	float GetWarmth() const;

	/**
	 *  How well the gear in these slots stops this kind of blow. Layers add up, and each kind of armour is better
	 *  against some blows than others: mail stops cuts but not hammers, padding soaks up blows but not points.
	 */
	float GetArmour(TConstArrayView<ECIRLEquipSlot> Slots, ECIRLDamageType DamageType) const;

private:

	/** Copies of an item sitting in slots other than ExceptSlot */
	int32 CountEquipped(FName ItemId, int32 ExceptSlot = INDEX_NONE) const;

	static constexpr int32 SlotCount = static_cast<int32>(ECIRLEquipSlot::Count);

	UPROPERTY()
	TArray<FCIRLItemStack> Items;

	FName Equipped[SlotCount];
};
