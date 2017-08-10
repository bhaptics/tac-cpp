// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Engine.h"
#include "CoreMinimal.h"
#include "HapticStructures.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EPosition : uint8
{
	All = 0, Left = 1, Right = 2,
	Vest = 3,
	Head = 4,
	VestFront = 201, VestBack = 202,
	GloveLeft = 203, GloveRight = 204,
	Custom1 = 251, Custom2 = 252, Custom3 = 253, Custom4 = 254
};

UENUM(BlueprintType)
enum class EFeeddbackMode: uint8
{
	PATH_MODE	UMETA(DisplayName = "PATHMODE"),
	DOT_MODE	UMETA(DisplayName = "DOTMODE")
};

USTRUCT(BlueprintType)
struct FDotPoint
{
	GENERATED_BODY()

	FDotPoint()
	{
		index = 0;
		intensity = 0;
	}

	FDotPoint(int32 _index, int32 _intensity)
	{
		index = _index;
		intensity = _intensity;
	}
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = Vars)
	int32 index;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vars)
	int32 intensity;
};

USTRUCT(BlueprintType)
struct FPathPoint
{
	GENERATED_BODY()
	FPathPoint()
	{
		x = 0;
		y = 0;
		intensity = 0;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vars)
	float x;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vars)
	float y;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vars)
	int32 intensity;

	FPathPoint(float _x, float _y, int32 _intensity)
	{
		x = _x;
		y = _y;
		intensity = _intensity;
	}
};

USTRUCT(BlueprintType)
struct FHapticFeedbackFrame
{
	GENERATED_BODY()
	
	FHapticFeedbackFrame()
	{
		position = EPosition::All;
		pathPoints.Add(FPathPoint());
		dotPoints.Add(FDotPoint());
		texture = 0;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vars)
	TArray<FPathPoint> pathPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vars)
	TArray<FDotPoint> dotPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vars)
	int32 texture;

	UPROPERTY(BlueprintReadWrite, Category = Vars)
	EPosition position;

	FHapticFeedbackFrame(EPosition _pos, const TArray<FPathPoint> &_pathPoints)
	{
		position = _pos;
		pathPoints = _pathPoints;
		texture = 0;
	}

	FHapticFeedbackFrame(EPosition _pos, const TArray<FDotPoint> &_dotPoints)
	{
		position = _pos;
		dotPoints = _dotPoints;
		texture = 0;
	}
};

USTRUCT(BlueprintType)
struct FHapticFeedback
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vars)
	EPosition Position;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vars)
	EFeeddbackMode Mode;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Vars)
	TArray<uint8> Values;

	FHapticFeedback(EPosition _pos, const int _val[], EFeeddbackMode _mod)
	{
		Position = _pos;
		Mode = _mod;
		//values.assign(20, 0);
		Values.AddZeroed(20);
		for (int i = 0; i < 20; i++)
		{
			Values[i] = _val[i];
		}
	}

	FHapticFeedback(EPosition _pos, const TArray<uint8> &_val, EFeeddbackMode _mod)
	{
		Position = _pos;
		Mode = _mod;
		Values.AddZeroed(20);
		for (int i = 0; i < 20; i++)
		{
			Values[i] = _val[i];
		}
	}

	FHapticFeedback() : Position(), Mode()
	{
		Values.AddZeroed(20);
	}
};

class TACTOSY_API HapticStructures
{
public:
	HapticStructures();
	~HapticStructures();
};