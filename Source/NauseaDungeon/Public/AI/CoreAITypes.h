#pragma once
#include "CoreMinimal.h"
#include "CoreAITypes.generated.h"

namespace CoreNoiseTag
{
	extern  const FName Generic;

	extern  const FName InventoryPickup;
	extern  const FName InventoryDrop;
	extern  const FName WeaponEquip;
	extern  const FName WeaponPutDown;
	extern  const FName WeaponFire;
	extern  const FName WeaponReload;

	extern  const FName MeleeSwing;
	extern  const FName MeleeHit;

	extern  const FName Damage;
	extern  const FName Death;
}

USTRUCT(BlueprintType)
struct FCoreNoiseParams
{
	GENERATED_USTRUCT_BODY()

	FCoreNoiseParams() {}
	
	FCoreNoiseParams(const float InLoudness, const float InMaxRadius)
	{
		Loudness = InLoudness;
		MaxRadius = InMaxRadius;
	}

	FCoreNoiseParams(const FName& InTag, const float InLoudness = 1.f, const float InMaxRadius = 0.f)
	{
		Tag = InTag;
		Loudness = InLoudness;
		MaxRadius = InMaxRadius;
	}

	bool MakeNoise(AActor* Instigator, const FVector& Location = FVector::ZeroVector, float LoudnessMultiplier = 1.f) const;

public:
	UPROPERTY(EditDefaultsOnly)
	float Loudness = 1.f;
	UPROPERTY(EditDefaultsOnly)
	float MaxRadius = 0.f;

	UPROPERTY(EditDefaultsOnly)
	FName Tag = NAME_None;
};