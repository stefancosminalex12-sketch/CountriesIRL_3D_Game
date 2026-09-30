// Crowns & Commoners

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Items/EquipmentSlot.h"
#include "ItemTypes.generated.h"

/** How a weapon hurts: armour stops some kinds better than others (used by combat) */
UENUM(BlueprintType)
enum class ECIRLDamageType : uint8
{
	None,
	Cut,
	Pierce,
	Blunt
};

/** The simple stand-in shape an item is shown as in the hand until its real model exists */
UENUM(BlueprintType)
enum class ECIRLItemShape : uint8
{
	None,
	Pole,		// long shaft with a head: bill, poleaxe, spear
	Staff,		// plain long shaft
	Blade,		// sword: grip, crossguard, blade
	Dagger,
	Club,
	Axe,
	Hammer,		// short shaft with a heavy head: war hammer, mace
	Bow,
	Crossbow,
	Shield,
	Buckler,
	Torch,
	Lantern
};

/**
 *  One kind of item: a row of the item table (Data/Items/*.csv -> DT_Items_*, Tools/Unreal/import_items.py).
 *  The row's name is the item's id, e.g. "helmet_kettle_hat". A DLC region adds items as another table.
 */
USTRUCT(BlueprintType)
struct FCIRLItemRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	FText Name;

	/** Short type line under the name, e.g. "Polearm, two hands" */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	FText Kind;

	/** A sentence or two of history */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	FText Description;

	/** Every slot the item may go in (a dagger: either belt half or either hand) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	TArray<ECIRLEquipSlot> Slots;

	/** In the main hand it needs both hands: the off hand can hold nothing */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	bool bTwoHanded = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
	float WeightKg = 0.f;

	/** How much it protects the part of the body it covers */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
	float Protection = 0.f;

	/** How warm it keeps you (weather and temperature) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
	float Warmth = 0.f;

	/** How much use it takes before it breaks, when new */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
	float Durability = 100.f;

	/** Weapons: damage of a good hit, and what kind */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
	float Damage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
	ECIRLDamageType DamageType = ECIRLDamageType::None;

	/** What it's worth new, in pence (12 pence = 1 shilling, 240 = 1 pound) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
	int32 ValuePence = 0;

	/** Icon texture name in /Game/CrownsAndCommoners/UI/Icons (without the T_ prefix) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	FName Icon;

	/** How long it is (cm): how far a weapon reaches, and the size of its stand-in shape */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
	float ReachCm = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	ECIRLItemShape Shape = ECIRLItemShape::None;

	bool FitsSlot(ECIRLEquipSlot Slot) const { return Slots.Contains(Slot); }
	bool IsWeapon() const { return Damage > 0.f; }

	/** Bows and crossbows shoot (later); they aren't swung */
	bool IsRanged() const { return Shape == ECIRLItemShape::Bow || Shape == ECIRLItemShape::Crossbow; }
};
