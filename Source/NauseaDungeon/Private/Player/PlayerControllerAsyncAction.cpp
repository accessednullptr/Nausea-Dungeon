// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.

#include "Player/PlayerControllerAsyncAction.h"
#include "NauseaHelpers.h"
#include "Player/PlayerStatistics/PlayerStatisticsComponent.h"

UPlayerControllerAsyncAction::UPlayerControllerAsyncAction(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UPlayerControllerAsyncAction::Activate()
{
	if (!OwningPlayerController.IsValid())
	{
		OnFailed();
		return;
	}
}

void UPlayerControllerAsyncAction::OnFailed()
{
	SetReadyToDestroy();
	bFailed = true;
}