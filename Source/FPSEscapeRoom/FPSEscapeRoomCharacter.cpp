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
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	TurnRateGamepad = 27.5f;

	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

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
	Super::BeginPlay();
	SetForwardBreatheModeParameters(false);
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
	AddMovementInput(GetActorForwardVector(), Value + bForwardMode * 2);
}

void AFPSEscapeRoomCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AFPSEscapeRoomCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void AFPSEscapeRoomCharacter::LookUpAtRate(float Rate)
{
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
