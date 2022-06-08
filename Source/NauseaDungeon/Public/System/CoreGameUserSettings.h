// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/InputSettings.h"
#include "CoreGameUserSettings.generated.h"

/**
 * 
 */
UCLASS()
class UCoreGameUserSettings : public UGameUserSettings
{
	GENERATED_UCLASS_BODY()
};

UCLASS()
class UCoreInputSettings : public UInputSettings
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintPure, Category = Settings)
	static UCoreInputSettings* GetCoreInputSettings();

public:
	UFUNCTION(BlueprintCallable, Category = Settings)
	static bool SetMouseXSensitivity(float InSensitivity);
	UFUNCTION(BlueprintCallable, Category = Settings)
	static bool SetMouseYSensitivityMultiplier(float InSensitivity);

	UFUNCTION(BlueprintCallable, Category = Settings)
	static void SetAimSensitivity(float InSensitivity);

	UFUNCTION(BlueprintCallable, Category = Settings)
	static float GetMouseXSensitivity();
	UFUNCTION(BlueprintCallable, Category = Settings)
	static float GetMouseYSensitivityMultiplier();

	UFUNCTION(BlueprintCallable, Category = Settings)
	static float GetAimSensitivity();

protected:
	UPROPERTY(config)
	float AimSensitivity = 1.f;
};