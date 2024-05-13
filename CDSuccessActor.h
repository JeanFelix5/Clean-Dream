// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AspirableActor.h"
#include "CDSuccessActor.generated.h"


UCLASS()
class CLEANDREAM_API ACDSuccessActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACDSuccessActor();

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SuccessToWin;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UAspirableActor* AspirableActor;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void OnHit(AActor* Shooter);

	UFUNCTION(Server, Reliable, Blueprintable)
	void BlowHit(AActor* Shooter);

	UFUNCTION(BlueprintImplementableEvent)
	void BlueprintModifier();

	UFUNCTION()
	void ApplyModifier();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attraction properties")
	float AspirationForceMagnitude = 600.0f; // Adjust the force strength as needed

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attraction properties")
	float DistanceToGrab = 200.0f; // Adjust the minimum distance to grab

public:
};
