// Fill out your copyright notice in the Description page of Project Settings.

#include "TactosyPrivatePCH.h"
#include "TactosyActor.h"
#include "AllowWindowsPlatformTypes.h"
#include "SDK/tactosy.hpp"
#include "SDK/thirdparty/json.hpp"
#include "SDK/thirdparty/easywsclient.hpp"
#include "HapticStructures.h"
#include "HideWindowsPlatformTypes.h"
#include "BhapticsUtilities.h"

tactosy::HapticPlayer *tactosy::HapticPlayer::hapticManager = 0;

TArray<FHapticFeedback> ATactosyActor::ChangedFeedbacks;

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

	BhapticsUtilities::Init();
	FString temp = BhapticsUtilities::GetExecutablePath();

	UE_LOG(LogTemp, Log, TEXT("%s"),*temp);

	if (!BhapticsUtilities::IsPlayerRunning())
	{
		UE_LOG(LogTemp, Log, TEXT("Player is not running"));

		if (BhapticsUtilities::IsPlayerInstalled())
		{
			UE_LOG(LogTemp, Log, TEXT("Player is installed - launching"));
			BhapticsUtilities::LaunchPlayer();
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Player is not Installed"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Player is running"));
	}

	BhapticsUtilities::Free();

	tactosy::HapticPlayer::instance()->dispatchFunctionVar = UpdateDisplayedFeedback;
	tactosy::HapticPlayer::instance()->init();
	
	//*
	InitialiseDots(Tactal);
	InitialiseDots(TactosyLeft);
	InitialiseDots(TactosyRight);
	InitialiseDots(TactotBack);
	InitialiseDots(TactotFront);
	//*/
}

void ATactosyActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	//Super::End
	tactosy::HapticPlayer::instance()->destroy();
}

void ATactosyActor::Destroyed()
{
	tactosy::HapticPlayer::instance()->destroy();
}

// Called every frame
void ATactosyActor::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	for (int i = 0; i < ChangedFeedbacks.Num();i++)
	{
		FHapticFeedback Feedback = ChangedFeedbacks[i];

		switch (Feedback.Position)
		{
		case EPosition::Right:
			VisualiseFeedback(Feedback, TactosyRight);
			break;
		case EPosition::Left:
			VisualiseFeedback(Feedback, TactosyLeft);
			break;
		case EPosition::VestFront:
			VisualiseFeedback(Feedback, TactotFront);
			break;
		case EPosition::VestBack:
			VisualiseFeedback(Feedback, TactotBack);
			break;
		case EPosition::Head:
			VisualiseFeedback(Feedback, Tactal);
			break;
		default:
			printf("Position not found.");
			break;
		}
	}
	ChangedFeedbacks.Empty();
}

void ATactosyActor::SubmitRegistered(const FString &key)
{
    std::string stdKey(TCHAR_TO_UTF8(*key));
	tactosy::HapticPlayer::instance()->submitRegistered(stdKey);
}

void ATactosyActor::SubmitRegisteredIntesityDuration(const FString &Key, float Intensity, float Duration)
{
	std::string StandardKey(TCHAR_TO_UTF8(*Key));
	tactosy::HapticPlayer::instance()->submitRegistered(StandardKey, Intensity, Duration);
}

void ATactosyActor::RegisterFeeback(const FString &key, const FString &path)
{
    std::string stdKey(TCHAR_TO_UTF8(*key));

    FString newPath = path;
    std::string stdPath(TCHAR_TO_UTF8(*newPath));

    if (!FPaths::FileExists(newPath))
    {
        UE_LOG(LogTemp, Log, TEXT("File does not exist : %s"), *newPath);
        return;
    }

	tactosy::HapticPlayer::instance()->registerFeedback(stdKey, stdPath);
}

bool ATactosyActor::LoadFeedbackFiles(TArray<FString>& Files, FString RootFolderFullPath)
{

	if (RootFolderFullPath.Len() < 1)
	{
		return false;
	}

	FPaths::NormalizeDirectoryName(RootFolderFullPath);

	IFileManager& FileManager = IFileManager::Get();

	FString Ext = "*.tactosy";

	FString FinalPath = RootFolderFullPath + "/" + Ext;
	FileManager.FindFiles(Files, *FinalPath, true, false);
	return true;
}

