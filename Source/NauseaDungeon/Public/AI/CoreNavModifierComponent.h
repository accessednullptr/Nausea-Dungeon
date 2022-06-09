// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NavModifierComponent.h"
#include "CoreNavModifierComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Navigation), meta = (BlueprintSpawnableComponent))
class NAUSEADUNGEON_API UCoreNavModifierComponent : public UNavModifierComponent
{
	GENERATED_UCLASS_BODY()
	
//~ Begin UNavRelevantComponent Interface
public:
	virtual void CalcAndCacheBounds() const override;
//~ End UNavRelevantComponent Interface

};
