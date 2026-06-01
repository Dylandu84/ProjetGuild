#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GuildTypes.h"
#include "InventoryComponent.h"
#include "ColonNPC.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
//  TÂCHES DU COLON — état courant de l'IA
// ─────────────────────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EColonTask : uint8
{
    Idle        UMETA(DisplayName = "Inactif"),
    Harvesting  UMETA(DisplayName = "Récolte"),
    Building    UMETA(DisplayName = "Construction"),
    Eating      UMETA(DisplayName = "Mange"),
    Sleeping    UMETA(DisplayName = "Dort"),
    Resting     UMETA(DisplayName = "Se repose"),
    Hauling     UMETA(DisplayName = "Transport"),
};

// ─────────────────────────────────────────────────────────────────────────────
//  COMPÉTENCES — niveau 0-20, détermine l'efficacité des tâches
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FColonSkills
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Logging = 0;       // Bûcheronnage
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Mining = 0;        // Minage
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Construction = 0;  // Construction
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Cooking = 0;       // Cuisine
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Medicine = 0;      // Médecine
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Combat = 0;        // Combat
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Crafting = 0;      // Artisanat
};

// ─────────────────────────────────────────────────────────────────────────────
//  BESOINS — valeurs 0-100, diminuent avec le temps
//  Si un besoin tombe à 0 → OnNeedCritical est appelé
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FColonNeeds
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) float Hunger = 80.f;   // Faim
    UPROPERTY(BlueprintReadWrite) float Energy = 100.f;  // Énergie
    UPROPERTY(BlueprintReadWrite) float Mood = 75.f;   // Moral
    UPROPERTY(BlueprintReadWrite) float Hygiene = 100.f;  // Hygiène
};

// ─────────────────────────────────────────────────────────────────────────────
//  DONNÉES COMPLÈTES DU COLON — générées par ColonGenerator
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FColonData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText FirstName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText LastName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Age = 25;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText BackStory;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<EAdventurerTrait> Traits;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FColonSkills Skills;
    UPROPERTY(BlueprintReadWrite)               FColonNeeds Needs;
    UPROPERTY(BlueprintReadWrite)               float Health = 100.f;
    UPROPERTY(BlueprintReadWrite)               bool bIsInjured = false;

    // Retourne "Prénom Nom"
    FText GetFullName() const
    {
        return FText::FromString(FirstName.ToString() + " " + LastName.ToString());
    }
};

// ─────────────────────────────────────────────────────────────────────────────
//  ACOLONNPC — personnage travailleur contrôlé par l'IA
//  Possédé automatiquement par BP_ColonAIController
//  Contient son propre inventaire pour stocker les ressources récoltées
// ─────────────────────────────────────────────────────────────────────────────
UCLASS()
class PROJETGUILD_API AColonNPC : public ACharacter
{
    GENERATED_BODY()

public:

    AColonNPC();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── DONNÉES DU COLON ──────────────────────────────────────────────────
    // Remplies par ColonGenerator au BeginPlay du Blueprint
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colon")
    FColonData ColonData;

    // Inventaire personnel — reçoit les ressources récoltées
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Colon")
    UInventoryComponent* InventoryComponent;

    // ── ÉTAT IA ───────────────────────────────────────────────────────────
    // Mis à jour par le Behavior Tree via AssignTask()
    UPROPERTY(BlueprintReadWrite, Category = "Colon|AI")
    EColonTask CurrentTask = EColonTask::Idle;

    // ── FONCTIONS APPELABLES ──────────────────────────────────────────────

    // Appelée par le BT pour changer la tâche courante
    UFUNCTION(BlueprintCallable, Category = "Colon")
    void AssignTask(EColonTask NewTask);

    // Texte affiché quand le joueur pointe vers ce colon
    UFUNCTION(BlueprintCallable, Category = "Colon")
    FText GetInteractionText() const;

    // ── EVENTS BLUEPRINT ──────────────────────────────────────────────────
    // Implémentés dans BP_ColonNPC pour les effets visuels / sons

    // Appelé quand une nouvelle tâche démarre
    UFUNCTION(BlueprintImplementableEvent, Category = "Colon")
    void OnTaskStarted(EColonTask Task);

    // Appelé quand un besoin atteint 0 (famine, épuisement...)
    UFUNCTION(BlueprintImplementableEvent, Category = "Colon")
    void OnNeedCritical(const FText& NeedName);

private:

    // Met à jour les besoins toutes les NeedUpdateInterval secondes
    void UpdateNeeds(float DeltaSeconds);

    // Accumulateur de temps pour l'update des besoins
    float NeedUpdateTimer = 0.f;

    // Intervalle entre chaque update des besoins (en secondes)
    const float NeedUpdateInterval = 1.f;
};