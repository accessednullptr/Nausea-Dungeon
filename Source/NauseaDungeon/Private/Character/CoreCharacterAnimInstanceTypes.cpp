// Copyright 2020-2022 Heavy Mettle Interactive. Published under the MIT License.


#include "Character/CoreCharacterAnimInstanceTypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "Character/CoreCharacterAnimInstance.h"
#include "Gameplay/StatusEffect/StatusEffectBase.h"
#include "Animation/AnimMontage.h"

FWeaponMontagePair FWeaponMontagePair::InvalidMontagePair = FWeaponMontagePair();

UAnimMontage* FAnimationMontageContainer::GetRandomMontage(const FRandomStream& Seed) const
{
	return MontageList.Num() == 0 ? nullptr : MontageList[Seed.RandHelper(MontageList.Num())];
}

UAnimMontage* FHitReactionAnimationContainer::GetMontage(EHitReactionStrength Strength) const
{
	return GetMontageForStream(Strength, FRandomStream(FMath::Rand()));
}

UAnimMontage* FHitReactionAnimationContainer::GetMontageForStream(EHitReactionStrength Strength, const FRandomStream& Seed) const
{
	switch (Strength)
	{
		case EHitReactionStrength::Light:
			return LightHitReaction.GetRandomMontage(FRandomStream(Seed.GetInitialSeed()));
		case EHitReactionStrength::Medium:
			return MediumHitReaction.GetRandomMontage(FRandomStream(Seed.GetInitialSeed()));
		case EHitReactionStrength::Heavy:
			return HeavyHitReaction.GetRandomMontage(FRandomStream(Seed.GetInitialSeed()));
	}

	return nullptr;
}

FName UAnimationObject::StatusLoopSection = "Loop";
FName UAnimationObject::StatusLoopEndSection = "LoopEnd";
UAnimationObject::UAnimationObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

#if WITH_EDITOR
void UAnimationObject::PostEditImport()
{
	Super::PostEditImport();
	UpdateVisibleProperties();
}

void UAnimationObject::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	UpdateVisibleProperties();
}

void UAnimationObject::UpdateVisibleProperties()
{
	
}
#endif //WITH_EDITOR

static FName FrontHitSection("Front");
static FName BackHitSection("Back");
static FName LeftHitSection("Left");
static FName RightHitSection("Right");
bool UAnimationObject::PlayHitReactionMontage(UCoreCharacterAnimInstance* AnimInstance, EHitReactionStrength Strength, EHitReactionDirection Direction, const FRandomStream& Seed) const
{
	if (!AnimInstance)
	{
		return false;
	}

	UAnimMontage* HitMontage = HitReactionAnimations.GetMontageForStream(Strength, Seed);
	
	if (!HitMontage)
	{
		return false;
	}

	AnimInstance->Montage_Play(HitMontage, 1.f, EMontagePlayReturnType::MontageLength, 0.f, false);

	FName SectionName = NAME_None;
	switch (Direction)
	{
	case EHitReactionDirection::Front:
		SectionName = HitMontage->IsValidSectionName(FrontHitSection) ? FrontHitSection : NAME_None;
		break;
	case EHitReactionDirection::Back:
		SectionName = HitMontage->IsValidSectionName(BackHitSection) ? BackHitSection : NAME_None;
		break;
	case EHitReactionDirection::Left:
		SectionName = HitMontage->IsValidSectionName(LeftHitSection) ? LeftHitSection : NAME_None;
		break;
	case EHitReactionDirection::Right:
		SectionName = HitMontage->IsValidSectionName(RightHitSection) ? RightHitSection : NAME_None;
		break;
	}
	
	if (SectionName != NAME_None)
	{
		AnimInstance->Montage_JumpToSection(SectionName, HitMontage);
	}

	return true;
}

