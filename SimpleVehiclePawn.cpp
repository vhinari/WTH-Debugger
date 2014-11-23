@@ -0,0 +1,345 @@
// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "SimpleVehicle.h"
#include "SimpleVehiclePawn.h"
#include "SimpleVehicleWheelFront.h"
#include "SimpleVehicleWheelRear.h"
#include "SimpleVehicleHud.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Vehicles/WheeledVehicleMovementComponent4W.h"
#include "Engine/SkeletalMesh.h"

// Needed for VR Headset
#include "Engine.h"
#include "IHeadMountedDisplay.h"

const FName ASimpleVehiclePawn::LookUpBinding("LookUp");
const FName ASimpleVehiclePawn::LookRightBinding("LookRight");
const FName ASimpleVehiclePawn::EngineAudioRPM("RPM");

#define LOCTEXT_NAMESPACE "VehiclePawn"

ASimpleVehiclePawn::ASimpleVehiclePawn(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	// Car mesh
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> CarMesh(TEXT("/Game/Vehicle/Vehicle_SkelMesh.Vehicle_SkelMesh"));
	Mesh->SetSkeletalMesh(CarMesh.Object);
	
	static ConstructorHelpers::FClassFinder<UObject> AnimBPClass(TEXT("/Game/Vehicle/VehicleAnimationBlueprint"));
	Mesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	Mesh->SetAnimInstanceClass(AnimBPClass.Class);

	// Setup friction materials
	static ConstructorHelpers::FObjectFinder<UPhysicalMaterial> SlipperyMat(TEXT("/Game/PhysicsMaterials/Slippery.Slippery"));
	SlipperyMaterial = SlipperyMat.Object;
		
	static ConstructorHelpers::FObjectFinder<UPhysicalMaterial> NonSlipperyMat(TEXT("/Game/PhysicsMaterials/NonSlippery.NonSlippery"));
	NonSlipperyMaterial = NonSlipperyMat.Object;

	UWheeledVehicleMovementComponent4W* Vehicle4W = CastChecked<UWheeledVehicleMovementComponent4W>(VehicleMovement);

	check(Vehicle4W->WheelSetups.Num() == 4);

	// Wheels/Tyres
	// Setup the wheels
	Vehicle4W->WheelSetups[0].WheelClass = USimpleVehicleWheelFront::StaticClass();
	Vehicle4W->WheelSetups[0].BoneName = FName("PhysWheel_FL");
	Vehicle4W->WheelSetups[0].AdditionalOffset = FVector(0.f, -8.f, 0.f);

	Vehicle4W->WheelSetups[1].WheelClass = USimpleVehicleWheelFront::StaticClass();
	Vehicle4W->WheelSetups[1].BoneName = FName("PhysWheel_FR");
	Vehicle4W->WheelSetups[1].AdditionalOffset = FVector(0.f, 8.f, 0.f);

	Vehicle4W->WheelSetups[2].WheelClass = USimpleVehicleWheelRear::StaticClass();
	Vehicle4W->WheelSetups[2].BoneName = FName("PhysWheel_BL");
	Vehicle4W->WheelSetups[2].AdditionalOffset = FVector(0.f, -8.f, 0.f);

	Vehicle4W->WheelSetups[3].WheelClass = USimpleVehicleWheelRear::StaticClass();
	Vehicle4W->WheelSetups[3].BoneName = FName("PhysWheel_BR");
	Vehicle4W->WheelSetups[3].AdditionalOffset = FVector(0.f, 8.f, 0.f);

	// Adjust the tire loading
	Vehicle4W->MinNormalizedTireLoad = 0.0f;
	Vehicle4W->MinNormalizedTireLoadFiltered = 0.2f;
	Vehicle4W->MaxNormalizedTireLoad = 2.0f;
	Vehicle4W->MaxNormalizedTireLoadFiltered = 2.0f;

	// Engine 
	// Torque setup
	Vehicle4W->MaxEngineRPM = 5700.0f;
	Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->Reset();
	Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(0.0f, 400.0f);
	Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(1890.0f, 500.0f);
	Vehicle4W->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(5730.0f, 400.0f);
 
	// Adjust the steering 
	Vehicle4W->SteeringCurve.GetRichCurve()->Reset();
	Vehicle4W->SteeringCurve.GetRichCurve()->AddKey(0.0f, 1.0f);
	Vehicle4W->SteeringCurve.GetRichCurve()->AddKey(40.0f, 0.7f);
	Vehicle4W->SteeringCurve.GetRichCurve()->AddKey(120.0f, 0.6f);
			
 	// Transmission	
	// We want 4wd
	Vehicle4W->DifferentialSetup.DifferentialType = EVehicleDifferential4W::LimitedSlip_4W;
	
	// Drive the front wheels a little more than the rear
	Vehicle4W->DifferentialSetup.FrontRearSplit = 0.65;

	// Automatic gearbox
	Vehicle4W->TransmissionSetup.bUseGearAutoBox = true;
	
	// Physics settings
	// Adjust the center of mass - the buggy is quite low
	Vehicle4W->COMOffset = FVector(8.0f, 0.0f, 0.0f);

	// Set the inertia scale. This controls how the mass of the vehicle is distributed.
	Vehicle4W->InertiaTensorScale = FVector(1.0f, 1.333f, 1.2f);

	// Create a spring arm component for our chase camera
	SpringArm = PCIP.CreateDefaultSubobject<USpringArmComponent>(this, TEXT("SpringArm"));
	SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 34.0f));
	SpringArm->SetWorldRotation(FRotator(-20.0f, 0.0f, 0.0f));
	SpringArm->AttachTo(RootComponent);
	SpringArm->TargetArmLength = 125.0f;
	SpringArm->bEnableCameraLag = false;
	SpringArm->bEnableCameraRotationLag = false;
	SpringArm->bInheritPitch = true;
	SpringArm->bInheritYaw = true;
	SpringArm->bInheritRoll = true;

	// Create the chase camera component 
	Camera = PCIP.CreateDefaultSubobject<UCameraComponent>(this, TEXT("ChaseCamera"));
	Camera->AttachTo(SpringArm, USpringArmComponent::SocketName);
	Camera->SetRelativeRotation(FRotator(10.0f, 0.0f, 0.0f));
	Camera->bUsePawnControlRotation = false;
	Camera->FieldOfView = 90.f;

	// Create In-Car camera component 
	InternalCameraOrigin = FVector(-34.0f, 0.0f, 50.0f);
	InternalCamera = PCIP.CreateDefaultSubobject<UCameraComponent>(this, TEXT("InternalCamera"));
	//InternalCamera->AttachTo(SpringArm, USpringArmComponent::SocketName);
	InternalCamera->bUsePawnControlRotation = false;
	InternalCamera->FieldOfView = 90.f;
	InternalCamera->SetRelativeLocation(InternalCameraOrigin);
	InternalCamera->AttachTo(Mesh);

	// In car HUD
	// Create text render component for in car speed display
	InCarSpeed = PCIP.CreateDefaultSubobject<UTextRenderComponent>(this, TEXT("IncarSpeed"));
	InCarSpeed->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f));
	InCarSpeed->SetRelativeLocation(FVector(35.0f, -6.0f, 20.0f));
	InCarSpeed->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	InCarSpeed->AttachTo(Mesh);

	// Create text render component for in car gear display
	InCarGear = PCIP.CreateDefaultSubobject<UTextRenderComponent>(this, TEXT("IncarGear"));
	InCarGear->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f));
	InCarGear->SetRelativeLocation(FVector(-30.0f, 35.0f, 20.0f));
	//InCarGear->SetRelativeLocation(FVector(35.0f, 5.0f, 20.0f));
	InCarGear->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	InCarGear->AttachTo(Mesh);
	
	// Setup the audio component and allocate it a sound cue
	static ConstructorHelpers::FObjectFinder<USoundCue> SoundCue(TEXT("/Game/Sound/Engine_Loop_Cue.Engine_Loop_Cue"));
	EngineSoundComponent = PCIP.CreateDefaultSubobject<UAudioComponent>(this, TEXT("EngineSound"));
	EngineSoundComponent->SetSound(SoundCue.Object);
	EngineSoundComponent->AttachTo(Mesh);

	// Colors for the in-car gear display. One for normal one for reverse
	GearDisplayReverseColor = FColor(255, 0, 0, 255);
	GearDisplayColor = FColor(255, 255, 255, 255);

	bIsLowFriction = false;
	bInReverseGear = false;
}

