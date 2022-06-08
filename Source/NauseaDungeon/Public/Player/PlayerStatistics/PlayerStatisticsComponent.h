// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Player/PlayerStatistics/PlayerExperienceSaveGame.h"
#include "Player/PlayerStatistics/PlayerStatisticsTypes.h"
#include "PlayerStatisticsComponent.generated.h"

class UInventory;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPlayerDataReadySignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPlayerStatisticUpdateSignature, UPlayerStatisticsComponent*, PlayerStatistics, EPlayerStatisticType, StatisticType, uint64, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPlayerExperienceUpdateSignature, UPlayerStatisticsComponent*, PlayerStatistics, uint64, Delta);

//Responsible for holding player experience, stats, and user selections (player class, per class inventory, etc.).
UCLASS( ClassGroup=(Custom))
class UPlayerStatisticsComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

	friend class UPlayerStatisticHelpers;

public:
	UFUNCTION(BlueprintCallable, Category = PlayerStatistics)
	ACorePlayerController* GetOwningPlayerController() const;

	UFUNCTION(BlueprintCallable, Category = PlayerStatistics)
	bool IsLocalPlayerController() const;

	UFUNCTION(BlueprintCallable, Category = PlayerStatistics)
	bool IsPlayerStatsReady() const { return bPlayerStatsReady; }

	UFUNCTION(BlueprintCallable, Category = PlayerStatistics)
	UPlayerExperienceSaveGame* GetPlayerData() const { return IsPlayerStatsReady() ? PlayerData : nullptr; }

	UFUNCTION()
	void LoadPlayerData();
	UFUNCTION()
	void ResetPlayerData();
	UFUNCTION()
	void UpdateClientPlayerData();

public:
	//Broadcasted when stats are ready.
	//On standalone/auth listen server, this will be on load of local save data.
	//On authoritative remote, this will be when data has been successfully received and stored.
	//On remote client, this will be when the server has sent an ack'd of the loaded save data sent from the client.
	UPROPERTY()
	FPlayerDataReadySignature OnPlayerDataReady;

	UPROPERTY()
	FPlayerStatisticUpdateSignature OnPlayerStatisticsUpdate;

	UPROPERTY()
	FPlayerExperienceUpdateSignature OnPlayerExperienceUpdate;

protected:
	void SendPlayerData();
	void ReceivePlayerData(FExperienceStruct& Experience, FPlayerStatisticsStruct& Statistics, FPlayerSelectionStruct& PlayerSelection);
	void PlayerDataReady();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Reliable_SendPlayerData(FExperienceStruct Experience, FPlayerStatisticsStruct Statistics, FPlayerSelectionStruct PlayerSelection);
	UFUNCTION(Client, Reliable)
	void Client_Reliable_SendPlayerDataAck(bool bSuccess);

	UFUNCTION(Client, Reliable)
	void Client_Reliable_UpdatePlayerData(FExperienceStruct Experience, FPlayerStatisticsStruct Statistics);

	UFUNCTION()
	bool RequestSave();

	void UpdatePlayerStatistic(EPlayerStatisticType StatisticType, float Delta);
	void PushPlayerStatisticCache();

	void UpdatePlayerExperience(uint32 Delta);

	uint64 GetExperience() const;

	bool CheckAndMarkStatusEffectReceived(TSubclassOf<class UStatusEffectBase> StatusEffectClass);

protected:
	UPROPERTY()
	bool bPlayerStatsReady = false;

	UPROPERTY()
	UPlayerExperienceSaveGame* PlayerData = nullptr;

	//An intermediary cache meant to temporarily store statistics until they are pushed to stat storage. Represented as a float for fine-grain collection. 
	UPROPERTY()
	TMap<EPlayerStatisticType, float> PlayerStatisticsMapCache;
	//True if we've got a pending push of stats.
	UPROPERTY()
	bool bPendingStatPush = false;
};
