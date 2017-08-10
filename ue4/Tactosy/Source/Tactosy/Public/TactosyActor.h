// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine.h"
#include "HapticStructures.h"
#include "GameFramework/Actor.h"
#include "TactosyActor.generated.h"


UCLASS()
class TACTOSY_API ATactosyActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATactosyActor();

	static TArray<FHapticFeedback> ChangedFeedbacks;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

    virtual void Destroyed() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Haptics")
		TArray<USceneComponent*> TactotFront;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Haptics")
		TArray<USceneComponent*> TactotBack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Haptics")
		TArray<USceneComponent*> TactosyLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Haptics")
		TArray<USceneComponent*> TactosyRight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Haptics")
		TArray<USceneComponent*> Tactal;

	UFUNCTION(BlueprintCallable,
		Category = "Tactosy")
	bool LoadFeedbackFiles(TArray<FString>& Files, FString RootFolderFullPath);

    UFUNCTION(BlueprintCallable, 
        meta = (DisplayName = "Submit Registered feedback with key", 
            Keywords = "Tactosy"), 
                Category = "Tactosy")
    void SubmitRegistered(const FString &key);

	 UFUNCTION(BlueprintCallable,
		meta = (DisplayName = "Submit Registered feedback with Intensity and Duration",
            Keywords = "Tactosy"), 
                Category = "Tactosy")
    void SubmitRegisteredIntesityDuration(const FString &Key, float Intensity, float Duration);

    UFUNCTION(BlueprintCallable,
        meta = (DisplayName = "Register tactosy feedback",
            Keywords = "Tactosy"),
        Category = "Tactosy")
    void RegisterFeeback(const FString &key, const FString &path);

	 UFUNCTION(BlueprintCallable,
		meta = (DisplayName = "Submit feedback using Bytes",
            Keywords = "Tactosy"), 
                Category = "Tactosy")
    void SubmitBytes(const FString &key, EPosition PositionEnum,const TArray<uint8>& motorBytes, int32 durationMillis);

	 UFUNCTION(BlueprintCallable,
		 meta = (DisplayName = "Submit feedback using Dots",
			 Keywords = "Tactosy"),
		 Category = "Tactosy")
	void SubmitDots(const FString &key, EPosition PositionEnum, const TArray<FDotPoint> DotPoints, int32 durationMillis);

	 UFUNCTION(BlueprintCallable,
		meta = (DisplayName = "Submit feedback using Paths",
            Keywords = "Tactosy"), 
                Category = "Tactosy")
    void SubmitPath(const FString &key, EPosition PositionEnum,const TArray<FPathPoint>PathPoints, int32 durationMillis);

	 UFUNCTION(BlueprintPure,
		 meta = (DisplayName = "Is any feedback currently playing",
			 Keywords = "Tactosy"),
		 Category = "Tactosy")
	 bool IsAnythingPlaying();

	 UFUNCTION(BlueprintPure,
		 meta = (DisplayName = "Is the specified feedback currently playing",
			 Keywords = "Tactosy"),
		 Category = "Tactosy")
	 bool IsRegisteredPlaying(const FString &key);

	 UFUNCTION(BlueprintCallable,
		 meta = (DisplayName = "Turn off all feedback",
			 Keywords = "Tactosy"),
		 Category = "Tactosy")
	 void TurnOffAllFeedback();

	 UFUNCTION(BlueprintCallable,
		 meta = (DisplayName = "Turn off specified feedback",
			 Keywords = "Tactosy"),
		 Category = "Tactosy")
	 void TurnOffRegisteredFeedback(const FString &key);

	 static void UpdateDisplayedFeedback(const char *strChr);

	 void InitialiseDots(TArray<USceneComponent*> Item);

	 void VisualiseFeedback(FHapticFeedback Feedback, TArray<USceneComponent*> Item);

	 UFUNCTION(BlueprintCallable,
		 meta = (DisplayName = "Set the TactSuit Variables",
			 Keywords = "Tactosy"),
		 Category = "Tactosy")
		 void SetTactoSuit(USceneComponent* SleeveLeft, USceneComponent* SleeveRight, USceneComponent* Head, USceneComponent* VestFront, USceneComponent* VestBack);

	 UFUNCTION(BlueprintCallable,
		 meta = (DisplayName = "Reset Player Connection",
			 Keywords = "Tactosy"),
		 Category = "Tactosy")
		 void Reset();
};
