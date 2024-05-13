// Fill out your copyright notice in the Description page of Project Settings.


#include "Vacuum/Vacuum.h"

#include "AIControllers/CDAIControllerEnemy.h"
#include "AspirableActors/CDSuccessActor.h"
#include "AspirableActors/RockActor.h"
#include "Characters/BaseCharacter.h"
#include "Components/AspirableActor.h"
#include "Controllers/BasePlayerController.h"
#include "Enemies/CDEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AVacuum::AVacuum()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal mesh component"));
	RootComponent = SkeletalMeshComponent;

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp"));
	AudioComponent->SetupAttachment(SkeletalMeshComponent);

	AudioComponentBlow = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp blow"));
	AudioComponentBlow->SetupAttachment(SkeletalMeshComponent);

	AudioComponentBlocked = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp blocked"));
	AudioComponentBlocked->SetupAttachment(SkeletalMeshComponent);
	
	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Niagara Emitter"));
	NiagaraComponent->SetupAttachment(SkeletalMeshComponent);

	NiagaraComponentBlow = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Niagara Emitter blow"));
	NiagaraComponentBlow->SetupAttachment(SkeletalMeshComponent);

	bReplicates = true;
}

// Called when the game starts or when spawned
void AVacuum::BeginPlay()
{
	Super::BeginPlay();
	
}

void AVacuum::PlayVacuumBlockedFeedback()
{
	if(AudioComponent)
	{
		//Stop the current sound
		AudioComponent->Stop();
	}

	if(bPlayedBlockedSoundAlready == false)
	{
		if(AudioComponentBlocked)
		{
			if(AudioComponentBlocked->IsPlaying() == false)
			{
				//Play the blocked vacuum sound
				AudioComponentBlocked = UGameplayStatics::SpawnSoundAtLocation(this, VacuumData.SoundVacuumAspirationBlocked,
					GetActorLocation());

				bPlayedBlockedSoundAlready = true;
			}
		}
		else //If the audio is null play the sound anyway (the audio component can become null)
		{
			//Play the blocked vacuum sound
			AudioComponentBlocked = UGameplayStatics::SpawnSoundAtLocation(this, VacuumData.SoundVacuumAspirationBlocked,
				GetActorLocation());

			bPlayedBlockedSoundAlready = true;
		}
	}
	Server_StopFeedbackAspiration(); //If the player vacuum is blocked stop the aspiration feedback
}

void AVacuum::PlayFeedback(bool bIsAspirating)
{
	if(bIsAspirating == true)
	{
		//Play aspiration feedbacks

		// Stop the current sound
		if (AudioComponent)
		{
			AudioComponent->Stop();
		}

		// Play the vacuum sound
		AudioComponent = UGameplayStatics::SpawnSoundAtLocation(this, VacuumData.SoundAspiration, GetActorLocation());

		if(ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner()))
		{
			if(Character->bIsdiggingForMoles == false)
			{
				//Play the aspiration vfx
				if(VacuumData.AspirationFX)
				{
					Server_PlayVFX(true);
				}
			}
		}
	}
	else
	{
		//Play expulsion feedbacks
		
		if (AudioComponentBlow)
		{
			// If AudioComponentBlow is valid, stop the current sound
			if (AudioComponentBlow->IsPlaying() == false)
			{
				AudioComponentBlow->Stop();

				//Play the vacuum sound
				AudioComponentBlow = UGameplayStatics::SpawnSoundAtLocation(this, VacuumData.SoundBlow, GetActorLocation());
			}
		}
		else
		{
			// AudioComponentBlow is not valid, initialize it before usage
			AudioComponentBlow = UGameplayStatics::SpawnSoundAtLocation(this, VacuumData.SoundBlow, GetActorLocation());
		}
		
		if(ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner()))
		{
			if(Character->bIsdiggingForMoles == false)
			{
				//Play the blow vfx
				if(VacuumData.AspirationFX)
				{
					Server_PlayVFX(false);
				}
			}
		}
	}
}

void AVacuum::Server_PlayVFX_Implementation(bool bIsAspirating)
{
	Multicast_PlayVFX(bIsAspirating);
}

