// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.


#include "AI/ActionDataObject/ActionBrainDataObject_Self.h"
#include "AI/CoreAIController.h"

UActionBrainDataObject_Self::UActionBrainDataObject_Self(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UActionBrainDataObject_Self::Initialize(AAIController* InOwningController)
{
	Super::Initialize(InOwningController);
}

void UActionBrainDataObject_Self::GetListOfActors(TArray<AActor*>& ActorList) const
{
	switch (SelectionMode)
	{
	case EDataSelectionMode::LocationOnly:
		return;
	case EDataSelectionMode::PreferLocation:
		TArray<FVector> LocationList;
		GetListOfLocations(LocationList);
		if (LocationList.Num() > 0) { return; }
	}

	ActorList.Empty(FMath::Max(1, ActorList.Max()));
	if (OwningController && OwningController->GetPawn())
	{
		ActorList.Add(OwningController->GetPawn());
	}
}

void UActionBrainDataObject_Self::GetListOfLocations(TArray<FVector>& LocationList) const
{
	switch (SelectionMode)
	{
	case EDataSelectionMode::ActorOnly:
		return;
	case EDataSelectionMode::PreferActor:
		TArray<AActor*> ActorList;
		GetListOfActors(ActorList);
		if (ActorList.Num() > 0) { return; }
	}

	LocationList.Empty(FMath::Max(1, LocationList.Max()));
	if (OwningController && OwningController->GetPawn())
	{
		LocationList.Add(OwningController->GetPawn()->GetActorLocation());
	}
}