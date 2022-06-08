// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.


#include "Player/PlayerStatistics/PlayerStatisticsComponent.h"
#include "NauseaHelpers.h"
#include "Player/PlayerStatistics/PlayerExperienceSaveGame.h"
#include "System/CoreGameState.h"
#include "Player/CorePlayerController.h"

DECLARE_CYCLE_STAT(TEXT("Push Player Statistics Data"),
	STAT_FSimpleDelegateGraphTask_PushingPlayerStatisticsData,
	STATGROUP_TaskGraphTasks);

UPlayerStatisticsComponent::UPlayerStatisticsComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

ACorePlayerController* UPlayerStatisticsComponent::GetOwningPlayerController() const
{
	return Cast<ACorePlayerController>(GetOwner());
}

bool UPlayerStatisticsComponent::IsLocalPlayerController() const
{
	ACorePlayerController* CorePlayerController = GetOwningPlayerController();

	if (!CorePlayerController)
	{
		return false;
	}

	return CorePlayerController->IsLocalPlayerController();
}

void UPlayerStatisticsComponent::LoadPlayerData()
{
	if (!ensure(IsLocalPlayerController()))
	{
		return;
	}

	PlayerData = UPlayerExperienceSaveGame::LoadPlayerSaveData(GetOwningPlayerController());

	if (!PlayerData)
	{
		return;
	}

	SendPlayerData();
}

void UPlayerStatisticsComponent::ResetPlayerData()
{
	if (!GetOwningPlayerController() || !GetOwningPlayerController()->IsNetMode(NM_Standalone))
	{
		return;
	}

	UPlayerExperienceSaveGame::ResetPlayerSaveData(GetOwningPlayerController());
	
	bPlayerStatsReady = false;

	PlayerData = UPlayerExperienceSaveGame::LoadPlayerSaveData(GetOwningPlayerController());
	
	SendPlayerData();
}

void UPlayerStatisticsComponent::UpdateClientPlayerData()
{
	if (!GetOwningPlayerController() || IsLocalPlayerController() || !GetPlayerData()
		|| GetOwningPlayerController()->GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	Client_Reliable_UpdatePlayerData(GetPlayerData()->GetExperienceData(), GetPlayerData()->GetPlayerStatisticsData());
}

void UPlayerStatisticsComponent::SendPlayerData()
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		Server_Reliable_SendPlayerData(PlayerData->GetExperienceData(), PlayerData->GetPlayerStatisticsData(), PlayerData->GetPlayerSelectionData());
		return;
	}

	if (IsPlayerStatsReady())
	{
		return;
	}

	PlayerDataReady();
}

void UPlayerStatisticsComponent::ReceivePlayerData(FExperienceStruct& Experience, FPlayerStatisticsStruct& Statistics, FPlayerSelectionStruct& PlayerSelection)
{
	if (IsPlayerStatsReady())
	{
		return;
	}

	PlayerData = UPlayerExperienceSaveGame::CreateRemoteAuthorityPlayerSaveData(GetOwningPlayerController());
	PlayerData->PushPlayerData(Experience, Statistics, PlayerSelection);
	PlayerDataReady();
}

void UPlayerStatisticsComponent::PlayerDataReady()
{
	bPlayerStatsReady = true;

	if (GetPlayerData())
	{

	}

	OnPlayerDataReady.Broadcast();

	if (!IsLocalPlayerController() && GetOwnerRole() == ROLE_Authority)
	{
		Client_Reliable_SendPlayerDataAck(true);
	}
}

bool UPlayerStatisticsComponent::Server_Reliable_SendPlayerData_Validate(FExperienceStruct Experience, FPlayerStatisticsStruct Statistics, FPlayerSelectionStruct PlayerSelection)
{
	return true;
}

void UPlayerStatisticsComponent::Server_Reliable_SendPlayerData_Implementation(FExperienceStruct Experience, FPlayerStatisticsStruct Statistics, FPlayerSelectionStruct PlayerSelection)
{
	if (IsPlayerStatsReady())
	{
		return;
	}

	ReceivePlayerData(Experience, Statistics, PlayerSelection);
}

