// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.


#include "Gameplay/PlayerOwnedStatusComponent.h"
#include "Player/PlayerOwnershipInterface.h"
#include "Player/CorePlayerState.h"

UPlayerOwnedStatusComponent::UPlayerOwnedStatusComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutomaticallyInitialize = false;
	bHideStatusComponentTeam = true;
}

void UPlayerOwnedStatusComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UPlayerOwnedStatusComponent::InitializeComponent()
{
	//Perform initialization here if not authority (APawn::SetPlayerDefaults will not run on simulated/autonomous proxies).
	//NOTE: bNetStartup means the player owned actor was loaded from level load, meaning it likely might be some time until possession/initialization.
	bAutomaticallyInitialize |= GetOwnerRole() != ROLE_Authority && !GetOwner()->bNetStartup;

	Super::InitializeComponent();
}

void UPlayerOwnedStatusComponent::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	bUseStatusComponentTeam = true;
	Super::SetGenericTeamId(NewTeamID);
}

FGenericTeamId UPlayerOwnedStatusComponent::GetGenericTeamId() const
{
	if (bUseStatusComponentTeam)
	{
		return TeamId;
	}

	if (GetOwningPlayerState())
	{
		return GetOwningPlayerState()->GetGenericTeamId();
	}

	return FGenericTeamId::NoTeam;
}

void UPlayerOwnedStatusComponent::InitializeStatusComponent()
{
	if (GetOwnerInterface())
	{
		return;
	}

	Super::InitializeStatusComponent();

	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	IPlayerOwnershipInterface* PlayerOwnershipInterface = Cast<IPlayerOwnershipInterface>(GetOwner());
	ensure(PlayerOwnershipInterface);
	
	if (!PlayerOwnershipInterface)
	{
		return;
	}

	CorePlayerState = PlayerOwnershipInterface->GetOwningPlayerState<ACorePlayerState>();
	
	if (!GetOwningPlayerState())
	{
		return;
	}

	if (!IsDead())
	{
		GetOwningPlayerState()->SetIsAlive(true);
	}
}

void UPlayerOwnedStatusComponent::TakeDamage(AActor* Actor, float& DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(Actor, DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void UPlayerOwnedStatusComponent::Died(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (GetOwningPlayerState())
	{
		GetOwningPlayerState()->SetIsAlive(false);
	}

	Super::Died(Damage, DamageEvent, EventInstigator, DamageCauser);
}

ACorePlayerState* UPlayerOwnedStatusComponent::GetOwningPlayerState() const
{
	return CorePlayerState;
}