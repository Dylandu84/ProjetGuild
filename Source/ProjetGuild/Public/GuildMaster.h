#pragma once

#include "CoreMinimal.h"
#include "BuildingPiece.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "InventoryComponent.h"
#include "GuildMaster.generated.h"


UCLASS()
class PROJETGUILD_API AGuildMaster : public ACharacter
{
    GENERATED_BODY()

public:

    AGuildMaster();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(
        class UInputComponent* PlayerInputComponent) override;
   
    UPROPERTY(BlueprintReadWrite, Category = "Building")
    ABuildingPiece* CurrentGhost = nullptr;
    // ── CAMÉRA FPS ───────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class USpringArmComponent* SpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class UCameraComponent* Camera;

    // ── ENHANCED INPUT ────────────────────────────────────────────────────
    // Le contexte de mapping — assigné dans le Blueprint
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputMappingContext* MappingContext;

    // Les actions d'input — assignées dans le Blueprint
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* OpenRecruitmentAction;

    // Exposée au Blueprint pour l'appeler dans Event Tick
    UFUNCTION(BlueprintCallable, Category = "Building")
    bool FindNearestSnapPoint(FVector CurrentLocation, FVector& OutSnapLocation, FRotator& OutSnapRotation);
   
    UFUNCTION(BlueprintCallable, Category = "Building")
    FVector GetGroundPosition(FVector StartLocation, AActor* IgnoredActor = nullptr);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
    UInventoryComponent* InventoryComponent;

  
private:

   
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void OpenRecruitment(const FInputActionValue& Value);
};