void AVacuum::Multicast_PlayVFX_Implementation(bool bIsAspirating)
{
	if(bIsAspirating == true)
	{
		if(ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner()))
		{
			if(NiagaraComponent)
			{
				if(NiagaraComponent->IsActive())
				{
					NiagaraComponent->SetHiddenInGame(false);
					//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::Printf(TEXT("aspiration fx active")));
				}
				else
				{
					//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::Printf(TEXT("aspiration fx playing")));
					NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(VacuumData.AspirationFX,NiagaraComponent,"None",
					Character->VacuumGunMesh->GetSocketLocation(FName("VFXAspirationPositionSocket")),
					GetActorRotation() + FRotator(-76.0f, 0.0f, 0.0f),EAttachLocation::KeepWorldPosition,true,true,ENCPoolMethod::None,true);
				}
			}
		}
	}
	else
	{
		if(ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner()))
		{
			if(NiagaraComponentBlow)
			{
				if(NiagaraComponentBlow->IsActive())
				{
					NiagaraComponentBlow->SetHiddenInGame(false);
					//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::Printf(TEXT("blow fx active")));
				}
				else
				{
					//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::Printf(TEXT("blow fx playing")));
					NiagaraComponentBlow = UNiagaraFunctionLibrary::SpawnSystemAttached(VacuumData.BlowFX,NiagaraComponent,"None",
					Character->VacuumGunMesh->GetSocketLocation(FName("VFXPositionSocket")),
					GetActorRotation() + FRotator(5.0f, 0.0f, 0.0f),EAttachLocation::KeepWorldPosition,true,true,ENCPoolMethod::None,true);
				}
			}
		}
	}
}

void AVacuum::Shoot()
{
	//Verify that the shooting rate is valid
	const float FireRate = VacuumData.ShootingRate;
	//checkf(FireRate > 0, TEXT("The shooting rate can't be < 0"));

	//Launch timer immediately to call the shoot function at every fire rate interval
	GetWorldTimerManager().SetTimer(FireTimerHandle, [&](){VacuumAspirationShoot();}, 60.0f/FireRate, true, 0.0f);

	if(ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner()))
	{
		if(Character->bIsObjectBlockingVacuum == false) //If the vacuum is not blocked, play the aspiration vfx. 
		{
			//Play vacuum feedback only once here and not every fire rate
			PlayFeedback(true);
		}
	}
}

void AVacuum::ShootBlow()
{
	//Verify that the shooting rate is valid
	const float FireRate = VacuumData.ShootingRate;
	//checkf(FireRate > 0, TEXT("The shooting rate can't be < 0"));

	//Launch timer immediately to call the shoot function at every fire rate interval
	GetWorldTimerManager().SetTimer(FireBlowTimerHandle, [&](){VacuumBlowShoot();}, 60.0f/FireRate, true, 0.0f);

	//Play vacuum feedback only once here and not every fire rate
	PlayFeedback(false);
}

void AVacuum::Server_StopShooting_Implementation()
{
	Multicast_StopShooting();
}

bool AVacuum::Server_StopShooting_Validate()
{
	return true;
}

void AVacuum::Multicast_StopShooting_Implementation()
{
	StopShooting();
}

void AVacuum::StopShooting()
{
	//Clear the timer if stop shooting
	GetWorldTimerManager().ClearTimer(FireTimerHandle);

	OnStopShootingDelegate.Broadcast();

	if(AudioComponent)
	{
		//Stop the current sound
		AudioComponent->Stop(); 

		if(ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner()))
		{
			if(Character->bIsObjectBlockingVacuum == false) //If the vacuum is not blocked, play the sound
			{
				//Play the vacuum stop sound
				AudioComponent = UGameplayStatics::SpawnSoundAtLocation(this, VacuumData.SoundAspirationStop, GetActorLocation());
			}
		}
	}

	bPlayedBlockedSoundAlready = false;
	
	Server_StopFeedbackAspiration();
}

