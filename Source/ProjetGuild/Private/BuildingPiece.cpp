#include "BuildingPiece.h"
#include "GuildManager.h"
#include "Components/SceneComponent.h"

ABuildingPiece::ABuildingPiece()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    SetRootComponent(MeshComponent);
}

void ABuildingPiece::BeginPlay()
{
    Super::BeginPlay();
}

// ─────────────────────────────────────────────────────────────────────────────
//  GHOST MODE
// ─────────────────────────────────────────────────────────────────────────────
void ABuildingPiece::SetGhostMode(bool bGhost)
{
    bIsGhost = bGhost;

    if (bGhost)
    {
        // Pas de collision en mode ghost
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);

        // Material ghost vert par défaut
        if (GhostMaterialValid)
        {
            MeshComponent->SetMaterial(0, GhostMaterialValid);
        }
    }
    else
    {
        // Collision normale une fois posé
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

        // Material normal
        if (NormalMaterial)
        {
            MeshComponent->SetMaterial(0, NormalMaterial);
        }
    }
}

void ABuildingPiece::UpdateGhostColor(bool bCanPlaceHere)
{
    bCanPlace = bCanPlaceHere;

    if (!bIsGhost) return;

    if (bCanPlaceHere && GhostMaterialValid)
    {
        MeshComponent->SetMaterial(0, GhostMaterialValid);
    }
    else if (!bCanPlaceHere && GhostMaterialInvalid)
    {
        MeshComponent->SetMaterial(0, GhostMaterialInvalid);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  PLACEMENT
// ─────────────────────────────────────────────────────────────────────────────
bool ABuildingPiece::Place()
{
    
    UE_LOG(LogTemp, Warning, TEXT("[Place] bIsGhost: %d"), bIsGhost);
    UE_LOG(LogTemp, Warning, TEXT("[Place] bCanPlace: %d"), bCanPlace);
    UE_LOG(LogTemp, Warning, TEXT("[Place] CanAfford: %d"), CanAffordToBuild());
    
    if (!bIsGhost) return false;
    if (!bCanPlace) return false;
    if (!CanAffordToBuild()) return false;

    UGuildManager* GM = UGuildManager::Get(this);
    if (!GM) return false;

    // Dépense les ressources
    for (const FResourceCost& Cost : PieceData.BuildCost)
    {
        GM->SpendResourceFromInventory(Cost.Resource, Cost.Amount);
    }

    // Sort du mode ghost
    SetGhostMode(false);
    bIsGhost = false;
    // Cache la pièce temporairement — on la verra quand on aura de vrais materials
    MeshComponent->SetVisibility(true);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SetActorTickEnabled(false);

    CurrentHealth = PieceData.MaxHealth;

    OnPlaced();

    UE_LOG(LogTemp, Log, TEXT("[BuildingPiece] %s posé !"),
        *PieceData.DisplayName.ToString());

    return true;
}

bool ABuildingPiece::CanAffordToBuild() const
{
    UGuildManager* GM = UGuildManager::Get(this);
    if (!GM) return false;

    for (const FResourceCost& Cost : PieceData.BuildCost)
    {
        if (GM->GetResourceAmount(Cost.Resource) < Cost.Amount)
        {
            return false;
        }
    }
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  SNAP POINTS
// ─────────────────────────────────────────────────────────────────────────────
TArray<USceneComponent*> ABuildingPiece::GetSnapPoints() const
{
    TArray<USceneComponent*> SnapPoints;
    TArray<USceneComponent*> Components;
    GetComponents<USceneComponent>(Components);

    for (USceneComponent* Comp : Components)
    {
        // On cherche les composants nommés "SnapPoint_"
        if (Comp->GetName().StartsWith(TEXT("SnapPoint_")))
        {
            SnapPoints.Add(Comp);
        }
    }

    return SnapPoints;
}
