#include "Characters/BaseCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AspirableActors/RockActor.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Controllers/BasePlayerController.h"
#include "Enemies/CDEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "Spawning/MoleSpawner/MoleBurrow.h"
#include "Spawning/MoleSpawner/MoleHole.h"


// Sets default values
ABaseCharacter::ABaseCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(GetMesh());

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent);

	AspiratorComponent = CreateDefaultSubobject<UAspiratorComponent>(TEXT("Aspirator Component"));

	//Mesh of the vacuum gun
	VacuumGunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Vacuum gun mesh"));
	VacuumGunMesh->SetupAttachment(GetMesh());

	//Position where the object should be grab (the muzzle of the vacuum)
	GrabbedObjectLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Grabbed Object Location"));
	GrabbedObjectLocation->SetupAttachment(GetCapsuleComponent());

	AspirableActor = CreateDefaultSubobject<UAspirableActor>(TEXT("Is aspirable actor"));

	bReplicates = true;
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseCharacter::ServerDispawnMole_Implementation()
{
	if (MoleHoleIsColliding != nullptr)
	{
		MoleHoleIsColliding->SetOwner(this);
		MoleHoleIsColliding->DispawnMole();
	}
}

void ABaseCharacter::ManageOccludedActors_Implementation()
{
		TArray<FHitResult> OutHits;
		TArray<AActorOccluding*> NewActorOccluded;
		FVector Start = CameraComponent->GetComponentLocation();
		FVector End = GetActorLocation();
		//DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1, 0, 1);

		GetWorld()->LineTraceMultiByChannel(OutHits, Start, End, ECC_Visibility);

		for (FHitResult Hit : OutHits) 
		{
			if (AActorOccluding* ActorOcludding = Cast<AActorOccluding>(Hit.GetActor()))
			{
				HideOccludedActor(ActorOcludding);
				NewActorOccluded.Add(Cast<AActorOccluding>(Hit.GetActor()));
			}
		}

		for (AActorOccluding* LastOcluddedActor : LastActorsOccluded)
		{
			if (!NewActorOccluded.Contains(LastOcluddedActor)) 
			{
				ShowOccludedActor(LastOcluddedActor);
			}
		}

		LastActorsOccluded = NewActorOccluded;
}

void ABaseCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	if(const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if(Subsystem)
			{
				Subsystem->AddMappingContext(MappingContext, 0);
			}
		}
	}
}

void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Ajouter tout les Inputs Actions
		if (VacuumInputAspiration)
		{
			PlayerEnhancedInputComponent->BindAction(VacuumInputAspiration,
				ETriggerEvent::Triggered,
				this,
				&ABaseCharacter::VacuumAspiration);
		}

		if (VacuumInputBlow)
		{
			PlayerEnhancedInputComponent->BindAction(VacuumInputBlow,
				ETriggerEvent::Triggered,
				this,
				&ABaseCharacter::VacuumBlow);
		}

		if (VacuumInputBlow)
		{
			PlayerEnhancedInputComponent->BindAction(VacuumInputBlow,
				ETriggerEvent::Triggered,
				this,
				&ABaseCharacter::VacuumHoldBlow);
		}

		if (InteractInput)
		{
			PlayerEnhancedInputComponent->BindAction(InteractInput,
				ETriggerEvent::Triggered,
				this,
				&ABaseCharacter::InterationInput);
		}
	}
}

void ABaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	//Setup loadout
	const ABasePlayerController* PlayerController = Cast<ABasePlayerController>(NewController);
	
	if(PlayerController == nullptr || !HasAuthority())
		return;

	AspiratorComponent->SetupLoadout(PlayerController->DataAsset->VacuumData[PlayerController->PrimaryVacuum]);
}

void ABaseCharacter::VacuumAspiration(const FInputActionValue& Value)
{
	if(bIsVaccumBlowing == true)	//return if is blowing
		return;
	
	if(Value.Get<bool>())
	{
		bIsVaccumAspirating = true;
		AspiratorComponent->StartFire();
		
	} else
	{
		bIsVaccumAspirating = false;
		AspiratorComponent->StopFire();
	}
}