void AVacuum::StopShootingBlow()
{
	//Clear the timer if stop shooting
	GetWorldTimerManager().ClearTimer(FireBlowTimerHandle);

	if(AudioComponentBlow)
	{
		//Stop the current sound
		AudioComponentBlow->Stop();
	}

	if(AudioComponent)
	{
		//Play the vacuum blow stop sound
		AudioComponent = UGameplayStatics::SpawnSoundAtLocation(this, VacuumData.SoundBlowStop, GetActorLocation());
	}

	Server_StopFeedbackBlow();
}

void AVacuum::Server_StopFeedbackAspiration_Implementation()
{
	Multicast_StopFeedbackAspiration();
}

void AVacuum::Server_StopFeedbackBlow_Implementation()
{
	Multicast_StopFeedbackBlow();
}

void AVacuum::Multicast_StopFeedbackAspiration_Implementation()
{
	if(NiagaraComponent)
	{
		//Hide the vfx
		NiagaraComponent->SetHiddenInGame(true);
	}
}

void AVacuum::Multicast_StopFeedbackBlow_Implementation()
{
	if(NiagaraComponentBlow)
	{
		//Hide the vfx
		NiagaraComponentBlow->SetHiddenInGame(true);
	}
}

bool AVacuum::VacuumAspirationShoot()
{
	//Check if the character isn't garbage collected while firing (because we are using timers)
	if(!this) return false;

	// Check if the object is still valid
	if (!IsValidLowLevel())
	{
		UE_LOG(LogTemp, Error, TEXT("AVacuum object is invalid"));
		return false;
	}
	
	// Check if the owner is valid
	AActor* myOwner = GetOwner();
	if (!myOwner || !myOwner->IsValidLowLevel())
	{
		UE_LOG(LogTemp, Error, TEXT("Owner is invalid"));
		return false;
	}
	

	if(ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner()))
	{
		if(Character->bIsObjectBlockingVacuum == true)
		{
			PlayVacuumBlockedFeedback();
			return false;
		}

		if(Character->bIsdiggingForMoles == true)
		{
			Server_StopFeedbackAspiration();
			return false;
		}
		
		//Shape trace
		TArray<FHitResult> HitResults; //HitResults array

		//Get forward vector
		const FVector ForwardVector = GetActorForwardVector();
	
		//Start location 
		const FVector SweepStart = GetActorLocation() + (ForwardVector * 60) + FVector(0,0,80);

		//End location
		const FVector SweepEnd = SweepStart+(ForwardVector*VacuumData.ShootingRange);

		//Create collision box
		FCollisionShape CollisionBox = FCollisionShape::MakeBox(VacuumData.ShootingBoxTraceSize);

		//draw box for debug
		DrawDebugBox(GetWorld(), SweepStart,  VacuumData.ShootingBoxTraceSize, FColor::Red, false, 1.0f);
		DrawDebugBox(GetWorld(), SweepEnd,  VacuumData.ShootingBoxTraceSize, FColor::Green, false, 1.0f);

		//Ignore self 
		FCollisionQueryParams Params = FCollisionQueryParams();
		Params.AddIgnoredActor(this);
		Params.AddIgnoredActor(GetOwner());
		Params.bTraceComplex = true;
	
		//Execute the shape trace
		// check if something got hit in the sweep
		TArray<FHitResult> HitResultsTmp;
		GetWorld()->SweepMultiByChannel(HitResultsTmp, SweepStart, SweepEnd, FQuat::Identity, ECC_Visibility, CollisionBox, Params);

		HitResults.Append(HitResultsTmp);
		
		//Server Shoot(Hit[], AActor[], )
		ServerShoot(HitResults, Character, this->GetTransform());
	}
	
	return true;
}

bool AVacuum::ServerShoot_Validate(const TArray<FHitResult>& Results, const AActor* InstigatorActor, const FTransform Transform)
{
	//Some validation
	if(InstigatorActor != nullptr)
	{
		return true;
	}

	return false;
}

