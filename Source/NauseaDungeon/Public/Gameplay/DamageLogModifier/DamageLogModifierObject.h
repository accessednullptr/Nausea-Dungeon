// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Gameplay/DamageLogInterface.h"
#include "DamageLogModifierObject.generated.h"

/**
 * Used to define arbitrary damage log modifiers that can't/shouldn't be using the actual class/instance that applies the modification as the instigator (such as armor)
 */
UCLASS(HideFunctions = (K2_GetDamageLogInstigatorName))
class UDamageLogModifierObject : public UObject, public IDamageLogInterface
{
	GENERATED_UCLASS_BODY()

//~ Begin IDamageLogInterface Interface
public:
	virtual FText GetDamageLogInstigatorName() const override { return InstigatorName; }
//~ End IDamageLogInterface Interface

protected:
	UPROPERTY(EditDefaultsOnly, Category = DamageLogModifier)
	FText InstigatorName = FText::GetEmpty();
};

UCLASS()
class UArmorDamageLogModifier : public UDamageLogModifierObject
{
	GENERATED_UCLASS_BODY()
};

UCLASS()
class UWeakpointDamageLogModifier : public UDamageLogModifierObject
{
	GENERATED_UCLASS_BODY()
};

UCLASS()
class UPartDestroyedFlatDamageLogModifier : public UDamageLogModifierObject
{
	GENERATED_UCLASS_BODY()
};

UCLASS()
class UPartDestroyedPercentDamageLogModifier : public UDamageLogModifierObject
{
	GENERATED_UCLASS_BODY()
};

UCLASS()
class UPartWeaknessDamageLogModifier : public UDamageLogModifierObject
{
	GENERATED_UCLASS_BODY()
};

UCLASS()
class UPartResistanceDamageLogModifier : public UDamageLogModifierObject
{
	GENERATED_UCLASS_BODY()
};

UCLASS()
class UWeaknessDamageLogModifier : public UDamageLogModifierObject
{
	GENERATED_UCLASS_BODY()
};

UCLASS()
class UResistanceDamageLogModifier : public UDamageLogModifierObject
{
	GENERATED_UCLASS_BODY()
};

UCLASS()
class UFriendlyFireScalingDamageLogModifier : public UDamageLogModifierObject
{
	GENERATED_UCLASS_BODY()
};