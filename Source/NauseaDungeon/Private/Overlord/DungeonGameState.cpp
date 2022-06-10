// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.


#include "Overlord/DungeonGameState.h"
#include "NauseaNetDefines.h"
#include "System/CoreWorldSettings.h"
#include "System/DungeonLevelScriptActor.h"
#include "Overlord/DungeonGameModeSettings.h"

ADungeonGameState::ADungeonGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void ADungeonGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_WITH_PARAMS_FAST(ADungeonGameState, CurrentWaveNumber, PushReplicationParams::Default);
	DOREPLIFETIME_WITH_PARAMS_FAST(ADungeonGameState, AutoStartTime, PushReplicationParams::Default);
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

UDungeonWaveSetup* ADungeonGameState::GetWaveSetup() const
{
	if (CurrentWaveSetup)
	{
		return CurrentWaveSetup;
	}

	if (ADungeonLevelScriptActor* DungeonLevelScriptActor = Cast<ADungeonLevelScriptActor>(GetWorld()->GetLevelScriptActor()))
	{
		if (UDungeonWaveSetup* WaveSetup = DungeonLevelScriptActor->GenerateDungeonWaveSettings())
		{
			ensure(!CurrentWaveSetup);
			CurrentWaveSetup = WaveSetup;
			ensure(WaveSetup);
		}
	}

	return CurrentWaveSetup;
}

float ADungeonGameState::GetWaveSpawnProgress() const
{
	if (WaveTotalSpawnCount <= 0)
	{
		return 1.f;
	}

	return float(WaveRemainingSpawnCount) / float(WaveTotalSpawnCount);
}

int64 ADungeonGameState::IncrementWaveNumber()
{
	CurrentWaveNumber++;
	MARK_PROPERTY_DIRTY_FROM_NAME(ADungeonGameState, CurrentWaveNumber, this);
	OnRep_CurrentWaveNumber(CurrentWaveNumber - 1);
	return CurrentWaveNumber;
}

int64 ADungeonGameState::SetWaveTotalSpawnCount(int64 NewWaveTotalSpawnCount)
{
	const int64 PreviousWaveTotalSpawnCount = NewWaveTotalSpawnCount;
	WaveTotalSpawnCount = NewWaveTotalSpawnCount;
	MARK_PROPERTY_DIRTY_FROM_NAME(ADungeonGameState, WaveTotalSpawnCount, this);
	OnRep_WaveTotalSpawnCount(PreviousWaveTotalSpawnCount);
	SetWaveRemainingSpawnCount(NewWaveTotalSpawnCount);
	return WaveTotalSpawnCount;
}

int64 ADungeonGameState::SetWaveRemainingSpawnCount(int64 NewWaveRemainingSpawnCount)
{
	const int64 PreviousWaveRemainingSpawnCount = WaveRemainingSpawnCount;
	WaveRemainingSpawnCount = NewWaveRemainingSpawnCount;
	MARK_PROPERTY_DIRTY_FROM_NAME(ADungeonGameState, WaveRemainingSpawnCount, this);
	OnRep_WaveRemainingSpawnCount(PreviousWaveRemainingSpawnCount);
	return WaveRemainingSpawnCount;
}

void ADungeonGameState::SetAutoStartTime(float InAutoStartTime)
{
	AutoStartTime = GetServerWorldTimeSeconds() + InAutoStartTime;
	MARK_PROPERTY_DIRTY_FROM_NAME(ADungeonGameState, AutoStartTime, this);
	OnRep_AutoStartTime(AutoStartTime);
}

void ADungeonGameState::UpdateDungeonCharacterClassList()
{
	CurrentDungeonCharacterClassList.Reset();

	UDungeonWaveSetup* WaveSetup = GetWaveSetup();

	if (!WaveSetup)
	{
		return;
	}

	WaveSetup->GetCharacterClassListForWave(CurrentWaveNumber, CurrentDungeonCharacterClassList);
}

void ADungeonGameState::OnRep_CurrentWaveNumber(int64 PreviousWaveNumber)
{
	UpdateDungeonCharacterClassList();
	OnCurrentWaveNumberChanged.Broadcast(this, CurrentWaveNumber, PreviousWaveNumber);
}


void ADungeonGameState::OnRep_WaveTotalSpawnCount(int64 PreviousWaveTotalSpawnCount)
{
	OnWaveTotalSpawnCountUpdate.Broadcast(this, WaveTotalSpawnCount, PreviousWaveTotalSpawnCount);
}

void ADungeonGameState::OnRep_WaveRemainingSpawnCount(int64 PreviousWaveRemainingSpawnCount)
{
	OnWaveRemainingSpawnCountUpdate.Broadcast(this, WaveRemainingSpawnCount, PreviousWaveRemainingSpawnCount);
}

void ADungeonGameState::OnRep_AutoStartTime(float PreviousAutoStartTime)
{
	const float TimeAdjustment = GetWorld()->GetTimeSeconds() - GetServerWorldTimeSeconds();
	OnAutoStartTimeUpdate.Broadcast(this, GetWorld()->GetTimeSeconds(), AutoStartTime + TimeAdjustment);
}