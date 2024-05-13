// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "Engine/DataAsset.h"
#include "VacuumData.generated.h"

USTRUCT(BlueprintType)
struct FVacuumDataAsset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName Name;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector ShootingBoxTraceSize = FVector(110.0f, 110.0f, 240.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=1))
	float ShootingRate = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ShootingRange = 450.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ShootingBlowStrength = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundWave* SoundAspiration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundWave* SoundAspirationStop;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundWave* SoundVacuumAspirationBlocked;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundWave* SoundBlow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USoundWave* SoundBlowStop;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UNiagaraSystem* AspirationFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UNiagaraSystem* BlowFX;


	//the vacuum mesh is set in the player blueprint in the vacuum mesh component
	//UPROPERTY(EditAnywhere, BlueprintReadOnly)
	//USkeletalMesh* Mesh;
};
/**
 * 
 */
UCLASS()
class CLEANDREAM_API UVacuumData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, meta=(TitleProperty="Name"))
	TArray<FVacuumDataAsset> VacuumData;
};
