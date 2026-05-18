// Copyright 2026 Heists. All Rights Reserved.

#include "Player/HeistsPlayerController.h"
#include "Character/HeistsCharacterBase.h"
#include "Character/HeistsRobber.h"
#include "Interaction/HeistsInteractionComponent.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputCoreTypes.h"
#include "GameFramework/SpringArmComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "UI/HeistsHUD.h"
#include "Net/UnrealNetwork.h"


AHeistsPlayerController::AHeistsPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	TargetZoom = 1200.f;
	CurrentZoom = 1200.f;
	bIsClickHeld = false;
	ClickHoldTimer = 0.f;
	CachedRadialTarget = nullptr;

	if (UInputMappingContext* LoadedMappingContext = LoadObject<UInputMappingContext>(
		nullptr,
		TEXT("/Game/Input/IMC_Default.IMC_Default")))
	{
		DefaultMappingContext = LoadedMappingContext;
	}

	if (UInputAction* LoadedMoveAction = LoadObject<UInputAction>(
		nullptr,
		TEXT("/Game/Input/Actions/IA_Move.IA_Move")))
	{
		IA_Move = LoadedMoveAction;
	}

	// Left-side UI reservations: chat strip and current objectives area.
	LeftScreenInputBlockZones.Add(FVector4f(0.00f, 0.00f, 0.50f, 0.18f));
	LeftScreenInputBlockZones.Add(FVector4f(0.00f, 0.68f, 0.50f, 1.00f));

	// Right-side UI reservations: action/radial cluster and lower progress/action area.
	RightScreenInputBlockZones.Add(FVector4f(0.72f, 0.48f, 1.00f, 1.00f));
	RightScreenInputBlockZones.Add(FVector4f(0.50f, 0.78f, 1.00f, 1.00f));
}

void AHeistsPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	// Регистрируем Input Mapping Context.
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::E, IE_Pressed, this, &AHeistsPlayerController::OpenInteractionRadial);
		InputComponent->BindKey(EKeys::One, IE_Pressed, this, &AHeistsPlayerController::ConfirmRadialActionSlot1);
		InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &AHeistsPlayerController::ConfirmRadialActionSlot2);
		InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &AHeistsPlayerController::ConfirmRadialActionSlot3);
		InputComponent->BindKey(EKeys::Four, IE_Pressed, this, &AHeistsPlayerController::ConfirmRadialActionSlot4);
		InputComponent->BindKey(EKeys::Five, IE_Pressed, this, &AHeistsPlayerController::ConfirmRadialActionSlot5);
		InputComponent->BindKey(EKeys::Six, IE_Pressed, this, &AHeistsPlayerController::ConfirmRadialActionSlot6);
		InputComponent->BindKey(EKeys::G, IE_Pressed, this, &AHeistsPlayerController::DropCarriedLoot);
		InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AHeistsPlayerController::HandleCameraDragPressed);
		InputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &AHeistsPlayerController::HandleCameraDragReleased);
		InputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &AHeistsPlayerController::HandleCameraDragPressed);
		InputComponent->BindKey(EKeys::RightMouseButton, IE_Released, this, &AHeistsPlayerController::HandleCameraDragReleased);
	}
}

void AHeistsPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (IA_Move)
		{
			EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AHeistsPlayerController::HandleMove);
		}
		if (IA_ClickMove)
		{
			EIC->BindAction(IA_ClickMove, ETriggerEvent::Started, this, &AHeistsPlayerController::HandleClickMove);
			EIC->BindAction(IA_ClickMove, ETriggerEvent::Completed, this, &AHeistsPlayerController::HandleClickMoveCompleted);
		}
		if (IA_CameraZoom)
		{
			EIC->BindAction(IA_CameraZoom, ETriggerEvent::Triggered, this, &AHeistsPlayerController::HandleCameraZoom);
		}
		if (IA_Interact)
		{
			EIC->BindAction(IA_Interact, ETriggerEvent::Started, this, &AHeistsPlayerController::HandleInteract);
		}
	}
}

void AHeistsPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Click-hold: продолжаем идти пока удержан ЛКМ
	if (IsLocalController() && bIsClickHeld)
	{
		ClickHoldTimer += DeltaTime;
		if (ClickHoldTimer >= 0.1f)
		{
			FVector Destination;
			if (GetClickDestination(Destination))
			{
				Server_MoveToLocation(Destination);
			}
		}
	}

	// Плавный зум камеры
	if (APawn* MyPawn = GetPawn())
	{
		if (USpringArmComponent* Arm = MyPawn->FindComponentByClass<USpringArmComponent>())
		{
			CurrentZoom = FMath::FInterpTo(CurrentZoom, TargetZoom, DeltaTime, ZoomSmoothSpeed);
			Arm->TargetArmLength = CurrentZoom;
		}
	}

	if (IsLocalController() && bIsCameraDragHeld)
	{
		float MouseX = 0.f;
		float MouseY = 0.f;
		if (GetMousePosition(MouseX, MouseY))
		{
			const FVector2D CurrentScreenPosition(MouseX, MouseY);
			RotateCameraFromDragDelta(CurrentScreenPosition - LastCameraDragScreenPosition);
			LastCameraDragScreenPosition = CurrentScreenPosition;
		}
	}
}

void AHeistsPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AHeistsPlayerController::HandleMove(const FInputActionValue& Value)
{
	if (!IsLocalController())
	{
		return;
	}

	const FVector2D MoveInput = Value.Get<FVector2D>();
	if (MoveInput.IsNearlyZero())
	{
		return;
	}

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	// Isometric camera basis: forward/right projected onto the ground plane.
	const FRotator ViewRotation = GetControlRotation();
	const FRotator YawRotation(0.f, ViewRotation.Yaw, 0.f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	ControlledPawn->AddMovementInput(ForwardDirection, MoveInput.Y);
	ControlledPawn->AddMovementInput(RightDirection, MoveInput.X);
}

void AHeistsPlayerController::HandleClickMove(const FInputActionValue& Value)
{
	if (bUseMobileStickMovement)
	{
		return;
	}

	bIsClickHeld = true;
	ClickHoldTimer = 0.f;

	FVector Destination;
	if (GetClickDestination(Destination))
	{
		Server_MoveToLocation(Destination);
	}
}

void AHeistsPlayerController::HandleClickMoveCompleted(const FInputActionValue& Value)
{
	bIsClickHeld = false;
	ClickHoldTimer = 0.f;
}

void AHeistsPlayerController::HandleCameraZoom(const FInputActionValue& Value)
{
	const float ZoomAxis = Value.Get<float>();
	TargetZoom = FMath::Clamp(TargetZoom - ZoomAxis * ZoomSpeed, ZoomMin, ZoomMax);
}

void AHeistsPlayerController::HandleInteract(const FInputActionValue& Value)
{
	OpenInteractionRadial();
}

bool AHeistsPlayerController::GetClickDestination(FVector& OutDestination) const
{
	FHitResult HitResult;
	// Трасируем под курсором по каналу Visibility
	if (GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, HitResult))
	{
		OutDestination = HitResult.Location;
		return true;
	}
	return false;
}

bool AHeistsPlayerController::IsScreenPositionBlockedForMovement(const FVector2D& ScreenPosition) const
{
	int32 SizeX = 0;
	int32 SizeY = 0;
	GetViewportSize(SizeX, SizeY);

	if (SizeX <= 0 || SizeY <= 0)
	{
		return false;
	}

	const FVector2f NormalizedPosition(
		static_cast<float>(ScreenPosition.X) / static_cast<float>(SizeX),
		static_cast<float>(ScreenPosition.Y) / static_cast<float>(SizeY));

	for (const FVector4f& Zone : LeftScreenInputBlockZones)
	{
		if (NormalizedPosition.X >= Zone.X && NormalizedPosition.X <= Zone.Z
			&& NormalizedPosition.Y >= Zone.Y && NormalizedPosition.Y <= Zone.W)
		{
			return true;
		}
	}

	return false;
}