void ABaseCharacter::VacuumBlow(const FInputActionValue& Value)
{
	if(bIsVaccumAspirating == true) //return if is aspirating
		return;
	
	if(Value.Get<bool>())
	{
		bIsVaccumBlowing = true;
		AspiratorComponent->StartFireBlow();
	} else
	{
		bIsVaccumBlowing = false;
		AspiratorComponent->StopFireBlow();
		if(bIsBlowInMoleHole)
		{
			bIsBlowInMoleHole = false;
			ServerDispawnMole();
		}
	}
}

void ABaseCharacter::VacuumHoldBlow(const FInputActionInstance& Instance)
{
	if(!bIsInteraction) return;		//return if is not interacting
	if(MoleHoleIsColliding == nullptr) return;

	if(HasAuthority())
	{
		for(auto moleHole : MoleHoleIsColliding->moleBurrow->MoleHoles)
		{
			if(moleHole != MoleHoleIsColliding)
			{
				if(!moleHole->isShakingDirt)
				{
					MulticastLaunchDirtShaking(moleHole);
				}
			}
		}
	}else
	{
		ServerLaunchDirtShaking();
	}
	
	if(Instance.GetElapsedTime() < 2) return;	//return if the time is not enough to spawn a mole
	
	
	if(bIsBlowInMoleHole) return;	//return if already blow in mole hole and a mole is spawned
	//if(MoleHoleIsColliding->SpawnedMole != nullptr) return;	//return if a mole is already spawned
	if(!bIsVaccumBlowing) return;	//return if a mole is already spawned
	
	MoleHoleIsColliding->SetOwner(this);
	ServerSpawnMole();
	bIsBlowInMoleHole = true;
}

void ABaseCharacter::InterationInput(const FInputActionValue& Value)
{
	if (MoleHoleIsColliding || TravelerIsColliding || ArmoryIsColliding)
	{
		if(bIsInteraction == true) //if is interacting
		{
			bIsInteraction = false;
			if(MoleHoleIsColliding)
			{
				Server_SetIsDiggingForMoles(false);
			}
			
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("no Interact anymore !")));

			//Show shoot vfx again
			if(bIsVaccumAspirating == true)
			{
				//show aspiration vfx
				AspiratorComponent->ShowFeedbackAfterEndInteractMole(true);
			}
			else if(bIsVaccumBlowing == true)
			{
				//show blow vfx
				AspiratorComponent->ShowFeedbackAfterEndInteractMole(false);
			}
			
			if(bIsBlowInMoleHole)
			{
				bIsBlowInMoleHole = false;
				ServerDispawnMole();
			}
			if(TravelerIsColliding)
			{
				OnPlayerUnsleep.Broadcast();
			}
			if(ArmoryIsColliding)
			{
				BlueprintClientBookshelfUnInterraction();
			}
			return;
		}
		
		if(Value.Get<bool>())
		{
			bIsInteraction = true;

			if(MoleHoleIsColliding)
			{
				Server_SetIsDiggingForMoles(true);
			}
			
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("Interact !")));
			
			if(bIsObjectBlockingVacuum == true)	//If the vacuum is currently blocked and the player is interacting
			{
				//Drop the grabbed object
				SetDropGrabbedObject(700.0f, nullptr);
			}
			
			if(TravelerIsColliding)
			{
				OnPlayerSleep.Broadcast();
			}
			
			if (ArmoryIsColliding)
			{
				BlueprintClientBookshelfInterraction();
			}
		}
	}
}

void ABaseCharacter::ServerSpawnMole_Implementation()
{
	if (MoleHoleIsColliding != nullptr)
	{
		MoleHoleIsColliding->SetOwner(this);
		MoleHoleIsColliding->SpawnMole();
	}
}

void ABaseCharacter::SetGrabbedObject_Implementation(UPrimitiveComponent* ObjectToGrab)
{
	if(ObjectToGrab)
	{
		bIsObjectBlockingVacuum = true;
		ObjectToGrab->SetSimulatePhysics(false);
		if(ARockActor* GrabbedRock = Cast<ARockActor>(ObjectToGrab->GetOwner()))
		{
			GrabbedRock->bIsRockCurrentlyGrabbedByPlayer = true;
		}
	}
	
	MulticastSetGrabbedObject(ObjectToGrab);
}