void AVacuum::ServerShoot_Implementation(const TArray<FHitResult>& Results, const AActor* InstigatorActor, const  FTransform Transform)
{
	//Confirm shoot
	/* if(ShootConfirm)
	 *Hit(Instigator)
	 *GetPlayerController()->ClientShotConfirmed();
	 **/
	
	if(const ABaseCharacter* PlayerInstigator = Cast<ABaseCharacter>(InstigatorActor))
	{
		// loop through HitResults array
		for (auto& Hit : Results)
		{
			// get current hit actor and check if have the AspirableActor component
			auto* ActorHit = Hit.GetActor();
			if(ActorHit == nullptr)
				return;
			
			if(const auto* AspirableActor = StaticCast<UAspirableActor*>(ActorHit->GetComponentByClass(UAspirableActor::StaticClass())))
			{
				
				if(ARockActor* Rock = Cast<ARockActor>(ActorHit))
				{
					if(Rock->bIsRockCurrentlyGrabbedByPlayer == false)
					{
						//If is a rock actor call the onHit function and send the character (the owner) as a ref (very important)
						Rock->OnHit(GetOwner());
					}
				}

				if(ACDSuccessActor* SuccessActor = Cast<ACDSuccessActor>(ActorHit))
				{
					//If is a success actor
					SuccessActor->OnHit(GetOwner());
				}
				
				if(ABaseCharacter* Character = Cast<ABaseCharacter>(ActorHit)) 
				{
					//Attract the other player
					Character->PlayerHitByAspiration(GetOwner());

					/*
					if(ABasePlayerController* PlayerController = Cast<ABasePlayerController>(PlayerInstigator->GetController()))
					{
						//The player hit the other player with the aspiration
						//PlayerController->ClientShotConfirmed();
					}
					*/
				}
				
				if(ACDEnemy* Enemy = Cast<ACDEnemy>(ActorHit))
				{
					FVector VacuumLocation = GetActorLocation();
					FVector EnemyLocation = Enemy->GetActorLocation();

					//Zero out the Z component for horizontal distance calculation
					VacuumLocation.Z = 0.0f;
					EnemyLocation.Z = 0.0f;
					float Distance = FVector::Distance(VacuumLocation, EnemyLocation);
				
					if(Distance <= Enemy->VacuumDistance)
					{
						//Increase player capture counter
						if(ABaseCharacter* Player = Cast<ABaseCharacter>(GetOwner()))
						{
							if(Enemy->bIsEnemyBeingCaptured == false)
							{
								Enemy->bIsEnemyBeingCaptured = true;

								// Add enemy to counter (using levelmanager)
								Enemy->OnDestroyUpdateScore();

								//Play Feedbacks Client only
								//Enemy->Client_PlayDeathFeedbacks();
								Enemy->Multicast_PlayDeathFeedbacks();
								
								//Play the captured animation in a multicast
								MulticastEnemyDeathAnimation(Enemy);
								
								//Add enemy to counter to this player only
								Player->EnemyCapturedCounter++;
								Player->OnPlayerEnemyCaptured.Broadcast();

								GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Captured counter: %d"), Player->EnemyCapturedCounter));
							}
						}
					}
					else
					{
						//Change the behavior of the enemy to attract him closer to the vacuum
						//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::Printf(TEXT("aspiration not close enough, attract the enemy near player")));
						if(ACDAIControllerEnemy* EnemyController = Cast<ACDAIControllerEnemy>(Enemy->GetController()); EnemyController != nullptr)
						{
							if(ABaseCharacter* Player = Cast<ABaseCharacter>(GetOwner()))
							{
								EnemyController->Server_StartAspiration(Player, this);	
							}
							else
							{
								GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::Printf(TEXT("Owner of Vacuum is not player base character")));
							}
						}
					}
				}
			}
		}
		
		//MulticastShoot(Results, PlayerInstigator);
	}
}

/*
void AVacuum::MulticastShoot_Implementation(const TArray<FHitResult>& Results, const AActor* InstigatorActor)
{
	if(InstigatorActor == nullptr)
		return;
	
	//Get instigator position for vfx
	FVector3d pos = InstigatorActor->GetActorLocation();

	// Get the local player
	APlayerController* LocalPlayer = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	// Get the controller of the instigator actor
	AController* InstigatorController = InstigatorActor->GetInstigatorController();
	
	if (LocalPlayer != InstigatorController)
	{
		//!= Instigator (not the one that is shooting)
		//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, FString::Printf(TEXT("!= Instigator (not the one that is shooting)")));
		
	}
	else
	{
		//Instigator (the one that is shooting)
		//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, FString::Printf(TEXT("Instigator (the one that is shooting)")));
	}
}
*/

