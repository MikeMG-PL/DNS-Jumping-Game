// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPSEscapeRoomCharacter.h"
#include "FPSEscapeRoomProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/InputSettings.h"


//////////////////////////////////////////////////////////////////////////
// AFPSEscapeRoomCharacter


AFPSEscapeRoomCharacter::AFPSEscapeRoomCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	TurnRateGamepad = 27.5f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));
}

void AFPSEscapeRoomCharacter::SetForwardBreatheModeParameters(bool Forward)
{
	if (Forward)
	{
		GetCharacterMovement()->AirControl = ForwardAirControl;
		GetCharacterMovement()->JumpZVelocity = ForwardJumpHeight;
		GetCharacterMovement()->MaxWalkSpeed = ForwardSpeed;
		GetCharacterMovement()->FallingLateralFriction = ForwardFallingFriction;
	}
	else
	{
		GetCharacterMovement()->AirControl = BreatheAirControl;
		GetCharacterMovement()->JumpZVelocity = BreatheJumpHeight;
		GetCharacterMovement()->MaxWalkSpeed = BreatheSpeed;
		GetCharacterMovement()->FallingLateralFriction = BreatheFallingFriction;
	}

	bForwardMode = Forward;
}

void AFPSEscapeRoomCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	SetForwardBreatheModeParameters(false);
}

// EDITOR ONLY
void AFPSEscapeRoomCharacter::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AFPSEscapeRoomCharacter, bForwardMode))
	{
		/* Because you are inside the class, you should see the value already changed */
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Zmiana!"));

		SetForwardBreatheModeParameters(bForwardMode);
	}
}

//////////////////////////////////////////////////////////////////////////// Input

void AFPSEscapeRoomCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("PrimaryAction", IE_Pressed, this, &AFPSEscapeRoomCharacter::OnPrimaryAction);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);

	// Bind movement events
	PlayerInputComponent->BindAxis("Forward", this, &AFPSEscapeRoomCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Strafe", this, &AFPSEscapeRoomCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "Mouse" versions handle devices that provide an absolute delta, such as a mouse.
	// "Gamepad" versions are for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &AFPSEscapeRoomCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &AFPSEscapeRoomCharacter::LookUpAtRate);
}

void AFPSEscapeRoomCharacter::OnPrimaryAction()
{
	// Trigger the OnItemUsed Event
	OnUseItem.Broadcast();
}

void AFPSEscapeRoomCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnPrimaryAction();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AFPSEscapeRoomCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

void AFPSEscapeRoomCharacter::MoveForward(float Value)
{
	// add movement in that direction
	AddMovementInput(GetActorForwardVector(), Value + bForwardMode * 2);
}

void AFPSEscapeRoomCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AFPSEscapeRoomCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AFPSEscapeRoomCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AFPSEscapeRoomCharacter::Bounce(bool bForwardModeOn)
{
	TimeSinceLastJumped += GetWorld()->GetDeltaSeconds();

	if (GetCharacterMovement()->IsMovingOnGround() && TimeSinceLastJumped >= MinTimeBetweenJumps && bForwardMode)
	{
		Jump();
		TimeSinceLastJumped = 0.f;
	}
}

void AFPSEscapeRoomCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	Bounce(bForwardMode);
	///////////////////////////////////////////////////////////////////////////// FOR DEBUGGING ONLY
	if (GetLocalViewingPlayerController()->WasInputKeyJustPressed(EKeys::Tab))
		SetForwardBreatheModeParameters(!bForwardMode);
}

bool AFPSEscapeRoomCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AFPSEscapeRoomCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AFPSEscapeRoomCharacter::EndTouch);

		return true;
	}

	return false;
}
