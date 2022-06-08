// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/StatusComponent.h"
#include "PlayerOwnedStatusComponent.generated.h"

class ACorePlayerState;

/**
 * 
 */
UCLASS()
class UPlayerOwnedStatusComponent : public UStatusComponent
{
	GENERATED_UCLASS_BODY()

//~ Begin UObject Interface 
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
//~ End UObject Interface

//~ Begin UActorComponent Interface 
protected:
	virtual void InitializeComponent() override;
//~ End UActorComponent Interface

//~ Begin IGenericTeamAgentInterface Interface
public:
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
//~ End IGenericTeamAgentInterface Interface

//~ Begin UStatusComponent Interface 
public:
	virtual void InitializeStatusComponent() override;
	virtual void TakeDamage(AActor* Actor, float& DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Died(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
//~ End UStatusComponent Interface

protected:
	UFUNCTION()
	ACorePlayerState* GetOwningPlayerState() const;

private:
	UPROPERTY()
	ACorePlayerState* CorePlayerState = nullptr;

	UPROPERTY()
	bool bUseStatusComponentTeam = false;

	FDelegateHandle DamageTakenDelegate;
	FDelegateHandle DamageDealtDelegate;
};