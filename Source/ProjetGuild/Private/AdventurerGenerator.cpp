#include "AdventurerGenerator.h"

// ─────────────────────────────────────────────────────────────────────────────
//  GÉNÉRATION PRINCIPALE
// ─────────────────────────────────────────────────────────────────────────────
FAdventurerData UAdventurerGenerator::GenerateAdventurer(
    int32 GuildReputation, int32 GuildLevel)
{
    FAdventurerData Adventurer;

    // ID unique
    Adventurer.UniqueID = FGuid::NewGuid();

    // Identité
    Adventurer.FirstName = GenerateFirstName();
    Adventurer.LastName = GenerateLastName();
    Adventurer.Class = GenerateClass(GuildReputation);

    // Stats selon réputation et niveau de guilde
    // Le niveau de guilde ajoute un bonus supplémentaire
    int32 EffectiveReputation = GuildReputation + (GuildLevel * 10);
    GenerateStats(Adventurer, EffectiveReputation);
    GenerateTraits(Adventurer, EffectiveReputation);

    // Backstory selon la classe
    Adventurer.BackStory = GenerateBackStory(Adventurer.Class);

    // Salaire calculé selon les stats
    Adventurer.DailyWage = GenerateWage(Adventurer);

    // État de départ
    Adventurer.Status = EAdventurerStatus::Available;
    Adventurer.CurrentMorale = 75;
    Adventurer.Loyalty = FMath::RandRange(40, 70);
    Adventurer.Fatigue = 0;

    UE_LOG(LogTemp, Log, TEXT("[Generator] Aventurier généré: %s le %s (Score: %d)"),
        *Adventurer.GetFullName().ToString(),
        *UEnum::GetValueAsString(Adventurer.Class),
        Adventurer.GetCombatScore());

    return Adventurer;
}

// ─────────────────────────────────────────────────────────────────────────────
//  NOMS
// ─────────────────────────────────────────────────────────────────────────────
FText UAdventurerGenerator::GenerateFirstName()
{
    TArray<FString> Names = {
        "Aldric", "Maren", "Vorak", "Syla", "Durgin", "Eira",
        "Theron", "Kessa", "Bram", "Lyra", "Gorin", "Nessa",
        "Cael", "Vira", "Harlan", "Zara", "Odric", "Tana",
        "Edwyn", "Rynn", "Baldur", "Seren", "Fergus", "Mira"
    };

    int32 Index = FMath::RandRange(0, Names.Num() - 1);
    return FText::FromString(Names[Index]);
}

FText UAdventurerGenerator::GenerateLastName()
{
    TArray<FString> Names = {
        "Ironforge", "Ashwood", "Coldwater", "Darkbane", "Stonefist",
        "Grimshaw", "Blackthorn", "Swiftblade", "Wraithwood", "Emberveil",
        "Duskmantle", "Ironwood", "Stormborn", "Shadowmere", "Goldvein",
        "Ravenscroft", "Thornwall", "Dusthallow", "Bleakwood", "Steelhand"
    };

    int32 Index = FMath::RandRange(0, Names.Num() - 1);
    return FText::FromString(Names[Index]);
}

FText UAdventurerGenerator::GenerateBackStory(EAdventurerClass Class)
{
    TArray<FString> WarriorStories = {
        "Ancien soldat qui a déserté après un massacre dont il était responsable.",
        "Garde du corps d'un noble assassiné. Il cherche maintenant du travail.",
        "Vétéran de trois guerres. Ses cauchemars l'ont suivi jusqu'ici.",
    };

    TArray<FString> RangerStories = {
        "Chasseuse des forêts du nord. Elle connaît des chemins que personne d'autre ne connaît.",
        "Ancien éclaireur militaire. Il préfère les arbres aux murs.",
        "Traqueur de primes reconverti. Ses cibles finissaient toujours par être trouvées.",
    };

    TArray<FString> MageStories = {
        "Érudit expulsé de son académie pour des recherches jugées trop dangereuses.",
        "Apprenti dont le maître a disparu mystérieusement. Elle cherche des réponses.",
        "Ancien conseiller royal. Une trahison l'a mis sur les routes.",
    };

    TArray<FString> RogueStories = {
        "Pickpocket devenu voleur professionnel. Il sait se faire discret.",
        "Ancienne espionne sans employeur. Ses compétences restent intactes.",
        "Contrebandier retraité. Il connaît tous les passages secrets de la région.",
    };

    TArray<FString> ClericStories = {
        "Prêtresse dont le temple a été détruit. Sa foi est intacte, sa patience moins.",
        "Guérisseur de campagne qui soignait les soldats des deux camps.",
        "Moine en rupture avec son ordre. Il cherche un sens différent au service.",
    };

    TArray<FString>* Stories = nullptr;
    switch (Class)
    {
    case EAdventurerClass::Warrior: Stories = &WarriorStories; break;
    case EAdventurerClass::Ranger:  Stories = &RangerStories;  break;
    case EAdventurerClass::Mage:    Stories = &MageStories;    break;
    case EAdventurerClass::Rogue:   Stories = &RogueStories;   break;
    case EAdventurerClass::Cleric:  Stories = &ClericStories;  break;
    default: Stories = &WarriorStories; break;
    }

    int32 Index = FMath::RandRange(0, Stories->Num() - 1);
    return FText::FromString((*Stories)[Index]);
}

