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
	DOREPLIFETIME(UHeistsCoverComponent, CurrentCoverActor);
	DOREPLIFETIME(UHeistsCoverComponent, CurrentCoverNormal);
}

void UHeistsCoverComponent::RefreshCoverState()
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World)
	{
		SetCoverState(false, nullptr, FVector::ZeroVector);
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
	SetCoverState(bHit, bHit ? Hit.GetActor() : nullptr, bHit ? Hit.ImpactNormal : FVector::ZeroVector);
}

void UHeistsCoverComponent::SetCoverState(bool bNewInCover, AActor* NewCoverActor, const FVector& NewCoverNormal)
{
	bIsInCover = bNewInCover;
	CurrentCoverActor = NewCoverActor;
	CurrentCoverNormal = NewCoverNormal;
}