void ATactosyActor::SubmitBytes(const FString &key, EPosition PositionEnum, const TArray<uint8>& motorBytes, int32 durationMillis)
{
	tactosy::Position HapticPosition = tactosy::Position::All;
	std::string stdKey(TCHAR_TO_UTF8(*key));

	switch (PositionEnum)
	{
	case EPosition::Left: 
		HapticPosition = tactosy::Position::Left;
		break;
	case EPosition::Right:
		HapticPosition = tactosy::Position::Right;
		break;
	case EPosition::Head:
		HapticPosition = tactosy::Position::Head;
		break;
	case EPosition::VestFront:
		HapticPosition = tactosy::Position::VestFront;
		break;
	case EPosition::VestBack:
		HapticPosition = tactosy::Position::VestBack;
		break;
	default:
		break;
	}
	
	if (motorBytes.Num() != 20)
	{
		printf("Invalid Point Array\n");
		return;
	}

	std::vector<uint8_t> SubmittedDots(20, 0);

	for (int32 i = 0; i < motorBytes.Num(); i++)
	{
		SubmittedDots[i] = motorBytes[i];
	}

	tactosy::HapticPlayer::instance()->submit(stdKey, HapticPosition, SubmittedDots, durationMillis);
}

void ATactosyActor::SubmitDots(const FString &key, EPosition PositionEnum, const TArray<FDotPoint> DotPoints, int32 durationMillis)
{
	tactosy::Position HapticPosition = tactosy::Position::All;
	std::string stdKey(TCHAR_TO_UTF8(*key));
	switch (PositionEnum)
	{
	case EPosition::Left:
		HapticPosition = tactosy::Position::Left;
		break;
	case EPosition::Right:
		HapticPosition = tactosy::Position::Right;
		break;
	case EPosition::Head:
		HapticPosition = tactosy::Position::Head;
		break;
	case EPosition::VestFront:
		HapticPosition = tactosy::Position::VestFront;
		break;
	case EPosition::VestBack:
		HapticPosition = tactosy::Position::VestBack;
		break;
	default:
		break;
	}

	std::vector<tactosy::DotPoint> SubmittedDots;

	for (int32 i = 0; i < DotPoints.Num(); i++)
	{
		SubmittedDots.push_back(tactosy::DotPoint(DotPoints[i].index,DotPoints[i].intensity));
	}

	tactosy::HapticPlayer::instance()->submit(stdKey, HapticPosition, SubmittedDots, durationMillis);
}

void ATactosyActor::SubmitPath(const FString &key, EPosition PositionEnum, const TArray<FPathPoint>PathPoints, int32 durationMillis)
{
	tactosy::Position HapticPosition = tactosy::Position::All;
	std::vector<tactosy::PathPoint> PathVector;
	std::string stdKey(TCHAR_TO_UTF8(*key));

	for (int32 i = 0; i < PathPoints.Num(); i++)
	{
		tactosy::PathPoint Point(PathPoints[i].x, PathPoints[i].y, PathPoints[i].intensity);
		PathVector.push_back(Point);
	}

	switch (PositionEnum)
	{
	case EPosition::Left:
		HapticPosition = tactosy::Position::Left;
		break;
	case EPosition::Right:
		HapticPosition = tactosy::Position::Right;
		break;
	case EPosition::Head:
		HapticPosition = tactosy::Position::Head;
		break;
	case EPosition::VestFront:
		HapticPosition = tactosy::Position::VestFront;
		break;
	case EPosition::VestBack:
		HapticPosition = tactosy::Position::VestBack;
		break;
	default:
		break;
	}

	
	tactosy::HapticPlayer::instance()->submit(stdKey, HapticPosition, PathVector, durationMillis);
}

bool ATactosyActor::IsAnythingPlaying()
{
	return tactosy::HapticPlayer::instance()->isPlaying();
}

