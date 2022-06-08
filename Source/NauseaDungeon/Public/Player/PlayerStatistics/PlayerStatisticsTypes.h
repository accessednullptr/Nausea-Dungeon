// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Player/PlayerOwnershipInterface.h"
#include "PlayerStatisticsTypes.generated.h"

class UStatusEffectBase;

template<typename InKeyType, typename InValueType>
struct TMapPairStruct
{
	typedef InKeyType KeyType;
	typedef InValueType ValueType;
	TMapPairStruct() {}
	TMapPairStruct(KeyType InKey, ValueType InValue) { Key = InKey; Value = InValue; }
	
	friend FArchive& operator<<(FArchive& Ar, TMapPairStruct<KeyType, ValueType>& Entry) { Ar << Entry.Key; Ar << Entry.Value; return Ar; }

	FArchive& operator<<(FArchive& Ar) { Ar << Key; Ar << Value; return Ar; }

public:
	KeyType Key;
	ValueType Value;

public:
	FORCEINLINE static bool NetSerializeTemplateMap(FArchive& Ar, TMap<KeyType, ValueType>& SerializedMap)
	{
		const bool bIsLoading = Ar.IsLoading();
		TArray<TMapPairStruct<KeyType, ValueType>> DataArray;

		if (!bIsLoading)
		{
			for (const TPair<KeyType, ValueType>& Entry : SerializedMap)
			{
				DataArray.Add(TMapPairStruct<KeyType, ValueType>(Entry.Key, Entry.Value));
			}
		}

		Ar << DataArray;

		if (bIsLoading)
		{
			SerializedMap.Empty(SerializedMap.Num());
			for (const TMapPairStruct<KeyType, ValueType>& Entry : DataArray)
			{
				SerializedMap.Add(Entry.Key) = Entry.Value;
			}
		}

		return true;
	}
};

//----------------
//EXPERIENCE

USTRUCT(BlueprintType)
struct FExperienceStruct
{
	GENERATED_USTRUCT_BODY()

	FExperienceStruct() {}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

public:
	const uint64& GetValue() const { return Value; }

	uint64 AddValue(uint64 Delta) { Value += Delta; return Value; }
	uint64 SetValue(uint64 InValue) { Value = InValue; return Value; }

	bool ReplaceWithLarger(const FExperienceStruct& Other);

protected:
	UPROPERTY(NotReplicated)
	uint64 Value = 0;
};
template<>
struct TStructOpsTypeTraits<FExperienceStruct> : public TStructOpsTypeTraitsBase2<FExperienceStruct>
{
	enum
	{
		WithNetSerializer = true
	};
};

//----------------
//PLAYER STATS


UENUM(BlueprintType)
enum class EPlayerStatisticType : uint8
{
	Invalid,
	DamageDealt,
	DamageReceived,
	DamageHealed
};

USTRUCT(BlueprintType)
struct FPlayerStatisticsStruct
{
	GENERATED_USTRUCT_BODY()

	FPlayerStatisticsStruct() {}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	FORCEINLINE uint64 Get(EPlayerStatisticType StatisticType) const { return PlayerStatisticMap.Contains(StatisticType) ? PlayerStatisticMap[StatisticType] : 0; }
	FORCEINLINE uint64 Set(EPlayerStatisticType StatisticType, uint64 Value) { return PlayerStatisticMap.FindOrAdd(StatisticType) = Value; }
	FORCEINLINE uint64 Add(EPlayerStatisticType StatisticType, uint64 Delta) { return PlayerStatisticMap.FindOrAdd(StatisticType) += Delta; }

	FORCEINLINE const TMap<EPlayerStatisticType, uint64>& Get() const { return PlayerStatisticMap; }
	FORCEINLINE TMap<EPlayerStatisticType, uint64>& GetMutable() { return PlayerStatisticMap; }

	bool ReplaceWithLarger(const FPlayerStatisticsStruct& Other);

protected:
	UPROPERTY(NotReplicated)
	TMap<EPlayerStatisticType, uint64> PlayerStatisticMap = TMap<EPlayerStatisticType, uint64>();
};
template<>
struct TStructOpsTypeTraits<FPlayerStatisticsStruct> : public TStructOpsTypeTraitsBase2<FPlayerStatisticsStruct>
{
	enum
	{
		WithNetSerializer = true
	};
};

//----------------
//PLAYER SELECTION

USTRUCT(BlueprintType)
struct FPlayerSelectionStruct
{
	GENERATED_USTRUCT_BODY()

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		bOutSuccess = true;
		return true;
	}
};
template<>
struct TStructOpsTypeTraits<FPlayerSelectionStruct> : public TStructOpsTypeTraitsBase2<FPlayerSelectionStruct>
{
	enum
	{
		WithNetSerializer = true
	};
};

USTRUCT(BlueprintType)
struct FPlayerLocalDataStruct
{
	GENERATED_USTRUCT_BODY()

	FORCEINLINE const TSet<TSoftClassPtr<UStatusEffectBase>>& GetReceivedStatusEffects() const { return ReceivedStatusEffectMap; }
	FORCEINLINE bool HasReceivedStatusEffect(TSoftClassPtr<UStatusEffectBase> StatusEffectSoftClass) const { return ReceivedStatusEffectMap.Contains(StatusEffectSoftClass); }
	FORCEINLINE FSetElementId MarkStatusEffectReceived(TSoftClassPtr<UStatusEffectBase> StatusEffectSoftClass) { return ReceivedStatusEffectMap.Add(StatusEffectSoftClass); }

	//Never store but keep track here.
	UPROPERTY(Transient)
	TSet<TSoftClassPtr<UStatusEffectBase>> ReceivedStatusEffectMap = TSet<TSoftClassPtr<UStatusEffectBase>>();
};

UCLASS()
class UPlayerStatisticHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION()
	static bool IncrementPlayerStatistic(TScriptInterface<IPlayerOwnershipInterface> PlayerOwnedInterface, EPlayerStatisticType StatisticType, float Amount, bool bImmediatelyStore = false);

	UFUNCTION()
	static bool IncrementPlayerExperience(TScriptInterface<IPlayerOwnershipInterface> PlayerOwnedInterface, uint64 Delta);

	UFUNCTION()
	static bool CheckAndMarkStatusEffectReceived(TScriptInterface<IPlayerOwnershipInterface> PlayerOwnedInterface, TSubclassOf<UStatusEffectBase> StatusEffectClass);
};