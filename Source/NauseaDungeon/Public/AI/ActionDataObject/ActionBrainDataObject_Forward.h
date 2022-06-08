// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "AI/ActionBrainDataObject.h"
#include "ActionBrainDataObject_Forward.generated.h"

/**
 * 
 */
UCLASS()
class UActionBrainDataObject_Forward : public UActionBrainDataObject
{
	GENERATED_UCLASS_BODY()
	
//~ Begin UActionBrainDataObject Interface
public:
	virtual void GetListOfActors(TArray<AActor*>& ActorList) const override;
	virtual void GetListOfLocations(TArray<FVector>& LocationList) const override;
//~ End UActionBrainDataObject Interface

protected:
	UPROPERTY(EditDefaultsOnly, Category = ActionDataObject)
	float ForwardOffset = 5000.f;
};