bool ATactosyActor::IsRegisteredPlaying(const FString &key)
{
	std::string stdKey(TCHAR_TO_UTF8(*key));
	return tactosy::HapticPlayer::instance()->isPlaying(stdKey);
}

void ATactosyActor::TurnOffAllFeedback()
{
	tactosy::HapticPlayer::instance()->turnOff();
}

void ATactosyActor::TurnOffRegisteredFeedback(const FString &key)
{
	std::string stdKey(TCHAR_TO_UTF8(*key));
	tactosy::HapticPlayer::instance()->turnOff(stdKey);
}

void ATactosyActor::UpdateDisplayedFeedback(const char *strChr)
{
	nlohmann::json j = nlohmann::json::parse(strChr);
	int pos = atoi((j.at("Position").get<std::string>()).c_str());
	std::string val = j.at("Values").get<std::string>();
	std::stringstream stream(val.substr(1, val.size() - 2));
	std::string item;
	TArray<uint8_t> values;

	while (std::getline(stream, item, ','))
	{
		values.Add(atoi(item.c_str()));
	}

	EPosition Position = (EPosition)pos;
	FHapticFeedback Feedback((EPosition)pos,values,EFeeddbackMode::DOT_MODE);
	ChangedFeedbacks.Add(Feedback);
	
}

void ATactosyActor::InitialiseDots(TArray<USceneComponent*> Item)
{
	for (int i = 0; i < Item.Num(); i++)
	{
		UStaticMeshComponent* DotMesh = Cast<UStaticMeshComponent>(Item[i]);
		if (DotMesh == NULL)
		{
			continue;
		}

		UMaterialInstanceDynamic* DotMaterial = DotMesh->CreateAndSetMaterialInstanceDynamicFromMaterial(0,DotMesh->GetMaterial(0));
		DotMesh->SetRelativeScale3D(FVector(0.2, 0.2, 0.2));
	}
}

void ATactosyActor::VisualiseFeedback(FHapticFeedback Feedback, TArray<USceneComponent*> Item)
{
	for (int i = 0; i < Item.Num(); i++)
	{
		FString ComponentName = Item[i]->GetName();
		ComponentName.RemoveAt(0, 2);
		float DotPosition = FCString::Atof(*ComponentName);
		float Scale = Feedback.Values[DotPosition] / 100.0;
		UStaticMeshComponent* DotMesh = Cast<UStaticMeshComponent>(Item[i]);
		if (DotMesh == NULL)
		{
			continue;
		}
		
		UMaterialInstanceDynamic* DotMaterial = Cast<UMaterialInstanceDynamic>(DotMesh->GetMaterial(0));
		DotMaterial->SetVectorParameterValue("Base Color", FLinearColor(0.8 + Scale*0.2, 0.8 + Scale*0.01, 0.8 - Scale*0.79, 0.2 - 0.2*Scale));
		DotMesh->SetRelativeScale3D(FVector(0.2 + 0.15*(Scale*0.8), 0.2 + 0.15*(Scale*0.8), 0.2 + 0.15*(Scale*0.8)));
	}
}

void ATactosyActor::SetTactoSuit(USceneComponent* SleeveLeft, USceneComponent* SleeveRight, USceneComponent* Head, USceneComponent* VestFront, USceneComponent* VestBack)
{
	SleeveLeft->GetChildrenComponents(false,TactosyLeft);
	SleeveRight->GetChildrenComponents(false, TactosyRight);
	Head->GetChildrenComponents(false, Tactal);
	VestFront->GetChildrenComponents(false, TactotFront);
	VestBack->GetChildrenComponents(false, TactotBack);
}

void ATactosyActor::Reset()
{
	FHapticFeedback BlankFeedback = FHapticFeedback();

	VisualiseFeedback(BlankFeedback, Tactal);
	VisualiseFeedback(BlankFeedback, TactosyLeft);
	VisualiseFeedback(BlankFeedback, TactosyRight);
	VisualiseFeedback(BlankFeedback, TactotFront);
	VisualiseFeedback(BlankFeedback, TactotBack);

	tactosy::HapticPlayer::instance()->destroy();
	tactosy::HapticPlayer::instance()->init();
}