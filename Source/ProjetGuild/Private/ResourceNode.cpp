#include "ResourceNode.h"
#include "InventoryComponent.h"
#include "GuildManager.h"

AResourceNode::AResourceNode()
{
    PrimaryActorTick.bCanEverTick = false;

    // Crée le mesh component — le visuel du node
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    SetRootComponent(MeshComponent);
}

void AResourceNode::BeginPlay()
{
    Super::BeginPlay();
}

// ─────────────────────────────────────────────────────────────────────────────
//  INTERACT — appelée par le joueur via Line Trace
//  Retourne true si la récolte est complète
// ─────────────────────────────────────────────────────────────────────────────
bool AResourceNode::Interact()
{
    // Node indisponible — en régénération ou déjà récolté
    if (!bIsAvailable) return false;

    CurrentHits++;

    UE_LOG(LogTemp, Log, TEXT("[ResourceNode] Coup %d/%d sur %s"),
        CurrentHits, NodeData.HitsRequired, *GetName());

    // Notifie le Blueprint pour les effets visuels
    OnHit(CurrentHits, NodeData.HitsRequired);

    // Récolte complète ?
    if (CurrentHits >= NodeData.HitsRequired)
    {
        GiveResources();

        // Notifie le Blueprint pour l'animation de chute/disparition
        OnHarvested();

        if (NodeData.bRegenerates)
        {
            StartRegeneration();
        }
        else
        {
            // Pas de régénération — on cache le mesh
            MeshComponent->SetVisibility(false);
            MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            bIsAvailable = false;
        }

        CurrentHits = 0;
        return true;
    }

    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
//  GIVE RESOURCES — ajoute les ressources a l'inventaire
// ─────────────────────────────────────────────────────────────────────────────
void AResourceNode::GiveResources()
{
    // Cherche le joueur dans le monde
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    // Cherche le composant inventaire sur le joueur
    UInventoryComponent* Inventory = PlayerPawn->FindComponentByClass<UInventoryComponent>();
    if (!Inventory)
    {
        UE_LOG(LogTemp, Warning, TEXT("[ResourceNode] Pas d'inventaire sur le joueur !"));
        return;
    }

    // Ajoute les ressources à l'inventaire
    for (const FResourceCost& Resource : NodeData.ResourcesGiven)
    {
        // Convertit EItemCategory en FName pour l'inventaire
        FString ItemName;
        switch (Resource.Resource)
        {
        case EItemCategory::Material: ItemName = "Wood"; break;
        case EItemCategory::Weapon:   ItemName = "Weapon"; break;
        case EItemCategory::Food:     ItemName = "Food"; break;
        default: ItemName = "Unknown"; break;
        }
        FName ItemID = FName(*ItemName);

        int32 Remaining = Inventory->AddItem(ItemID, Resource.Amount);

        if (Remaining > 0)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[ResourceNode] Inventaire plein — %d items perdus"), Remaining);
        }
        else
        {
            UE_LOG(LogTemp, Log,
                TEXT("[ResourceNode] +%d %s dans l'inventaire"),
                Resource.Amount, *ItemID.ToString());
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  START REGENERATION
// ─────────────────────────────────────────────────────────────────────────────
void AResourceNode::StartRegeneration()
{
    bIsAvailable = false;
    MeshComponent->SetVisibility(false);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    if (UGuildManager* GM = UGuildManager::Get(this))
    {
        RegenerationDay = GM->GetCurrentDay() + NodeData.RegenerationDays;

        // S'abonne au delegate OnNewDay pour savoir quand régénérer
        GM->OnNewDay.AddDynamic(this, &AResourceNode::OnNewDayReceived);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  ON NEW DAY — vérifie si c'est le moment de régénérer
// ─────────────────────────────────────────────────────────────────────────────
void AResourceNode::OnNewDayReceived(int32 DayNumber)
{
    if (DayNumber >= RegenerationDay)
    {
        bIsAvailable = true;
        CurrentHits = 0;
        MeshComponent->SetVisibility(true);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

        // Se désabonne du delegate
        if (UGuildManager* GM = UGuildManager::Get(this))
        {
            GM->OnNewDay.RemoveDynamic(this, &AResourceNode::OnNewDayReceived);
        }

        OnRegenerated();

        UE_LOG(LogTemp, Log, TEXT("[ResourceNode] Régénéré !"));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  GET INTERACTION TEXT
// ─────────────────────────────────────────────────────────────────────────────
FText AResourceNode::GetInteractionText() const
{
    if (!bIsAvailable)
        return FText::FromString("En régénération...");

    switch (NodeData.NodeType)
    {
    case EResourceNodeType::Tree:
        return FText::FromString("Couper [E]");
    case EResourceNodeType::Rock:
        return FText::FromString("Miner [E]");
    case EResourceNodeType::Plant:
    case EResourceNodeType::Mushroom:
        return FText::FromString("Récolter [E]");
    case EResourceNodeType::Crate:
        return FText::FromString("Ouvrir [E]");
    default:
        return FText::FromString("Interagir [E]");
    }
}