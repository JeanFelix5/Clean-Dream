// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CDGameState.generated.h"

/**
 * 
 */
UCLASS()
class CLEANDREAM_API ACDGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ACDGameState();

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintReadWrite)
	int NbPlayerSleeping = 0;

	UFUNCTION(BlueprintCallable)
	void IncreaseSleep();

	UFUNCTION(BlueprintCallable)
	void DecreaseSleep();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool PlayerWon = false;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_ScoreUpdated)
	int CurrentLevelMaximumScore;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_ScoreUpdated)
	int CurrentLevelScore = 0;
	
	UPROPERTY()
	FTimerHandle GameTimerHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Units=seconds), ReplicatedUsing=OnRep_TimerUpdated)
	float RemainingTime = 60.f;

	UFUNCTION()
	void OnRep_ScoreUpdated();

	UFUNCTION()
	void OnRep_TimerUpdated();

	UFUNCTION()
	void UpdateRemainingTime();

	UFUNCTION()
	void TriggerLose();
};
