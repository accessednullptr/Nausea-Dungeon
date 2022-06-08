// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "CoreGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FPlayerKilledSignature, AController*, Killer, AController*, Killed, ACoreCharacter*, KilledCharacter, const struct FDamageEvent&, DamageEvent);

/**
 * 
 */
UCLASS()
class ACoreGameMode : public AGameMode
{
	GENERATED_UCLASS_BODY()

//ACoreGameMode command line arguments.
public:
	static const FString OptionDifficulty;
	static const FString OptionModifiers;

//~ Begin AGameModeBase Interface
public:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void InitGameState() override;
	virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;
	virtual void Broadcast(AActor* Sender, const FString& Msg, FName Type = NAME_None) override;
//~ End AGameModeBase Interface
	
//~ Begin AGameMode Interface
public:
	virtual void Say(const FString& Msg) override;
protected:
	virtual bool ReadyToStartMatch_Implementation() override;
//~ End AGameMode Interface

public:
	UFUNCTION(BlueprintCallable, Category = GameMode)
	uint8 GetGameDifficulty() const;

	uint8 GetDefaultGameDifficulty() const { return DefaultDifficulty; }

public:
	UPROPERTY(BlueprintAssignable, Category = GameMode)
	FPlayerKilledSignature OnPlayerKilled;
	
protected:
	UFUNCTION()
	void PlayerKilled(UStatusComponent* Component, float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

	UFUNCTION()
	virtual void NotifyKilled(AController* Killer, AController* Killed, ACoreCharacter* KilledCharacter, const struct FDamageEvent& DamageEvent);

protected:
	UPROPERTY(globalconfig)
	uint8 DefaultDifficulty = 0;
};