bool AHeistsPlayerController::IsScreenPositionCameraDragZone(const FVector2D& ScreenPosition) const
{
	if (!bEnableRightSideCameraDrag)
	{
		return false;
	}

	int32 SizeX = 0;
	int32 SizeY = 0;
	GetViewportSize(SizeX, SizeY);

	if (SizeX <= 0 || SizeY <= 0)
	{
		return false;
	}

	const FVector2f NormalizedPosition(
		static_cast<float>(ScreenPosition.X) / static_cast<float>(SizeX),
		static_cast<float>(ScreenPosition.Y) / static_cast<float>(SizeY));

	if (NormalizedPosition.X < 0.5f)
	{
		return false;
	}

	for (const FVector4f& Zone : RightScreenInputBlockZones)
	{
		if (NormalizedPosition.X >= Zone.X && NormalizedPosition.X <= Zone.Z
			&& NormalizedPosition.Y >= Zone.Y && NormalizedPosition.Y <= Zone.W)
		{
			return false;
		}
	}

	return true;
}

bool AHeistsPlayerController::Server_MoveToLocation_Validate(const FVector& Destination)
{
	// Базовая проверка — точка в разумных пределах карты
	return !Destination.ContainsNaN() && Destination.Size() < 100000.f;
}

void AHeistsPlayerController::Server_MoveToLocation_Implementation(const FVector& Destination)
{
	APawn* MyPawn = GetPawn();
	if (!MyPawn) return;

	UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, Destination);
}


void AHeistsPlayerController::OnClickMovePressed()
{
	if (bUseMobileStickMovement)
	{
		return;
	}

	// Можно вызвать из BP напрямую
	FVector Destination;
	if (GetClickDestination(Destination))
	{
		Server_MoveToLocation(Destination);
	}
}

void AHeistsPlayerController::OnClickMoveReleased()
{
	bIsClickHeld = false;
}

void AHeistsPlayerController::OnCameraZoom(float AxisValue)
{
	TargetZoom = FMath::Clamp(TargetZoom - AxisValue * ZoomSpeed, ZoomMin, ZoomMax);
}

void AHeistsPlayerController::TriggerAbilitySlot(int32 SlotIndex)
{
	Server_TriggerAbilitySlot(SlotIndex);
}

void AHeistsPlayerController::OpenInteractionRadial()
{
	AHeistsRobber* Robber = Cast<AHeistsRobber>(GetPawn());
	UHeistsInteractionComponent* InteractionComponent = Robber ? Robber->GetInteractionComponent() : nullptr;
	if (!InteractionComponent)
	{
		CancelInteractionRadial();
		return;
	}

	AActor* Target = InteractionComponent->FindBestInteractable();
	if (!Target)
	{
		CancelInteractionRadial();
		return;
	}

	TArray<FHeistsInteractionAction> Actions = InteractionComponent->GetAvailableActionsForTarget(Target);
	Actions.RemoveAll(
		[](const FHeistsInteractionAction& Action)
		{
			return !Action.bIsEnabled || Action.ActionId == EHeistsInteractionActionId::None;
		});

	if (Actions.IsEmpty())
	{
		CancelInteractionRadial();
		return;
	}

	if (Actions.Num() == 1)
	{
		InteractionComponent->RequestInteraction(Target, Actions[0].ActionId);
		CancelInteractionRadial();
		return;
	}

	CachedRadialTarget = Target;
	CachedRadialActions = MoveTemp(Actions);
	bIsInteractionRadialOpen = true;
	RefreshInteractionHUD();
}

void AHeistsPlayerController::ConfirmInteractionAction(EHeistsInteractionActionId ActionId)
{
	if (!bIsInteractionRadialOpen || !CachedRadialTarget || ActionId == EHeistsInteractionActionId::None)
	{
		return;
	}

	AHeistsRobber* Robber = Cast<AHeistsRobber>(GetPawn());
	UHeistsInteractionComponent* InteractionComponent = Robber ? Robber->GetInteractionComponent() : nullptr;
	if (InteractionComponent)
	{
		InteractionComponent->RequestInteraction(CachedRadialTarget, ActionId);
	}

	CancelInteractionRadial();
}