void ASimpleVehiclePawn::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// set up gameplay key bindings
	check(InputComponent);

	InputComponent->BindAxis("MoveForward", this, &ASimpleVehiclePawn::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ASimpleVehiclePawn::MoveRight);
	InputComponent->BindAxis(LookUpBinding);
	InputComponent->BindAxis(LookRightBinding);

	InputComponent->BindAction("Handbrake", IE_Pressed, this, &ASimpleVehiclePawn::OnHandbrakePressed);
	InputComponent->BindAction("Handbrake", IE_Released, this, &ASimpleVehiclePawn::OnHandbrakeReleased);
	InputComponent->BindAction("SwitchCamera", IE_Pressed, this, &ASimpleVehiclePawn::OnToggleCamera);

	InputComponent->BindAction("ResetVR", IE_Pressed, this, &ASimpleVehiclePawn::OnResetVR); 
}

void ASimpleVehiclePawn::MoveForward(float Val)
{
	GetVehicleMovementComponent()->SetThrottleInput(Val);

}

void ASimpleVehiclePawn::MoveRight(float Val)
{
	GetVehicleMovementComponent()->SetSteeringInput(Val);
}

void ASimpleVehiclePawn::OnHandbrakePressed()
{
	GetVehicleMovementComponent()->SetHandbrakeInput(true);
}

void ASimpleVehiclePawn::OnHandbrakeReleased()
{
	GetVehicleMovementComponent()->SetHandbrakeInput(false);
}

void ASimpleVehiclePawn::OnToggleCamera()
{
	EnableIncarView(!bInCarCameraActive);
}

