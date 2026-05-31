#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GuildTypes.h"
#include "InventoryComponent.h"
#include "ColonNPC.generated.h"

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

USTRUCT(BlueprintType)
struct FColonSkills
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Logging = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Mining = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Construction = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Cooking = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Medicine = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Combat = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Crafting = 0;
};

USTRUCT(BlueprintType)
struct FColonNeeds
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite) float Hunger = 80.f;
    UPROPERTY(BlueprintReadWrite) float Energy = 100.f;
    UPROPERTY(BlueprintReadWrite) float Mood = 75.f;
    UPROPERTY(BlueprintReadWrite) float Hygiene = 100.f;
};

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

    FText GetFullName() const
    {
        return FText::FromString(FirstName.ToString() + " " + LastName.ToString());
    }

    bool HasTrait(EAdventurerTrait Trait) const
    {
        return Traits.Contains(Trait);
    }
};

UCLASS()
class PROJETGUILD_API AColonNPC : public ACharacter
{
    GENERATED_BODY()

public:

    AColonNPC();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colon")
    FColonData ColonData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Colon")
    UInventoryComponent* InventoryComponent;

    UPROPERTY(BlueprintReadWrite, Category = "Colon|AI")
    EColonTask CurrentTask = EColonTask::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Colon|AI")
    AActor* CurrentTarget = nullptr;

    UFUNCTION(BlueprintCallable, Category = "Colon")
    void AssignTask(EColonTask NewTask, AActor* Target = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Colon")
    void Interact();

    UFUNCTION(BlueprintPure, Category = "Colon")
    FText GetInteractionText() const;

    UFUNCTION(BlueprintCallable, Category = "Colon|Needs")
    void UpdateNeeds(float DeltaSeconds);

    UFUNCTION(BlueprintImplementableEvent, Category = "Colon")
    void OnTaskStarted(EColonTask Task, AActor* Target);

    UFUNCTION(BlueprintImplementableEvent, Category = "Colon")
    void OnTaskCompleted(EColonTask Task);

    UFUNCTION(BlueprintImplementableEvent, Category = "Colon")
    void OnNeedCritical(const FText& NeedName);

private:

    float NeedUpdateTimer = 0.f;
    const float NeedUpdateInterval = 1.f;
};