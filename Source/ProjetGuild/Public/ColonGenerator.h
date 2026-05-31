#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ColonNPC.h"
#include "ColonGenerator.generated.h"

UCLASS()
class PROJETGUILD_API UColonGenerator : public UObject
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category = "Colon|Generator")
    static FColonData GenerateColon(int32 DomainReputation = 0);

private:

    static FText GenerateFirstName();
    static FText GenerateLastName();
    static FText GenerateBackStory(FColonSkills& Skills);
    static FColonSkills GenerateSkills(int32 Reputation);
    static TArray<EAdventurerTrait> GenerateTraits(int32 Reputation);
    static int32 GenerateAge();
    static int32 ReputationScaledRandom(int32 Min, int32 Max, int32 RepBonus, int32 Reputation);
};