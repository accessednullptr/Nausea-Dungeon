// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "System/CoreGameState.h"
#include "DungeonGameState.generated.h"

class ADungeonGameState;
class UDungeonWaveSetup;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCurrentWaveNumberChangedSignature, ADungeonGameState*, DungeonGameState, int64, NewWave, int64, PreviousWave);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAutoStartTimeUpdateSignature, ADungeonGameState*, DungeonGameState, float, StartTime, float, EndTime);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FWaveRemainingSpawnCountUpdateSignature, ADungeonGameState*, DungeonGameState, int64, NewCount, int64, PreviousCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FWaveTotalSpawnCountUpdateSignature, ADungeonGameState*, DungeonGameState, int64, NewCount, int64, PreviousCount);

/**
 * 
 */
UCLASS()
class NAUSEADUNGEON_API ADungeonGameState : public ACoreGameState
{
	GENERATED_UCLASS_BODY()
	
//~ Begin AActor Interface
public:
	virtual void PostInitializeComponents() override;
//~ End AActor Interface

//~ Begin AGameState Interface
protected:
	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasEnded() override;
//~ End AGameState Interface

public:
	UFUNCTION(BlueprintCallable, Category = GameState)
	UDungeonWaveSetup* GetWaveSetup() const;

	UFUNCTION(BlueprintCallable, Category = GameState)
	int64 GetCurrentWaveNumber() const { return CurrentWaveNumber; }
	UFUNCTION(BlueprintCallable, Category = GameState)
	int64 GetWaveTotalSpawnCount() const { return WaveTotalSpawnCount; }
	UFUNCTION(BlueprintCallable, Category = GameState)
	int64 GetWaveRemainingSpawnCount() const { return WaveRemainingSpawnCount; }
	UFUNCTION(BlueprintCallable, Category = GameState)
	float GetWaveSpawnProgress() const;

	UFUNCTION(BlueprintCallable, Category = GameState)
	const TArray<TSubclassOf<ADungeonCharacter>>& GetCurrentDungeonCharacterClassList() const { return CurrentDungeonCharacterClassList; }

	int64 IncrementWaveNumber();

	int64 SetWaveTotalSpawnCount(int64 NewWaveTotalSpawnCount);
	int64 SetWaveRemainingSpawnCount(int64 NewWaveRemainingSpawnCount);
	
	void SetAutoStartTime(float InAutoStartTime);

public:
	UPROPERTY(BlueprintAssignable, Category = GameState)
	FCurrentWaveNumberChangedSignature OnCurrentWaveNumberChanged;
	UPROPERTY(BlueprintAssignable, Category = GameState)
	FAutoStartTimeUpdateSignature OnAutoStartTimeUpdate;

	UPROPERTY(BlueprintAssignable, Category = GameState)
	FWaveTotalSpawnCountUpdateSignature OnWaveTotalSpawnCountUpdate;
	UPROPERTY(BlueprintAssignable, Category = GameState)
	FWaveRemainingSpawnCountUpdateSignature OnWaveRemainingSpawnCountUpdate;

protected:
	void UpdateDungeonCharacterClassList();

	UFUNCTION()
	void OnRep_CurrentWaveNumber(int64 PreviousWaveNumber);

	UFUNCTION()
	void OnRep_WaveTotalSpawnCount(int64 PreviousWaveTotalSpawnCount);
	UFUNCTION()
	void OnRep_WaveRemainingSpawnCount(int64 PreviousWaveRemainingSpawnCount);

	UFUNCTION()
	void OnRep_AutoStartTime(float PreviousAutoStartTime);

protected:
	UPROPERTY(Transient, ReplicatedUsing = OnRep_CurrentWaveNumber)
	int64 CurrentWaveNumber = 0;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_WaveTotalSpawnCount)
	int64 WaveTotalSpawnCount = -1;
	UPROPERTY(Transient, ReplicatedUsing = OnRep_WaveRemainingSpawnCount)
	int64 WaveRemainingSpawnCount = -1;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_AutoStartTime)
	float AutoStartTime = 0;

	UPROPERTY(Transient)
	TArray<TSubclassOf<ADungeonCharacter>> CurrentDungeonCharacterClassList;

	UPROPERTY(Transient)
	mutable UDungeonWaveSetup* CurrentWaveSetup = nullptr;
};
