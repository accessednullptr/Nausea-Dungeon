// Fill out your copyright notice in the Description page of Project Settings.


#include "System/DungeonLevelScriptActor.h"

ADungeonLevelScriptActor::ADungeonLevelScriptActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

UDungeonWaveSetup* ADungeonLevelScriptActor::GenerateDungeonWaveSettings()
{
	if (GeneratedDungeonWaveSettings)
	{
		return GeneratedDungeonWaveSettings;
	}

	GeneratedDungeonWaveSettings = K2_GenerateDungeonWaveSettings();
	return GeneratedDungeonWaveSettings;
}