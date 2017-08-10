// Fill out your copyright notice in the Description page of Project Settings.
#include "TactosyPrivatePCH.h"
#include "Tactosy.h"
#include "BhapticsUtilities.h"

using namespace std;

typedef char*(*_GetExePath)();
typedef bool(*_IsPlayerInstalled)();
typedef bool(*_IsPlayerRunning)();
typedef void(*_LaunchPlayer)();

_IsPlayerInstalled m_IsPlayerInstalled;
_IsPlayerRunning m_IsPlayerRunning;
_LaunchPlayer m_LaunchPlayer;
_GetExePath m_GetExePath;

void *v_dllHandle;

BhapticsUtilities::BhapticsUtilities()
{

}

void BhapticsUtilities::Init()
{
	//IsInitialised = false;
	//ExecutableFilePath = "";

	FString FilePath = *FPaths::GamePluginsDir();
	FilePath.Append("Tactosy/bHapticUtility.dll");

	if (FPaths::FileExists(FilePath))
	{
		v_dllHandle = FPlatformProcess::GetDllHandle(*FilePath);

		if (v_dllHandle != NULL)
		{
			return;
		}
	}
	UE_LOG(LogTemp, Log, TEXT("Could not find dll in %s"),*FilePath);

}

void BhapticsUtilities::Free()
{
	if (v_dllHandle != NULL)
	{
		m_IsPlayerInstalled = NULL;
		m_IsPlayerRunning = NULL;
		m_LaunchPlayer = NULL;
		m_GetExePath = NULL;

		FPlatformProcess::FreeDllHandle(v_dllHandle);
		v_dllHandle = NULL;
	}
}

BhapticsUtilities::~BhapticsUtilities()
{

}

FString BhapticsUtilities::GetExecutablePath()
{
	if (v_dllHandle != NULL)
	{
		m_GetExePath = NULL;

		FString ProcName = "getExePath";
		m_GetExePath = (_GetExePath)FPlatformProcess::GetDllExport(v_dllHandle, *ProcName);
		if (m_GetExePath != NULL)
		{
			char* returnVal = m_GetExePath();
			FString ReturnString = FString(returnVal);
			return ReturnString;
		}
	}
	UE_LOG(LogTemp, Log, TEXT("Executable Path Function Failed"));
	return"";
}

bool BhapticsUtilities::IsPlayerInstalled()
{
	if (v_dllHandle != NULL)
	{
		m_IsPlayerInstalled = NULL;

		FString ProcName = "isPlayerInstalled";
		m_IsPlayerInstalled = (_IsPlayerInstalled)FPlatformProcess::GetDllExport(v_dllHandle, *ProcName);
		if (m_IsPlayerInstalled != NULL)
		{
			return m_IsPlayerInstalled();
		}
	}
	UE_LOG(LogTemp, Log, TEXT("Install Check Failed"));
	return false;
}

bool BhapticsUtilities::IsPlayerRunning()
{
	if (v_dllHandle != NULL)
	{
		m_IsPlayerRunning = NULL;
		FString ProcName = "isPlayerRunning";
		m_IsPlayerRunning = (_IsPlayerRunning)FPlatformProcess::GetDllExport(v_dllHandle, *ProcName);
		if (m_IsPlayerRunning != NULL)
		{
			return m_IsPlayerRunning();
		}
	}
	UE_LOG(LogTemp, Log, TEXT("Running Check Failed"));
	return false;
}

void BhapticsUtilities::LaunchPlayer()
{

	if (v_dllHandle != NULL)
	{
		m_LaunchPlayer = NULL;

		FString ProcName = "launchPlayer";
		m_LaunchPlayer = (_LaunchPlayer)FPlatformProcess::GetDllExport(v_dllHandle, *ProcName);
		if (m_LaunchPlayer != NULL)
		{
			m_LaunchPlayer();
			return;
		}
		UE_LOG(LogTemp, Log, TEXT("Function Failed"));
	}
	UE_LOG(LogTemp, Log, TEXT("Launching Function Failed"));

}
