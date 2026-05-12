#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GuildTypes.h"
#include "BuildingPiece.generated.h"

// Types de pièces constructibles
UENUM(BlueprintType)
enum class EBuildingPieceType : uint8
{
    Foundation  UMETA(DisplayName = "Fondation"),
    Wall        UMETA(DisplayName = "Mur"),
    WallWindow  UMETA(DisplayName = "Mur avec fenêtre"),
    WallDoor    UMETA(DisplayName = "Mur avec porte"),
    Roof        UMETA(DisplayName = "Toit"),
    RoofCorner  UMETA(DisplayName = "Toit coin"),
    Stairs      UMETA(DisplayName = "Escalier"),
    Floor       UMETA(DisplayName = "Sol"),
};

// Matériaux de construction
UENUM(BlueprintType)
enum class EBuildingMaterial : uint8
{
    Wood    UMETA(DisplayName = "Bois"),
    Stone   UMETA(DisplayName = "Pierre"),
    Mixed   UMETA(DisplayName = "Mixte"),
};

// Données d'une pièce constructible
USTRUCT(BlueprintType)
struct FBuildingPieceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Piece")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Piece")
    EBuildingPieceType PieceType = EBuildingPieceType::Foundation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Piece")
    EBuildingMaterial Material = EBuildingMaterial::Wood;

    // Coût en ressources pour construire cette pièce
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Piece")
    TArray<FResourceCost> BuildCost;

    // Points de vie de la pièce
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Piece")
    int32 MaxHealth = 100;
};

UCLASS()
class PROJETGUILD_API ABuildingPiece : public AActor
{
    GENERATED_BODY()

public:

    ABuildingPiece();
    virtual void BeginPlay() override;

    // ── DONNÉES ───────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building Piece")
    FBuildingPieceData PieceData;

    // Le mesh de la pièce
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Building Piece")
    class UStaticMeshComponent* MeshComponent;

    // ── ÉTAT ──────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Building Piece")
    bool bIsGhost = false;          // Mode preview — pas encore posé

    UPROPERTY(BlueprintReadOnly, Category = "Building Piece")
    bool bCanPlace = false;         // Est-ce qu'on peut poser ici ?

    UPROPERTY(BlueprintReadOnly, Category = "Building Piece")
    int32 CurrentHealth = 100;

    // ── GHOST MODE ────────────────────────────────────────────────────────
    // Active le mode ghost — translucide, pas de collision
    UFUNCTION(BlueprintCallable, Category = "Building Piece")
    void SetGhostMode(bool bGhost);

    // Met à jour la couleur du ghost selon si on peut poser ou non
    UFUNCTION(BlueprintCallable, Category = "Building Piece")
    void UpdateGhostColor(bool bCanPlaceHere);

    // ── PLACEMENT ─────────────────────────────────────────────────────────
    // Pose la pièce définitivement — consomme les ressources
    UFUNCTION(BlueprintCallable, Category = "Building Piece")
    bool Place();

    // Vérifie si on a assez de ressources pour poser
    UFUNCTION(BlueprintPure, Category = "Building Piece")
    bool CanAffordToBuild() const;

    // ── SOCKETS DE SNAP ───────────────────────────────────────────────────
    // Les points où d'autres pièces peuvent se connecter
    // Définis dans le Blueprint via des SceneComponents nommés "SnapPoint_X"
    UFUNCTION(BlueprintCallable, Category = "Building Piece")
    TArray<USceneComponent*> GetSnapPoints() const;

    // ── MATERIALS ─────────────────────────────────────────────────────────
    // Materials assignés dans le Blueprint
    UPROPERTY(EditDefaultsOnly, Category = "Materials")
    class UMaterialInterface* GhostMaterialValid;    // Vert translucide

    UPROPERTY(EditDefaultsOnly, Category = "Materials")
    class UMaterialInterface* GhostMaterialInvalid;  // Rouge translucide

    UPROPERTY(EditDefaultsOnly, Category = "Materials")
    class UMaterialInterface* NormalMaterial;        // Material normal

    // ── EVENTS ────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintImplementableEvent, Category = "Building Piece")
    void OnPlaced();

    UFUNCTION(BlueprintImplementableEvent, Category = "Building Piece")
    void OnDestroyed_Building();
};