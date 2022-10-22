// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPSEscapeRoomGameMode.h"
#include "FPSEscapeRoomCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFPSEscapeRoomGameMode::AFPSEscapeRoomGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