void ABaseCharacter::MulticastSetGrabbedObject_Implementation(UPrimitiveComponent* ObjectToGrab)
{
	GrabbedObject = ObjectToGrab;

	if(GrabbedObject)
	{
		//Attach the object to the muzzle socket of the vacuum
		GrabbedObject->AttachToComponent(VacuumGunMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "MuzzleSocket");
		//Attach the object to the Grab object location (old method)
		//GrabbedObject->AttachToComponent(GrabbedObjectLocation, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
}

void ABaseCharacter::SetDropGrabbedObject_Implementation(const float ShootingBlowStrength, const ACDEnemy* EnemyDetected)
{
	if(GrabbedObject)
	{
		bIsObjectBlockingVacuum = false;
		GrabbedObject->SetSimulatePhysics(true);
		if(ARockActor* GrabbedRock = Cast<ARockActor>(GrabbedObject->GetOwner()))
		{
			GrabbedRock->bIsRockCurrentlyGrabbedByPlayer = false;
		}
	}
	
	MulticastSetDropGrabbedObject(ShootingBlowStrength, EnemyDetected);
}

void ABaseCharacter::MulticastSetDropGrabbedObject_Implementation(const float ShootingBlowStrength, const ACDEnemy* EnemyDetected)
{
	if(GrabbedObject)
	{
		//Detach the object to the Grab object location
		GrabbedObject->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

		if(EnemyDetected == nullptr)
		{
			// If no enemy is detected, shoot in the forward direction of the character
			GrabbedObject->AddImpulse(GetActorForwardVector() * ShootingBlowStrength, NAME_None, true);
		}
		else
		{
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("enemy detected")));
			
			// If enemy detected shoot on him by calculating the required direction
			// Calculate direction towards the enemy
			FVector DirectionToEnemy = (EnemyDetected->GetActorLocation() - GrabbedObject->GetComponentLocation()).GetSafeNormal();

			// Apply impulse towards the enemy
			GrabbedObject->AddImpulse(DirectionToEnemy * 4000, NAME_None, true);
		}

		if(UStaticMeshComponent* RockMesh = Cast<UStaticMeshComponent>(GrabbedObject))
		{
			RockMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

			//Get the owner actor of this mesh
			if(ARockActor* Rock = Cast<ARockActor>(RockMesh->GetOwner()))
			{
				Rock->RockThrownToEnemy = true;
			}
		}
	}
}

void ABaseCharacter::PlayerHitByAspiration_Implementation(AActor* Shooter)
{
	if(!Shooter) return;
	if(this->bIsdiggingForMoles == true) return; //If the player is currently interacting for moles don't move him and return
	
	if(ABaseCharacter* ShooterCharacter = Cast<ABaseCharacter>(Shooter))
	{
		if(ShooterCharacter->bIsObjectBlockingVacuum == true)
		{
			return;
		}
		
		//Add force to move the player towards the GrabbedObjectLocation
		FVector Direction = ShooterCharacter->GetActorLocation() - GetActorLocation();
		Direction.Z = 0.0f;	//Make the direction in Z null to have only horizontal movement
		Direction.Normalize();

		//Setup the force to apply
		const FVector Force = Direction * VacuumAttractionForceTowardsShooter;

		//Apply force to the current player
		GetCharacterMovement()->AddImpulse(Force, true);
	}
}

void ABaseCharacter::PlayerHitByExpulsion_Implementation(AActor* Shooter)
{
	if(!Shooter) return;
	if(this->bIsdiggingForMoles == true) return; //If the player is currently interacting for moles don't move him and return
	
	if(ABaseCharacter* ShooterCharacter = Cast<ABaseCharacter>(Shooter))
	{
		// Calculate direction from one player to the other
		FVector BlowDirection = GetActorLocation() - ShooterCharacter->GetActorLocation();
		BlowDirection.Z = 0.0f; //Make the direction in Z null to have only horizontal movement
		BlowDirection.Normalize();

		//Setup the force to apply
		const FVector Force = BlowDirection * 350.0f;

		if(GetCharacterMovement()->IsFalling()) return; //Can't apply impulse to falling character
		
		//Add force to move the player away 
		GetCharacterMovement()->AddImpulse(Force, true);
	}
}

