// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.


#include "Overlord/DungeonGameModeSettings.h"
#include "System/CoreSingleton.h"
#include "System/CoreGameMode.h"
#include "Character/DungeonCharacter.h"

UDungeonGameModeSettings::UDungeonGameModeSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UDungeonWaveSetup::UDungeonWaveSetup(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

TArray<UWaveConfiguration*> UDungeonWaveSetup::GetWaveConfiguration(int64 WaveNumber) const
{
	bool bIsOverrideWave = false;
	int32 OverridePriority = INDEX_NONE;
	UWaveConfiguration* BaseConfiguration = DefaultWaveConfiguration;
	TArray<UWaveConfiguration*> Result;

	for (const FWaveModifierEntry& Entry : ModifierList)
	{
		const UWaveModifier* Modifier = Entry.GetModifier();

		if (!Modifier || !Modifier->AppliesToWave(WaveNumber))
		{
			continue;
		}
		
		if (!Modifier->IsOverrideWave() || Modifier->GetModifierPriority() < OverridePriority)
		{
			continue;
		}

		UWaveConfiguration* Configuration = Entry.GetConfiguration();
		if (!Configuration)
		{
			continue;
		}

		BaseConfiguration = Configuration;
		bIsOverrideWave = true;
		OverridePriority = Modifier->GetModifierPriority();
	}

	ensure(BaseConfiguration);
	Result.Add(BaseConfiguration);

	for (const FWaveModifierEntry& Entry : ModifierList)
	{
		const UWaveModifier* Modifier = Entry.GetModifier();

		if (!Modifier || !Modifier->AppliesToWave(WaveNumber))
		{
			continue;
		}

		if (!Modifier->IsAdditiveWave() || Modifier->GetModifierPriority() < OverridePriority)
		{
			continue;
		}
		
		UWaveConfiguration* Configuration = Entry.GetConfiguration();
		if (!Configuration)
		{
			continue;
		}

		Result.Add(Configuration);
	}

	return Result;
}

bool UDungeonWaveSetup::InitializeWaveSetup(int64 WaveNumber)
{
	TArray<UWaveConfiguration*> WaveConfiguration = GetWaveConfiguration(WaveNumber);

	if (WaveConfiguration.Num() == 0)
	{
		return false;
	}

	for (UWaveConfiguration* Wave : WaveConfiguration)
	{
		Wave->InitializeForWave(WaveNumber, this);
	}

	InitializedWave = WaveNumber;
	return true;
}

bool UDungeonWaveSetup::StartWave(int64 WaveNumber)
{
	if (InitializedWave != WaveNumber)
	{
		return false;
	}

	TArray<UWaveConfiguration*> WaveConfiguration = GetWaveConfiguration(WaveNumber);
	
	if (WaveConfiguration.Num() == 0)
	{
		return false;
	}

	for (UWaveConfiguration* Wave : WaveConfiguration)
	{
		Wave->StartSpawning();
	}

	return true;
}

void UDungeonWaveSetup::OnWaveCharacterListEmpty()
{
	if (DefaultWaveConfiguration && DefaultWaveConfiguration->IsInitialized() && !DefaultWaveConfiguration->IsDoneSpawning())
	{
		return;
	}

	for (const FWaveModifierEntry& Entry : ModifierList)
	{
		const UWaveConfiguration* WaveConfiguration = Entry.GetConfiguration();

		if (!WaveConfiguration || !WaveConfiguration->IsInitialized())
		{
			continue;
		}

		if (!WaveConfiguration->IsDoneSpawning())
		{
			return;
		}
	}

	OnWaveCompleted.Broadcast(this, InitializedWave);
}

bool UDungeonWaveSetup::IsWaveAutoStart(const TArray<UWaveConfiguration*>& Configuration) const
{
	for (UWaveConfiguration* WaveConfig : Configuration)
	{
		if (!WaveConfig)
		{
			continue;
		}

		if (WaveConfig->IsAutoStartWave())
		{
			return true;
		}
	}

	return false;
}

int32 UDungeonWaveSetup::GetWaveAutoStartTime(const TArray<UWaveConfiguration*>& Configuration) const
{
	int32 LowestAutoStartTime = MAX_int32;

	for (UWaveConfiguration* WaveConfig : Configuration)
	{
		if (!WaveConfig)
		{
			continue;
		}

		if (!WaveConfig->IsAutoStartWave())
		{
			continue;
		}

		if (LowestAutoStartTime < WaveConfig->GetAutoStartTime())
		{
			continue;
		}

		LowestAutoStartTime = WaveConfig->GetAutoStartTime();
	}

	return LowestAutoStartTime == MAX_int32 ? INDEX_NONE : LowestAutoStartTime;
}

UDungeonWaveSetup* UDungeonWaveSetup::CreateWaveSetup(UObject* WorldContextObject, TSubclassOf<UDungeonWaveSetup> SetupClass, const TArray<FWaveModifierEntry>& AdditionalModifiers, bool bClearSetupClassModifiers,
	UWaveConfiguration* DefaultWaveConfigurationOverride, UCurveFloat* DefaultWaveSizeScalingOverride, UCurveFloat* DefaultWaveRateScalingOverride, int64 LastWaveNumberOverride)
{
	SetupClass = SetupClass ? SetupClass : UDungeonWaveSetup::StaticClass();

	UDungeonWaveSetup* Setup = NewObject<UDungeonWaveSetup>(WorldContextObject, SetupClass);

	if (!Setup)
	{
		return nullptr;
	}

	if (bClearSetupClassModifiers)
	{
		Setup->ModifierList.Reset();
	}

	if (AdditionalModifiers.Num() != 0)
	{
		Setup->ModifierList.Append(AdditionalModifiers);
	}

	if (DefaultWaveConfigurationOverride)
	{
		Setup->DefaultWaveConfiguration = DefaultWaveConfigurationOverride;
	}

	if (DefaultWaveSizeScalingOverride)
	{
		Setup->DefaultWaveSizeScaling = DefaultWaveSizeScalingOverride;
	}

	if (DefaultWaveRateScalingOverride)
	{
		Setup->DefaultWaveRateScaling = DefaultWaveRateScalingOverride;
	}

	if (LastWaveNumberOverride != INDEX_NONE)
	{
		Setup->LastWaveNumber = LastWaveNumberOverride;
	}

	return Setup;
}

FWaveModifierEntry UDungeonWaveSetup::CreateModifierEntry(UWaveModifier* Modifier, UWaveConfiguration* Configuration)
{
	return {Modifier, Configuration};
}

UWaveConfiguration::UWaveConfiguration(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SpawnerTags = FGameplayTagContainer(FGameplayTag::RequestGameplayTag("Spawner.Generic"));
}

int32 UWaveConfiguration::InitializeForWave(int64 WaveNumber, UDungeonWaveSetup* Setup)
{
	CleanUp();
	InitializedWaveNumber = WaveNumber;
	TotalSpawnCount = GetSpawnCount(WaveNumber, Setup);
	NumberSpawned = 0;
	RequestedSpawnCount = 0;

	CurrentSpawnTimeOffset = SpawnTimeOffset;
	if (SpawnTimeOffsetWaveCurve)
	{
		CurrentSpawnTimeOffset = FMath::CeilToInt(float(CurrentSpawnTimeOffset) * SpawnTimeOffsetWaveCurve->GetFloatValue(WaveNumber));

		if (!bIgnoreSettingsWaveSpawnRateCurveForTimeOffset && Setup && Setup->GetDefaultWaveRateScaling())
		{
			CurrentSpawnTimeOffset = FMath::CeilToInt(float(CurrentSpawnTimeOffset) * Setup->GetDefaultWaveRateScaling()->GetFloatValue(WaveNumber));
		}
	}

	CurrentSpawnInterval = SpawnIntervalTime;
	if (SpawnIntervalTimeWaveCurve)
	{
		CurrentSpawnInterval = FMath::CeilToInt(float(CurrentSpawnInterval) * SpawnIntervalTimeWaveCurve->GetFloatValue(WaveNumber));

		if (!bIgnoreSettingsWaveSpawnRateCurveForTimeInterval && Setup && Setup->GetDefaultWaveRateScaling())
		{
			CurrentSpawnInterval = FMath::CeilToInt(float(CurrentSpawnInterval) * Setup->GetDefaultWaveRateScaling()->GetFloatValue(WaveNumber));
		}
	}

	CurrentSpawnBatchAmount = SpawnBatchAmount;
	if (SpawnBatchAmountWaveCurve)
	{
		CurrentSpawnBatchAmount = FMath::CeilToInt(float(CurrentSpawnBatchAmount) * SpawnBatchAmountWaveCurve->GetFloatValue(WaveNumber));

		if (!bIgnoreSettingsWaveSpawnSizeCurveForSpawnBatchAmount && Setup && Setup->GetDefaultWaveSizeScaling())
		{
			CurrentSpawnBatchAmount = FMath::CeilToInt(float(CurrentSpawnBatchAmount) * Setup->GetDefaultWaveSizeScaling()->GetFloatValue(WaveNumber));
		}
	}

	for (UWaveSpawnGroup* Group : SpawnWaveGroups)
	{
		if (!Group)
		{
			continue;
		}

		Group->Initialize();
	}

	return TotalSpawnCount;
}

void UWaveConfiguration::CleanUp()
{
	StopSpawning();
	TotalSpawnCount = -1;
	NumberSpawned = -1;
	RequestedSpawnCount = 0;

	CurrentSpawnTimeOffset = -1;

	CurrentSpawnInterval = -1;

	CurrentSpawnBatchAmount = -1;
}

void UWaveConfiguration::StartSpawning()
{
	if (!GetWorld())
	{
		return;
	}

	if (CurrentSpawnTimeOffset > 0)
	{
		TWeakObjectPtr<UWaveConfiguration> WeakThis(this);
		GetWorld()->GetTimerManager().SetTimer(NextSpawnIntervalTimerHandle, FTimerDelegate::CreateWeakLambda(this, [WeakThis]()
		{
			if(!WeakThis.IsValid())
			{
				return;
			}

			//Clear before call so that we don't confuse anyone when calling PerformSpawn.
			WeakThis->GetWorld()->GetTimerManager().ClearTimer(WeakThis->NextSpawnIntervalTimerHandle);
			WeakThis->PerformSpawn();
		}), CurrentSpawnTimeOffset, true);
	}
	else
	{
		PerformSpawn();
	}
}

bool UWaveConfiguration::IsDoneSpawning() const
{
	return (TotalSpawnCount - NumberSpawned) <= 0;
}

int32 UWaveConfiguration::NumberRemainingToSpawn() const
{
	return (TotalSpawnCount - NumberSpawned) - RequestedSpawnCount;
}

bool UWaveConfiguration::IsSpawnIntervalTimerActive() const
{
	if (!GetWorld())
	{
		return false;
	}

	return GetWorld()->GetTimerManager().IsTimerActive(NextSpawnIntervalTimerHandle);
}

int32 UWaveConfiguration::GetSpawnCount(int64 WaveNumber, UDungeonWaveSetup* Setup) const
{
	const float LocalScaling = WaveSizeScalingCurve ? WaveSizeScalingCurve->GetFloatValue(WaveNumber) : 1.f;
	const float SettingsScaling = (!bIgnoreSettingsWaveSizeScalingCurve && Setup && Setup->GetDefaultWaveSizeScaling()) ? Setup->GetDefaultWaveSizeScaling()->GetFloatValue(WaveNumber) : 1.f;
	return FMath::CeilToInt(float(BaseSpawnCount) * LocalScaling * SettingsScaling);
}

UWaveConfiguration* UWaveConfiguration::CreateWaveConfiguration(UObject* WorldContextObject, TSubclassOf<UWaveConfiguration> WaveClass, FGameplayTagContainer AdditionalSpawnerTags,
	bool bClearWaveClassSpawnTags, int32 BaseSpawnCountOverride, int32 SpawnTimeOffsetOverride, int32 SpawnIntervalTimeOverride, int32 SpawnBatchAmountOverride)
{
	WaveClass = WaveClass ? WaveClass : UWaveConfiguration::StaticClass();

	UWaveConfiguration* Wave = NewObject<UWaveConfiguration>(WorldContextObject, WaveClass);

	if (!Wave)
	{
		return nullptr;
	}

	if (BaseSpawnCountOverride != INDEX_NONE)
	{
		Wave->BaseSpawnCount = BaseSpawnCountOverride;
	}

	if (SpawnTimeOffsetOverride != INDEX_NONE)
	{
		Wave->SpawnTimeOffset = SpawnTimeOffsetOverride;
	}

	if (SpawnIntervalTimeOverride != INDEX_NONE)
	{
		Wave->SpawnIntervalTime = SpawnIntervalTimeOverride;
	}

	if (SpawnBatchAmountOverride != INDEX_NONE)
	{
		Wave->SpawnBatchAmount = SpawnBatchAmountOverride;
	}

	if (bClearWaveClassSpawnTags)
	{
		if (AdditionalSpawnerTags.IsEmpty())
		{
			Wave->SpawnerTags = FGameplayTagContainer(FGameplayTag::RequestGameplayTag("Spawner.Generic"));
		}
		else
		{
			Wave->SpawnerTags = AdditionalSpawnerTags;
		}
	}
	else
	{
		if (AdditionalSpawnerTags.IsEmpty())
		{
			Wave->SpawnerTags.AppendTags(AdditionalSpawnerTags);
		}
	}

	return Wave;
}

void UWaveConfiguration::StopSpawning()
{
	if (GetWorld())
	{
		GetWorld()->GetLatentActionManager().RemoveActionsForObject(this);
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}

	USpawnCharacterSystem::CancelRequestsForObject(this, this);
}

void UWaveConfiguration::SetNextSpawnInterval()
{
	TWeakObjectPtr<UWaveConfiguration> WeakThis(this);
	GetWorld()->GetTimerManager().SetTimer(NextSpawnIntervalTimerHandle, FTimerDelegate::CreateWeakLambda(this, [WeakThis]()
	{
		if(!WeakThis.IsValid())
		{
			return;
		}

		//Clear before call so that we don't confuse anyone when calling PerformSpawn.
		WeakThis->GetWorld()->GetTimerManager().ClearTimer(WeakThis->NextSpawnIntervalTimerHandle);
		WeakThis->PerformSpawn();

	}), CurrentSpawnInterval, false);
}

int32 UWaveConfiguration::PerformSpawn()
{
	int32 NumSpawned = 0;

	TWeakObjectPtr<UWaveConfiguration> WeakThis(this);

	TArray<TSubclassOf<ADungeonCharacter>> CharacterSpawnList = GetNextSpawnList();
	FActorSpawnParameters ASP = FActorSpawnParameters();
	ASP.bDeferConstruction = true;

	TArray<ISpawnLocationInterface*> SpawnLocationList = GetSpawnLocationList();

	for (TSubclassOf<ADungeonCharacter> CharacterClass : CharacterSpawnList)
	{
		ISpawnLocationInterface* SpawnLocation = nullptr;

		for (ISpawnLocationInterface* TestSpawnLocationInterface : SpawnLocationList)
		{
			if (!TestSpawnLocationInterface->HasAvailableSpawnTransform(CharacterClass))
			{
				continue;
			}

			SpawnLocation = TestSpawnLocationInterface;
			break;
		}

		if (!SpawnLocation)
		{
			RefundFailedSpawn(CharacterClass);
			continue;
		}

		FTransform Transform;
		if (!SpawnLocation->GetSpawnTransform(CharacterClass, Transform))
		{
			RefundFailedSpawn(CharacterClass);
			continue;
		}

		const bool bResult = USpawnCharacterSystem::RequestSpawn(this, CharacterClass, Transform, ASP, FCharacterSpawnRequestDelegate::CreateWeakLambda(this, [WeakThis](const FSpawnRequest& Request, ACoreCharacter* Character)
		{
			if (!WeakThis.IsValid())
			{
				return;
			}

			WeakThis->OnSpawnRequestResult(Request, Character);
		}));

		if (bResult)
		{
			RequestedSpawnCount++;
		}
	}

	if (!IsSpawnIntervalTimerActive())
	{
		SetNextSpawnInterval();
	}

	return NumSpawned;
}

void UWaveConfiguration::OnSpawnRequestResult(const FSpawnRequest& Request, ACoreCharacter* Character)
{
	RequestedSpawnCount--;

	if(!Character)
	{
		OnSpawnFailed(Request);
		return;
	}

	if (ACoreGameMode* GameMode = GetWorld()->GetAuthGameMode<ACoreGameMode>())
	{
		GameMode->SetPlayerDefaults(Character);
	}

	NumberSpawned++;

	Character->FinishSpawning(Character->GetActorTransform());
}

void UWaveConfiguration::OnSpawnFailed(const FSpawnRequest& Request)
{
	RefundFailedSpawn(TSubclassOf<ADungeonCharacter>(Request.GetCharacterClass()));

	if (IsDoneSpawning() || IsSpawnIntervalTimerActive())
	{
		return;
	}
	
	SetNextSpawnInterval();
}

void UWaveConfiguration::RefundFailedSpawn(TSubclassOf<ADungeonCharacter> CharacterClass)
{
	for (UWaveSpawnGroup* Group : SpawnWaveGroups)
	{
		if (!Group)
		{
			continue;
		}

		if (Group->RefundSpawnClass(CharacterClass))
		{
			return;
		}
	}
}

TArray<TSubclassOf<ADungeonCharacter>> UWaveConfiguration::GetNextSpawnList()
{
	int32 NumberToSpawn = FMath::Min(NumberRemainingToSpawn(), CurrentSpawnBatchAmount);
	TArray<TSubclassOf<ADungeonCharacter>> SpawnList;
	SpawnList.Reserve(NumberToSpawn);

	int32 MaxTries = 4;

	while (SpawnList.Num() < NumberToSpawn && MaxTries > 0)
	{
		for (UWaveSpawnGroup* Group : SpawnWaveGroups)
		{
			if (!Group || !Group->HasRemainingSpawns())
			{
				continue;
			}

			while (Group->HasRemainingSpawns())
			{
				SpawnList.Add(Group->GetNextSpawnClass());

				if (SpawnList.Num() >= NumberToSpawn)
				{
					return SpawnList;
				}
			}
		}

		for (UWaveSpawnGroup* Group : SpawnWaveGroups)
		{
			if (!Group)
			{
				continue;
			}

			Group->Initialize();
		}

		MaxTries--;
	}

	return SpawnList;
}

TArray<ISpawnLocationInterface*> UWaveConfiguration::GetSpawnLocationList()
{
	static TArray<AActor*> TaggedActorList;
	UCoreSingleton::GetActorsWithTag(this, TaggedActorList, SpawnerTags);

	TArray<ISpawnLocationInterface*> TaggedSpawnLocations;
	TaggedSpawnLocations.Reserve(TaggedActorList.Num());

	for (AActor* Actor : TaggedActorList)
	{
		ISpawnLocationInterface* SpawnLocationInterface = Cast<ISpawnLocationInterface>(Actor);

		if (!SpawnLocationInterface)
		{
			continue;
		}

		TaggedSpawnLocations.Add(SpawnLocationInterface);
	}

	return TaggedSpawnLocations;
}

UWaveModifier::UWaveModifier(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

bool UWaveModifier::AppliesToWave(int64 WaveNumber) const
{
	if (WaveNumber < StartingWaveNumber)
	{
		return false;
	}

	if (DesiredWaveNumber.Num() != 0 && !DesiredWaveNumber.Contains(WaveNumber))
	{
		return false;
	}

	if (DesiredWaveInterval.Num() != 0)
	{
		const bool bIsDesiredWaveInterval = DesiredWaveInterval.ContainsByPredicate([WaveNumber](int32 Interval)
			{
				return (WaveNumber % Interval) == 0;
			});

		if (bIsDesiredWaveInterval)
		{
			return true;
		}

		return false;
	}

	return true;
}

UWaveModifier* UWaveModifier::CreateWaveModifier(UObject* WorldContextObject, TSubclassOf<UWaveModifier> ModifierClass,
	EWaveModifierType ModifierTypeOverride, int32 StartingWaveNumberOverride, int32 PriorityOverride)
{
	ModifierClass = ModifierClass ? ModifierClass : UWaveModifier::StaticClass();

	UWaveModifier* Modifer = NewObject<UWaveModifier>(WorldContextObject, ModifierClass);

	if (!Modifer)
	{
		return nullptr;
	}

	if (ModifierTypeOverride != EWaveModifierType::None)
	{
		Modifer->ModifierType = ModifierTypeOverride;
	}

	if (StartingWaveNumberOverride != INDEX_NONE)
	{
		Modifer->StartingWaveNumber = StartingWaveNumberOverride;
	}

	if (PriorityOverride != INDEX_NONE)
	{
		Modifer->Priority = PriorityOverride;
	}

	return Modifer;
}

void FSpawnGroupEntry::Initialize()
{
	if (!LoadedCharacter)
	{
		LoadedCharacter = Character.LoadSynchronous();
	}
	RemainingToSpawn = SpawnCount;
}

TSubclassOf<ADungeonCharacter> FSpawnGroupEntry::GetNextSpawnClass()
{
	if (RemainingToSpawn <= 0)
	{
		return nullptr;
	}

	RemainingToSpawn--;
	return LoadedCharacter;
}

bool FSpawnGroupEntry::RefundSpawnClass(TSubclassOf<ADungeonCharacter> CharacterClass)
{
	if (LoadedCharacter != CharacterClass)
	{
		return false;
	}

	RemainingToSpawn++;
	return true;
}

UWaveSpawnGroup::UWaveSpawnGroup(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void UWaveSpawnGroup::Initialize()
{
	for (FSpawnGroupEntry& Entry : SpawnList)
	{
		Entry.Initialize();
	}
}

bool UWaveSpawnGroup::HasRemainingSpawns() const
{
	for (const FSpawnGroupEntry& Entry : SpawnList)
	{
		if (Entry.HasRemainingSpawns())
		{
			return true;
		}
	}

	return false;
}

TSubclassOf<ADungeonCharacter> UWaveSpawnGroup::GetNextSpawnClass()
{
	TSubclassOf<ADungeonCharacter> SpawnClass = nullptr;
	for (FSpawnGroupEntry& Entry : SpawnList)
	{
		SpawnClass = Entry.GetNextSpawnClass();

		if (SpawnClass)
		{
			break;
		}
	}

	return SpawnClass;
}

bool UWaveSpawnGroup::RefundSpawnClass(TSubclassOf<ADungeonCharacter> CharacterClass)
{
	for (FSpawnGroupEntry& Entry : SpawnList)
	{
		if (Entry.RefundSpawnClass(CharacterClass))
		{
			return true;
		}
	}

	return false;
}