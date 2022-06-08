#pragma once

#define NAUSEA_DEBUG_DRAW !(UE_BUILD_SHIPPING || UE_BUILD_TEST)

#define IS_K2_FUNCTION_IMPLEMENTED(Object, FunctionName)\
(Object ? (Object->FindFunction(GET_FUNCTION_NAME_CHECKED(std::remove_pointer<decltype(Object)>::type, FunctionName)) ? Object->FindFunction(GET_FUNCTION_NAME_CHECKED(std::remove_pointer<decltype(Object)>::type, FunctionName))->IsInBlueprint() : false) : false)

#define ECC_RigidBody ECollisionChannel::ECC_GameTraceChannel1
#define ECC_DungeonPawn ECollisionChannel::ECC_GameTraceChannel2
#define ECC_PlacementTrace ECollisionChannel::ECC_GameTraceChannel3
#define ECC_PlacementObject ECollisionChannel::ECC_GameTraceChannel4