void ABaseCharacter::Server_SetIsDiggingForMoles_Implementation(bool bIsDigging)
{
	bIsdiggingForMoles = bIsDigging;
}

void ABaseCharacter::DispawnMole()
{
	bIsBlowInMoleHole = false;
	if (MoleHoleIsColliding != nullptr)
	{
		MoleHoleIsColliding->SetOwner(this);
		MoleHoleIsColliding->DispawnMole();
	}
}

void ABaseCharacter::ServerLaunchDirtShaking_Implementation()
{
	if(MoleHoleIsColliding)
	{
		if(MoleHoleIsColliding->moleBurrow)
		{
			for(auto moleHole : MoleHoleIsColliding->moleBurrow->MoleHoles)
			{
				if(moleHole != MoleHoleIsColliding)
				{
					if(!moleHole->isShakingDirt)
					{
						moleHole->LaunchShakingDirtNiagara();
						MulticastLaunchDirtShaking(moleHole);
					}
				}
			}
		}
	}
}

void ABaseCharacter::MulticastLaunchDirtShaking_Implementation(AMoleHole* HoleToShakeDirt)
{
	HoleToShakeDirt->LaunchShakingDirtNiagara();
}

ACDEnemy* ABaseCharacter::DetectEnemyForShootProjectile()
{
	//Shape trace
	TArray<FHitResult> HitResults; //HitResults array

	const FVector BoxDimension =  FVector(250.0f, 250.0f, 300.0f);

	//Get forward vector
	const FVector ForwardVector = GetActorForwardVector();
	
	//Start location 
	const FVector SweepStart = GetActorLocation() + (ForwardVector * 60) + FVector(0,0,60);

	//End location
	const FVector SweepEnd = SweepStart+(ForwardVector * 1500.0f);

	//Create collision box
	const FCollisionShape CollisionBox = FCollisionShape::MakeBox(BoxDimension);

	//draw box for debug
	DrawDebugBox(GetWorld(), SweepStart,   BoxDimension, FColor::Purple, false, 5.0f);
	DrawDebugBox(GetWorld(), SweepEnd,   BoxDimension, FColor::Orange, false, 5.0f);

	//Ignore self 
	FCollisionQueryParams Params = FCollisionQueryParams();
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());
	Params.bTraceComplex = true;
	
	//Execute the shape trace
	// check if something got hit in the sweep
	GetWorld()->SweepMultiByChannel(HitResults, SweepStart, SweepEnd, FQuat::Identity, ECC_Visibility, CollisionBox, Params);
	
	for (auto& Hit : HitResults)
	{
		// get current hit actor
		auto* ActorHit = Hit.GetActor();

		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Actor Touched: %s"), *ActorHit->GetName()));
		if(ACDEnemy* EnemyDetected = Cast<ACDEnemy>(ActorHit))
		{
			//Return the detected enemy
			return EnemyDetected;
		}
	}
	
	return nullptr;
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseCharacter, VacuumGunMesh);
	DOREPLIFETIME(ABaseCharacter, GrabbedObject);
	DOREPLIFETIME(ABaseCharacter, GrabbedObjectLocation);
	DOREPLIFETIME(ABaseCharacter, bIsObjectBlockingVacuum);
	DOREPLIFETIME(ABaseCharacter, MoleHoleIsColliding);
	DOREPLIFETIME(ABaseCharacter, bIsdiggingForMoles);
}

void ABaseCharacter::HideOccludedActor(AActorOccluding* OccludedActor)
{
	UStaticMeshComponent* ActorMesh = Cast<UStaticMeshComponent>(OccludedActor->GetComponentByClass(UStaticMeshComponent::StaticClass()));
	for (int i = 0; i < ActorMesh->GetNumMaterials(); ++i) 
	{
		ActorMesh->SetMaterial(i, TransparentMaterial);
	}
}

void ABaseCharacter::ShowOccludedActor(AActorOccluding* OccludedActor)
{
	UStaticMeshComponent* ActorMesh = Cast<UStaticMeshComponent>(OccludedActor->GetComponentByClass(UStaticMeshComponent::StaticClass()));
	for (int i = 0; i < ActorMesh->GetNumMaterials(); ++i)
	{
		ActorMesh->SetMaterial(i, OccludedActor->ActorAncientMaterial);
	}
}
