#include "ColonNPC.h"
#include "GameFramework/CharacterMovementComponent.h"

// ─────────────────────────────────────────────────────────────────────────────
//  CONSTRUCTEUR
//  Configure le mouvement et crée l'inventaire du colon
// ─────────────────────────────────────────────────────────────────────────────
AColonNPC::AColonNPC()
{
    PrimaryActorTick.bCanEverTick = true;

    // Physique normale — le colon tombe correctement
    GetCharacterMovement()->GravityScale = 1.0f;
    GetCharacterMovement()->MaxWalkSpeed = 300.f;

    // Permet la physique même sans controller (pendant transition BT)
    GetCharacterMovement()->bRunPhysicsWithNoController = true;

    // Évitement automatique entre colons (RVO = Reciprocal Velocity Obstacles)
    GetCharacterMovement()->bUseRVOAvoidance = true;

    // L'AIController prend possession automatiquement dès le spawn ou placement
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    // Inventaire personnel du colon — reçoit les ressources récoltées
    InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(
        TEXT("InventoryComponent"));
    GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);

    // Configure le Nav Agent pour correspondre au RecastNavMesh
    GetCharacterMovement()->NavAgentProps.AgentRadius = 35.f;
    GetCharacterMovement()->NavAgentProps.AgentHeight = 180.f;
    GetCharacterMovement()->NavAgentProps.bCanWalk = true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  BEGIN PLAY
// ─────────────────────────────────────────────────────────────────────────────
void AColonNPC::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("[ColonNPC] %s spawné — prêt."),
        *ColonData.GetFullName().ToString());
}

// ─────────────────────────────────────────────────────────────────────────────
//  TICK — met à jour les besoins à intervalle régulier
// ─────────────────────────────────────────────────────────────────────────────
void AColonNPC::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    NeedUpdateTimer += DeltaTime;
    if (NeedUpdateTimer >= NeedUpdateInterval)
    {
        NeedUpdateTimer = 0.f;
        UpdateNeeds(NeedUpdateInterval);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  ASSIGN TASK — change la tâche courante et notifie le Blueprint
// ─────────────────────────────────────────────────────────────────────────────
void AColonNPC::AssignTask(EColonTask NewTask)
{
    CurrentTask = NewTask;

    UE_LOG(LogTemp, Log, TEXT("[ColonNPC] %s — Nouvelle tâche : %d"),
        *ColonData.GetFullName().ToString(), (int32)NewTask);

    // Notifie le Blueprint pour les animations / sons
    OnTaskStarted(NewTask);
}

// ─────────────────────────────────────────────────────────────────────────────
//  GET INTERACTION TEXT — texte affiché au joueur
// ─────────────────────────────────────────────────────────────────────────────
FText AColonNPC::GetInteractionText() const
{
    return FText::FromString(
        FString::Printf(TEXT("Parler à %s [E]"),
            *ColonData.GetFullName().ToString()));
}

// ─────────────────────────────────────────────────────────────────────────────
//  UPDATE NEEDS — diminue les besoins et déclenche les alertes critiques
//  Appelée toutes les NeedUpdateInterval secondes via Tick
// ─────────────────────────────────────────────────────────────────────────────
void AColonNPC::UpdateNeeds(float DeltaSeconds)
{
    // La faim diminue toujours, quelle que soit la tâche
    ColonData.Needs.Hunger = FMath::Max(0.f,
        ColonData.Needs.Hunger - 0.5f * DeltaSeconds);

    // L'énergie ne diminue pas au repos ou en dormant
    float EnergyDrain = (CurrentTask == EColonTask::Idle ||
        CurrentTask == EColonTask::Sleeping) ? 0.f : 1.f;
    ColonData.Needs.Energy = FMath::Max(0.f,
        ColonData.Needs.Energy - EnergyDrain * DeltaSeconds);

    // Alertes critiques → le Blueprint peut réagir (changer de tâche, etc.)
    if (ColonData.Needs.Hunger <= 0.f)
        OnNeedCritical(FText::FromString("Famine"));
    if (ColonData.Needs.Energy <= 0.f)
        OnNeedCritical(FText::FromString("Épuisement"));
}