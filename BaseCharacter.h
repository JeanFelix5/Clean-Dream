#pragma once

#include "CoreMinimal.h"
#include "InputMappingContext.h"
#include "Camera/CameraComponent.h"
#include "Components/AspirableActor.h"
#include "Components/AspiratorComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "TravelerObject/CDTravelerObject.h"
#include <Environment/ActorOccluding.h>

#include "Success/CDSucessArmory.h"
#include "BaseCharacter.generated.h"

class ACDEnemy;
class AMoleHole;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerEnemyCaptured);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerSleep);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerUnsleep);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayersTravel);

UCLASS()
class CLEANDREAM_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsVaccumAspirating = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsVaccumBlowing = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsInteraction = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsBlowInMoleHole = false;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	AMoleHole* MoleHoleIsColliding;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	ACDTravelerObject* TravelerIsColliding;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	ACDSucessArmory* ArmoryIsColliding;

	UPROPERTY(BlueprintReadWrite)
	bool BookshelfColliding = false;

	/**
	* Event called when a enemy is captured
	*/
	UPROPERTY(BlueprintAssignable)
	FOnPlayerEnemyCaptured OnPlayerEnemyCaptured;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerSleep OnPlayerSleep;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerUnsleep OnPlayerUnsleep;
	
	UFUNCTION(BlueprintImplementableEvent)
	void BlueprintClientSleepInterraction();

	UFUNCTION(BlueprintImplementableEvent)
	void BlueprintClientUnsleepInterraction();

	UFUNCTION(BlueprintImplementableEvent)
	void BlueprintClientBookshelfInterraction();

	UFUNCTION(BlueprintImplementableEvent)
	void BlueprintClientBookshelfUnInterraction();

	UPROPERTY(BlueprintAssignable)
	FOnPlayersTravel OnPlayersTravel;
	
	UFUNCTION(BlueprintImplementableEvent)
	void PlayAspirationFeedback();

	UFUNCTION(BlueprintImplementableEvent)
	void PlayBlowingFeedback();

	//Replication of the grab and drop object
	UFUNCTION(Server, Reliable)
	void SetGrabbedObject(UPrimitiveComponent* ObjectToGrab);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetGrabbedObject(UPrimitiveComponent* ObjectToGrab);

	UFUNCTION(Server, Reliable)
	void SetDropGrabbedObject(const float ShootingBlowStrength, const ACDEnemy* EnemyDetected);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetDropGrabbedObject(const float ShootingBlowStrength, const ACDEnemy* EnemyDetected);
	//end of replication functions for grab and drop
	
	UPROPERTY(Replicated)
	UPrimitiveComponent* GrabbedObject;

	//Is an aspirable object blocking the aspirator and preventing it from aspirating
	UPROPERTY(Replicated)
	bool bIsObjectBlockingVacuum = false;

	//Very important variable because this check if the player in interacting with a mole hole or not. (Used for the replicated animation and other logic)
	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bIsdiggingForMoles = false;

	UFUNCTION(Server, Reliable)
	void Server_SetIsDiggingForMoles(bool bIsDigging);

	//mesh of the vacuum
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* VacuumGunMesh;

	UPROPERTY(Replicated, EditAnywhere)
	USceneComponent* GrabbedObjectLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UAspiratorComponent* AspiratorComponent;

	//To make the player aspirable
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UAspirableActor* AspirableActor;

	//Number of enemy aspirated by a player during a game
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int EnemyCapturedCounter = 0;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void PlayerHitByAspiration(AActor* Shooter);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void PlayerHitByExpulsion(AActor* Shooter);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vacuum params")
	float VacuumAttractionForceTowardsShooter = 500.0f;

	UFUNCTION(Server, Reliable)
	void ServerSpawnMole();

	UFUNCTION(Server, Reliable)
	void ServerDispawnMole();

	UFUNCTION()
	void DispawnMole();

	UFUNCTION(Server, Reliable)
	void ServerLaunchDirtShaking();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastLaunchDirtShaking(AMoleHole* HoleToShakeDirt);
	
	UFUNCTION()
	ACDEnemy* DetectEnemyForShootProjectile();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPlayerInLobbyBed = false;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/*
	* If actors are in front of the camera set them a transparent material
	*/
	UFUNCTION(BlueprintCallable, Client, Reliable)
	void ManageOccludedActors();

	/*
	* The material set to object pbstructing the view
	*/
	UPROPERTY(EditAnywhere, Category="Camera")
	UMaterialInterface* TransparentMaterial = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UCameraComponent* CameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UInputMappingContext* MappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UInputAction* VacuumInputAspiration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UInputAction* VacuumInputBlow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UInputAction* InteractInput;
	
	virtual void PawnClientRestart() override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void PossessedBy(AController* NewController) override;

	UFUNCTION(BlueprintCallable)
	void VacuumAspiration(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable)
	void VacuumBlow(const FInputActionValue& Value);

	UFUNCTION()
	void VacuumHoldBlow(const FInputActionInstance& Instance);

	UFUNCTION()
	void InterationInput(const FInputActionValue& Value);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	private :
		/*
		* Use to change the material of actor occluding the camera
		*/
		void HideOccludedActor(AActorOccluding* OccludedActor);

		/*
		* Use to change the material of actor not occluding the camera
		*/
		void ShowOccludedActor(AActorOccluding* OccludedActor);

		/*
		* To know what actor was occluded and check if they are still in front of the camera, also contain their ancient material
		*/
		TArray<AActorOccluding*> LastActorsOccluded;
};
