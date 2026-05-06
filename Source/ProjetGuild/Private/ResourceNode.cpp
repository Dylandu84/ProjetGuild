#include "ResourceNode.h"
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
//  GIVE RESOURCES — ajoute les ressources au GuildManager
// ─────────────────────────────────────────────────────────────────────────────
void AResourceNode::GiveResources()
{
    UGuildManager* GM = UGuildManager::Get(this);
    if (!GM) return;

    for (const FResourceCost& Resource : NodeData.ResourcesGiven)
    {
        // On utilise AddResource si elle existe — sinon on ajoute à l'inventaire
        UE_LOG(LogTemp, Log, TEXT("[ResourceNode] Donne %d x %d"),
            (int32)Resource.Resource, Resource.Amount);

        GM->AddResourceToInventory(Resource.Resource, Resource.Amount);
        // GM->AddResource(Resource.Resource, Resource.Amount);
    }

    UE_LOG(LogTemp, Log, TEXT("[ResourceNode] Récolte complète !"));
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