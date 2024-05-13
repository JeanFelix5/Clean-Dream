// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Vacuum/VacuumData.h"
#include "AspiratorComponent.generated.h"


class ABaseCharacter;
class AVacuum;
//This component is placed in the player to spawn the weapon and attach it to the corresponding socket on the mesh
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CLEANDREAM_API UAspiratorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAspiratorComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	//Reference to the vacuum
	UPROPERTY(Replicated, VisibleAnywhere)
	AVacuum* PlayerVacuum;

	//Name of the socket
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AspiratorSocketName = "AspiratorSocket";

public:	

	UFUNCTION()
	void StartFire();

	UFUNCTION()
	void StopFire();

	UFUNCTION()
	void StartFireBlow();

	UFUNCTION()
	void StopFireBlow();

	UFUNCTION(BlueprintCallable, Category="Combat", Server, Reliable)
	void SetupLoadout(const FVacuumDataAsset& Primary);

	UFUNCTION(BlueprintCallable, Category="Combat", NetMulticast, Reliable) //Multicast
	void ClientSetupLoadout(const FVacuumDataAsset& Primary);

	UFUNCTION()
	void ShowFeedbackAfterEndInteractMole(bool bIsAspirating);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

