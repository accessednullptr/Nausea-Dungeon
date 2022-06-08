// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "System/CoreGameMode.h"
#include "DungeonGameMode.generated.h"

class ADungeonCharacter;
class UDungeonGameModeSettings;
class UDungeonWaveSetup;
class UWaveConfiguration;

UCLASS(minimalapi)
class ADungeonGameMode : public ACoreGameMode
{
	GENERATED_UCLASS_BODY()

//~ Begin AGameModeBase Interface
public:
	virtual void SetPlayerDefaults(APawn* PlayerPawn) override;
//~ End AGameModeBase Interface

//~ Begin AGameMode Interface
protected:
	virtual void HandleMatchHasStarted() override;
//~ Begin AGameMode Interface

public:
	//ADungeonCharacter* KilledCharacter, AController* EventInstigator, AActor* DamageCauser, int32& CoinAmount
	DECLARE_EVENT_FourParams(ADungeonGameMode, FProcessCoinAmountForKillEvent, ADungeonCharacter*, AController*, AActor*, int32&)
	FProcessCoinAmountForKillEvent ProcessCoinAmountForKill;

protected:
	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void OnPawnDied(UStatusComponent* Component, float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

	UFUNCTION()
	void GrantCoinsForKill(ADungeonCharacter* KilledCharacter, AController* EventInstigator, AActor* DamageCauser);

	UFUNCTION()
	void AddPawnToWaveCharacters(ADungeonCharacter* Character);
	UFUNCTION()
	void RemovePawnFromWaveCharacters(ADungeonCharacter* Character);

	UFUNCTION()
	void OnWaveCompleted(UDungeonWaveSetup* WaveSetup, int64 WaveNumber);
	UFUNCTION()
	void StartNextWave();

protected:
	//Dump wave characters we've spawned into this list (removed on destroyed/killed) so that we know when there are no characters spawned.
	UPROPERTY(Transient)
	TSet<TWeakObjectPtr<ADungeonCharacter>> WaveCharacters;

	//Cache our wave config list for the current wave number here.
	UPROPERTY(Transient)
	UDungeonWaveSetup* CurrentWaveSetup = nullptr;
	UPROPERTY(Transient)
	FTimerHandle AutoStartWaveTimerHandle;
};