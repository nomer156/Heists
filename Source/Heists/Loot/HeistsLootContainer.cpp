// Copyright 2026 Heists. All Rights Reserved.

#include "Loot/HeistsLootContainer.h"

#include "Loot/HeistsLootBag.h"
#include "Net/UnrealNetwork.h"

AHeistsLootContainer::AHeistsLootContainer()
{
	LootBagClass = AHeistsLootBag::StaticClass();

	FHeistsInteractionAction SearchAction;
	SearchAction.ActionId = EHeistsInteractionActionId::Search;
	SearchAction.DisplayName = FText::FromString(TEXT("Search"));
	SearchAction.TaskType = EHeistsInteractionTaskType::HoldProgress;
	SearchAction.Color = EHeistsInteractionColor::Yellow;
	SearchAction.Duration = 1.5f;
	Actions.Add(SearchAction);
}

void AHeistsLootContainer::ApplyInteractionResult(AActor* Interactor, EHeistsInteractionActionId ActionId)
{
	if (ActionId != EHeistsInteractionActionId::Search || bWasSearched || !LootBagClass)
	{
		return;
	}

	bWasSearched = true;
	const FVector SpawnLocation = GetActorLocation() + LootSpawnOffset;
	GetWorld()->SpawnActor<AHeistsLootBag>(LootBagClass, SpawnLocation, GetActorRotation());
}

void AHeistsLootContainer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHeistsLootContainer, bWasSearched);
}
