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

	if (ADungeonCharacter* DungeonCharacter = Cast<ADungeonCharacter>(PlayerPawn))
	{
		AddPawnToWaveCharacters(DungeonCharacter);
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

	if (ADungeonLevelScriptActor* DungeonLevelScriptActor = Cast<ADungeonLevelScriptActor>(GetWorld()->GetLevelScriptActor()))
	{
		if (UDungeonWaveSetup* WaveSetup = DungeonLevelScriptActor->GenerateDungeonWaveSettings())
		{
			ensure(!CurrentWaveSetup);
			CurrentWaveSetup = WaveSetup;
			CurrentWaveSetup->OnWaveCompleted.AddDynamic(this, &ADungeonGameMode::OnWaveCompleted);

			ADungeonGameState* DungeonGameState = GetGameState<ADungeonGameState>();
			ensure(DungeonGameState);
			
			int64 WaveNumber = DungeonGameState->IncrementWaveNumber();

			WaveSetup->InitializeWaveSetup(WaveNumber);
			WaveSetup->StartWave(WaveNumber);
		}
	}
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
}

void ADungeonGameMode::AddPawnToWaveCharacters(ADungeonCharacter* Character)
{
	WaveCharacters.Add(Character);
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

	if (CurrentWaveSetup)
	{
		CurrentWaveSetup->OnWaveCharacterListEmpty();
	}
}

void ADungeonGameMode::OnWaveCompleted(UDungeonWaveSetup* WaveSetup, int64 WaveNumber)
{
	if (!WaveSetup)
	{
		return;
	}

	TArray<UWaveConfiguration*> WaveConfigurationList = WaveSetup->GetWaveConfiguration(WaveNumber + 1);
	if (!WaveSetup->IsWaveAutoStart(WaveConfigurationList))
	{
		return;
	}

	TWeakObjectPtr<ADungeonGameMode> WeakThis(this);
	int32 WaveAutoStartTime = WaveSetup->GetWaveAutoStartTime(WaveConfigurationList);
	GetWorld()->GetTimerManager().SetTimer(AutoStartWaveTimerHandle, FTimerDelegate::CreateWeakLambda(this, [WeakThis]()
	{
		if (!WeakThis.IsValid())
		{
			return;
		}

		WeakThis->StartNextWave();
	}), WaveAutoStartTime, false);
}

void ADungeonGameMode::StartNextWave()
{
	ensure(CurrentWaveSetup);

	ADungeonGameState* DungeonGameState = GetGameState<ADungeonGameState>();
	ensure(DungeonGameState);

	int64 WaveNumber = DungeonGameState->IncrementWaveNumber();
	
	CurrentWaveSetup->InitializeWaveSetup(WaveNumber);
	CurrentWaveSetup->StartWave(WaveNumber);
}