void AVacuum::MulticastEnemyDeathAnimation_Implementation(ACDEnemy* EnemyToDestroy)
{
	EnemyToDestroy->DestroyAnimation(GetActorLocation());
}

bool AVacuum::VacuumBlowShoot()
{
	//Check if the character isn't garbage collected while firing (because we are using timers)
	if(!this) return false;

	// Check if the object is still valid
	if (!IsValidLowLevel())
	{
		UE_LOG(LogTemp, Error, TEXT("AVacuum object is invalid"));
		return false;
	}

	// Check if the owner is valid
	AActor* myOwner = GetOwner();
	if (!myOwner || !myOwner->IsValidLowLevel())
	{
		UE_LOG(LogTemp, Error, TEXT("Owner is invalid"));
		return false;
	}

	
	ABaseCharacter* CharacterRef = Cast<ABaseCharacter>(GetOwner());
	
	if(CharacterRef && CharacterRef->bIsObjectBlockingVacuum == true)
	{
		if(AudioComponentBlocked)
		{
			if(AudioComponentBlocked->IsPlaying()) //if the blocked sound is playing, stop it if it is expulsed
			{
				AudioComponentBlocked->Stop();
			}
		}

		bPlayedBlockedSoundAlready = false;
		
		//Detect if there is an enemy in front and drop the grabbed object
		ACDEnemy* EnemyDetected =  CharacterRef->DetectEnemyForShootProjectile();
		if(EnemyDetected == nullptr)
		{
			//Double check
			EnemyDetected = CharacterRef->DetectEnemyForShootProjectile();
		}
		//Shoot the grabbed object
		CharacterRef->SetDropGrabbedObject(VacuumData.ShootingBlowStrength, EnemyDetected);
	}
	else
	{
		if(CharacterRef)
		{
			if(CharacterRef->bIsdiggingForMoles == true)
			{
				Server_StopFeedbackBlow();
				return false;
			}
			
			//Shape trace
			TArray<FHitResult> HitResults; //HitResults array

			//Get forward vector
			const FVector ForwardVector = GetActorForwardVector();
	
			//Start location 
			const FVector SweepStart = GetActorLocation() + (ForwardVector * 60) + FVector(0,0,80);

			//End location
			const FVector SweepEnd = SweepStart+(ForwardVector*VacuumData.ShootingRange);

			//Create collision box
			FCollisionShape CollisionBox = FCollisionShape::MakeBox(VacuumData.ShootingBoxTraceSize);

			//draw box for debug
			DrawDebugBox(GetWorld(), SweepStart,  VacuumData.ShootingBoxTraceSize, FColor::Yellow, false, 1.0f);
			DrawDebugBox(GetWorld(), SweepEnd,  VacuumData.ShootingBoxTraceSize, FColor::Blue, false, 1.0f);

			//Ignore self 
			FCollisionQueryParams Params = FCollisionQueryParams();
			Params.AddIgnoredActor(this);
			Params.AddIgnoredActor(GetOwner());
			Params.bTraceComplex = true;
	
			//Execute the shape trace
			// check if something got hit in the sweep
			TArray<FHitResult> HitResultsTmp;
			GetWorld()->SweepMultiByChannel(HitResultsTmp, SweepStart, SweepEnd, FQuat::Identity, ECC_Visibility, CollisionBox, Params);

			HitResults.Append(HitResultsTmp);
		
			//Server Shoot(Hit[], AActor[], )
			ServerShootBlow(HitResults, CharacterRef, this->GetTransform());
		}
	}
	
	return true;
}

bool AVacuum::ServerShootBlow_Validate(const TArray<FHitResult>& Results, const AActor* InstigatorActor, const FTransform Transform)
{
	//Some validation
	if(InstigatorActor != nullptr)
	{
		return true;
	}

	return false;
}

