// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Vacuum/VacuumData.h"
#include "Components/SceneComponent.h"
#include "Enemies/CDEnemy.h"
#include "Vacuum.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStopShootingDelegateSignature);

UCLASS()
class CLEANDREAM_API AVacuum : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVacuum();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
	USkeletalMeshComponent* SkeletalMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UAudioComponent* AudioComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UAudioComponent* AudioComponentBlow;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UAudioComponent* AudioComponentBlocked;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UNiagaraComponent* NiagaraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UNiagaraComponent* NiagaraComponentBlow;

	//Timer for the aspiration
	UPROPERTY()
	FTimerHandle FireTimerHandle;

	//Timer for the blow
	UPROPERTY()
	FTimerHandle FireBlowTimerHandle;

	UPROPERTY()
	bool bPlayedBlockedSoundAlready = false;

	UFUNCTION()
	void PlayVacuumBlockedFeedback();

public:
	UPROPERTY(BlueprintAssignable)
	FOnStopShootingDelegateSignature OnStopShootingDelegate;
	
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category="Aspirator")
	FVacuumDataAsset VacuumData;

	UFUNCTION(BlueprintCallable)
	void PlayFeedback(bool bIsAspirating);
	
	UFUNCTION(Server, Reliable)
	void Server_PlayVFX(bool bIsAspirating);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayVFX(bool bIsAspirating);

	UFUNCTION()
	void Shoot();

	UFUNCTION()
	void ShootBlow();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerShoot(const TArray<FHitResult>& Results, const AActor* InstigatorActor, const FTransform Transform);

	//UFUNCTION(NetMulticast, Reliable)
	//void MulticastShoot(const TArray<FHitResult>& Results, const AActor* InstigatorActor);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerShootBlow(const TArray<FHitResult>& Results, const AActor* InstigatorActor, const FTransform Transform);

	//UFUNCTION(NetMulticast, Reliable)
	//void MulticastShootBlow(const TArray<FHitResult>& Results, const AActor* InstigatorActor);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastEnemyDeathAnimation(ACDEnemy* EnemyToDestroy);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StopShooting();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopShooting();
	
	UFUNCTION()
	void StopShooting();

	UFUNCTION(BlueprintCallable)
	void StopShootingBlow();

	UFUNCTION(Server, Reliable)
	void Server_StopFeedbackAspiration();

	UFUNCTION(Server, Reliable)
	void Server_StopFeedbackBlow();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopFeedbackAspiration();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopFeedbackBlow();
	
	UFUNCTION()
	bool VacuumAspirationShoot();

	UFUNCTION()
	bool VacuumBlowShoot();

	UFUNCTION()
	void SetAspiratorData(const FVacuumDataAsset& Data);

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	void StopShootingBeforeServerTravel();
};
