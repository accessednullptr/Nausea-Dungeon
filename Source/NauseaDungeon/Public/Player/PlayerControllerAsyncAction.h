// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "PlayerControllerAsyncAction.generated.h"

class ACorePlayerController;

/**
 * 
 */
UCLASS()
class UPlayerControllerAsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()
	
//~ Begin UBlueprintAsyncActionBase Interface
	virtual void Activate() override;
//~ End UBlueprintAsyncActionBase Interface

protected:
	//Generic failure state. Can be called via UPlayerControllerAsyncAction::Activate if WorldContextObject or OwningPlayerController is not valid.
	virtual void OnFailed();

protected:
	UPROPERTY()
	TWeakObjectPtr<ACorePlayerController> OwningPlayerController;

	UPROPERTY()
	bool bFailed = false;
};