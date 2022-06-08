// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "System/CoreGameState.h"
#include "DungeonGameState.generated.h"

class ADungeonGameState;
class UDungeonGameModeSettings;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCurrentWaveNumberChangedSignature, ADungeonGameState*, DungeonGameState, int64, NewWave, int64, PreviousWave);

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
	int64 GetCurrentWaveNumber() const { return CurrentWaveNumber; }

	int64 IncrementWaveNumber();

public:
	UPROPERTY(BlueprintAssignable, Category = GameState)
	FCurrentWaveNumberChangedSignature OnCurrentWaveNumberChanged;

protected:
	UFUNCTION()
	void OnRep_CurrentWaveNumber(int64 PreviousWaveNumber);

protected:
	UPROPERTY(Transient, ReplicatedUsing = OnRep_CurrentWaveNumber)
	int64 CurrentWaveNumber = 0;
};
