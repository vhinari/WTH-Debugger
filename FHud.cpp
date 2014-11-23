// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "F.h"
#include "FHUD.h"
#include "FPawn.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "CanvasItem.h"
// Needed for VR Headset
#include "Engine.h"
#include "IHeadMountedDisplay.h"

#define LOCTEXT_NAMESPACE "VehicleHUD"

AFHUD::AFHUD(const class FPostConstructInitializeProperties& PCIP) 
	: Super(PCIP)
{
	static ConstructorHelpers::FObjectFinder<UFont> Font(TEXT("/Engine/EngineFonts/RobotoDistanceField"));
	HUDFont = Font.Object;
}

void AFHUD::DrawHUD()
{
	Super::DrawHUD();

	// Calculate ratio from 720p
	const float HUDXRatio = Canvas->SizeX / 1280.f;
	const float HUDYRatio = Canvas->SizeY / 720.f;

	// We dont want the onscreen hud when using a HMD device	
	if ((GEngine->HMDDevice.IsValid() == false ) || ((GEngine->HMDDevice.IsValid() == true) && (GEngine->HMDDevice->IsStereoEnabled() == false)))
	{
		// Get our vehicle so we can check if we are in car. If we are we don't want onscreen HUD
		AFPawn* Vehicle = Cast<AFPawn>(GetOwningPawn());
		if ((Vehicle != nullptr) && (Vehicle->bInCarCameraActive == false))
		{
			FVector2D ScaleVec(HUDYRatio * 1.4f, HUDYRatio * 1.4f);

			FCanvasTextItem LapTimerMilSecTextItem(FVector2D(HUDXRatio * 110.f, HUDYRatio * 5), Vehicle->LapTimerMilSecDisplayString, HUDFont, FLinearColor::White);
			LapTimerMilSecTextItem.Scale = ScaleVec;
			Canvas->DrawItem(LapTimerMilSecTextItem);

			FCanvasTextItem LapTimerSecondsTextItem(FVector2D(HUDXRatio * 50.f, HUDYRatio * 5), Vehicle->LapTimerSecondsDisplayString, HUDFont, FLinearColor::Blue);
			LapTimerSecondsTextItem.Scale = ScaleVec;
			Canvas->DrawItem(LapTimerSecondsTextItem);

			FCanvasTextItem LapTimerMinutesTextItem(FVector2D(HUDXRatio * 10.f, HUDYRatio * 5), Vehicle->LapTimerMinutesDisplayString, HUDFont, FLinearColor::Red);
			LapTimerMinutesTextItem.Scale = ScaleVec;
			Canvas->DrawItem(LapTimerMinutesTextItem);

			// Speed
			FCanvasTextItem SpeedTextItem(FVector2D(HUDXRatio * 1105.f, HUDYRatio * 555), Vehicle->SpeedDisplayString, HUDFont, FLinearColor::White);
			SpeedTextItem.Scale = ScaleVec;
			Canvas->DrawItem(SpeedTextItem);

			// Gear
			FCanvasTextItem GearTextItem(FVector2D(HUDXRatio * 1105.f, HUDYRatio * 600.f), Vehicle->GearDisplayString, HUDFont, Vehicle->bInReverseGear == false ? Vehicle->GearDisplayColor : Vehicle->GearDisplayReverseColor);
			GearTextItem.Scale = ScaleVec;
			Canvas->DrawItem(GearTextItem);
		}
	}
}

#undef LOCTEXT_NAMESPACE
