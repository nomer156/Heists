// Copyright 2026 Heists. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Interaction/HeistsInteractionTypes.h"
#include "HeistsPlayerController.generated.h"

class AHeistsCharacterBase;
class UGameplayAbility;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

/**
 * AHeistsPlayerController
 *
 * Контроллер игрока. Владеет: Enhanced Input, click-to-move, изометрической камерой.
 * Реплицируется: Server RPC для всех действий игрока.
 * Blueprint наследник: BP_HeistsPlayerController — настройка Input Actions и камеры.
 */
UCLASS()
class HEISTS_API AHeistsPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AHeistsPlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// --- Click-to-Move ---

	// Клик ЛКМ — переместить персонажа к точке
	UFUNCTION(BlueprintCallable, Category = "Heists|Input")
	void OnClickMovePressed();

	UFUNCTION(BlueprintCallable, Category = "Heists|Input")
	void OnClickMoveReleased();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_MoveToLocation(const FVector& Destination);

	// --- Mobile landscape movement ---

	UFUNCTION(BlueprintCallable, Category = "Heists|Input|Mobile")
	void SetMobileStickMovementEnabled(bool bEnabled) { bUseMobileStickMovement = bEnabled; }

	UFUNCTION(BlueprintPure, Category = "Heists|Input|Mobile")
	bool IsMobileStickMovementEnabled() const { return bUseMobileStickMovement; }

	UFUNCTION(BlueprintCallable, Category = "Heists|Input|Mobile")
	bool IsScreenPositionBlockedForMovement(const FVector2D& ScreenPosition) const;

	UFUNCTION(BlueprintCallable, Category = "Heists|Input|Camera")
	bool IsScreenPositionCameraDragZone(const FVector2D& ScreenPosition) const;

	UFUNCTION(BlueprintCallable, Category = "Heists|Input|Actions")
	void TriggerAbilitySlot(int32 SlotIndex);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_TriggerAbilitySlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Heists|Input|Interaction")
	void OpenInteractionRadial();

	UFUNCTION(BlueprintCallable, Category = "Heists|Input|Interaction")
	void ConfirmInteractionAction(EHeistsInteractionActionId ActionId);

	UFUNCTION(BlueprintCallable, Category = "Heists|Input|Interaction")
	void ConfirmInteractionActionByIndex(int32 ActionIndex);

	UFUNCTION(BlueprintCallable, Category = "Heists|Input|Interaction")
	void CancelInteractionRadial();

	UFUNCTION(BlueprintCallable, Category = "Heists|Input|Loot")
	void DropCarriedLoot();

	UFUNCTION(BlueprintPure, Category = "Heists|Input|Interaction")
	bool IsInteractionRadialOpen() const { return bIsInteractionRadialOpen; }

	UFUNCTION(BlueprintPure, Category = "Heists|Input|Interaction")
	AActor* GetRadialTarget() const { return CachedRadialTarget; }

	UFUNCTION(BlueprintPure, Category = "Heists|Input|Interaction")
	TArray<FHeistsInteractionAction> GetRadialActions() const { return CachedRadialActions; }

	// --- Камера ---

	// Zoom колесо мыши
	UFUNCTION(BlueprintCallable, Category = "Heists|Camera")
	void OnCameraZoom(float AxisValue);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heists|Camera")
	float ZoomMin = 600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heists|Camera")
	float ZoomMax = 2000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heists|Camera")
	float ZoomSpeed = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heists|Camera")
	float ZoomSmoothSpeed = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heists|Camera|Mobile")
	bool bEnableRightSideCameraDrag = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heists|Camera|Mobile")
	float CameraDragYawSpeed = 0.18f;

	// --- Enhanced Input ---

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heists|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heists|Input")
	TObjectPtr<UInputAction> IA_ClickMove;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heists|Input")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heists|Input")
	TObjectPtr<UInputAction> IA_CameraZoom;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heists|Input")
	TObjectPtr<UInputAction> IA_Interact;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heists|Input|Actions")
	TArray<TSubclassOf<UGameplayAbility>> AbilitySlotAbilities;

	// Primary mobile mode: left-half invisible stick movement. PC click-to-move remains a dev fallback.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heists|Input|Mobile")
	bool bUseMobileStickMovement = true;

	// Normalized screen rectangles (MinX, MinY, MaxX, MaxY) that consume left-side input for chat/tasks.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heists|Input|Mobile")
	TArray<FVector4f> LeftScreenInputBlockZones;

	// Normalized screen rectangles that reserve right-side touch for UI instead of camera drag.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Heists|Input|Mobile")
	TArray<FVector4f> RightScreenInputBlockZones;

protected:
	// Текущий целевой zoom (интерполируется в Tick)
	float TargetZoom;
	float CurrentZoom;

	bool bIsClickHeld;
	float ClickHoldTimer;
	bool bIsCameraDragHeld = false;
	FVector2D LastCameraDragScreenPosition = FVector2D::ZeroVector;

	UPROPERTY(Transient)
	TObjectPtr<AActor> CachedRadialTarget;

	UPROPERTY(Transient)
	TArray<FHeistsInteractionAction> CachedRadialActions;

	bool bIsInteractionRadialOpen = false;

	// Обработчики Enhanced Input
	void HandleMove(const FInputActionValue& Value);
	void HandleClickMove(const FInputActionValue& Value);
	void HandleClickMoveCompleted(const FInputActionValue& Value);
	void HandleCameraZoom(const FInputActionValue& Value);
	void HandleInteract(const FInputActionValue& Value);
	void ConfirmRadialActionSlot1();
	void ConfirmRadialActionSlot2();
	void ConfirmRadialActionSlot3();
	void ConfirmRadialActionSlot4();
	void ConfirmRadialActionSlot5();
	void ConfirmRadialActionSlot6();
	void HandleCameraDragPressed();
	void HandleCameraDragReleased();
	void RotateCameraFromDragDelta(const FVector2D& ScreenDelta);
	void RefreshInteractionHUD() const;

	// Trace под курсором для определения точки назначения
	bool GetClickDestination(FVector& OutDestination) const;
};
