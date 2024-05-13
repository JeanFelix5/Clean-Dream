// Fill out your copyright notice in the Description page of Project Settings.


#include "AspirableActors/RockActor.h"

#include "Characters/BaseCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Vacuum/Vacuum.h"

// Sets default values
ARockActor::ARockActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static mesh"));
	StaticMeshComponent->SetupAttachment(RootComponent);
	
	AspirableActor = CreateDefaultSubobject<UAspirableActor>(TEXT("Is aspirable actor"));

	bReplicates = true;
}

// Called when the game starts or when spawned
void ARockActor::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
		StaticMeshComponent->SetSimulatePhysics(true);
}

void ARockActor::OnHit_Implementation(AActor* Shooter)
{
	if(!Shooter) return;
	
	if(ABaseCharacter* CharacterRef = Cast<ABaseCharacter>(Shooter))
	{
		if(CharacterRef->bIsObjectBlockingVacuum == true)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("The aspirator is blocked, right click to push it out (SHOW THIS IN A UI)"));
			return;
		}
		
		FVector RockLocation = GetActorLocation();
		FVector GrabbedObjectLocation = CharacterRef->GrabbedObjectLocation->GetComponentLocation();

		// Zero out the Z component for horizontal distance calculation
		RockLocation.Z = 0.0f;
		GrabbedObjectLocation.Z = 0.0f;
		float Distance = FVector::Distance(RockLocation, GrabbedObjectLocation);

		if (Distance < DistanceToGrab)
		{
			CharacterRef->SetGrabbedObject(StaticMeshComponent);
			StaticMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		}
		else
		{
			//Enable physics just to be sure before addImpulse 
			StaticMeshComponent->SetSimulatePhysics(true);
			
			// Add force to move towards the GrabbedObjectLocation
			const FVector ForceDirection = (GrabbedObjectLocation - RockLocation).GetSafeNormal();
			StaticMeshComponent->AddImpulse(ForceDirection * AspirationForceMagnitude, NAME_None, true);
		}
	}
}

void ARockActor::BlowHit_Implementation(AActor* Shooter)
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

void ARockActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARockActor, StaticMeshComponent);
}