// ─────────────────────────────────────────────────────────────────────────────
//  CLASSE
//  Réputation basse = classes communes (Warrior, Rogue)
//  Réputation haute = classes rares (Mage, Cleric) et plus de variété
// ─────────────────────────────────────────────────────────────────────────────
EAdventurerClass UAdventurerGenerator::GenerateClass(int32 Reputation)
{
    // On normalise la réputation en 0-100
    int32 Rep = FMath::Clamp(Reputation + 100, 0, 200) / 2;

    int32 Roll = FMath::RandRange(0, 99);

    if (Rep < 20)
    {
        // Guilde inconnue — surtout des guerriers et voleurs désespérés
        if (Roll < 60) return EAdventurerClass::Warrior;
        if (Roll < 85) return EAdventurerClass::Rogue;
        if (Roll < 95) return EAdventurerClass::Ranger;
        return EAdventurerClass::Cleric;
    }
    else if (Rep < 50)
    {
        // Guilde connue — plus de variété
        if (Roll < 35) return EAdventurerClass::Warrior;
        if (Roll < 55) return EAdventurerClass::Ranger;
        if (Roll < 70) return EAdventurerClass::Rogue;
        if (Roll < 85) return EAdventurerClass::Cleric;
        return EAdventurerClass::Mage;
    }
    else
    {
        // Guilde réputée — toutes les classes disponibles équitablement
        if (Roll < 25) return EAdventurerClass::Warrior;
        if (Roll < 45) return EAdventurerClass::Ranger;
        if (Roll < 60) return EAdventurerClass::Rogue;
        if (Roll < 78) return EAdventurerClass::Cleric;
        return EAdventurerClass::Mage;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  STATS
//  La réputation influence la qualité des stats
//  Réputation -100 → stats entre 20-40
//  Réputation 0    → stats entre 35-55
//  Réputation +100 → stats entre 55-80
// ─────────────────────────────────────────────────────────────────────────────
void UAdventurerGenerator::GenerateStats(FAdventurerData& Adventurer, int32 Reputation)
{
    // Stats de base pour tout le monde
    Adventurer.Strength = ReputationScaledRandom(20, 40, 40, Reputation);
    Adventurer.Agility = ReputationScaledRandom(20, 40, 40, Reputation);
    Adventurer.Intelligence = ReputationScaledRandom(20, 40, 40, Reputation);
    Adventurer.Endurance = ReputationScaledRandom(20, 40, 40, Reputation);
    Adventurer.Charisma = ReputationScaledRandom(20, 40, 40, Reputation);

    // Bonus de classe — chaque classe excelle dans ses stats principales
    switch (Adventurer.Class)
    {
    case EAdventurerClass::Warrior:
        Adventurer.Strength += FMath::RandRange(10, 20);
        Adventurer.Endurance += FMath::RandRange(5, 15);
        break;
    case EAdventurerClass::Ranger:
        Adventurer.Agility += FMath::RandRange(10, 20);
        Adventurer.Endurance += FMath::RandRange(5, 10);
        break;
    case EAdventurerClass::Mage:
        Adventurer.Intelligence += FMath::RandRange(15, 25);
        Adventurer.Agility -= FMath::RandRange(5, 10); // Fragile
        break;
    case EAdventurerClass::Rogue:
        Adventurer.Agility += FMath::RandRange(10, 20);
        Adventurer.Intelligence += FMath::RandRange(5, 10);
        break;
    case EAdventurerClass::Cleric:
        Adventurer.Intelligence += FMath::RandRange(10, 15);
        Adventurer.Charisma += FMath::RandRange(10, 15);
        break;
    }

    // Clamp toutes les stats entre 1 et 100
    Adventurer.Strength = FMath::Clamp(Adventurer.Strength, 1, 100);
    Adventurer.Agility = FMath::Clamp(Adventurer.Agility, 1, 100);
    Adventurer.Intelligence = FMath::Clamp(Adventurer.Intelligence, 1, 100);
    Adventurer.Endurance = FMath::Clamp(Adventurer.Endurance, 1, 100);
    Adventurer.Charisma = FMath::Clamp(Adventurer.Charisma, 1, 100);
}

// ─────────────────────────────────────────────────────────────────────────────
//  TRAITS
//  Réputation basse → plus de traits négatifs
//  Réputation haute → plus de traits positifs, moins de négatifs
// ─────────────────────────────────────────────────────────────────────────────
void UAdventurerGenerator::GenerateTraits(FAdventurerData& Adventurer, int32 Reputation)
{
    TArray<EAdventurerTrait> PositiveTraits = {
        EAdventurerTrait::Brave,
        EAdventurerTrait::Loyal,
        EAdventurerTrait::Veteran,
        EAdventurerTrait::FastLearner,
        EAdventurerTrait::TeamPlayer
    };

    TArray<EAdventurerTrait> NegativeTraits = {
        EAdventurerTrait::Alcoholic,
        EAdventurerTrait::Greedy,
        EAdventurerTrait::Traumatized,
        EAdventurerTrait::Gambler,
        EAdventurerTrait::Arrogant
    };

    TArray<EAdventurerTrait> MixedTraits = {
        EAdventurerTrait::Reckless,
        EAdventurerTrait::Loner,
        EAdventurerTrait::Ambitious
    };

    // Nombre de traits : 2 à 4
    int32 NumTraits = FMath::RandRange(2, 4);

    // Probabilité d'un trait positif selon la réputation
    // Rep -100 → 20% positif, Rep +100 → 80% positif
    int32 PositiveChance = FMath::Clamp(50 + Reputation / 2, 20, 80);

    for (int32 i = 0; i < NumTraits; i++)
    {
        EAdventurerTrait NewTrait;
        int32 Roll = FMath::RandRange(0, 99);

        if (Roll < PositiveChance)
        {
            // Trait positif
            int32 Idx = FMath::RandRange(0, PositiveTraits.Num() - 1);
            NewTrait = PositiveTraits[Idx];
        }
        else if (Roll < PositiveChance + 15)
        {
            // Trait mixte
            int32 Idx = FMath::RandRange(0, MixedTraits.Num() - 1);
            NewTrait = MixedTraits[Idx];
        }
        else
        {
            // Trait négatif
            int32 Idx = FMath::RandRange(0, NegativeTraits.Num() - 1);
            NewTrait = NegativeTraits[Idx];
        }

        // Pas de trait en double
        if (!Adventurer.Traits.Contains(NewTrait))
        {
            Adventurer.Traits.Add(NewTrait);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  SALAIRE
//  Basé sur le score de combat — un aventurier compétent coûte plus cher
// ─────────────────────────────────────────────────────────────────────────────
int32 UAdventurerGenerator::GenerateWage(const FAdventurerData& Adventurer)
{
    int32 BaseWage = 5;
    int32 ScoreBonus = Adventurer.GetCombatScore() / 5;

    // Les Mages et Clercs coûtent plus cher
    int32 ClassBonus = 0;
    if (Adventurer.Class == EAdventurerClass::Mage)   ClassBonus = 10;
    if (Adventurer.Class == EAdventurerClass::Cleric)  ClassBonus = 8;

    // Légère variation aléatoire
    int32 RandomVariation = FMath::RandRange(-2, 3);

    return FMath::Max(5, BaseWage + ScoreBonus + ClassBonus + RandomVariation);
}

// ─────────────────────────────────────────────────────────────────────────────
//  UTILITAIRE — STAT SCALÉE PAR LA RÉPUTATION
// ─────────────────────────────────────────────────────────────────────────────
int32 UAdventurerGenerator::ReputationScaledRandom(
    int32 MinBase, int32 MaxBase, int32 RepBonus, int32 Reputation)
{
    // Normalise la réputation en 0-1
    float RepNormalized = FMath::Clamp((Reputation + 100.f) / 200.f, 0.f, 1.f);

    // Bonus apporté par la réputation
    int32 Bonus = FMath::RoundToInt(RepBonus * RepNormalized);

    return FMath::RandRange(MinBase, MaxBase) + Bonus;
}