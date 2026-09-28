// CountriesIRL 3D Game

#include "Items/EquipmentSlot.h"

#define LOCTEXT_NAMESPACE "CIRLEquipSlot"

namespace CIRLEquipSlot
{
	namespace
	{
		struct FSlotInfo
		{
			FText Name;
			FText Holds;
			const TCHAR* Silhouette;
		};

		const FSlotInfo& Info(ECIRLEquipSlot Slot)
		{
			static const FSlotInfo Slots[] =
			{
				{ LOCTEXT("Helmet", "Helmet"), LOCTEXT("HelmetHolds", "Kettle hat, sallet, armet, or a straw or felt hat."), TEXT("icon_helmet_kettle_hat") },
				{ LOCTEXT("Coif", "Coif"), LOCTEXT("CoifHolds", "Linen coif, padded arming cap or a wool hood, worn under the helmet."), TEXT("icon_coif_linen") },
				{ LOCTEXT("WeaponMain", "Main Hand"), LOCTEXT("WeaponMainHolds", "Bill, poleaxe, sword, bow, crossbow, spear, axe or mace."), TEXT("icon_weapon_bill") },
				{ LOCTEXT("WeaponOff", "Off Hand"), LOCTEXT("WeaponOffHolds", "Buckler, dagger, torch or lantern. Two-handed weapons need both hands."), TEXT("icon_offhand_buckler") },
				{ LOCTEXT("Back", "Back"), LOCTEXT("BackHolds", "A shield, pavise or bow carried on the back."), TEXT("icon_back_heater_shield") },
				{ LOCTEXT("Cloak", "Cloak"), LOCTEXT("CloakHolds", "Wool cloak against wind, rain and cold."), TEXT("icon_cloak_wool") },
				{ LOCTEXT("Belt1", "Belt"), LOCTEXT("Belt1Holds", "Dagger, knife, sword, arrow bag, purse or torch."), TEXT("icon_belt_rondel_dagger") },
				{ LOCTEXT("Belt2", "Belt"), LOCTEXT("Belt2Holds", "Dagger, knife, sword, arrow bag, purse or torch."), TEXT("icon_belt_torch") },
				{ LOCTEXT("Gambeson", "Gambeson"), LOCTEXT("GambesonHolds", "Padded jack or arming doublet: the padding worn under armour."), TEXT("icon_gambeson_padded_jack") },
				{ LOCTEXT("Tunic", "Tunic"), LOCTEXT("TunicHolds", "Livery jacket, tabard or a plain wool tunic, worn on top."), TEXT("icon_tunic_plain_wool") },
				{ LOCTEXT("Mail", "Mail"), LOCTEXT("MailHolds", "Mail shirt or mail skirt."), TEXT("icon_mail_shirt") },
				{ LOCTEXT("Plate", "Plate"), LOCTEXT("PlateHolds", "Brigandine, jack of plates, breastplate or a full harness."), TEXT("icon_plate_brigandine") },
				{ LOCTEXT("Ring", "Ring"), LOCTEXT("RingHolds", "A signet ring (nobles seal letters and orders with it) or a plain ring."), TEXT("icon_ring_signet") },
				{ LOCTEXT("Necklace", "Necklace"), LOCTEXT("NecklaceHolds", "A livery collar that shows whose side you are on, or a pendant."), TEXT("icon_necklace_ss_collar") },
				{ LOCTEXT("Gloves", "Gloves"), LOCTEXT("GlovesHolds", "Leather gloves or plate gauntlets."), TEXT("icon_gloves_leather") },
				{ LOCTEXT("Boots", "Boots"), LOCTEXT("BootsHolds", "Ankle boots, riding boots or plate sabatons."), TEXT("icon_boots_ankle") },
			};
			static_assert(UE_ARRAY_COUNT(Slots) == static_cast<int32>(ECIRLEquipSlot::Count), "One entry per slot");
			return Slots[FMath::Clamp(static_cast<int32>(Slot), 0, static_cast<int32>(ECIRLEquipSlot::Count) - 1)];
		}
	}

	FText Name(ECIRLEquipSlot Slot) { return Info(Slot).Name; }
	FText Holds(ECIRLEquipSlot Slot) { return Info(Slot).Holds; }
	FName SilhouetteIcon(ECIRLEquipSlot Slot) { return FName(Info(Slot).Silhouette); }
}

#undef LOCTEXT_NAMESPACE
