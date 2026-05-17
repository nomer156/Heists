// Copyright 2026 Heists. All Rights Reserved.

#include "Character/HeistsCharacterBase.h"
#include "AbilitySystem/HeistsAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayAbilitySpec.h"
#include "Player/HeistsPlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"


AHeistsCharacterBase::AHeistsCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// --- Камера (Spring Arm + Camera) ---
	// Базовые настройки, финальное положение и FOV задаётся в BP
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 1200.f;
	SpringArm->SetRelativeRotation(FRotator(-55.f, 0.f, 0.f)); // Изометрия ~55°
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bInheritRoll = false;
	SpringArm->bDoCollisionTest = false; // Камера не прячется за геометрию

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	// --- Движение ---
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed = 400.f;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// --- Начальное состояние ---
	bIsAlive = true;
	bIsCarryingLoot = false;
	CarryWeight = 0.f;
	bDefaultEffectsApplied = false;
	bDefaultAbilitiesGranted = false;
}

UAbilitySystemComponent* AHeistsCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AHeistsCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHeistsCharacterBase, bIsAlive);
	DOREPLIFETIME(AHeistsCharacterBase, bIsCarryingLoot);
	DOREPLIFETIME(AHeistsCharacterBase, CarryWeight);
}

void AHeistsCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void AHeistsCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// GAS инициализируется на сервере при Possess
	if (HasAuthority())
	{
		InitializeGAS();
		InitializeRole();
	}
}

void AHeistsCharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// GAS инициализируется на клиенте при репликации PlayerState
	InitializeGAS();
}

void AHeistsCharacterBase::InitializeGAS()
{
	AHeistsPlayerState* HeistsPlayerState = GetPlayerState<AHeistsPlayerState>();
	if (!HeistsPlayerState)
	{
		AbilitySystemComponent = nullptr;
		AttributeSet = nullptr;
		return;
	}

	AbilitySystemComponent = HeistsPlayerState->GetAbilitySystemComponent();
	AttributeSet = HeistsPlayerState->GetHeistsAttributeSet();

	if (!AbilitySystemComponent)
	{
		return;
	}

	// PlayerState owns persistent GAS state; this pawn is the current avatar.
	AbilitySystemComponent->InitAbilityActorInfo(HeistsPlayerState, this);

	// На сервере — применяем эффекты и способности
	if (HasAuthority())
	{
		ApplyDefaultEffects();
		GrantDefaultAbilities();
	}
}

void AHeistsCharacterBase::ApplyDefaultEffects()
{
	if (!AbilitySystemComponent || bDefaultEffectsApplied)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	for (const TSubclassOf<UGameplayEffect>& EffectClass : DefaultEffects)
	{
		if (!EffectClass)
		{
			continue;
		}

		FGameplayEffectSpecHandle EffectSpec = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.f, EffectContext);
		if (EffectSpec.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());
		}
	}

	bDefaultEffectsApplied = true;
}

void AHeistsCharacterBase::GrantDefaultAbilities()
{
	if (!AbilitySystemComponent || !HasAuthority() || bDefaultAbilitiesGranted)
	{
		return;
	}

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilities)
	{
		if (!AbilityClass)
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(AbilityClass, 1);
		AbilitySystemComponent->GiveAbility(AbilitySpec);
	}

	bDefaultAbilitiesGranted = true;
}

void AHeistsCharacterBase::Die()
{
	if (!HasAuthority())
	{
		return;
	}

	bIsAlive = false;
	Multicast_OnDeath();
}

void AHeistsCharacterBase::Multicast_OnDeath_Implementation()
{
	// Отключаем движение и коллизию
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Физика рэгдолла (можно расширить в BP)
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}
