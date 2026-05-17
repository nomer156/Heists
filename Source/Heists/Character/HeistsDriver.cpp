// Copyright 2026 Heists. All Rights Reserved.

#include "Character/HeistsDriver.h"
#include "Net/UnrealNetwork.h"

AHeistsDriver::AHeistsDriver()
{
	bIsInVehicle = false;
	bEvacReady = false;
}

void AHeistsDriver::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AHeistsDriver, bIsInVehicle);
	DOREPLIFETIME(AHeistsDriver, bEvacReady);
}

void AHeistsDriver::InitializeRole()
{
	UE_LOG(LogTemp, Log, TEXT("AHeistsDriver::InitializeRole"));
}

void AHeistsDriver::EnterVehicle(AActor* Vehicle)
{
	if (!HasAuthority()) { Server_EnterVehicle(Vehicle); return; }
	Server_EnterVehicle_Implementation(Vehicle);
}

bool AHeistsDriver::Server_EnterVehicle_Validate(AActor* Vehicle)
{
	return IsValid(Vehicle);
}

void AHeistsDriver::Server_EnterVehicle_Implementation(AActor* Vehicle)
{
	if (!HasAuthority()) return;
	bIsInVehicle = true;
	// TODO: Attach character to vehicle, disable character movement, enable vehicle movement
	UE_LOG(LogTemp, Log, TEXT("Driver entered vehicle: %s"), *GetNameSafe(Vehicle));
}

void AHeistsDriver::ExitVehicle()
{
	if (!HasAuthority()) { Server_ExitVehicle(); return; }
	Server_ExitVehicle_Implementation();
}

void AHeistsDriver::Server_ExitVehicle_Implementation()
{
	if (!HasAuthority()) return;
	bIsInVehicle = false;
	// TODO: Detach from vehicle, restore character movement
}

void AHeistsDriver::SignalEvacReady()
{
	if (!HasAuthority()) { Server_SignalEvacReady(); return; }
	Server_SignalEvacReady_Implementation();
}

void AHeistsDriver::Server_SignalEvacReady_Implementation()
{
	if (!HasAuthority()) return;
	bEvacReady = true;
	Multicast_OnEvacReady();
}

void AHeistsDriver::Multicast_OnEvacReady_Implementation()
{
	// Уведомление всем клиентам — показать UI/звук "Машина готова!"
	UE_LOG(LogTemp, Log, TEXT("Driver: Evac Ready signal sent to all players"));
}
