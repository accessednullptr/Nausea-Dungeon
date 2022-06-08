#pragma once

#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

#define FAST_ARRAY_SERIALIZER_OPERATORS(ElementType, ArrayVariable)\
public:\
	FORCEINLINE TArray<ElementType>& operator*() { return ArrayVariable; }\
	FORCEINLINE TArray<ElementType>* operator->() { return &ArrayVariable; }\
	FORCEINLINE const TArray<ElementType>& operator*() const { return ArrayVariable; }\
	FORCEINLINE const TArray<ElementType>* operator->() const { return &ArrayVariable; }\
	FORCEINLINE ElementType& operator[](int32 Index) { return ArrayVariable[Index]; }\
	FORCEINLINE const ElementType& operator[](int32 Index) const { return ArrayVariable[Index]; }\
\

namespace PushReplicationParams
{
	extern  const FDoRepLifetimeParams Default;
	extern  const FDoRepLifetimeParams InitialOnly;
	extern  const FDoRepLifetimeParams OwnerOnly;
	extern  const FDoRepLifetimeParams SkipOwner;
	extern  const FDoRepLifetimeParams SimulatedOnly;
	extern  const FDoRepLifetimeParams AutonomousOnly;
	extern  const FDoRepLifetimeParams SimulatedOrPhysics;
	extern  const FDoRepLifetimeParams InitialOrOwner;
	extern  const FDoRepLifetimeParams Custom;
	extern  const FDoRepLifetimeParams ReplayOrOwner;
	extern  const FDoRepLifetimeParams ReplayOnly;
	extern  const FDoRepLifetimeParams SimulatedOnlyNoReplay;
	extern  const FDoRepLifetimeParams SimulatedOrPhysicsNoReplay;
	extern  const FDoRepLifetimeParams SkipReplay;
	extern  const FDoRepLifetimeParams Never;
}