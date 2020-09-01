// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FireflyCharacter.generated.h"
DECLARE_DELEGATE_OneParam(FCharacterChangedSignature, int32);
class AnimalData {
public:
	AnimalData(float radius, float height, float speed, float jumpVelocity, FVector meshOffset, const FString& mesh, const FString& animBP) {
		CapsuleRadius = radius;
		CapsuleHeight = height;
		MaxMoveSpeed = speed;
		JumpVelocity = jumpVelocity;
		MeshOffset = meshOffset;
		SkeletalMesh = mesh;
		AnimBlueprint = animBP;
	}
	float CapsuleRadius;
	float CapsuleHeight;
	float MaxMoveSpeed;
	float JumpVelocity;
	FVector MeshOffset;
	FString SkeletalMesh;
	FString AnimBlueprint;
};
UENUM()
enum class EAnimal : uint8 {
	Human	UMETA(DisplayName = "Human"),
	Fox		UMETA(DisplayName = "Fox"),
	Rabbit	UMETA(DisplayName = "Rabbit")
};

UCLASS(config=Game)
class AFireflyCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	AFireflyCharacter();

	void PostEditChangeProperty(FPropertyChangedEvent& e);
	
	virtual void BeginPlay() override;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);
	
	void RandomTransform();

	void TransformTo(int32);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, CallInEditor)
	void TransformTo(EAnimal eAnimal);
	void TrnasformTo_Implementation(EAnimal eAnimal);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface
	UFUNCTION(BlueprintCallable)
	FORCEINLINE EAnimal GetAnimal() const { return m_eAnimal; }

private:
	TMap<EAnimal, AnimalData> m_AnimalDataMap;
	TMap<EAnimal, USkeletalMesh*> m_SkeletalMeshMap;
	TMap<EAnimal, UAnimBlueprint*> m_AnimBPMap;
	EAnimal m_eAnimal;

public:
	static FString DefaultAnimBPPath;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAnimal DefaultAnimal;
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

