// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "BadassMultiplayer/Character/MultiplayerCharacter.h"
#include "Net/UnrealNetwork.h"


AWeapon::AWeapon():
	WeaponState(EWeaponState::EWS_Initial)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Weapon is replicated across all intances of the game
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>("WeaponMesh");
	SetRootComponent(WeaponMesh);
	AreaSphere = CreateDefaultSubobject<USphereComponent>("AreaSphere");
	AreaSphere->SetupAttachment(RootComponent);

	// WeaponMesh will block all collision except for pawn so it can bounce around when dropped
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Area sphere will have no collision to start off becuase we only want its collision to exisit on the server
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority()) // Check to make sure all of this code happens on the server
	{
		// Enable collision to overlap with the player characters
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnAreaSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnAreaSphereEndOverlap);
	}

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
	
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Although we set the bReplicates on the AWeapon Constructor to true, if we can to replicate specific variables as well we need to specify them here
	DOREPLIFETIME_CONDITION(AWeapon, WeaponState, COND_OwnerOnly);
}


void AWeapon::OnAreaSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMultiplayerCharacter* MultiplayerCharacter = Cast<AMultiplayerCharacter>(OtherActor);
	if (MultiplayerCharacter)
	{
		// On overalp the OverlappingWeapon will be set which starts its replication
		MultiplayerCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnAreaSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AMultiplayerCharacter* MultiplayerCharacter = Cast<AMultiplayerCharacter>(OtherActor);
	if (MultiplayerCharacter)
	{
		// No longer overlapping so set OverlappingWeapon to null
		MultiplayerCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false); // Propogate hiding pickup widget to all clients
		break;
	}
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;

	// Modify weapon variables on the server, which will propogate to clients regularly since when the variable changes the RepNotify will be called
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	}
}


void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

