#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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

    // ── CAMÉRA FPS ───────────────────────────────────────────────────────
    // La caméra attachée à la tête du personnage
    // SpringArm = bras qui évite que la caméra traverse les murs
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class USpringArmComponent* SpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class UCameraComponent* Camera;

private:

    // ── MOUVEMENT ────────────────────────────────────────────────────────
    // Avancer / reculer / gauche / droite
    void MoveForward(float Value);
    void MoveRight(float Value);

    // Regarder avec la souris
    void LookUp(float Value);
    void LookRight(float Value);
};