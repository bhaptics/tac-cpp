// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine.h"
#include "GameFramework/Actor.h"
#include "TactosyActor.generated.h"

UCLASS()
class TACTOSY_API ATactosyActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATactosyActor();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

    virtual void Destroyed() override;

    UFUNCTION(BlueprintCallable, 
        meta = (DisplayName = "Send tactosy signal by key", 
            Keywords = "Tactosy"), 
                Category = "Tactosy")
    void SendSignal(const FString &key);

    UFUNCTION(BlueprintCallable,
        meta = (DisplayName = "Register tactosy feedback",
            Keywords = "Tactosy"),
        Category = "Tactosy")
    void RegisterFeeback(const FString &key, const FString &path);
};
