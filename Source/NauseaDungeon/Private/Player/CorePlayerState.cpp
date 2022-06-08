// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.


#include "Player/CorePlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"
#include "NauseaNetDefines.h"
#include "System/CoreGameState.h"
#include "Character/VoiceComponent.h"

ACorePlayerState::ACorePlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	VoiceCommandComponent = CreateDefaultSubobject<UVoiceCommandComponent>(TEXT("VoiceCommandComponent"));
}

void ACorePlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_WITH_PARAMS_FAST(ACorePlayerState, bIsAlive, PushReplicationParams::Default);
	DOREPLIFETIME_WITH_PARAMS_FAST(ACorePlayerState, TeamID, PushReplicationParams::Default);
}

void ACorePlayerState::PostNetReceive()
{
	Super::PostNetReceive();
}

bool ACorePlayerState::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	check(Channel);
	check(Bunch);
	check(RepFlags);

	bool WroteSomething = false;
	bool bCachedNetInitial = RepFlags->bNetInitial;

	for (UActorComponent* Component : ReplicatedComponents)
	{
		if (Component && Component->GetIsReplicated())
		{
			RepFlags->bNetInitial = Channel->ReplicationMap.Find(Component) == nullptr;
			WroteSomething |= Component->ReplicateSubobjects(Channel, Bunch, RepFlags);

			WroteSomething = Channel->ReplicateSubobject(Component, *Bunch, *RepFlags) || WroteSomething;
		}
	}

	RepFlags->bNetInitial = bCachedNetInitial;

	return WroteSomething;
}

void ACorePlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (GetLocalRole() != ROLE_Authority)
	{
		if (ACoreGameState* CoreGameState = GetWorld()->GetGameState<ACoreGameState>())
		{
			CoreGameState->OnPlayerStateReady(this);
		}
	}
}

void ACorePlayerState::SetIsAlive(bool bInIsAlive)
{
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	bIsAlive = bInIsAlive;
	OnRep_IsAlive();
	MARK_PROPERTY_DIRTY_FROM_NAME(ACorePlayerState, bIsAlive, this);
	ForceNetUpdate();
}

void ACorePlayerState::OnRep_bIsSpectator()
{
	Super::OnRep_bIsSpectator();
	OnSpectatorChanged.Broadcast(this, IsSpectator());
}

void ACorePlayerState::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamID = NewTeamID;
	OnRep_TeamID();
	MARK_PROPERTY_DIRTY_FROM_NAME(ACorePlayerState, TeamID, this);
}

bool ACorePlayerState::IsActivePlayer() const
{
	if (IsABot() || IsSpectator() || IsInactive())
	{
		return false;
	}

	return true;
}

void ACorePlayerState::SetDefaultGenericTeamId(const FGenericTeamId& NewTeamID)
{
	SetGenericTeamId(NewTeamID);
}

void ACorePlayerState::OnRep_IsAlive()
{
	OnAliveChanged.Broadcast(this, IsAlive());
}

void ACorePlayerState::OnRep_TeamID()
{
	OnTeamChanged.Broadcast(this, GetGenericTeamId());
}