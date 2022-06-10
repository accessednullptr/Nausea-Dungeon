// Copyright Epic Games, Inc. All Rights Reserved.

#include "Overlord/DungeonGameMode.h"
#include "Overlord/DungeonGameState.h"
#include "System/DungeonLevelScriptActor.h"
#include "Overlord/DungeonGameModeSettings.h"
#include "Player/DungeonPlayerController.h"
#include "Player/DungeonPlayerState.h"
#include "Character/DungeonCharacter.h"
#include "Overlord/TrapBase.h"
#include "Gameplay/StatusComponent.h"

ADungeonGameMode::ADungeonGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerControllerClass = ADungeonPlayerController::StaticClass();
	PlayerStateClass = ADungeonPlayerState::StaticClass();
}

void ADungeonGameMode::SetPlayerDefaults(APawn* PlayerPawn)
{
	Super::SetPlayerDefaults(PlayerPawn);
	
	if (!PlayerPawn || PlayerPawn->IsPendingKillPending())
	{
		return;
	}

	if (!PlayerPawn->OnDestroyed.IsAlreadyBound(this, &ADungeonGameMode::OnPawnDestroyed))
	{
		PlayerPawn->OnDestroyed.AddDynamic(this, &ADungeonGameMode::OnPawnDestroyed);
	}

	UStatusComponent* StatusComponent = UStatusInterfaceStatics::GetStatusComponent(PlayerPawn);

	if (StatusComponent && !StatusComponent->OnDied.IsAlreadyBound(this, &ADungeonGameMode::OnPawnDied))
	{
		StatusComponent->OnDied.AddDynamic(this, &ADungeonGameMode::OnPawnDied);
	}
}

void ADungeonGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	ADungeonGameState* DungeonGameState = GetGameState<ADungeonGameState>();
	ensure(DungeonGameState);

	UDungeonWaveSetup* WaveSetup = DungeonGameState->GetWaveSetup();
	ensure(WaveSetup);
	WaveSetup->OnWaveCompleted.AddDynamic(this, &ADungeonGameMode::OnWaveCompleted);

	const int64 WaveNumber = DungeonGameState->IncrementWaveNumber();
	const int64 WaveSize = WaveSetup->InitializeWaveSetup(WaveNumber);
	WaveSetup->StartWave(WaveNumber);
	DungeonGameState->SetWaveTotalSpawnCount(WaveSize);
}

void ADungeonGameMode::AddPawnToWaveCharacters(ADungeonCharacter* Character)
{
	if (!Character)
	{
		return;
	}

	WaveCharacters.Add(Character);
}

void ADungeonGameMode::OnPawnDestroyed(AActor* DestroyedActor)
{
	if (!GetWorld() || GetWorld()->bIsTearingDown)
	{
		return;
	}
	
	if (ADungeonCharacter* DungeonCharacter = Cast<ADungeonCharacter>(DestroyedActor))
	{
		RemovePawnFromWaveCharacters(DungeonCharacter);
	}
}

void ADungeonGameMode::OnPawnDied(UStatusComponent* Component, float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!Component)
	{
		return;
	}

	ADungeonCharacter* Character = Cast<ADungeonCharacter>(Component->GetOwner());

	if (!Character)
	{
		return;
	}

	GrantCoinsForKill(Character, EventInstigator, DamageCauser);
	RemovePawnFromWaveCharacters(Character);
}

void ADungeonGameMode::GrantCoinsForKill(ADungeonCharacter* KilledCharacter, AController* EventInstigator, AActor* DamageCauser)
{
	const ATrapBase* TrapDamageCauser = Cast<ATrapBase>(DamageCauser);

	if (!TrapDamageCauser)
	{
		return;
	}

	ADungeonPlayerState* DungeonPlayerState = Cast<ADungeonPlayerState>(TrapDamageCauser->GetOwner());

	if (!DungeonPlayerState)
	{
		return;
	}

	int32 CoinValue = KilledCharacter->GetCoinValue();
	ProcessCoinAmountForKill.Broadcast(KilledCharacter, EventInstigator, DamageCauser, CoinValue);
	DungeonPlayerState->AddTrapCoins(CoinValue);
	
	ADungeonGameState* DungeonGameState = GetGameState<ADungeonGameState>();
	ensure(DungeonGameState);
	DungeonGameState->SetWaveRemainingSpawnCount(DungeonGameState->GetWaveRemainingSpawnCount() - 1);
}

void ADungeonGameMode::RemovePawnFromWaveCharacters(ADungeonCharacter* Character)
{
	if (!WaveCharacters.Contains(Character))
	{
		return;
	}

	WaveCharacters.Remove(Character);

	if (WaveCharacters.Num() != 0)
	{
		return;
	}

	ADungeonGameState* DungeonGameState = GetGameState<ADungeonGameState>();
	ensure(DungeonGameState);
	if (UDungeonWaveSetup* WaveSetup = DungeonGameState->GetWaveSetup())
	{
		WaveSetup->OnWaveCharacterListEmpty();
	}
}

void ADungeonGameMode::OnWaveCompleted(UDungeonWaveSetup* WaveSetup, int64 WaveNumber)
{
	if (!WaveSetup)
	{
		return;
	}
	
	ADungeonGameState* DungeonGameState = GetGameState<ADungeonGameState>();
	ensure(DungeonGameState);

	UDungeonWaveSetup* CurrentWaveSetup = DungeonGameState->GetWaveSetup();

	if (WaveSetup != CurrentWaveSetup)
	{
		return;
	}
	
	TArray<UWaveConfiguration*> WaveConfigurationList = CurrentWaveSetup->GetWaveConfiguration(WaveNumber + 1);
	if (!WaveSetup->IsWaveAutoStart(WaveConfigurationList))
	{
		return;
	}

	const int32 WaveAutoStartTime = WaveSetup->GetWaveAutoStartTime(WaveConfigurationList);
	PerformAutoStart(WaveAutoStartTime);
}

void ADungeonGameMode::StartNextWave()
{
	ADungeonGameState* DungeonGameState = GetGameState<ADungeonGameState>();
	ensure(DungeonGameState);

	UDungeonWaveSetup* WaveSetup = DungeonGameState->GetWaveSetup();
	ensure(WaveSetup);

	const int64 WaveNumber = DungeonGameState->IncrementWaveNumber();
	
	const int64 WaveSize = WaveSetup->InitializeWaveSetup(WaveNumber);
	DungeonGameState->SetWaveTotalSpawnCount(WaveSize);

	WaveSetup->StartWave(WaveNumber);
}

void ADungeonGameMode::PerformAutoStart(int32 AutoStartTime)
{
	ADungeonGameState* DungeonGameState = GetGameState<ADungeonGameState>();
	ensure(DungeonGameState);

	TWeakObjectPtr<ADungeonGameMode> WeakThis(this);
	GetWorld()->GetTimerManager().SetTimer(AutoStartWaveTimerHandle, FTimerDelegate::CreateWeakLambda(this, [WeakThis]()
		{
			if (!WeakThis.IsValid())
			{
				return;
			}

			WeakThis->StartNextWave();
		}), AutoStartTime, false);

	DungeonGameState->SetAutoStartTime(AutoStartTime);
}