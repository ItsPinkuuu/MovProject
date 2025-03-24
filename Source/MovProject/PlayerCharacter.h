#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "MovProjectCharacter.h"
#include "GameFramework/Character.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class MOVPROJECT_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()
	
	// First person camera
	UPROPERTY(EditAnywhere)
	UCameraComponent* Camera;

	// MappingContext
	UPROPERTY(EditAnywhere, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* InputMappingContext;
	
	// Jump Input Action
	UPROPERTY(EditAnywhere, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	// Move Input Action
	UPROPERTY(EditAnywhere, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	// Look Input Action
	UPROPERTY(EditAnywhere, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DashAction;

	UPROPERTY(EditAnywhere, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bCanDoubleJump = false;

	UPROPERTY(EditAnywhere, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsGrounded = true;

	UPROPERTY(EditAnywhere, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CharLaunchForce;

	UPROPERTY(EditAnywhere, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float DashForce;
	
public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:

	virtual void Landed(const FHitResult& Hit) override;

	void DoubleJump();

	/** INPUT FUNCTIONS */
	// Called for movement input
	void Move(const FInputActionValue& Value);

	// Called for look input
	void Look(const FInputActionValue& Value);

	// Called for dashing
	void Dash();
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void NotifyControllerChanged();
};
