// Fill out your copyright notice in the Description page of Project Settings.

#include "TactosyPrivatePCH.h"
#include "TactosyActor.h"
#include "AllowWindowsPlatformTypes.h"
#include "SDK/tactosy.hpp"
#include "HideWindowsPlatformTypes.h"

tactosy::HapticPlayer hapticPlayer;

// Sets default values
ATactosyActor::ATactosyActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ATactosyActor::BeginPlay()
{
	Super::BeginPlay();
    hapticPlayer.init();
}

void ATactosyActor::Destroyed()
{
    hapticPlayer.destroy();
}

// Called every frame
void ATactosyActor::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
}

void ATactosyActor::SubmitRegistered(const FString &key)
{
    std::string stdKey(TCHAR_TO_UTF8(*key));
    hapticPlayer.submitRegistered(stdKey);
}

void ATactosyActor::RegisterFeeback(const FString &key, const FString &path)
{
    std::string stdKey(TCHAR_TO_UTF8(*key));

    FString newPath = FPaths::Combine(FPaths::GameContentDir(), path);
    std::string stdPath(TCHAR_TO_UTF8(*newPath));

    if (!FPaths::FileExists(newPath))
    {
        UE_LOG(LogTemp, Log, TEXT("File does not exist : %s"), *newPath);
        return;
    }

    hapticPlayer.registerFeedback(stdKey, stdPath);
}

