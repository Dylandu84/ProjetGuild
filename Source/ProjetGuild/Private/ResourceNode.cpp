#include "ResourceNode.h"
#include "InventoryComponent.h"
#include "GuildManager.h"

// ─────────────────────────────────────────────────────────────────────────────
//  CONSTRUCTEUR
// ─────────────────────────────────────────────────────────────────────────────
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
//  INTERACT — appelée par le joueur FPS ou un ColonNPC
//  Harvester = l'actor qui récolte (colon ou joueur)
//  nullptr = fallback automatique sur le joueur FPS
//  Retourne true si la récolte est complète
// ─────────────────────────────────────────────────────────────────────────────
bool AResourceNode::Interact(AActor* Harvester)
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
        // Passe le Harvester pour que les ressources aillent au bon inventaire
        GiveResources(Harvester);

        // Notifie le Blueprint pour l'animation de chute/disparition
        OnHarvested();

        if (NodeData.bRegenerates)
        {
            StartRegeneration();
        }
        else
        {
            // Pas de régénération — on cache le mesh définitivement
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
//  GIVE RESOURCES — ajoute les ressources à l'inventaire du Harvester
//  Harvester peut être le joueur FPS ou un ColonNPC
//  Si nullptr → fallback automatique sur le joueur FPS
// ─────────────────────────────────────────────────────────────────────────────
void AResourceNode::GiveResources(AActor* Harvester)
{
    // Détermine la cible — Harvester en priorité, sinon joueur FPS
    AActor* Target = Harvester;
    if (!Target)
    {
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC) Target = PC->GetPawn();
    }

    if (!Target) return;

    // Cherche le composant inventaire sur la cible
    UInventoryComponent* Inventory = Target->FindComponentByClass<UInventoryComponent>();
    if (!Inventory)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[ResourceNode] Pas d'inventaire sur %s !"),
            *Target->GetName());
        return;
    }

    // Ajoute chaque ressource configurée dans NodeData
    for (const FResourceCost& Resource : NodeData.ResourcesGiven)
    {
        // Convertit EItemCategory en FName pour l'inventaire
        FString ItemName;
        switch (Resource.Resource)
        {
        case EItemCategory::Material: ItemName = "Wood";    break;
        case EItemCategory::Weapon:   ItemName = "Weapon";  break;
        case EItemCategory::Food:     ItemName = "Food";    break;
        default:                      ItemName = "Unknown"; break;
        }
        FName ItemID = FName(*ItemName);

        int32 Remaining = Inventory->AddItem(ItemID, Resource.Amount);

        if (Remaining > 0)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[ResourceNode] Inventaire plein — %d %s perdus"),
                Remaining, *ItemID.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Log,
                TEXT("[ResourceNode] +%d %s → inventaire de %s"),
                Resource.Amount, *ItemID.ToString(), *Target->GetName());
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  START REGENERATION — cache le node et attend le bon jour pour réapparaître
// ─────────────────────────────────────────────────────────────────────────────
void AResourceNode::StartRegeneration()
{
    bIsAvailable = false;
    MeshComponent->SetVisibility(false);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    if (UGuildManager* GM = UGuildManager::Get(this))
    {
        // Calcule le jour de réapparition
        RegenerationDay = GM->GetCurrentDay() + NodeData.RegenerationDays;

        // S'abonne au delegate OnNewDay pour savoir quand régénérer
        GM->OnNewDay.AddDynamic(this, &AResourceNode::OnNewDayReceived);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  ON NEW DAY — vérifie si c'est le moment de régénérer
//  Appelée automatiquement par le delegate OnNewDay du GuildManager
// ─────────────────────────────────────────────────────────────────────────────
void AResourceNode::OnNewDayReceived(int32 DayNumber)
{
    if (DayNumber >= RegenerationDay)
    {
        bIsAvailable = true;
        CurrentHits = 0;
        MeshComponent->SetVisibility(true);
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

        // Se désabonne du delegate — plus besoin d'écouter les jours
        if (UGuildManager* GM = UGuildManager::Get(this))
        {
            GM->OnNewDay.RemoveDynamic(this, &AResourceNode::OnNewDayReceived);
        }

        OnRegenerated();

        UE_LOG(LogTemp, Log, TEXT("[ResourceNode] %s régénéré !"), *GetName());
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  GET INTERACTION TEXT — texte affiché au joueur selon le type de node
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