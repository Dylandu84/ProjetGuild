#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GuildTypes.h"
#include "ResourceNode.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
//  TYPES DE RESSOURCES RÉCOLTABLES
//  Chaque type a son propre comportement visuel et ses ressources
// ─────────────────────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EResourceNodeType : uint8
{
    Tree        UMETA(DisplayName = "Arbre"),
    Rock        UMETA(DisplayName = "Rocher"),
    Plant       UMETA(DisplayName = "Plante"),
    Mushroom    UMETA(DisplayName = "Champignon"),
    Crate       UMETA(DisplayName = "Caisse"),
};

// ─────────────────────────────────────────────────────────────────────────────
//  DONNÉES D'UN NODE — configurable dans les Blueprints
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FResourceNodeData
{
    GENERATED_BODY()

    // Type du node — détermine le comportement
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Node")
    EResourceNodeType NodeType = EResourceNodeType::Tree;

    // Nombre de coups nécessaires pour récolter
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Node")
    int32 HitsRequired = 3;

    // Ressources données à la récolte
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Node")
    TArray<FResourceCost> ResourcesGiven;

    // Est-ce que le node se régénère ?
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Node")
    bool bRegenerates = true;

    // Nombre de jours avant régénération
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Node")
    int32 RegenerationDays = 3;

    // Distance max d'interaction en cm
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Node")
    float InteractionRange = 250.f;
};

UCLASS()
class PROJETGUILD_API AResourceNode : public AActor
{
    GENERATED_BODY()

public:

    AResourceNode();

    virtual void BeginPlay() override;

    // ── CONFIGURATION ─────────────────────────────────────────────────────
    // Configurable directement dans le Blueprint ou le niveau
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource Node")
    FResourceNodeData NodeData;

    // Le mesh du node — arbre, rocher, etc.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resource Node")
    class UStaticMeshComponent* MeshComponent;

    // ── INTERACTION ───────────────────────────────────────────────────────
    // Appelée quand le joueur interagit — retourne true si récolte terminée
    UFUNCTION(BlueprintCallable, Category = "Resource Node")
    bool Interact();

    // Est-ce que le node est disponible pour interaction ?
    UFUNCTION(BlueprintPure, Category = "Resource Node")
    bool IsAvailable() const { return bIsAvailable; }

    // Texte affiché au joueur quand il s'approche
    UFUNCTION(BlueprintPure, Category = "Resource Node")
    FText GetInteractionText() const;

    // ── EVENTS BLUEPRINT ──────────────────────────────────────────────────
    // Ces events sont implémentés dans le Blueprint — pas en C++
    // Ça permet de gérer les animations et effets visuels dans UE5

    // Appelé à chaque coup — pour les effets visuels (particules, son)
    UFUNCTION(BlueprintImplementableEvent, Category = "Resource Node")
    void OnHit(int32 HitCount , int32 RequiredHits);

    // Appelé quand la récolte est complète — pour l'animation de chute etc.
    UFUNCTION(BlueprintImplementableEvent, Category = "Resource Node")
    void OnHarvested();

    // Appelé quand le node se régénère
    UFUNCTION(BlueprintImplementableEvent, Category = "Resource Node")
    void OnRegenerated();

private:

    // Nombre de coups actuels
    int32 CurrentHits = 0;

    // Est-ce que le node est disponible ?
    bool bIsAvailable = true;

    // Jour de régénération
    int32 RegenerationDay = -1;

    // Donne les ressources au GuildManager
    void GiveResources();

    // Démarre le timer de régénération
    void StartRegeneration();

    // Ajoute cette ligne ici
    UFUNCTION()
    void OnNewDayReceived(int32 DayNumber);
};