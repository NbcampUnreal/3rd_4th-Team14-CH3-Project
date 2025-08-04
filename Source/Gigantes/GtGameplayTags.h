#pragma once

#include "NativeGameplayTags.h"

namespace GtGameplayTags
{
	/** Attribute Primary Tags */
	GIGANTES_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Primary_Health);

	/** Input Tags */
	GIGANTES_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Move);
	GIGANTES_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look);
	GIGANTES_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Jump);
	
	/** Status Tags */
	GIGANTES_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Action_WallRunning);
	GIGANTES_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Action_WallRunning_Left);
	GIGANTES_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Action_WallRunning_Right);
	GIGANTES_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Dead);
}