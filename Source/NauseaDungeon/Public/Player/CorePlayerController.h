// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/LocalMessage.h"
#include "Player/PlayerOwnershipInterface.h"
#include "CorePlayerController.generated.h"

class ACorePlayerState;
class UPlayerClassComponent;
class UPlayerPromptComponent;
class ANauseaPlayerCameraManager;
class UCoreLocalMessage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FReceivedPlayerStateSignature, ACorePlayerController*, PlayerController, ACorePlayerState*, PlayerState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPlayerPawnUpdatedSignature, ACorePlayerController*, PlayerController, ACoreCharacter*, Pawn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FPlayerPawnKilledSignature, ACoreCharacter*, KilledCharacter, TSubclassOf<UDamageType>, DamageType, float, Damage, APlayerState*, KillerPlayerState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FReceivedLocalMessageSignature, const FName&, MessageType, int32, MessageIndex, const FString&, MessageString, ACorePlayerState*, PlayerStateA, ACorePlayerState*, PlayerStateB, const UObject*, OptionalObject);

/**
 * 
 */
UCLASS(Config=Game)
class ACorePlayerController : public APlayerController, public IGenericTeamAgentInterface, public IPlayerOwnershipInterface
{
	GENERATED_UCLASS_BODY()
	
//ACorePlayerController message types.
public:
	static const FName MessageTypeSay;
	static const FName MessageTypeTeamSay;

//~ Begin AActor Interface	
protected:
	virtual void BeginPlay() override;
public:
	virtual void CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult) override;
//~ End AActor Interface

//~ Begin AController Interface
public:
	virtual void OnRep_Pawn() override;
	virtual void SetPawn(APawn* InPawn) override;
protected:
	virtual void OnRep_PlayerState() override;
//~ End AController Interface
	
//~ Begin APlayerController Interface
public:
	virtual void AcknowledgePossession(APawn* InPawn) override;
	virtual void SpawnPlayerCameraManager() override;
	virtual void SetCameraMode(FName NewCamMode) override;
	virtual void ClientSetCameraMode_Implementation(FName NewCamMode) override { SetCameraMode(NewCamMode); }
	virtual bool CanRestartPlayer() override;
	virtual void AutoManageActiveCameraTarget(AActor* SuggestedTarget) override;
	virtual void ClientTeamMessage_Implementation(APlayerState* SenderPlayerState, const FString& S, FName Type, float MsgLifeTime) override;
protected:
	virtual void SetupInputComponent() override;
//~ End APlayerController Interface

//~ Begin IGenericTeamAgentInterface Interface
public:
	virtual FGenericTeamId GetGenericTeamId() const override;
//~ End IGenericTeamAgentInterface Interface

//~ Begin IPlayerOwnershipInterface Interface
public:
	virtual ACorePlayerState* GetOwningPlayerState() const override;
	virtual UPlayerStatisticsComponent* GetPlayerStatisticsComponent() const override { return PlayerStatisticsComponent; }
	virtual AController* GetOwningController() const { return const_cast<ACorePlayerController*>(this); }
	virtual APawn* GetOwningPawn() const override { return GetPawn(); }
//~ End IPlayerOwnershipInterface Interface

public:
	UFUNCTION(BlueprintCallable, Category = PlayerController)
	UPlayerPromptComponent* GetPlayerPromptComponent() const { return PlayerPromptComponent; }

	UFUNCTION(BlueprintCallable, Category = PlayerController)
	ACorePlayerCameraManager* GetPlayerCameraManager() const { return CorePlayerCameraManager; }

	void ForceCameraMode(FName NewCameraMode);

	UFUNCTION(exec)
	void ExecSetCameraMode(const FString& NewCameraMode);

	UFUNCTION()
	void ResetPlayerStats();

	UFUNCTION(BlueprintCallable)
	void SendPlayerSay(const FString& String);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Reliable_Say(const FString& String);
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Reliable_TeamSay(const FString& String);
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Reliable_AdminSay(const FString& String);

	UFUNCTION()
	virtual void ReceivedLocalMessage(const FClientReceiveData& ClientData);

public:
	UPROPERTY(BlueprintAssignable, Category = PlayerController)
	FReceivedPlayerStateSignature OnReceivedPlayerState;

	UPROPERTY(BlueprintAssignable, Category = PlayerController)
	FPlayerPawnUpdatedSignature OnPawnUpdated;
	UPROPERTY(BlueprintAssignable, Category = PlayerController)
	FPlayerPawnKilledSignature OnPlayerPawnKilled;

	UPROPERTY(BlueprintAssignable, Category = PlayerController)
	FReceivedLocalMessageSignature OnReceivedLocalMessage;

protected:
	UFUNCTION()
	void OnPlayerDataReady();

	UFUNCTION()
	void PlayerPawnKilled(UStatusComponent* Component, float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

	UFUNCTION(Client, Reliable)
	void Client_Reliable_PlayerPawnKilled(ACoreCharacter* KilledCharacter, TSubclassOf<UDamageType> DamageType, float Damage, APlayerState* KillerPlayerState);

private:
	UPROPERTY(EditDefaultsOnly, Category = PlayerController)
	TSubclassOf<UCoreLocalMessage> ClientMessageClass = nullptr;

	UPROPERTY()
	ACorePlayerCameraManager* CorePlayerCameraManager = nullptr;

	UPROPERTY()
	UPlayerStatisticsComponent* PlayerStatisticsComponent = nullptr;

	UPROPERTY()
	UPlayerPromptComponent* PlayerPromptComponent = nullptr;
};