bool UAnimationObject::PlayStatusStartMontage(UStatusEffectBase* StatusEffect, UCoreCharacterAnimInstance* AnimInstance, EStatusBeginType BeginType, EHitReactionDirection Direction) const
{
	if (!AnimInstance)
	{
		return false;
	}

	if (!StatusEffectAnimations.StatusMap.Contains(StatusEffect->GetStatusType()))
	{
		return false;
	}

	UAnimMontage* StatusMontage = StatusEffectAnimations.StatusMap[StatusEffect->GetStatusType()].GetRandomMontage(0);
	const bool bRestartOnRefresh = StatusEffectAnimations.StatusMap[StatusEffect->GetStatusType()].bRestartAnimationOnRefresh;

	if (!StatusMontage)
	{
		return false;
	}

	if (!AnimInstance->DoesMontageHaveRegisteredLoopTimerHandle(StatusMontage) || BeginType == EStatusBeginType::Initial || bRestartOnRefresh)
	{
		AnimInstance->Montage_Stop(0.2f, StatusMontage);
		AnimInstance->Montage_Play(StatusMontage, 1.f, EMontagePlayReturnType::MontageLength, 0.f, false);
	}

	AnimInstance->RevokeMontageLoopTimerHandle(StatusMontage);
	FName SectionName = NAME_None;
	switch (Direction)
	{
	case EHitReactionDirection::Front:
		SectionName = StatusMontage->IsValidSectionName(FrontHitSection) ? FrontHitSection : NAME_None;
		break;
	case EHitReactionDirection::Back:
		SectionName = StatusMontage->IsValidSectionName(BackHitSection) ? BackHitSection : NAME_None;
		break;
	case EHitReactionDirection::Left:
		SectionName = StatusMontage->IsValidSectionName(LeftHitSection) ? LeftHitSection : NAME_None;
		break;
	case EHitReactionDirection::Right:
		SectionName = StatusMontage->IsValidSectionName(RightHitSection) ? RightHitSection : NAME_None;
		break;
	}

	if (SectionName != NAME_None)
	{
		AnimInstance->Montage_JumpToSection(SectionName, StatusMontage);
	}

	int32 SectionIndex = StatusMontage->GetSectionIndex(StatusLoopEndSection);

	if (SectionIndex == INDEX_NONE)
	{
		if (!bRestartOnRefresh)
		{
			const float StatusTimeRemaining = StatusEffect->GetStatusTimeRemaining();
			FTimerHandle LoopEndHandle;
			StatusEffect->GetWorld()->GetTimerManager().SetTimer(LoopEndHandle, StatusTimeRemaining, false);
			AnimInstance->RegisterMontageLoopTimerHandle(StatusMontage, LoopEndHandle);
		}

		return true;
	}

	auto PlayMontageEnd = [](TWeakObjectPtr<UStatusEffectBase> StatusEffect, TWeakObjectPtr<UAnimMontage> AnimMontage, TWeakObjectPtr<UCoreCharacterAnimInstance> AnimInstance)
	{
		if (!StatusEffect.IsValid() || !AnimInstance.IsValid() || !AnimMontage.IsValid())
		{
			return;
		}

		AnimInstance->Montage_Stop(0.25f, AnimMontage.Get());
		AnimInstance->Montage_Play(AnimMontage.Get(), 1.f, EMontagePlayReturnType::MontageLength, 0.f, false);
		AnimInstance->Montage_JumpToSection(StatusLoopEndSection, AnimMontage.Get());
		AnimInstance->RevokeMontageLoopTimerHandle(AnimMontage.Get());
	};

	const float StatusTimeRemaining = StatusEffect->GetStatusTimeRemaining();

	//If we have no idea when this montage ends assume it's being manually controlled elsewhere.
	if (StatusTimeRemaining == -1.f)
	{
		return true;
	}

	const float TimeUntilStartLoopEnd = StatusTimeRemaining - StatusMontage->GetSectionLength(SectionIndex);

	//If this effect is already about to end then start montage end immediately.
	if (TimeUntilStartLoopEnd <= 0.f)
	{
		PlayMontageEnd(StatusEffect, StatusMontage, AnimInstance);
		return true;
	}

	FTimerHandle LoopEndHandle;
	StatusEffect->GetWorld()->GetTimerManager().SetTimer(LoopEndHandle,
		FTimerDelegate::CreateWeakLambda(StatusEffect->GetTypedOuter<AActor>(), PlayMontageEnd, TWeakObjectPtr<UStatusEffectBase>(StatusEffect), TWeakObjectPtr<UAnimMontage>(StatusMontage), TWeakObjectPtr<UCoreCharacterAnimInstance>(AnimInstance)),
		TimeUntilStartLoopEnd, false);
	
	AnimInstance->RegisterMontageLoopTimerHandle(StatusMontage, LoopEndHandle);
	return true;
}

bool UAnimationObject::PlayStatusEndMontage(UStatusEffectBase* StatusEffect, UCoreCharacterAnimInstance* AnimInstance, EStatusEndType EndType) const
{
	if (!AnimInstance)
	{
		return false;
	}

	if (!StatusEffectAnimations.StatusMap.Contains(StatusEffect->GetStatusType()))
	{
		return false;
	}

	UAnimMontage* StatusMontage = StatusEffectAnimations.StatusMap[StatusEffect->GetStatusType()].GetRandomMontage(0);

	if (!StatusMontage)
	{
		return false;
	}

	AnimInstance->RevokeMontageLoopTimerHandle(StatusMontage);
	AnimInstance->Montage_Stop(StatusEffectAnimations.StatusMap[StatusEffect->GetStatusType()].StopBlendTime, StatusMontage);

	return true;
}