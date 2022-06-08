// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "AI/ActionBrainDataObject.h"
#include "ActionBrainDataObject_Enemy.generated.h"

class UEnemySelectionComponent;

/**
 * 
 */
UCLASS()
class UActionBrainDataObject_Enemy : public UActionBrainDataObject
{
	GENERATED_UCLASS_BODY()

//~ Begin UActionBrainDataObject Interface
public:
	virtual void Initialize(AAIController* InOwningController) override;
	virtual void GetListOfActors(TArray<AActor*>& ActorList) const override;
	virtual void GetListOfLocations(TArray<FVector>& LocationList) const override;
//~ Begin UActionBrainDataObject Interface

protected:
	UPROPERTY(Transient)
	UEnemySelectionComponent* OwningEnemySelectionComponent = nullptr;
};

UCLASS()
class UActionBrainDataObject_AllEnemies : public UActionBrainDataObject
{
	GENERATED_UCLASS_BODY()
	
//~ Begin UActionBrainDataObject Interface
public:
	virtual void Initialize(AAIController* InOwningController) override;
	virtual void GetListOfActors(TArray<AActor*>& ActorList) const override;
	virtual void GetListOfLocations(TArray<FVector>& LocationList) const override;
//~ Begin UActionBrainDataObject Interface
	
protected:
	UPROPERTY(Transient)
	UEnemySelectionComponent* OwningEnemySelectionComponent = nullptr;
};