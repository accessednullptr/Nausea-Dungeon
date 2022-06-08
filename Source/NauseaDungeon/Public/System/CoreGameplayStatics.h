// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CoreGameplayStatics.generated.h"

class UWeapon;
class UFireMode;

/**
 * 
 */
UCLASS()
class UCoreGameplayStatics : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = GameplayStatics)
	static FString GetNetRoleNameForActor(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = GameplayStatics)
	static FString GetNetRoleName(ENetRole Role);

	UFUNCTION(BlueprintCallable, Category = GameplayStatics)
	static FString GetProjectVersion();
};