void UPlayerStatisticsComponent::Client_Reliable_SendPlayerDataAck_Implementation(bool bSuccess)
{
	if (!bSuccess)
	{
		SendPlayerData();
		return;
	}

	PlayerDataReady();
}

void UPlayerStatisticsComponent::Client_Reliable_UpdatePlayerData_Implementation(FExperienceStruct Experience, FPlayerStatisticsStruct Statistics)
{
	if (!GetPlayerData())
	{
		return;
	}

	GetPlayerData()->ReceiveServerStatistics(Statistics);
	GetPlayerData()->ReceiveServerExperience(Experience);
	UPlayerExperienceSaveGame::RequestSave(GetPlayerData());
}

bool UPlayerStatisticsComponent::RequestSave()
{
	if (!GetPlayerData())
	{
		return false;
	}

	if (GetPlayerData()->GetSaveType() == ESaveGameType::FromLoad)
	{
		return UPlayerExperienceSaveGame::RequestSave(GetPlayerData());
	}
	else if(GetPlayerData()->GetSaveType() == ESaveGameType::FromRPC)
	{
		UpdateClientPlayerData();
	}

	return false;
}

void UPlayerStatisticsComponent::UpdatePlayerStatistic(EPlayerStatisticType StatisticType, float Delta)
{
	PlayerStatisticsMapCache.FindOrAdd(StatisticType) += Delta;

	if (PlayerStatisticsMapCache[StatisticType] > 1.f && !bPendingStatPush)
	{
		bPendingStatPush = true;

		FSimpleDelegateGraphTask::CreateAndDispatchWhenReady(
			FSimpleDelegateGraphTask::FDelegate::CreateUObject(this, &UPlayerStatisticsComponent::PushPlayerStatisticCache),
			GET_STATID(STAT_FSimpleDelegateGraphTask_PushingPlayerStatisticsData), nullptr, ENamedThreads::GameThread);
	}
}

void UPlayerStatisticsComponent::PushPlayerStatisticCache()
{
	TArray<EPlayerStatisticType> KeyList;
	PlayerStatisticsMapCache.GenerateKeyArray(KeyList);

	UPlayerExperienceSaveGame* CurrentPlayerData = GetPlayerData();

	if (!CurrentPlayerData)
	{
		bPendingStatPush = false;
		return;
	}

	for (EPlayerStatisticType Key : KeyList)
	{
		const float TruncatedFloatValue = FMath::TruncToFloat(PlayerStatisticsMapCache[Key]);
		PlayerStatisticsMapCache[Key] -= TruncatedFloatValue;
		
		const uint64 ValueAdded = uint64(TruncatedFloatValue);
		CurrentPlayerData->AddPlayerStatisticsValue(Key, ValueAdded);
		
		OnPlayerStatisticsUpdate.Broadcast(this, Key, ValueAdded);
	}

	bPendingStatPush = false;
}

void UPlayerStatisticsComponent::UpdatePlayerExperience(uint32 Delta)
{
	UPlayerExperienceSaveGame* CurrentPlayerData = GetPlayerData();

	if (!CurrentPlayerData)
	{
		return;
	}

	CurrentPlayerData->AddExperienceValue(Delta);
	RequestSave();
	OnPlayerExperienceUpdate.Broadcast(this, Delta);
}

uint64 UPlayerStatisticsComponent::GetExperience() const
{
	UPlayerExperienceSaveGame* CurrentPlayerData = GetPlayerData();

	if (!CurrentPlayerData)
	{
		return 0;
	}

	return CurrentPlayerData->GetExperienceValue();
}

bool UPlayerStatisticsComponent::CheckAndMarkStatusEffectReceived(TSubclassOf<UStatusEffectBase> StatusEffectClass)
{
	UPlayerExperienceSaveGame* CurrentPlayerData = GetPlayerData();

	if (!CurrentPlayerData)
	{
		return false;
	}

	return CurrentPlayerData->CheckAndMarkStatusEffectReceived(StatusEffectClass);
}