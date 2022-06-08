// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/CoreCharacter.h"
#include "DungeonCharacter.generated.h"

class USkeletalMeshComponentBudgeted;
class UCurveFloat;

UCLASS(Blueprintable)
class ADungeonCharacter : public ACoreCharacter
{
	GENERATED_UCLASS_BODY()

//~ Begin AActor Interface
public:
	virtual void PreRegisterAllComponents() override;
//~ End AActor Interface

//~ Begin IStatusInterface Interface
public:
	virtual void Died(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
//~ End IStatusInterface Interface

public:
	int32 GetCoinValue() const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = Mesh)
	UPhysicsAsset* RagdollPhysicsAsset = nullptr;

	//Base amount of coins to grant per kill of this character.
	UPROPERTY(EditDefaultsOnly, Category = Dungeon)
	int32 BaseCoinValue = 5;

	//Curve applied specially to this character based on difficulty.
	UPROPERTY(EditDefaultsOnly, Category = Dungeon)
	UCurveFloat* BaseCoinValueDifficultyScale = nullptr;

	//Curve applied specially to this character based on wave.
	UPROPERTY(EditDefaultsOnly, Category = Dungeon)
	UCurveFloat* BaseCoinValueWaveScale = nullptr;
};