void AVacuum::ServerShootBlow_Implementation(const TArray<FHitResult>& Results, const AActor* InstigatorActor, const FTransform Transform)
{
	//Confirm shoot
	/* if(ShootConfirm)
	 *Hit(Instigator)
	 *GetPlayerController()->ClientShotConfirmed();
	 **/

	if(const ABaseCharacter* PlayerInstigator = Cast<ABaseCharacter>(InstigatorActor))
	{
		// loop through HitResults array
		for (auto& Hit : Results)
		{
			// get current hit actor and check if have the AspirableActor component
			auto* ActorHit = Hit.GetActor();
			if(ActorHit == nullptr)
				return;

			if(const auto* AspirableActor = StaticCast<UAspirableActor*>(ActorHit->GetComponentByClass(UAspirableActor::StaticClass())))
			{
				
				if(ARockActor* Rock = Cast<ARockActor>(ActorHit))
				{
					if (Rock->GetVelocity().Size() > 60.0f) //Change the value if necessary to obtain a better result
					{
						//the rock is still moving do nothing
						//GEngine->AddOnScreenDebugMessage(1, 1.0f, FColor::Yellow, TEXT("Rock blow is still moving"));
					}
					else
					{
						if(Rock->bIsRockCurrentlyGrabbedByPlayer == false)
						{
							//The rock is not moving so, can apply the force
							Rock->BlowHit(GetOwner());
						}
					}
				}

				if(ACDSuccessActor* SuccessActor = Cast<ACDSuccessActor>(ActorHit))
				{
					if (SuccessActor->GetVelocity().Size() > 200.0f) //Change the value if necessary to obtain a better result
					{
						//the rock is still moving do nothing
					}
					else
					{
						//If is a success actor
						SuccessActor->BlowHit(GetOwner());
					}
				}
				
				if(ABaseCharacter* Character = Cast<ABaseCharacter>(ActorHit))
				{
					//Push the other player
					Character->PlayerHitByExpulsion(GetOwner());

					/*
					if(ABasePlayerController* PlayerController = Cast<ABasePlayerController>(PlayerInstigator->GetController()))
					{
						//The player hit the other player with the expulsion (blow)
						//PlayerController->ClientShotConfirmed();
					}
					*/
				}

				/*
				if(ACDEnemy* Enemy = Cast<ACDEnemy>(ActorHit))
				{
					//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Cyan, FString::Printf(TEXT("Enemy hit by expulsion")));

					//Change the behavior of the enemy to push him further away from the vacuum
					if(ACDAIControllerEnemy* EnemyController = Cast<ACDAIControllerEnemy>(Enemy->GetController()); EnemyController != nullptr)
					{
						if(ABaseCharacter* Player = Cast<ABaseCharacter>(GetOwner()))
						{
							//Push the enemy away
						}
						else
						{
							GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::Printf(TEXT("Owner of Vacuum is not player base character")));
						}
					}
				}
				*/
			}
		}
		
		//MulticastShootBlow(Results, PlayerInstigator);
	}
}

/*
void AVacuum::MulticastShootBlow_Implementation(const TArray<FHitResult>& Results, const AActor* InstigatorActor)
{
	if(InstigatorActor == nullptr)
		return;
	
	FVector3d pos = InstigatorActor->GetActorLocation();

	// Get the local player
	APlayerController* LocalPlayer = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	// Get the controller of the instigator actor
	AController* InstigatorController = InstigatorActor->GetInstigatorController();
	
	if (LocalPlayer != InstigatorController)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, FString::Printf(TEXT("!= Instigator (not the one that is shooting)")));
		
	}
	else
	{
		//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, FString::Printf(TEXT("Instigator (the one that is shooting)")));
	
	}
}
*/

void AVacuum::SetAspiratorData(const FVacuumDataAsset& Data)
{
	//Set the aspirator data from the data asset on spawn
	VacuumData = Data;
}

void AVacuum::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AVacuum, SkeletalMeshComponent);
	DOREPLIFETIME(AVacuum, VacuumData);
}

void AVacuum::StopShootingBeforeServerTravel()
{
	//Clear the timer if stop shooting
	GetWorldTimerManager().ClearTimer(FireTimerHandle);

	//Clear the timer if stop shooting
	GetWorldTimerManager().ClearTimer(FireBlowTimerHandle);
}