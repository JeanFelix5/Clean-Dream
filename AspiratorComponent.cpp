// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/AspiratorComponent.h"

#include "Characters/BaseCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Vacuum/Vacuum.h"

// Sets default values for this component's properties
UAspiratorComponent::UAspiratorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void UAspiratorComponent::BeginPlay()
{
	Super::BeginPlay();
	
	ABaseCharacter* Character = Cast<ABaseCharacter>(GetOuter());
	if(Character == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Character class"));
		return;
	}

	/* Commented this block because it is no longer needed
	 * We attach the vacuum in the AnimBP when the character is plugging a mole hole and detach after the animation is done
		const USkeletalMeshSocket* AspiratorSocket = Character->GetMesh()->GetSocketByName(AspiratorSocketName);
		if(AspiratorSocket == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("No socket for the weapon"));
			return;
		}

		//Attach the vacuum mesh to the hand socket of the character mesh
		Character->VacuumGunMesh->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, AspiratorSocketName);
	*/
}

void UAspiratorComponent::StartFire()
{
	if(PlayerVacuum)
	{
		PlayerVacuum->Shoot(); 
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("weapon not found in component"));
	}
	
}

void UAspiratorComponent::StopFire()
{
	if(PlayerVacuum)
	{
		PlayerVacuum->Server_StopShooting(); 
	}
}

void UAspiratorComponent::StartFireBlow()
{
	if(PlayerVacuum)
	{
		PlayerVacuum->ShootBlow(); 
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("weapon not found in component"));
	}
}

void UAspiratorComponent::StopFireBlow()
{
	if(PlayerVacuum)
	{
		PlayerVacuum->StopShootingBlow(); 
	}
}

void UAspiratorComponent::SetupLoadout_Implementation(const FVacuumDataAsset& Primary)
{
	ClientSetupLoadout(Primary);
}

void UAspiratorComponent::ClientSetupLoadout_Implementation(const FVacuumDataAsset& Primary)
{
	ABaseCharacter* Character = Cast<ABaseCharacter>(GetOuter());
	if(Character)
	{
		PlayerVacuum = Cast<AVacuum>(GetWorld()->SpawnActor(AVacuum::StaticClass()));
		if(PlayerVacuum)
		{
			PlayerVacuum->SetOwner(GetOwner());
			PlayerVacuum->SetAspiratorData(Primary);
			PlayerVacuum->SetActorHiddenInGame(false);
			
			//Attach the spawned vacuum the grab object location to ensure it is always facing in front and not influenced by animation
			PlayerVacuum->AttachToComponent(Character->GrabbedObjectLocation, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AspiratorSocketName);
			//PlayerVacuum->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, AspiratorSocketName);
		}
	}
}

void UAspiratorComponent::ShowFeedbackAfterEndInteractMole(bool bIsAspirating)
{
	if(PlayerVacuum)
	{
		PlayerVacuum->Server_PlayVFX(bIsAspirating); 
	}
}

void UAspiratorComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAspiratorComponent, PlayerVacuum);
}
