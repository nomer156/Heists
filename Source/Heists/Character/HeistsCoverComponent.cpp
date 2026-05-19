// Copyright 2026 Heists. All Rights Reserved.

#include "Character/HeistsCoverComponent.h"

#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"

UHeistsCoverComponent::UHeistsCoverComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UHeistsCoverComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHeistsCoverComponent, bIsInCover);
	DOREPLIFETIME(UHeistsCoverComponent, CoverState);
	DOREPLIFETIME(UHeistsCoverComponent, CurrentCoverActor);
	DOREPLIFETIME(UHeistsCoverComponent, CurrentCoverNormal);
}

void UHeistsCoverComponent::RefreshCoverState()
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World)
	{
		SetCoverState(EHeistsCoverState::None, nullptr, FVector::ZeroVector);
		return;
	}

	if (!Owner->HasAuthority())
	{
		return;
	}

	const FVector TraceStart = Owner->GetActorLocation() + FVector(0.f, 0.f, CoverProbeHeight);
	const FVector TraceEnd = TraceStart + Owner->GetActorForwardVector() * CoverProbeDistance;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(HeistsCoverProbe), false);
	QueryParams.AddIgnoredActor(Owner);

	FHitResult Hit;
	const bool bHit = World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams);
	SetCoverState(bHit ? EHeistsCoverState::InCover : EHeistsCoverState::None, bHit ? Hit.GetActor() : nullptr, bHit ? Hit.ImpactNormal : FVector::ZeroVector);
}

void UHeistsCoverComponent::RequestEnterCover()
{
	if (AActor* Owner = GetOwner())
	{
		if (Owner->HasAuthority())
		{
			Server_SetCoverState_Implementation(EHeistsCoverState::InCover);
			return;
		}
	}

	Server_SetCoverState(EHeistsCoverState::InCover);
}

void UHeistsCoverComponent::RequestExitCover()
{
	if (AActor* Owner = GetOwner())
	{
		if (Owner->HasAuthority())
		{
			Server_SetCoverState_Implementation(EHeistsCoverState::None);
			return;
		}
	}

	Server_SetCoverState(EHeistsCoverState::None);
}

void UHeistsCoverComponent::RequestTogglePeek()
{
	const EHeistsCoverState DesiredState = IsPeeking() ? EHeistsCoverState::InCover : EHeistsCoverState::Peeking;
	if (AActor* Owner = GetOwner())
	{
		if (Owner->HasAuthority())
		{
			Server_SetCoverState_Implementation(DesiredState);
			return;
		}
	}

	Server_SetCoverState(DesiredState);
}

void UHeistsCoverComponent::Server_SetCoverState_Implementation(EHeistsCoverState NewState)
{
	if (NewState == EHeistsCoverState::None)
	{
		SetCoverState(EHeistsCoverState::None, nullptr, FVector::ZeroVector);
		return;
	}

	if (!CurrentCoverActor)
	{
		RefreshCoverState();
	}

	if (!CurrentCoverActor)
	{
		SetCoverState(EHeistsCoverState::None, nullptr, FVector::ZeroVector);
		return;
	}

	SetCoverState(NewState, CurrentCoverActor, CurrentCoverNormal);
}

FName UHeistsCoverComponent::GetCoverStateName() const
{
	switch (CoverState)
	{
	case EHeistsCoverState::InCover:
		return TEXT("InCover");
	case EHeistsCoverState::Peeking:
		return TEXT("Peeking");
	case EHeistsCoverState::None:
	default:
		return TEXT("None");
	}
}

void UHeistsCoverComponent::SetCoverState(EHeistsCoverState NewState, AActor* NewCoverActor, const FVector& NewCoverNormal)
{
	CoverState = NewState;
	bIsInCover = CoverState != EHeistsCoverState::None;
	CurrentCoverActor = NewCoverActor;
	CurrentCoverNormal = NewCoverNormal;
}
