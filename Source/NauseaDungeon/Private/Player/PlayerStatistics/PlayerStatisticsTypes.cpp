// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.


#include "Player/PlayerStatistics/PlayerStatisticsTypes.h"
#include "Player/PlayerOwnershipInterface.h"
#include "Player/PlayerStatistics/PlayerStatisticsComponent.h"

bool FExperienceStruct::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = true;
	Ar << Value;
	return true;
}

bool FExperienceStruct::ReplaceWithLarger(const FExperienceStruct& Other)
{
	Value = FMath::Max(Value, Other.Value);
	return true;
}

typedef TMapPairStruct<EPlayerStatisticType, uint64> FPlayerStatisticsPairStruct;
bool FPlayerStatisticsStruct::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = true;
	return FPlayerStatisticsPairStruct::NetSerializeTemplateMap(Ar, PlayerStatisticMap);
}

bool FPlayerStatisticsStruct::ReplaceWithLarger(const FPlayerStatisticsStruct& Other)
{
	TArray<EPlayerStatisticType> OtherKeyList;
	Other.Get().GenerateKeyArray(OtherKeyList);

	for (EPlayerStatisticType Key : OtherKeyList)
	{
		Set(Key, FMath::Max(Get(Key), Other.Get(Key)));
	}

	return true;
}

UPlayerStatisticHelpers::UPlayerStatisticHelpers(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

bool UPlayerStatisticHelpers::IncrementPlayerStatistic(TScriptInterface<IPlayerOwnershipInterface> PlayerOwnedInterface, EPlayerStatisticType StatisticType, float Amount, bool bImmediatelyStore)
{
	if (!PlayerOwnedInterface || !PlayerOwnedInterface->GetPlayerStatisticsComponent())
	{
		return false;
	}

	if (PlayerOwnedInterface->GetPlayerStatisticsComponent()->GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}

	PlayerOwnedInterface->GetPlayerStatisticsComponent()->UpdatePlayerStatistic(StatisticType, Amount);
	return true;
}

bool UPlayerStatisticHelpers::IncrementPlayerExperience(TScriptInterface<IPlayerOwnershipInterface> PlayerOwnedInterface, uint64 Delta)
{
	if (!PlayerOwnedInterface || !PlayerOwnedInterface->GetPlayerStatisticsComponent())
	{
		return false;
	}

	if (PlayerOwnedInterface->GetPlayerStatisticsComponent()->GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}

	PlayerOwnedInterface->GetPlayerStatisticsComponent()->UpdatePlayerExperience(Delta);
	return true;
}


bool UPlayerStatisticHelpers::CheckAndMarkStatusEffectReceived(TScriptInterface<IPlayerOwnershipInterface> PlayerOwnedInterface, TSubclassOf<UStatusEffectBase> StatusEffectClass)
{
	if (!PlayerOwnedInterface || !PlayerOwnedInterface->GetPlayerStatisticsComponent())
	{
		return false;
	}

	return PlayerOwnedInterface->GetPlayerStatisticsComponent()->CheckAndMarkStatusEffectReceived(StatusEffectClass);
}