// Copyright Epic Games, Inc. All Rights Reserved.

#include "FireflyGameMode.h"
#include "FireflyCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFireflyGameMode::AFireflyGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
