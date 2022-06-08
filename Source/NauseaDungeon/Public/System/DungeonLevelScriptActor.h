// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LevelScriptActor.h"
#include "DungeonLevelScriptActor.generated.h"

class UDungeonWaveSetup;

/**
 * 
 */
UCLASS()
class NAUSEADUNGEON_API ADungeonLevelScriptActor : public ALevelScriptActor
{
	GENERATED_UCLASS_BODY()
	

public:
	UDungeonWaveSetup* GenerateDungeonWaveSettings();

protected:
	UFUNCTION(BlueprintImplementableEvent, BlueprintAuthorityOnly, Category = WaveSettings, meta = (DisplayName="Generate Dungeon Wave Settings", ScriptName="GenerateDungeonWaveSettings"))
	UDungeonWaveSetup* K2_GenerateDungeonWaveSettings();

protected:
	UPROPERTY(Transient)
	UDungeonWaveSetup* GeneratedDungeonWaveSettings = nullptr;
};
