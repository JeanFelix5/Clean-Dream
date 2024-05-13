// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStates/CDGameState.h"

#include <algorithm>

#include "Controllers/BasePlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PlayerStates/CDGamePlayerState.h"
#include "Vacuum/Vacuum.h"

ACDGameState::ACDGameState()
{
}

void ACDGameState::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(
		GameTimerHandle,
		this,
		&ACDGameState::UpdateRemainingTime,
		1.f,
		true,
		0.0f
		);
	}
}

void ACDGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACDGameState, CurrentLevelScore);
	DOREPLIFETIME(ACDGameState, CurrentLevelMaximumScore);
	DOREPLIFETIME(ACDGameState, RemainingTime);
}

void ACDGameState::IncreaseSleep()
{
	NbPlayerSleeping += 1;
}

void ACDGameState::DecreaseSleep()
{
	NbPlayerSleeping -= 1;
}

void ACDGameState::OnRep_ScoreUpdated()
{
	ABasePlayerController* PlayerController = Cast<ABasePlayerController>(GetWorld()->GetFirstPlayerController());

	if(PlayerController)
	{
		PlayerController->OnScoreUpdated.Broadcast();
	}
}

void ACDGameState::OnRep_TimerUpdated()
{
	ABasePlayerController* PlayerController = Cast<ABasePlayerController>(GetWorld()->GetFirstPlayerController());

	if(PlayerController)
	{
		PlayerController->OnTimerUpdated.Broadcast();
	}
}

void ACDGameState::UpdateRemainingTime()
{
	RemainingTime--;

	RemainingTime = std::clamp(RemainingTime,0.0f,1000.0f);

	ABasePlayerController* PlayerController = Cast<ABasePlayerController>(GetWorld()->GetFirstPlayerController());

	if(PlayerController)
	{
		PlayerController->OnTimerUpdated.Broadcast();
	}
	
	if (RemainingTime <= 0)
	{
		TArray<AActor*> VacuumArray;
		//Stop shooting for all players
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVacuum::StaticClass(), VacuumArray);

		for(auto& vacuumActor  : VacuumArray)
		{
			if(AVacuum* vacuum = Cast<AVacuum>(vacuumActor))
			{
				vacuum->StopShootingBeforeServerTravel();
			}
		}
		
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
		{
			TriggerLose();
		},  0.15f, false);
	}
}

void ACDGameState::TriggerLose()
{
	GEngine->AddOnScreenDebugMessage(71,
			5.f,
			FColor::Cyan,
			TEXT("TriggerLose"));

	if(HasAuthority())
	{
		// Things to do when time's up
		for(APlayerState* PlayerState : PlayerArray)
		{
			ACDGamePlayerState* GamePlayerState = Cast<ACDGamePlayerState>(PlayerState);
		
			if(GamePlayerState)
			{
				GamePlayerState->bHasWon = false;
			}
		}

		if(UWorld* World = GetWorld())
		{
			GetWorld()->ServerTravel("LVL_End?listen", false);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("World is not valid on Trigger Lose !"));
		}
	}
}
