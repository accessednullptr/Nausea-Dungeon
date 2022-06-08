// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.

#include "Character/CoreCharacterMovementComponent.h"
#include "CoreMinimal.h"
#include "Stats/Stats.h"
#include "Components/CapsuleComponent.h"
#include "Character/CoreCharacter.h"
#include "Gameplay/StatusComponent.h"
#include "Player/CorePlayerState.h"

DECLARE_STATS_GROUP(TEXT("CoreCharacterMovement"), STATGROUP_CoreCharacterMovement, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("CoreCharacterMovement - Calculate Max Speed"), STAT_CoreCharacterMovement_CalculateMaxSpeed, STATGROUP_CoreCharacterMovement);
DECLARE_CYCLE_STAT(TEXT("CoreCharacterMovement - Calculate Max Acceleration"), STAT_CoreCharacterMovement_CalculateMaxAcceleration, STATGROUP_CoreCharacterMovement);

UCoreCharacterMovementComponent::UCoreCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NavAgentProps.bCanWalk = true;
	NavAgentProps.bCanCrouch = true;
	NavAgentProps.bCanJump = true;

	bUseAccelerationForPaths = true;
}

void UCoreCharacterMovementComponent::BeginPlay()
{
	SetGroundMovementMode(DefaultLandMovementMode);

	if (GetOwnerRole() != ROLE_Authority)
	{
		InitializeMovementComponent();
	}

	Super::BeginPlay();
}

float UCoreCharacterMovementComponent::GetMaxSpeed() const
{
	if (IsReplayPayloadAvailable())
	{
		return ReplayPayload.MaxSpeed;
	}

	float CurrentMaxSpeed = Super::GetMaxSpeed();
	
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreCharacterMovement_CalculateMaxSpeed);
		float MovementSpeedModifier = 1.f;
		OnProcessMovementSpeed.Broadcast(GetCoreCharacter(), MovementSpeedModifier);
		CurrentMaxSpeed *= MovementSpeedModifier;
	}

	return CurrentMaxSpeed;
}

float UCoreCharacterMovementComponent::GetMaxAcceleration() const
{
	float CurrentMaxAcceleration = Super::GetMaxAcceleration();

	if(!IsClientReplayingMoves())
	{
		SCOPE_CYCLE_COUNTER(STAT_CoreCharacterMovement_CalculateMaxAcceleration);
		float MovementSpeedModifier = 1.f;
		OnProcessMovementSpeed.Broadcast(GetCoreCharacter(), MovementSpeedModifier);
		CurrentMaxAcceleration *= MovementSpeedModifier;
	}

	return CurrentMaxAcceleration;
}

void UCoreCharacterMovementComponent::Crouch(bool bClientSimulation)
{
	Super::Crouch(bClientSimulation);
}

void UCoreCharacterMovementComponent::UnCrouch(bool bClientSimulation)
{
	Super::UnCrouch(bClientSimulation);
}

void UCoreCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UCoreCharacterMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateAfterMovement(DeltaSeconds);
}

void UCoreCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
}

void UCoreCharacterMovementComponent::PerformAirControlForPathFollowing(FVector Direction, float ZDiff)
{
	Acceleration = Direction.GetClampedToMaxSize(GetMaxAcceleration());
}

float UCoreCharacterMovementComponent::BoostAirControl(float DeltaTime, float TickAirControl, const FVector& FallAcceleration)
{
	return Super::BoostAirControl(DeltaTime, TickAirControl, FallAcceleration);
}

bool UCoreCharacterMovementComponent::StepUp(const FVector& GravDir, const FVector& Delta, const FHitResult& Hit, struct UCharacterMovementComponent::FStepDownResult* OutStepDownResult)
{
	if (!Super::StepUp(GravDir, Delta, Hit, OutStepDownResult))
	{
		return false;
	}

	bSteppedUp = Hit.Distance > 0.25f;
	return true;
}

FNetworkPredictionData_Client* UCoreCharacterMovementComponent::GetPredictionData_Client() const
{
	checkSlow(PawnOwner != nullptr);
	checkSlow(PawnOwner->GetLocalRole() < ROLE_Authority || (PawnOwner->GetRemoteRole() == ROLE_AutonomousProxy && GetNetMode() == NM_ListenServer));
	checkSlow(GetNetMode() == NM_Client || GetNetMode() == NM_ListenServer);

	if (!ClientPredictionData)
	{
		UCoreCharacterMovementComponent* MutableThis = const_cast<UCoreCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_CoreCharacter(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

void UCoreCharacterMovementComponent::SetPlayerDefaults()
{
	InitializeMovementComponent();
}

void UCoreCharacterMovementComponent::InitializeMovementComponent()
{
	if (CoreCharacterOwner)
	{
		return;
	}

	CoreCharacterOwner = Cast<ACoreCharacter>(CharacterOwner);

	if (!CoreCharacterOwner || GetOwnerRole() <= ROLE_SimulatedProxy)
	{
		return;
	}

	TWeakObjectPtr<UStatusComponent> StatusComponent = GetCoreCharacter()->GetStatusComponent();
	if (StatusComponent.IsValid())
	{
		OnProcessMovementSpeed.AddWeakLambda(StatusComponent.Get(), [StatusComponent](const ACoreCharacter*, float& Multiplier)
		{
			if (StatusComponent.IsValid())
			{
				Multiplier *= StatusComponent->GetMovementSpeedModifier();
			}
		});

		OnProcessRotationRate.AddWeakLambda(StatusComponent.Get(), [StatusComponent](const ACoreCharacter*, float& Multiplier)
		{
			if (StatusComponent.IsValid())
			{
				Multiplier *= StatusComponent->GetRotationRateModifier();
			}
		});
	}
}

void UCoreCharacterMovementComponent::ModifyRotationRate(FRotator& Rotation) const
{
	float Modifier = 1.f;
	OnProcessRotationRate.Broadcast(GetCoreCharacter(), Modifier);
	Rotation *= Modifier;
}

bool UCoreCharacterMovementComponent::IsClientReplayingMoves() const
{
	if (!GetCoreCharacter())
	{
		return false;
	}

	return GetCoreCharacter()->IsCurrentlyReplayingMoves();
}

bool UCoreCharacterMovementComponent::IsReplayPayloadAvailable() const
{
	if (!ReplayPayload.bValid)
	{
		return false;
	}

	if (!GetCoreCharacter())
	{
		return false;
	}

	return GetCoreCharacter()->IsCurrentlyReplayingMoves();
}

void FSavedMove_CoreCharacter::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	if (UCoreCharacterMovementComponent* CoreCharacterMovement = Cast<UCoreCharacterMovementComponent>(Character->GetCharacterMovement()))
	{
		CoreCharacterMovement->ReplayPayload.MaxSpeed = MaxSpeed;
		CoreCharacterMovement->ReplayPayload.bValid = true;
	}
}

void FSavedMove_CoreCharacter::PostUpdate(ACharacter* Character, EPostUpdateMode PostUpdateMode)
{
	Super::PostUpdate(Character, PostUpdateMode);

	if (UCoreCharacterMovementComponent* CoreCharacterMovement = Cast<UCoreCharacterMovementComponent>(Character->GetCharacterMovement()))
	{
		if (PostUpdateMode == FSavedMove_Character::PostUpdate_Replay)
		{
			CoreCharacterMovement->ReplayPayload.bValid = false;
		}
	}
}

FNetworkPredictionData_Client_CoreCharacter::FNetworkPredictionData_Client_CoreCharacter(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{

}

FSavedMovePtr FNetworkPredictionData_Client_CoreCharacter::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_CoreCharacter());
}