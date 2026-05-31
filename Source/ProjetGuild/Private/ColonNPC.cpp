#include "ColonNPC.h"
#include "GameFramework/CharacterMovementComponent.h"

AColonNPC::AColonNPC()
{
    PrimaryActorTick.bCanEverTick = true;

    GetCharacterMovement()->GravityScale = 1.0f;
    GetCharacterMovement()->MaxWalkSpeed = 300.f;
    GetCharacterMovement()->bRunPhysicsWithNoController = true;
    GetCharacterMovement()->bUseRVOAvoidance = true;

    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
}

void AColonNPC::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("[ColonNPC] %s initialisé — Tâche: Idle"),
        *ColonData.GetFullName().ToString());
}

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

void AColonNPC::AssignTask(EColonTask NewTask, AActor* Target)
{
    CurrentTask = NewTask;
    CurrentTarget = Target;

    UE_LOG(LogTemp, Log, TEXT("[ColonNPC] %s — Nouvelle tâche: %d"),
        *ColonData.GetFullName().ToString(), (int32)NewTask);

    OnTaskStarted(NewTask, Target);
}

void AColonNPC::Interact()
{
    UE_LOG(LogTemp, Log, TEXT("[ColonNPC] Interaction avec %s"),
        *ColonData.GetFullName().ToString());
}

FText AColonNPC::GetInteractionText() const
{
    return FText::FromString(
        FString::Printf(TEXT("Parler à %s [E]"),
            *ColonData.GetFullName().ToString()));
}

void AColonNPC::UpdateNeeds(float DeltaSeconds)
{
    ColonData.Needs.Hunger = FMath::Max(0.f,
        ColonData.Needs.Hunger - 0.5f * DeltaSeconds);

    float EnergyDrain = (CurrentTask == EColonTask::Idle ||
        CurrentTask == EColonTask::Sleeping) ? 0.f : 1.f;
    ColonData.Needs.Energy = FMath::Max(0.f,
        ColonData.Needs.Energy - EnergyDrain * DeltaSeconds);

    if (ColonData.Needs.Hunger < 20.f)
        ColonData.Needs.Mood = FMath::Max(0.f,
            ColonData.Needs.Mood - 2.f * DeltaSeconds);
    if (ColonData.Needs.Energy < 20.f)
        ColonData.Needs.Mood = FMath::Max(0.f,
            ColonData.Needs.Mood - 1.f * DeltaSeconds);

    if (ColonData.Needs.Hunger <= 0.f)
        OnNeedCritical(FText::FromString("Famine"));
    if (ColonData.Needs.Energy <= 0.f)
        OnNeedCritical(FText::FromString("Épuisement"));
    if (ColonData.Needs.Mood <= 10.f)
        OnNeedCritical(FText::FromString("Dépression"));
}