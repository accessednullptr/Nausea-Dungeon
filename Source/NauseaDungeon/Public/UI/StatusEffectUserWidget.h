// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "UI/CoreUserWidget.h"
#include "Gameplay/StatusType.h"
#include "StatusEffectUserWidget.generated.h"

class UStatusEffectBase;
class UStatusEffectInstantBase;
class UStatusEffectCumulativeBase;

/**
 * 
 */
UCLASS(NotBlueprintable)
class UStatusEffectUserWidget : public UCoreUserWidget
{
	GENERATED_UCLASS_BODY()

//~ Begin UUserWidget Interface	
public:
	virtual void NativePreConstruct() override;
//~ End UUserWidget Interface

public:
	UStatusEffectBase* GetOwningStatusEffect() const { return StatusEffect; }

protected:
	UFUNCTION()
	virtual void ReceiveStatusEffectEnd(UStatusEffectBase* Status, EStatusEndType EndType) {}
	
protected:
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, meta = (ExposeOnSpawn = true))
	UStatusEffectBase* StatusEffect = nullptr;

private:
	UPROPERTY()
	bool bHasPerformedFirstPreConsturct = false;
};

UCLASS(Blueprintable)
class UBaseStatusEffectUserWidget : public UStatusEffectUserWidget
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Status Effect User Widget")
	UStatusEffectBase* GetStatusEffect() const
	{
		return StatusEffect;
	}

	virtual void ReceiveStatusEffectEnd(UStatusEffectBase* Status, EStatusEndType EndType) override { OnStatusEffectEnd(Status, EndType); }

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Status Effect User Widget")
	void OnStatusEffectEnd(UStatusEffectBase* Status, EStatusEndType EndType);
};

UCLASS(Blueprintable)
class UBasicStatusEffectUserWidget : public UStatusEffectUserWidget
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Status Effect User Widget")
	UStatusEffectBasic* GetStatusEffect() const;

	virtual void ReceiveStatusEffectEnd(UStatusEffectBase* Status, EStatusEndType EndType) override;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Status Effect User Widget")
	void OnStatusEffectEnd(UStatusEffectBasic* Status, EStatusEndType EndType);
};