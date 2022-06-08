// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "AI/CorePathFollowingComponent.h"
#include "DungeonPathFollowingComponent.generated.h"

/**
 * 
 */
UCLASS()
class NAUSEADUNGEON_API UDungeonPathFollowingComponent : public UCorePathFollowingComponent
{
	GENERATED_UCLASS_BODY()
	
//~ Begin UActorComponent Interface
public:
	virtual void BeginPlay() override;
//~ End UActorComponent Interface

//~ Begin UPathFollowingComponent Interface
public:
	virtual void OnLanded() override;
	virtual void OnPathFinished(const FPathFollowingResult& Result) override;
//~ End UPathFollowingComponent Interface
};
