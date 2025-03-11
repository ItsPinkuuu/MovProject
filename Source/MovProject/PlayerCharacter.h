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
	
public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:

	/** INPUT FUNCTIONS */
	// Called for movement input
	void Move(const FInputActionValue& Value);

	// Called for look input
	void Look(const FInputActionValue& Value);
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void NotifyControllerChanged();
};
