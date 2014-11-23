// Fill out your copyright notice in the Description page of Project Settings.

#include "SimpleVehicle.h"
#include "PickUp.h"


APickUp::APickUp(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	bIsActivate = true;

	BaseCollision = PCIP.CreateDefaultSubobject<USphereComponent>(this, TEXT("BaseCollsion"));

	RootComponent = BaseCollision;

	PickupMesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("PickUpMesh"));

	PickupMesh->SetSimulatePhysics(true);
	PickupMesh->AttachTo(RootComponent);
}

void APickUp::PickedUp_Implementation()
{
	if (iCountFlags - 1 == iCountFlags)++iCountFlags;
}


