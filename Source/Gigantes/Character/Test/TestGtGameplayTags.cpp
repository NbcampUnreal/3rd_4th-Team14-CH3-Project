#include "TestGtGameplayTags.h"

namespace GtGameplayTags
{
	/** Loadout Tags */

	UE_DEFINE_GAMEPLAY_TAG(Loadout_Slot_Weapon, "Loadout.Slot.Weapon");
	UE_DEFINE_GAMEPLAY_TAG(Loadout_Slot_Consumable, "Loadout.Slot.Consumable");
	UE_DEFINE_GAMEPLAY_TAG(Loadout_Slot_Grenade, "Loadout.Slot.Grenade");
	UE_DEFINE_GAMEPLAY_TAG(Loadout_Slot_Weapon_Primary, "Loadout.Slot.Weapon.Primary");
	UE_DEFINE_GAMEPLAY_TAG(Loadout_Slot_Weapon_Secondary, "Loadout.Slot.Weapon.Secondary");
	UE_DEFINE_GAMEPLAY_TAG(Loadout_Slot_Grenade_Normal, "Loadout.Slot.Grenade.Normal");
	UE_DEFINE_GAMEPLAY_TAG(Loadout_Slot_Consumable_Potion, "Loadout.Slot.Consumable.Potion");

	/** Test Item Tags */
	UE_DEFINE_GAMEPLAY_TAG(Item_Weapon_TestRifle, "Item.Weapon.TestRifle");
}