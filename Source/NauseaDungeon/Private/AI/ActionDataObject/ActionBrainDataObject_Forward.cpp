// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.


#include "AI/ActionDataObject/ActionBrainDataObject_Forward.h"
#include "AI/CoreAIController.h"

UActionBrainDataObject_Forward::UActionBrainDataObject_Forward(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SelectionMode = EDataSelectionMode::LocationOnly;
}

void UActionBrainDataObject_Forward::GetListOfActors(TArray<AActor*>& ActorList) const
{
	ActorList.Reset();
	return;
}

void UActionBrainDataObject_Forward::GetListOfLocations(TArray<FVector>& LocationList) const
{
	LocationList.Reset(1);
	if (const APawn* Pawn = OwningController ? OwningController->GetPawn() : nullptr)
	{
		LocationList.Add(Pawn->GetActorLocation() + (Pawn->GetActorForwardVector() * ForwardOffset));
	}
}
