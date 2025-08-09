#include "GtGameplayTags.h"

namespace GtGameplayTags
{
	/** Attribute Primary Tags */
	UE_DEFINE_GAMEPLAY_TAG(Attribute_Primary_Health, "Attribute.Primary.Health");

	/** Input Tags */
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Move, "InputTag.Move");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Look, "InputTag.Look");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Jump, "InputTag.Jump");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Crouch, "InputTag.Crouch");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Sprint, "InputTag.Sprint");
	
	/** Status Tags */
	UE_DEFINE_GAMEPLAY_TAG(Status_Action, "Status.Action");
	UE_DEFINE_GAMEPLAY_TAG(Status_Action_WallRunning, "Status.Action.WallRunning");
	UE_DEFINE_GAMEPLAY_TAG(Status_Action_WallRunning_Left, "Status.Action.WallRunning.Left");
	UE_DEFINE_GAMEPLAY_TAG(Status_Action_WallRunning_Right, "Status.Action.WallRunning.Right");
	UE_DEFINE_GAMEPLAY_TAG(Status_Action_Crouching, "Status.Action.Crouching");
	UE_DEFINE_GAMEPLAY_TAG(Status_Action_Sliding, "Status.Action.Sliding");
	UE_DEFINE_GAMEPLAY_TAG(Status_Action_Sprinting, "Status.Action.Sprinting");
	UE_DEFINE_GAMEPLAY_TAG(Status_Dead, "Status.Dead");
}