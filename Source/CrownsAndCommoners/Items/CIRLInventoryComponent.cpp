// Crowns & Commoners

#include "Items/CIRLInventoryComponent.h"
#include "Items/CIRLItemDatabase.h"
#include "Items/CIRLItemSettings.h"

const FCIRLItemRow* UCIRLInventoryComponent::FindItem(FName ItemId) const
{
	const UCIRLItemDatabase* Database = UCIRLItemDatabase::Get(this);
	return Database && !ItemId.IsNone() ? Database->Find(ItemId) : nullptr;
}

void UCIRLInventoryComponent::AddItem(FName ItemId, int32 Count)
{
	if (Count <= 0 || !FindItem(ItemId))
	{
		return;
	}
	if (FCIRLItemStack* Stack = Items.FindByPredicate([ItemId](const FCIRLItemStack& S) { return S.ItemId == ItemId; }))
	{
		Stack->Count += Count;
	}
	else
	{
		FCIRLItemStack NewStack;
		NewStack.ItemId = ItemId;
		NewStack.Count = Count;
		Items.Add(NewStack);
	}
	OnChanged.Broadcast();
}

int32 UCIRLInventoryComponent::RemoveItem(FName ItemId, int32 Count)
{
	const int32 Index = Items.IndexOfByPredicate([ItemId](const FCIRLItemStack& S) { return S.ItemId == ItemId; });
	if (Index == INDEX_NONE || Count <= 0)
	{
		return 0;
	}

	const int32 Removed = FMath::Min(Count, Items[Index].Count);
	Items[Index].Count -= Removed;
	const int32 Left = Items[Index].Count;
	if (Left <= 0)
	{
		Items.RemoveAt(Index);
	}
	// No copy left for a slot that was wearing one: it comes off
	for (int32 Slot = SlotCount - 1; Slot >= 0 && CountEquipped(ItemId) > Left; --Slot)
	{
		if (Equipped[Slot] == ItemId)
		{
			Equipped[Slot] = NAME_None;
		}
	}
	OnChanged.Broadcast();
	return Removed;
}

int32 UCIRLInventoryComponent::CountOf(FName ItemId) const
{
	const FCIRLItemStack* Stack = Items.FindByPredicate([ItemId](const FCIRLItemStack& S) { return S.ItemId == ItemId; });
	return Stack ? Stack->Count : 0;
}

int32 UCIRLInventoryComponent::CountEquipped(FName ItemId, int32 ExceptSlot) const
{
	int32 Count = 0;
	for (int32 Slot = 0; Slot < SlotCount; ++Slot)
	{
		Count += (Slot != ExceptSlot && Equipped[Slot] == ItemId) ? 1 : 0;
	}
	return Count;
}

bool UCIRLInventoryComponent::IsSlotBlocked(ECIRLEquipSlot Slot, FName* OutBy) const
{
	if (Slot != ECIRLEquipSlot::WeaponOff)
	{
		return false;
	}
	const FName Main = GetEquipped(ECIRLEquipSlot::WeaponMain);
	const FCIRLItemRow* MainItem = FindItem(Main);
	if (MainItem && MainItem->bTwoHanded)
	{
		if (OutBy)
		{
			*OutBy = Main;
		}
		return true;
	}
	return false;
}

bool UCIRLInventoryComponent::CanEquip(FName ItemId, ECIRLEquipSlot Slot) const
{
	const FCIRLItemRow* Item = FindItem(ItemId);
	return Item && Item->FitsSlot(Slot) && !IsSlotBlocked(Slot)
		&& CountOf(ItemId) > CountEquipped(ItemId, static_cast<int32>(Slot));
}

bool UCIRLInventoryComponent::Equip(FName ItemId, ECIRLEquipSlot Slot)
{
	if (!CanEquip(ItemId, Slot))
	{
		return false;
	}
	Equipped[static_cast<int32>(Slot)] = ItemId;

	// A two-handed weapon in the main hand leaves nothing for the off hand
	if (Slot == ECIRLEquipSlot::WeaponMain && FindItem(ItemId)->bTwoHanded)
	{
		Equipped[static_cast<int32>(ECIRLEquipSlot::WeaponOff)] = NAME_None;
	}
	OnChanged.Broadcast();
	return true;
}

void UCIRLInventoryComponent::Unequip(ECIRLEquipSlot Slot)
{
	FName& InSlot = Equipped[static_cast<int32>(Slot)];
	if (!InSlot.IsNone())
	{
		InSlot = NAME_None;
		OnChanged.Broadcast();
	}
}

