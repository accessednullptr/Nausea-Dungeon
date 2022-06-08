// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.


#include "Player/PlayerStatistics/PlayerExperienceSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LocalPlayer.h"
#include "Player/CorePlayerController.h"
#include "Player/CorePlayerState.h"
#include "Gameplay/StatusEffect/StatusEffectBase.h"

UPlayerExperienceSaveGame::UPlayerExperienceSaveGame(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UPlayerExperienceSaveGame::MarkFromLoad()
{
	SaveGameType = ESaveGameType::FromLoad;
}

void UPlayerExperienceSaveGame::MarkFromRPC()
{
	SaveGameType = ESaveGameType::FromRPC;
}

uint64 UPlayerExperienceSaveGame::GetExperienceValue() const
{
	return Experience.GetValue();
}

uint64 UPlayerExperienceSaveGame::SetExperienceValue(uint64 InValue)
{
	return Experience.SetValue(InValue);
}

uint64 UPlayerExperienceSaveGame::AddExperienceValue(uint64 Delta)
{
	return Experience.AddValue(Delta);
}

FString UPlayerExperienceSaveGame::GetPlayerID(ACorePlayerController* PlayerController)
{
	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();

	if (!LocalPlayer)
	{
		return "";
	}

#if WITH_EDITOR
	FString SlotName;
	if (GEditor)
	{
		const int32 ID = LocalPlayer->GetWorld() ? (LocalPlayer->GetWorld()->GetOutermost() ? LocalPlayer->GetWorld()->GetOutermost()->PIEInstanceID : 0) : 0;
		SlotName = FString::Printf(TEXT("EditorClient%i"), ID);
	}
	else
	{
		SlotName = LocalPlayer->GetPreferredUniqueNetId().ToString();
	}
#else
	const FString SlotName = LocalPlayer->GetPreferredUniqueNetId().ToString();
#endif //WITH_EDITOR

	return SlotName;
}

UPlayerExperienceSaveGame* UPlayerExperienceSaveGame::LoadPlayerSaveData(ACorePlayerController* PlayerController)
{
	if (!PlayerController || !PlayerController->IsLocalPlayerController())
	{
		return nullptr;
	}

	const FString SlotName = GetPlayerID(PlayerController);

	if (SlotName == "")
	{
		UPlayerExperienceSaveGame* SaveGame = Cast<UPlayerExperienceSaveGame>(UGameplayStatics::CreateSaveGameObject(UPlayerExperienceSaveGame::StaticClass()));
		SaveGame->MarkFromLoad();
		SaveGame->LoadedSlotName = "";
		return SaveGame;
	}

	UPlayerExperienceSaveGame* SaveGame = nullptr;

	if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		SaveGame = Cast<UPlayerExperienceSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	}
	else
	{
		SaveGame = Cast<UPlayerExperienceSaveGame>(UGameplayStatics::CreateSaveGameObject(UPlayerExperienceSaveGame::StaticClass()));
	}
	
#if WITH_EDITOR
	if (GEditor)
	{
		SaveGame->MarkEditorSave();
	}
#endif //WITH_EDITOR

	UGameplayStatics::AsyncSaveGameToSlot(SaveGame, SlotName, 0);
	SaveGame->MarkFromLoad();
	SaveGame->LoadedSlotName = SlotName;
	return SaveGame;
}

UPlayerExperienceSaveGame* UPlayerExperienceSaveGame::CreateRemoteAuthorityPlayerSaveData(ACorePlayerController* PlayerController)
{
	UPlayerExperienceSaveGame* SaveGame = Cast<UPlayerExperienceSaveGame>(UGameplayStatics::CreateSaveGameObject(UPlayerExperienceSaveGame::StaticClass()));
	SaveGame->MarkFromRPC();
	return SaveGame;
}

bool UPlayerExperienceSaveGame::ResetPlayerSaveData(ACorePlayerController* PlayerController)
{
	const FString SlotName = GetPlayerID(PlayerController);

	if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		return false;
	}

	return UGameplayStatics::DeleteGameInSlot(SlotName, 0);
}

bool UPlayerExperienceSaveGame::RequestSave(UPlayerExperienceSaveGame* SaveGame)
{
	if (!SaveGame || SaveGame->GetSaveType() != ESaveGameType::FromLoad)
	{
		return false;
	}

	if (SaveGame->IsSaving())
	{
		SaveGame->bPendingSave = true;
		return true;
	}

	SaveGame->bPendingSave = false;
	UGameplayStatics::AsyncSaveGameToSlot(SaveGame, SaveGame->LoadedSlotName, 0, FAsyncSaveGameToSlotDelegate::CreateUObject(SaveGame, &UPlayerExperienceSaveGame::SaveCompleted));
	return true;
}

bool UPlayerExperienceSaveGame::ReceiveServerStatistics(const FPlayerStatisticsStruct& ServerStatistics)
{
	Statistics.ReplaceWithLarger(ServerStatistics);
	return true;
}

bool UPlayerExperienceSaveGame::ReceiveServerExperience(const FExperienceStruct& ServerExperience)
{
	Experience.ReplaceWithLarger(ServerExperience);
	return true;
}

void UPlayerExperienceSaveGame::SaveCompleted(const FString& SlotName, const int32 UserIndex, bool bSuccess)
{
	bIsSaving = false;

	if (bPendingSave)
	{
		RequestSave(this);
	}
}

bool UPlayerExperienceSaveGame::PushPlayerData(FExperienceStruct& InExperience, FPlayerStatisticsStruct& InStatistics, FPlayerSelectionStruct& InPlayerSelection)
{
	if (SaveGameType != ESaveGameType::FromRPC)
	{
		return false;
	}

	Experience = MoveTemp(InExperience);
	Statistics = MoveTemp(InStatistics);
	PlayerSelection = MoveTemp(InPlayerSelection);

	return true;
}

uint64 UPlayerExperienceSaveGame::GetPlayerStatisticsValue(EPlayerStatisticType StatisticsType) const
{
	return Statistics.Get(StatisticsType);
}

uint64 UPlayerExperienceSaveGame::SetPlayerStatisticsValue(EPlayerStatisticType StatisticsType, uint64 InValue)
{
	return Statistics.Set(StatisticsType, InValue);
}

uint64 UPlayerExperienceSaveGame::AddPlayerStatisticsValue(EPlayerStatisticType StatisticsType, uint64 Delta)
{
	return Statistics.Add(StatisticsType, Delta);
}

bool UPlayerExperienceSaveGame::CheckAndMarkStatusEffectReceived(TSubclassOf<UStatusEffectBase> StatusEffectClass)
{
	if (LocalData.HasReceivedStatusEffect(TSoftClassPtr<UStatusEffectBase>(StatusEffectClass)))
	{
		return true;
	}

	LocalData.MarkStatusEffectReceived(TSoftClassPtr<UStatusEffectBase>(StatusEffectClass));
	return false;
}