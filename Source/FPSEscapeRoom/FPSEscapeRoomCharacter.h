// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FPSEscapeRoomCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UAnimMontage;
class USoundBase;

// Declaration of the delegate that will be called when the Primary Action is triggered
// It is declared as dynamic so it can be accessed also in Blueprints
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUseItem);

UCLASS(config=Game)
class AFPSEscapeRoomCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	float MinTimeBetweenJumps = 0.f;
	float TimeSinceLastJumped = 0.f;

public:
	AFPSEscapeRoomCharacter();

protected:
	virtual void BeginPlay();

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Camera)
	float TurnRateGamepad;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	bool bForwardMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	float ForwardAirControl = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	float ForwardJumpHeight = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	float ForwardSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	float ForwardFallingFriction = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	float BreatheAirControl = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	float BreatheJumpHeight = 450.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	float BreatheSpeed = 450.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	float BreatheFallingFriction = 0.75f;

	UFUNCTION(BlueprintCallable)
	void SetForwardBreatheModeParameters(bool Forward);
	
	/** Delegate to whom anyone can subscribe to receive this event */
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnUseItem OnUseItem;
protected:
	
	void OnPrimaryAction();

	void MoveForward(float Val);

	void MoveRight(float Val);

	void TurnAtRate(float Rate);

	void LookUpAtRate(float Rate);

	void Bounce(bool bForwardModeOn);

	virtual void Tick(float DeltaSeconds) override;
	
	struct TouchData
	{
		TouchData() { bIsPressed = false;Location=FVector::ZeroVector;}
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};
	void BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);
	TouchData	TouchItem;
	
protected:
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	bool EnableTouchscreenMovement(UInputComponent* InputComponent);

public:
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};

