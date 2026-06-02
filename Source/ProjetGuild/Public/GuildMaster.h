#pragma once

#include "CoreMinimal.h"
#include "BuildingPiece.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "InventoryComponent.h"
#include "GuildMaster.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
//  DELEGATE — notifie le HUD quand la vie change
// ─────────────────────────────────────────────────────────────────────────────
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, CurrentHealth, float, MaxHealth);

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

    // Override de TakeDamage — appelé automatiquement par UE5
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
        class AController* EventInstigator, AActor* DamageCauser) override;

    // ── BÂTIMENT ──────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadWrite, Category = "Building")
    ABuildingPiece* CurrentGhost = nullptr;

    UFUNCTION(BlueprintCallable, Category = "Building")
    bool FindNearestSnapPoint(FVector CurrentLocation, FVector& OutSnapLocation, FRotator& OutSnapRotation);

    UFUNCTION(BlueprintCallable, Category = "Building")
    FVector GetGroundPosition(FVector StartLocation, AActor* IgnoredActor = nullptr);

    // ── CAMÉRA FPS ────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class USpringArmComponent* SpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class UCameraComponent* Camera;

    // ── ENHANCED INPUT ────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputMappingContext* MappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* OpenRecruitmentAction;

    // ── INVENTAIRE ────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    UInventoryComponent* InventoryComponent;

    // ── SANTÉ ─────────────────────────────────────────────────────────────
    // Points de vie maximum
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Santé")
    float MaxHealth = 100.f;

    // Points de vie actuels
    UPROPERTY(BlueprintReadOnly, Category = "Santé")
    float CurrentHealth = 100.f;

    // Delegate — notifie le HUD quand la vie change
    UPROPERTY(BlueprintAssignable, Category = "Santé")
    FOnHealthChanged OnHealthChanged;

    // Applique des dégâts directement (appelable depuis Blueprint)
    UFUNCTION(BlueprintCallable, Category = "Santé")
    void ApplyDamage(float Amount);

    // Soigne le joueur
    UFUNCTION(BlueprintCallable, Category = "Santé")
    void Heal(float Amount);

    // Est-ce que le joueur est mort ?
    UFUNCTION(BlueprintPure, Category = "Santé")
    bool IsDead() const { return CurrentHealth <= 0.f; }

    // Event Blueprint — appelé quand le joueur meurt
    UFUNCTION(BlueprintImplementableEvent, Category = "Santé")
    void OnDeath();

private:

    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void OpenRecruitment(const FInputActionValue& Value);

    // Appelé en interne quand la vie tombe à 0
    void Die();
};