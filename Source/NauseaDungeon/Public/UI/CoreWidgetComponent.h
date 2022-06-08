// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "CoreWidgetComponent.generated.h"

/**
 * 
 */
UCLASS()
class NAUSEADUNGEON_API UCoreWidgetComponent : public UWidgetComponent
{
	GENERATED_UCLASS_BODY()
	
//~ Begin UWidgetComponent Interface
public:
	virtual void InitWidget() override;
//~ Begin UWidgetComponent Interface
};
