// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.


#include "System/CoreGameplayStatics.h"
#include "GameFramework/Actor.h"
#include "Gameplay/CoreDamageType.h"
#include "Gameplay/StatusType.h"
#include "GeneralProjectSettings.h"

UCoreGameplayStatics::UCoreGameplayStatics(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FString UCoreGameplayStatics::GetNetRoleNameForActor(AActor* Actor)
{
	if (Actor)
	{
		return GetNetRoleName(Actor->GetLocalRole());
	}

	return GetNetRoleName(ROLE_None);
}

FString UCoreGameplayStatics::GetNetRoleName(ENetRole Role)
{
	switch (Role)
	{
	case ROLE_Authority:
		return "AUTHORITY";
	case ROLE_AutonomousProxy:
		return "AUTONOMOUS";
	case ROLE_SimulatedProxy:
		return "SIMULATED";
	case ROLE_None:
		return "NONE";
	}

	return "NONE";
}

FString UCoreGameplayStatics::GetProjectVersion()
{
	return GetDefault<UGeneralProjectSettings>()->ProjectVersion;
}