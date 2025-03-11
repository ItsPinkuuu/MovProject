// Copyright Epic Games, Inc. All Rights Reserved.

#include "MovProjectGameMode.h"
#include "MovProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMovProjectGameMode::AMovProjectGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
