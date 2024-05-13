// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AspirableActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitSignature,AActor*,Instigator);

//This actor component is placed on actors that can interact with the aspirator
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CLEANDREAM_API UAspirableActor : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAspirableActor();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	

	//Hit delegate
	UPROPERTY(BlueprintAssignable)
	FOnHitSignature OnHitDelegate;
};
