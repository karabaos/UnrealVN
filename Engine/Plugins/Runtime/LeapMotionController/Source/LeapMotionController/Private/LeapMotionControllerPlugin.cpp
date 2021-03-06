// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "LeapMotionControllerPrivatePCH.h"
#include "LeapMotionControllerPlugin.h"

#include "Leap_NoPI.h"

#include "LeapMotionDevice.h"


IMPLEMENT_MODULE(FLeapMotionControllerPlugin, LeapMotionController)
DEFINE_LOG_CATEGORY(LogLeapMotion)

static void* LeapDllHandle = 0;

void FLeapMotionControllerPlugin::StartupModule()
{
	FString LeapDllRoot = FPaths::EngineDir() / TEXT("Binaries/ThirdParty/Leap/") / (PLATFORM_64BITS ? TEXT("Win64/") : TEXT("Win32/"));
	FPlatformProcess::PushDllDirectory(*LeapDllRoot);
	LeapDllHandle = FPlatformProcess::GetDllHandle(*(LeapDllRoot + "Leap.dll"));
	FPlatformProcess::PopDllDirectory(*LeapDllRoot);

	// Continue with initialization if dll loads successfully
	if (LeapDllHandle)
	{
		// Attempt to create the device, and start it up.  Caches a pointer to the device if it successfully initializes
		TSharedPtr<FLeapMotionDevice> LeapStartup(new FLeapMotionDevice);
		if (LeapStartup->StartupDevice())
		{
			LeapDevice = LeapStartup;
		}
	}
	else
	{
		UE_LOG(LogLeapMotion, Error, TEXT("Could not load Leap.dll. Leap Motion Controller plugin is inactive."));
	}
}

void FLeapMotionControllerPlugin::ShutdownModule()
{
    if (LeapDevice.IsValid())
    {
        LeapDevice->ShutdownDevice();
        LeapDevice = nullptr;
    }
}




