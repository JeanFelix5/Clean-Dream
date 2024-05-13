// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/AspirableActor.h"
#include "GameFramework/Actor.h"
#include "RockActor.generated.h"

UCLASS()
class CLEANDREAM_API ARockActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARockActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* StaticMeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UAspirableActor* AspirableActor;
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void OnHit(AActor* Shooter);

	UFUNCTION(Server, Reliable, Blueprintable)
	void BlowHit(AActor* Shooter);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attraction properties")
	float AspirationForceMagnitude = 10000.0f; // Adjust the force strength as needed

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attraction properties")
	float DistanceToGrab = 200.0f; // Adjust the minimum distance to grab

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool RockThrownToEnemy = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsRockCurrentlyGrabbedByPlayer = false;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
