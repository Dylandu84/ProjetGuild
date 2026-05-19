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
        MeshComponent->SetCollisionProfileName(TEXT("BlockAll"));
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
    MeshComponent->SetCollisionProfileName(TEXT("BlockAll"));
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
//  GET SNAP POINT WORLD LOCATION
//  Convertit la position locale du snap point en position mondiale
// ─────────────────────────────────────────────────────────────────────────────
FVector ABuildingPiece::GetSnapPointWorldLocation(const FSnapPointData& SnapPoint) const
{
    // On transforme l'offset local en position mondiale
    // en tenant compte de la rotation et position de la pièce
    return GetActorTransform().TransformPosition(SnapPoint.LocalOffset);
}

// ─────────────────────────────────────────────────────────────────────────────
//  FIND COMPATIBLE SNAP POINT
//  Cherche dans les snap points de cette pièce un point compatible
//  avec les types du ghost, et assez proche de SearchLocation
// ─────────────────────────────────────────────────────────────────────────────
bool ABuildingPiece::FindCompatibleSnapPoint(
    const TArray<ESnapType>& GhostCompatibleTypes,
    FVector SearchLocation,
    FVector& OutLocation,
    FRotator& OutRotation) const
{
    UE_LOG(LogTemp, Log, TEXT("[Snap] Checking piece SnapPoints: %d"), SnapPoints.Num());
    
    const float SnapRadius = 250.f;
    float NearestDistance = SnapRadius;
    bool bFound = false;

    for (const FSnapPointData& SnapPoint : SnapPoints)
    {
        UE_LOG(LogTemp, Log, TEXT("[Snap] SnapPoint type: %d, Compatible count: %d"),
            (int32)SnapPoint.SnapType, SnapPoint.CompatibleTypes.Num());
        
        for (ESnapType GhostType : GhostCompatibleTypes)
        {
            UE_LOG(LogTemp, Log, TEXT("[Snap] Ghost type: %d"), (int32)GhostType);
            UE_LOG(LogTemp, Log, TEXT("[Snap] Compatible contains: %d"),
                SnapPoint.CompatibleTypes.Contains(GhostType) ? 1 : 0);
        }
        // Vérifie si ce snap point est compatible avec le ghost
        bool bCompatible = false;
        for (ESnapType GhostType : GhostCompatibleTypes)
        {
            if (SnapPoint.CompatibleTypes.Contains(GhostType))
            {
                bCompatible = true;
                break;
            }
        }

        if (!bCompatible) continue;

        // Calcule la position mondiale du snap point
        FVector WorldLocation = GetSnapPointWorldLocation(SnapPoint);
        float Distance = FVector::Dist(SearchLocation, WorldLocation);
        UE_LOG(LogTemp, Log, TEXT("[Snap] Distance au snap: %f / SnapRadius: %f"),
            Distance, SnapRadius);

        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            OutLocation = WorldLocation;
            OutRotation = SnapPoint.SnapRotation;
            bFound = true;
        }
    }

    return bFound;
}