void ASimpleVehiclePawn::EnableIncarView(const bool bState)
{
	//if (bState != bInCarCameraActive)
	//{
	//	bInCarCameraActive = bState;
	//	
	///*	if (!bState == true)
	//	{*/
			OnResetVR();
			Camera->Deactivate();
			InternalCamera->Activate();
			
			APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
			if ( (PlayerController != nullptr) && (PlayerController->PlayerCameraManager != nullptr ) )
			{
				PlayerController->PlayerCameraManager->bFollowHmdOrientation = true;
			}
	//	}
		//else
		//{
		//	InternalCamera->Deactivate();
		//	Camera->Activate();
		//}
		
		InCarSpeed->SetVisibility(bInCarCameraActive);
		InCarGear->SetVisibility(bInCarCameraActive);
	//}
}

void ASimpleVehiclePawn::Tick(float Delta)
{
	// Setup the flag to say we are in reverse gear
	bInReverseGear = VehicleMovement->GetCurrentGear() < 0;
	
	// Update phsyics material
	UpdatePhysicsMaterial();

	// Update the strings used in the hud (incar and onscreen)
	UpdateHUDStrings();

	// Set the string in the incar hud
	SetupInCarHUD();

	if ((GEngine->HMDDevice.IsValid() == false) || ((GEngine->HMDDevice.IsValid() == true ) && ( (GEngine->HMDDevice->IsHeadTrackingAllowed() == false) || (GEngine->IsStereoscopic3D() == false))))
	{
		if ( (InputComponent) && (bInCarCameraActive == true ))
		{
			FRotator HeadRotation = InternalCamera->RelativeRotation;
			HeadRotation.Pitch += InputComponent->GetAxisValue(LookUpBinding);
			HeadRotation.Yaw += InputComponent->GetAxisValue(LookRightBinding);
			InternalCamera->RelativeRotation = HeadRotation;
		}
	}	

	// Pass the engine RPM to the sound component
	float RPMToAudioScale = 2500.0f / VehicleMovement->GetEngineMaxRotationSpeed();
	EngineSoundComponent->SetFloatParameter(EngineAudioRPM, VehicleMovement->GetEngineRotationSpeed()*RPMToAudioScale);
}

void ASimpleVehiclePawn::BeginPlay()
{
	// Enable in car view if HMD is attached
	EnableIncarView(GEngine->HMDDevice.IsValid());

	// Start an engine sound playing
	EngineSoundComponent->Play();
}

void ASimpleVehiclePawn::OnResetVR()
{
	if (GEngine->HMDDevice.IsValid())
	{
		GEngine->HMDDevice->ResetOrientationAndPosition();
		InternalCamera->SetRelativeLocation(InternalCameraOrigin);
		GetController()->SetControlRotation(FRotator());
	}
}

void ASimpleVehiclePawn::UpdateHUDStrings()
{
	float KPH = FMath::Abs(VehicleMovement->GetForwardSpeed()) * 0.036f;
	int32 KPH_int = FMath::FloorToInt(KPH);
	float RapTime = GetTickCount();

	// Using FText because this is display text that should be localizable
	SpeedDisplayString = FText::Format(LOCTEXT("SpeedFormat", "{0} km/h"), FText::AsNumber(KPH_int));
	//RapTimeDisplayString = FText::Format(LOCTEXT("RapTime", "{00:00:000} sec"), FText::AsNumber(RapTime));
	
	if (bInReverseGear == true)
	{
		GearDisplayString = FText(LOCTEXT("ReverseGear", "R"));
	}
	else
	{
		int32 Gear = VehicleMovement->GetCurrentGear();
		GearDisplayString = (Gear == 0) ? LOCTEXT("N", "N") : FText::AsNumber(Gear);
	}	
}

void ASimpleVehiclePawn::SetupInCarHUD()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if ((PlayerController != nullptr) && (InCarSpeed.IsValid() == true) && (InCarGear.IsValid()==true) )
	{
		// Setup the text render component strings
		//const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
		//FVector HUD_Pos = FVector(ViewportSize.X / 1280.f, ViewportSize.Y / 720.f, 0);

		//InCarSpeed->SetRelativeLocation(HUD_Pos);
		InCarSpeed->SetText(SpeedDisplayString.ToString());
		InCarGear->SetText(GearDisplayString.ToString());

		
		if (bInReverseGear == false)
		{
			InCarGear->SetTextRenderColor(GearDisplayColor);
		}
		else
		{
			InCarGear->SetTextRenderColor(GearDisplayReverseColor);
		}
	}
}

void ASimpleVehiclePawn::UpdatePhysicsMaterial()
{
	if (GetActorUpVector().Z < 0)
	{
		if (bIsLowFriction == true)
		{
			Mesh->SetPhysMaterialOverride(NonSlipperyMaterial);
			bIsLowFriction = false;
		}
		else
		{
			Mesh->SetPhysMaterialOverride(SlipperyMaterial);
			bIsLowFriction = true;
		}
	}
}

#undef LOCTEXT_NAMESPACE

