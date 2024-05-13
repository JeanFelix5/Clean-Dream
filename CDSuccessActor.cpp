// Fill out your copyright notice in the Description page of Project Settings.


#include "AspirableActors/CDSuccessActor.h"

#include "Characters/BaseCharacter.h"
#include "Components/AspirableActor.h"
#include "Net/UnrealNetwork.h"


// Sets default values
ACDSuccessActor::ACDSuccessActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static mesh"));
	StaticMeshComponent->SetupAttachment(RootComponent);

	AspirableActor = CreateDefaultSubobject<UAspirableActor>(TEXT("Is aspirable actor"));

	bReplicates = true;
}

void ACDSuccessActor::OnHit_Implementation(AActor* Shooter)
{
	if(!Shooter) return;
	
	if(ABaseCharacter* CharacterRef = Cast<ABaseCharacter>(Shooter))
	{
		if(CharacterRef->bIsObjectBlockingVacuum == true)
		{
			return;
		}
		
		FVector SuccessLocation = GetActorLocation();
		FVector GrabbedObjectLocation = CharacterRef->GrabbedObjectLocation->GetComponentLocation();

		// Zero out the Z component for horizontal distance calculation
		SuccessLocation.Z = 0.0f;
		GrabbedObjectLocation.Z = 0.0f;
		float Distance = FVector::Distance(SuccessLocation, GrabbedObjectLocation);

		if (Distance < DistanceToGrab)
		{
			//Aspirate the success actor and execute modifier
			ApplyModifier();
		}
		else
		{
			//Enable physics just to be sure before addImpulse 
			StaticMeshComponent->SetSimulatePhysics(true);
			
			// Add force to move towards the GrabbedObjectLocation
			const FVector ForceDirection = (GrabbedObjectLocation - SuccessLocation).GetSafeNormal();
			StaticMeshComponent->AddImpulse(ForceDirection * AspirationForceMagnitude, NAME_None, true);
		}
	}
}

void ACDSuccessActor::BlowHit_Implementation(AActor* Shooter)
{
	if(!Shooter) return;
	
	if(ABaseCharacter* CharacterRef = Cast<ABaseCharacter>(Shooter))
	{
		// Calculate direction from player to rock actor
		FVector BlowDirection = GetActorLocation() - CharacterRef->GetActorLocation();
		BlowDirection.Normalize();

		StaticMeshComponent->SetSimulatePhysics(true);

		// Apply impulse in the calculated direction
		StaticMeshComponent->AddImpulse(BlowDirection * AspirationForceMagnitude, NAME_None, true);
	}
}

void ACDSuccessActor::ApplyModifier()
{
	BlueprintModifier();
}

void ACDSuccessActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACDSuccessActor, StaticMeshComponent);
}

// Called when the game starts or when spawned
void ACDSuccessActor::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
		StaticMeshComponent->SetSimulatePhysics(true);
}

