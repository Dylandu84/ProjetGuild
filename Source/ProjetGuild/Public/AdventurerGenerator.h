#pragma once

#include "CoreMinimal.h"
#include "GuildTypes.h"
#include "AdventurerGenerator.generated.h"

UCLASS()
class PROJETGUILD_API UAdventurerGenerator : public UObject
{
    GENERATED_BODY()

public:

    // ── FONCTION PRINCIPALE ───────────────────────────────────────────────
    // Génère un aventurier selon l'état actuel de la guilde
    // Plus la réputation est haute, meilleur sera l'aventurier
    UFUNCTION(BlueprintCallable, Category = "Guild|Adventurers")
    static FAdventurerData GenerateAdventurer(
        int32 GuildReputation,      // -100 à 100
        int32 GuildLevel = 1        // 1 à 5
    );

private:

    // ── GÉNÉRATEURS INTERNES ──────────────────────────────────────────────
    static FText GenerateFirstName();
    static FText GenerateLastName();
    static FText GenerateBackStory(EAdventurerClass Class);

    // Génère une classe selon la réputation
    // Réputation basse = classes communes, haute = classes rares
    static EAdventurerClass GenerateClass(int32 Reputation);

    // Génère les stats selon la réputation et la classe
    static void GenerateStats(FAdventurerData& Adventurer, int32 Reputation);

    // Génère 2 à 4 traits — mix positifs/négatifs selon la réputation
    static void GenerateTraits(FAdventurerData& Adventurer, int32 Reputation);

    // Génère le salaire selon les stats et la classe
    static int32 GenerateWage(const FAdventurerData& Adventurer);

    // Utilitaire — valeur aléatoire dans une plage influencée par la réputation
    // MinBase/MaxBase = plage de base, RepBonus = bonus max apporté par la réputation
    static int32 ReputationScaledRandom(
        int32 MinBase, int32 MaxBase,
        int32 RepBonus, int32 Reputation);
};