void AHeistsPlayerController::ConfirmInteractionActionByIndex(int32 ActionIndex)
{
	if (!CachedRadialActions.IsValidIndex(ActionIndex))
	{
		return;
	}

	ConfirmInteractionAction(CachedRadialActions[ActionIndex].ActionId);
}

void AHeistsPlayerController::CancelInteractionRadial()
{
	CachedRadialTarget = nullptr;
	CachedRadialActions.Reset();
	bIsInteractionRadialOpen = false;
	RefreshInteractionHUD();
}

void AHeistsPlayerController::DropCarriedLoot()
{
	if (AHeistsRobber* Robber = Cast<AHeistsRobber>(GetPawn()))
	{
		Robber->DropLoot();
	}
}

void AHeistsPlayerController::ConfirmRadialActionSlot1()
{
	ConfirmInteractionActionByIndex(0);
}

void AHeistsPlayerController::ConfirmRadialActionSlot2()
{
	ConfirmInteractionActionByIndex(1);
}

void AHeistsPlayerController::ConfirmRadialActionSlot3()
{
	ConfirmInteractionActionByIndex(2);
}

void AHeistsPlayerController::ConfirmRadialActionSlot4()
{
	ConfirmInteractionActionByIndex(3);
}

void AHeistsPlayerController::ConfirmRadialActionSlot5()
{
	ConfirmInteractionActionByIndex(4);
}

void AHeistsPlayerController::ConfirmRadialActionSlot6()
{
	ConfirmInteractionActionByIndex(5);
}

void AHeistsPlayerController::HandleCameraDragPressed()
{
	if (!IsLocalController())
	{
		return;
	}

	float MouseX = 0.f;
	float MouseY = 0.f;
	if (!GetMousePosition(MouseX, MouseY))
	{
		return;
	}

	const FVector2D ScreenPosition(MouseX, MouseY);
	if (!IsScreenPositionCameraDragZone(ScreenPosition))
	{
		return;
	}

	bIsCameraDragHeld = true;
	LastCameraDragScreenPosition = ScreenPosition;
}

void AHeistsPlayerController::HandleCameraDragReleased()
{
	bIsCameraDragHeld = false;
}

void AHeistsPlayerController::RotateCameraFromDragDelta(const FVector2D& ScreenDelta)
{
	if (ScreenDelta.IsNearlyZero())
	{
		return;
	}

	APawn* MyPawn = GetPawn();
	USpringArmComponent* Arm = MyPawn ? MyPawn->FindComponentByClass<USpringArmComponent>() : nullptr;
	if (!Arm)
	{
		return;
	}

	FRotator CameraRotation = Arm->GetRelativeRotation();
	CameraRotation.Yaw += ScreenDelta.X * CameraDragYawSpeed;
	Arm->SetRelativeRotation(CameraRotation);

	const FRotator DesiredControlRotation(0.f, CameraRotation.Yaw, 0.f);
	SetControlRotation(DesiredControlRotation);
}

void AHeistsPlayerController::RefreshInteractionHUD() const
{
	if (AHeistsHUD* HeistsHUD = Cast<AHeistsHUD>(GetHUD()))
	{
		HeistsHUD->RefreshInteractionMenu();
	}
}

bool AHeistsPlayerController::Server_TriggerAbilitySlot_Validate(int32 SlotIndex)
{
	return SlotIndex >= 0 && SlotIndex < 8;
}

void AHeistsPlayerController::Server_TriggerAbilitySlot_Implementation(int32 SlotIndex)
{
	AHeistsCharacterBase* HeistsPawn = Cast<AHeistsCharacterBase>(GetPawn());
	if (!HeistsPawn)
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = HeistsPawn->GetAbilitySystemComponent();
	if (!AbilitySystemComponent)
	{
		return;
	}

	if (AbilitySlotAbilities.IsValidIndex(SlotIndex) && AbilitySlotAbilities[SlotIndex])
	{
		AbilitySystemComponent->TryActivateAbilityByClass(AbilitySlotAbilities[SlotIndex]);
		return;
	}

	UE_LOG(LogTemp, Verbose, TEXT("Ability slot %d has no assigned ability yet."), SlotIndex);
}
