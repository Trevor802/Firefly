// Copyright Epic Games, Inc. All Rights Reserved.

#include "FireflyCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"

//////////////////////////////////////////////////////////////////////////
// AFireflyCharacter
FString AFireflyCharacter::DefaultAnimBPPath(TEXT("AnimBlueprint'/Game/Mannequin/Animations/ThirdPerson_AnimBP.ThirdPerson_AnimBP'"));
AFireflyCharacter::AFireflyCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
	/*m_AnimalDataMap.Add(EAnimal::Human, FAnimalData(42.f, 96.f, 600.f, 600.f, FVector(0, 0, -94.f),
		TEXT("SkeletalMesh'/Game/EinFantasyWizard/Characters/Meshes/Kid_Wizard_001.Kid_Wizard_001'"),  
		TEXT("AnimBlueprint'/Game/Xiubo/Kid_AnimBP.Kid_AnimBP'")));
	m_AnimalDataMap.Add(EAnimal::Fox, FAnimalData(80.f, 80.f, 800.f, 700.f, FVector(-12.f, 0, -70.f),
		TEXT("SkeletalMesh'/Game/PolyArtFox/Meshes/SK_Mane_Wolf.SK_Mane_Wolf'"),
		TEXT("AnimBlueprint'/Game/Xiubo/Fox_AnimBP.Fox_AnimBP'")));
	m_AnimalDataMap.Add(EAnimal::Rabbit, FAnimalData(29.f, 29.f, 500.f, 800.f, FVector(-12.f, 0, -30.f),
		TEXT("SkeletalMesh'/Game/Rabbit/Meshes/Poly_Art/SK_PA_Rabbit_Common.SK_PA_Rabbit_Common'"),
		TEXT("AnimBlueprint'/Game/Xiubo/Rabit_AnimBP.Rabit_AnimBP'")));
	for (const auto& pair : m_AnimalDataMap) {
		ConstructorHelpers::FObjectFinder<USkeletalMesh> ModelPath(*pair.Value.SkeletalMesh);
		m_SkeletalMeshMap.Add(pair.Key, ModelPath.Object);
		if (pair.Value.AnimBlueprint == TEXT("")) {
			continue;
		}
		ConstructorHelpers::FObjectFinder<UAnimBlueprint> AnimBP(*pair.Value.AnimBlueprint);
		m_AnimBPMap.Add(pair.Key, AnimBP.Object);
	}*/
}
#if WITH_EDITOR
void AFireflyCharacter::PostEditChangeProperty(struct FPropertyChangedEvent& e) {
	Super::PostEditChangeProperty(e);
	FName propertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;
	if (propertyName == GET_MEMBER_NAME_CHECKED(AFireflyCharacter, DefaultAnimal)) {
		UE_LOG(LogTemp, Warning, TEXT("Animal Changed"));
		TransformTo(DefaultAnimal);
	}
}
#endif

void AFireflyCharacter::BeginPlay() {
	Super::BeginPlay();
	TransformTo(DefaultAnimal);
}
//////////////////////////////////////////////////////////////////////////
// Input

void AFireflyCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Transform", IE_Pressed, this, &AFireflyCharacter::RandomTransform);
	PlayerInputComponent->BindAction<FCharacterChangedSignature>("TransformToHuman", IE_Pressed, this, &AFireflyCharacter::TransformTo, 0);
	PlayerInputComponent->BindAction<FCharacterChangedSignature>("TransformToFox", IE_Pressed, this, &AFireflyCharacter::TransformTo, 1);
	PlayerInputComponent->BindAction<FCharacterChangedSignature>("TransformToRabbit", IE_Pressed, this, &AFireflyCharacter::TransformTo, 2);

	PlayerInputComponent->BindAxis("MoveForward", this, &AFireflyCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFireflyCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AFireflyCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AFireflyCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AFireflyCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AFireflyCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AFireflyCharacter::OnResetVR);
}

void AFireflyCharacter::RandomTransform() {
	TransformTo((EAnimal)FMath::RandRange(0, 2));
}

void AFireflyCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AFireflyCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AFireflyCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AFireflyCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AFireflyCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AFireflyCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetCameraRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		//UGameplayStatics::GetPlayerCameraManager(GetWorld(), -1)->GetCameraRotation();
		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AFireflyCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetCameraRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AFireflyCharacter::TransformTo(int32 animalID) {
	TransformTo((EAnimal)animalID);
}

void AFireflyCharacter::TransformTo_Implementation(EAnimal eAnimal) {
	m_eAnimal = eAnimal;
	GetCapsuleComponent()->SetCapsuleSize(AnimalDataArray[(int)eAnimal].CapsuleRadius, AnimalDataArray[(int)eAnimal].CapsuleHeight);
	GetCharacterMovement()->MaxWalkSpeed = AnimalDataArray[(int)eAnimal].MaxMoveSpeed;
	GetCharacterMovement()->JumpZVelocity = AnimalDataArray[(int)eAnimal].JumpVelocity;
	GetCharacterMovement()->AirControl = AnimalDataArray[(int)eAnimal].AirControl;
	GetMesh()->SetSkeletalMesh(AnimalDataArray[(int)eAnimal].SM);
	GetMesh()->SetRelativeLocation(AnimalDataArray[(int)eAnimal].MeshOffset);
	/*if (!m_AnimBPMap.Contains(eAnimal)) {
		GetMesh()->SetAnimInstanceClass(nullptr);
		return;
	}*/
	GetMesh()->SetAnimInstanceClass(AnimalDataArray[(int)eAnimal].AnimBP);
}
