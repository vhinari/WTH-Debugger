// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/WheeledVehicle.h"
#include "FPawn.generated.h"

class UPhysicalMaterial;
class UCameraComponent;
class USpringArmComponent;
class UTextRenderComponent;
class UInputComponent;

UCLASS(config=Game)
class AFPawn : public AWheeledVehicle
{
	GENERATED_UCLASS_BODY()

	/** Spring arm that will offset the camera */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly)
	TSubobjectPtr<USpringArmComponent> SpringArm;

	/** Camera component that will be our viewpoint */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly)
	TSubobjectPtr<UCameraComponent> Camera;

	/** Camera component for the In-Car view */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly)
	TSubobjectPtr<UCameraComponent> InternalCamera;

	/** Text component for the In-Car speed */
	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly)
	TSubobjectPtr<UTextRenderComponent> InCarSpeed;

	/** Text component for the In-Car gear */
	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly)
	TSubobjectPtr<UTextRenderComponent> InCarGear;



	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly)
	TSubobjectPtr<UTextRenderComponent> InCarLapTimerMilsec;

	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly)
	TSubobjectPtr<UTextRenderComponent> InCarLapTimerSeconds;

	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly)
	TSubobjectPtr<UTextRenderComponent> InCarLapTimerMinutes;



	/** Audio component for the engine sound */
	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly)
	TSubobjectPtr<UAudioComponent> EngineSoundComponent;

	/** The current speed as a string eg 10 km/h */
	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly)
	FText SpeedDisplayString;

	/** The current gear as a string (R,N, 1,2 etc) */
	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly)
	FText GearDisplayString;


	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly)
	FText LapTimerMinutesDisplayString;

	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly)
	FText LapTimerSecondsDisplayString;

	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly)
	FText LapTimerMilSecDisplayString;



	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly)
	/** The color of the incar gear text in forward gears */
	FColor	GearDisplayColor;

	/** The color of the incar gear text when in reverse */
	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly)
	FColor	GearDisplayReverseColor;

	/** Are we using incar camera */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly)
	bool bInCarCameraActive;

	/** Are we in reverse gear */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly)
	bool bInReverseGear;

	/** Initial offset of incar camera */
	FVector InternalCameraOrigin;

	// Begin Pawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End Pawn interface

	// Begin Actor interface
	virtual void Tick(float Delta) override;
	virtual void BeginPlay() override;
	// End Actor interface

	/** Handle pressing forwards */
	void MoveForward(float Val);

	/** Setup the strings used on the hud */
	void SetupInCarHUD();

	/** Update the physics material used by the vehicle mesh */
	void UpdatePhysicsMaterial();

	/** Handle pressing right */
	void MoveRight(float Val);
	/** Handle handbrake pressed */
	void OnHandbrakePressed();
	/** Handle handbrake released */
	void OnHandbrakeReleased();
	/** Switch between cameras */
	void OnToggleCamera();
	/** Handle reset VR device */
	void OnResetVR();

	static const FName LookUpBinding;
	static const FName LookRightBinding;
	static const FName EngineAudioRPM;

private:
	/** 
	 * Activate In-Car camera. Enable camera and sets visibility of incar hud display
	 *
	 * @param	bState true will enable in car view and set visibility of various
	 */
	void EnableIncarView( const bool bState );

	/** Update the gear and speed strings */
	void UpdateHUDStrings();

	/* Are we on a 'slippery' surface */
	bool bIsLowFriction;
	/** Slippery Material instance */
	UPhysicalMaterial* SlipperyMaterial;
	/** Non Slippery Material instance */
	UPhysicalMaterial* NonSlipperyMaterial;

};
