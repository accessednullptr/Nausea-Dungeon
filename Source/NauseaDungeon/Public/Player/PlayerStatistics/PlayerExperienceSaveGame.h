// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Player/PlayerStatistics/PlayerStatisticsTypes.h"
#include "PlayerExperienceSaveGame.generated.h"

class UInventory;
class UStatusEffectBase;
class UPlayerClassComponent;
class ACorePlayerController;

UENUM(BlueprintType)
enum class ESaveGameType : uint8
{
	Invalid,
	FromLoad,
	FromRPC
};

/**
 * 
 */
UCLASS()
class UPlayerExperienceSaveGame : public USaveGame
{
	GENERATED_UCLASS_BODY()
	
public:
	//Marks that this save game's data was from a local file load.
	UFUNCTION()
	void MarkFromLoad();
	//Marks that this save game's data was received from an RPC.
	UFUNCTION()
	void MarkFromRPC();

	UFUNCTION()
	ESaveGameType GetSaveType() const { return SaveGameType; }

	UFUNCTION()
	bool IsSaving() const { return bIsSaving; }
	UFUNCTION()
	bool IsPendingSave() const { return bPendingSave; }

	UFUNCTION()
	uint64 GetExperienceValue() const;
	UFUNCTION()
	uint64 SetExperienceValue(uint64 InValue);
	UFUNCTION()
	uint64 AddExperienceValue(uint64 Delta);

	const FExperienceStruct& GetExperienceData() const { return Experience; }
	const FPlayerStatisticsStruct& GetPlayerStatisticsData() const { return Statistics; }
	const FPlayerSelectionStruct& GetPlayerSelectionData() const { return PlayerSelection; }

	FExperienceStruct& GetExperienceDataMutable() { return Experience; }
	FPlayerStatisticsStruct& GetPlayerStatisticsDataMutable() { return Statistics; }
	FPlayerSelectionStruct& GetPlayerSelectionDataMutable() { return PlayerSelection; }

	UFUNCTION()
	static FString GetPlayerID(ACorePlayerController* PlayerController);
	UFUNCTION()
	static UPlayerExperienceSaveGame* LoadPlayerSaveData(ACorePlayerController* PlayerController);
	UFUNCTION()
	static UPlayerExperienceSaveGame* CreateRemoteAuthorityPlayerSaveData(ACorePlayerController* PlayerController);
	UFUNCTION()
	static bool ResetPlayerSaveData(ACorePlayerController* PlayerController);

	UFUNCTION()
	static bool RequestSave(UPlayerExperienceSaveGame* SaveGame);

	UFUNCTION()
	bool ReceiveServerStatistics(const FPlayerStatisticsStruct& ServerStatistics);
	UFUNCTION()
	bool ReceiveServerExperience(const FExperienceStruct& ServerExperience);

	void SaveCompleted(const FString& SlotName, const int32 UserIndex, bool bSuccess);
	
	//Performs MoveTemp operations on parameters. Returns false if failed.
	bool PushPlayerData(FExperienceStruct& InExperience, FPlayerStatisticsStruct& InStatistics, FPlayerSelectionStruct& InPlayerSelection);

	UFUNCTION()
	uint64 GetPlayerStatisticsValue(EPlayerStatisticType StatisticsType) const;
	UFUNCTION()
	uint64 SetPlayerStatisticsValue(EPlayerStatisticType StatisticsType, uint64 InValue);
	UFUNCTION()
	uint64 AddPlayerStatisticsValue(EPlayerStatisticType StatisticsType, uint64 Delta);

	UFUNCTION()
	bool CheckAndMarkStatusEffectReceived(TSubclassOf<UStatusEffectBase> StatusEffectClass);

protected:
	UPROPERTY()
	FExperienceStruct Experience = FExperienceStruct();

	UPROPERTY()
	FPlayerStatisticsStruct Statistics = FPlayerStatisticsStruct();

	UPROPERTY()
	FPlayerSelectionStruct PlayerSelection = FPlayerSelectionStruct();

	UPROPERTY()
	FPlayerLocalDataStruct LocalData = FPlayerLocalDataStruct();

	UPROPERTY(Transient)
	ESaveGameType SaveGameType = ESaveGameType::Invalid;
	
	UPROPERTY(Transient)
	FString LoadedSlotName = "";
	UPROPERTY(Transient)
	bool bIsSaving = false;
	UPROPERTY(Transient)
	bool bPendingSave = false;

	UPROPERTY(Transient)
	uint64 ExperienceGainSinceSave = 0;

#if WITH_EDITOR

public:
	void MarkEditorSave() { bIsEditorSave = true; }

	bool bIsEditorSave = false;
	
	bool IsEditorSave() const { return bIsEditorSave; }

#else

public:
	bool IsEditorSave() const { return false; }

#endif //WITH_EDITOR
};
