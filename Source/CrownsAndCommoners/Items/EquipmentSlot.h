// CountriesIRL 3D Game

#pragma once

#include "CoreMinimal.h"
#include "EquipmentSlot.generated.h"

/**
 *  Where an item is worn or carried (Master file > UI & HUD > Slot layout). The Equipment screen shows them as
 *  8 boxes split in half; items will name the slot they go in.
 */
UENUM(BlueprintType)
enum class ECIRLEquipSlot : uint8
{
	Helmet,
	Coif,
	WeaponMain,
	WeaponOff,
	Back,
	Cloak,
	Belt1,
	Belt2,
	Gambeson,
	Tunic,
	Mail,
	Plate,
	Ring,
	Necklace,
	Gloves,
	Boots,
	Count UMETA(Hidden)
};

/** Display info for a slot */
namespace CIRLEquipSlot
{
	/** Short name, e.g. "Helmet" */
	FText Name(ECIRLEquipSlot Slot);

	/** What goes in it, e.g. "Kettle hat, sallet, armet, or a straw or felt hat." */
	FText Holds(ECIRLEquipSlot Slot);

	/** Icon (in /Game/CrownsAndCommoners/UI/Icons) drawn faintly while the slot is empty */
	FName SilhouetteIcon(ECIRLEquipSlot Slot);
}
