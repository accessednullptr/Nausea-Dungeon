// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GenericTeamAgentInterface.h"
#include "Player/PlayerOwnershipInterface.h"
#include "Gameplay/DamageLogInterface.h"
#include "CorePlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAliveChangedSignature, ACorePlayerState*, PlayerState, bool, bAlive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSpectatorChangedSignature, ACorePlayerState*, PlayerState, bool, bIsSpectating);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTeamChangedSignature, ACorePlayerState*, PlayerState, const FGenericTeamId&, NewTeam);

/**
 * 
 */
UCLASS(HideFunctions = (K2_GetDamageLogInstigatorName))
class ACorePlayerState : public APlayerState,
	public IGenericTeamAgentInterface,
	public IPlayerOwnershipInterface,
	public IDamageLogInterface
{
	GENERATED_UCLASS_BODY()
	
//~ Begin UObject Interface
public:
	virtual void PostNetReceive() override;
//~ End UObject Interface

//~ Begin AActor Interface
public:
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
protected:
	virtual void BeginPlay() override;
//~ End AActor Interface

//~ Begin APlayerState Interface
protected:
	virtual void OnRep_bIsSpectator() override;
//~ End APlayerState Interface

//~ Begin IGenericTeamAgentInterface Interface
public:
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamID; }
//~ End IGenericTeamAgentInterface Interface

//~ Begin IPlayerOwnershipInterface Interface
public:
	virtual ACorePlayerState* GetOwningPlayerState() const override { return const_cast<ACorePlayerState*>(this); }
	virtual UVoiceCommandComponent* GetVoiceCommandComponent() const override { return VoiceCommandComponent; }
	virtual FGenericTeamId GetOwningTeamId() const override { return GetGenericTeamId(); }
//~ End IPlayerOwnershipInterface Interface
	
//~ Begin IDamageLogInterface Interface
public:
	virtual FText GetDamageLogInstigatorName() const override { return FText::FromString(GetPlayerName()); }
//~ End IDamageLogInterface Interface

public:
	UFUNCTION(BlueprintCallable, Category = PlayerState)
	bool IsAlive() const { return bIsAlive; }

	void SetIsAlive(bool bInIsAlive);

	UFUNCTION(BlueprintCallable, Category = PlayerState)
	virtual bool IsActivePlayer() const;

public:
	UPROPERTY(BlueprintAssignable, Category = PlayerState)
	FSpectatorChangedSignature OnSpectatorChanged;

	UPROPERTY(BlueprintAssignable, Category = PlayerState)
	FAliveChangedSignature OnAliveChanged;

	UPROPERTY(BlueprintAssignable, Category = PlayerState)
	FTeamChangedSignature OnTeamChanged;

protected:
	//Used internally to set the default value of TeamID in a constructor.
	UFUNCTION()
	void SetDefaultGenericTeamId(const FGenericTeamId& NewTeamID);

	UFUNCTION()
	virtual void OnRep_IsAlive();

	UFUNCTION()
	virtual void OnRep_TeamID();

private:
	//Is this player alive.
	UPROPERTY(ReplicatedUsing = OnRep_IsAlive)
	bool bIsAlive = false;

	UPROPERTY(ReplicatedUsing = OnRep_TeamID)
	FGenericTeamId TeamID = FGenericTeamId::NoTeam;

	UPROPERTY(EditDefaultsOnly, Category = PlayerState)
	UVoiceCommandComponent* VoiceCommandComponent = nullptr;
};
