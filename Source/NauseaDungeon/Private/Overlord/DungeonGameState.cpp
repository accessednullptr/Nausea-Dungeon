// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.


#include "Overlord/DungeonGameState.h"
#include "NauseaNetDefines.h"
#include "System/CoreWorldSettings.h"
#include "Overlord/DungeonGameModeSettings.h"

ADungeonGameState::ADungeonGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void ADungeonGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_WITH_PARAMS_FAST(ADungeonGameState, CurrentWaveNumber, PushReplicationParams::Default);
}

void ADungeonGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void ADungeonGameState::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
}

void ADungeonGameState::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();
}

int64 ADungeonGameState::IncrementWaveNumber()
{
	CurrentWaveNumber++;
	OnRep_CurrentWaveNumber(CurrentWaveNumber - 1);
	MARK_PROPERTY_DIRTY_FROM_NAME(ADungeonGameState, CurrentWaveNumber, this);
	return CurrentWaveNumber;
}

void ADungeonGameState::OnRep_CurrentWaveNumber(int64 PreviousWaveNumber)
{
	OnCurrentWaveNumberChanged.Broadcast(this, CurrentWaveNumber, PreviousWaveNumber);
}