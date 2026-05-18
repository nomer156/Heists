// Copyright 2026 Heists. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HeistsCoverComponent.generated.h"

UCLASS(ClassGroup = (Heists), meta = (BlueprintSpawnableComponent))
class HEISTS_API UHeistsCoverComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHeistsCoverComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Heists|Cover")
	void RefreshCoverState();

	UFUNCTION(BlueprintPure, Category = "Heists|Cover")
	bool IsInCover() const { return bIsInCover; }

	UFUNCTION(BlueprintPure, Category = "Heists|Cover")
	AActor* GetCurrentCoverActor() const { return CurrentCoverActor; }

	UFUNCTION(BlueprintPure, Category = "Heists|Cover")
	FVector GetCurrentCoverNormal() const { return CurrentCoverNormal; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heists|Cover", meta = (ClampMin = "0.0"))
	float CoverProbeDistance = 110.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heists|Cover", meta = (ClampMin = "0.0"))
	float CoverProbeHeight = 45.f;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Heists|Cover")
	bool bIsInCover = false;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Heists|Cover")
	TObjectPtr<AActor> CurrentCoverActor;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Heists|Cover")
	FVector CurrentCoverNormal = FVector::ZeroVector;

	void SetCoverState(bool bNewInCover, AActor* NewCoverActor, const FVector& NewCoverNormal);
};