bool UCIRLInventoryComponent::EquipInFreeSlot(FName ItemId)
{
	const FCIRLItemRow* Item = FindItem(ItemId);
	if (!Item)
	{
		return false;
	}
	for (const ECIRLEquipSlot Slot : Item->Slots)
	{
		if (GetEquipped(Slot).IsNone() && Equip(ItemId, Slot))
		{
			return true;
		}
	}
	return false;
}

void UCIRLInventoryComponent::GetChoicesForSlot(ECIRLEquipSlot Slot, TArray<FName>& OutItems) const
{
	OutItems.Reset();
	for (const FCIRLItemStack& Stack : Items)
	{
		const FCIRLItemRow* Item = FindItem(Stack.ItemId);
		if (Item && Item->FitsSlot(Slot) && Stack.Count > CountEquipped(Stack.ItemId, static_cast<int32>(Slot)))
		{
			OutItems.Add(Stack.ItemId);
		}
	}
}

void UCIRLInventoryComponent::CopyFrom(const UCIRLInventoryComponent& Other)
{
	Items = Other.Items;
	for (int32 Slot = 0; Slot < SlotCount; ++Slot)
	{
		Equipped[Slot] = Other.Equipped[Slot];
	}
	OnChanged.Broadcast();
}

float UCIRLInventoryComponent::GetCarriedWeight() const
{
	float Weight = 0.f;
	// Test builds hand the player one of everything: only what's worn counts then, or nobody could move
	if (GetDefault<UCIRLItemSettings>()->bGiveAllItemsForTesting)
	{
		for (int32 Slot = 0; Slot < SlotCount; ++Slot)
		{
			if (const FCIRLItemRow* Item = FindItem(Equipped[Slot]))
			{
				Weight += Item->WeightKg;
			}
		}
		return Weight;
	}
	for (const FCIRLItemStack& Stack : Items)
	{
		if (const FCIRLItemRow* Item = FindItem(Stack.ItemId))
		{
			Weight += Item->WeightKg * Stack.Count;
		}
	}
	return Weight;
}

float UCIRLInventoryComponent::GetLoadRatio() const
{
	return GetCarriedWeight() / FMath::Max(GetMaxCarryWeight(), 1.f);
}

float UCIRLInventoryComponent::GetArmour(TConstArrayView<ECIRLEquipSlot> Slots, ECIRLDamageType DamageType) const
{
	float Total = 0.f;
	for (const ECIRLEquipSlot Slot : Slots)
	{
		const FCIRLItemRow* Item = FindItem(GetEquipped(Slot));
		if (!Item)
		{
			continue;
		}
		// How each layer does against a cut / a point / a blunt blow
		float Cut = 1.f, Pierce = 1.f, Blunt = 1.f;
		switch (Slot)
		{
		case ECIRLEquipSlot::Mail:		Cut = 1.4f; Pierce = 0.7f; Blunt = 0.5f; break;
		case ECIRLEquipSlot::Gambeson:
		case ECIRLEquipSlot::Coif:		Cut = 0.9f; Pierce = 0.6f; Blunt = 1.3f; break;
		case ECIRLEquipSlot::Plate:
		case ECIRLEquipSlot::Helmet:
		case ECIRLEquipSlot::Gloves:
		case ECIRLEquipSlot::Boots:		Cut = 1.2f; Pierce = 1.f; Blunt = 0.8f; break;
		default: break;
		}
		const float Against = DamageType == ECIRLDamageType::Cut ? Cut : (DamageType == ECIRLDamageType::Pierce ? Pierce : Blunt);
		Total += Item->Protection * Against;
	}
	return Total;
}

float UCIRLInventoryComponent::GetMaxCarryWeight() const
{
	return GetDefault<UCIRLItemSettings>()->MaxCarryWeightKg;
}

float UCIRLInventoryComponent::GetProtection() const
{
	float Total = 0.f;
	for (int32 Slot = 0; Slot < SlotCount; ++Slot)
	{
		// Things hung on the belt or the back don't protect; a shield does once it's in the hand
		const bool bCarried = Slot == static_cast<int32>(ECIRLEquipSlot::Belt1) || Slot == static_cast<int32>(ECIRLEquipSlot::Belt2)
			|| Slot == static_cast<int32>(ECIRLEquipSlot::Back);
		if (const FCIRLItemRow* Item = bCarried ? nullptr : FindItem(Equipped[Slot]))
		{
			Total += Item->Protection;
		}
	}
	return Total;
}

float UCIRLInventoryComponent::GetWarmth() const
{
	float Total = 0.f;
	for (int32 Slot = 0; Slot < SlotCount; ++Slot)
	{
		if (const FCIRLItemRow* Item = FindItem(Equipped[Slot]))
		{
			Total += Item->Warmth;
		}
	}
	return Total;
}
