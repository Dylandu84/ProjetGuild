#include "ColonGenerator.h"

// ─────────────────────────────────────────────────────────────────────────────
//  GENERATE COLON — FONCTION PRINCIPALE
// ─────────────────────────────────────────────────────────────────────────────
FColonData UColonGenerator::GenerateColon(int32 DomainReputation)
{
    FColonData Colon;

    Colon.FirstName = GenerateFirstName();
    Colon.LastName = GenerateLastName();
    Colon.Age = GenerateAge();
    Colon.Skills = GenerateSkills(DomainReputation);
    Colon.Traits = GenerateTraits(DomainReputation);
    Colon.BackStory = GenerateBackStory(Colon.Skills);

    // Besoins de départ — tout le monde commence raisonnablement bien
    Colon.Needs.Hunger = FMath::RandRange(60.f, 90.f);
    Colon.Needs.Energy = FMath::RandRange(70.f, 100.f);
    Colon.Needs.Mood = FMath::RandRange(50.f, 80.f);
    Colon.Needs.Hygiene = FMath::RandRange(60.f, 100.f);
    Colon.Health = FMath::RandRange(80.f, 100.f);

    UE_LOG(LogTemp, Log, TEXT("[ColonGenerator] Colon généré: %s %s, Age: %d"),
        *Colon.FirstName.ToString(), *Colon.LastName.ToString(), Colon.Age);

    return Colon;
}

// ─────────────────────────────────────────────────────────────────────────────
//  NOMS
// ─────────────────────────────────────────────────────────────────────────────
FText UColonGenerator::GenerateFirstName()
{
    TArray<FString> Names = {
        "Aldric", "Maren", "Vorak", "Syla", "Durgin", "Eira",
        "Bram", "Lyra", "Gorin", "Nessa", "Cael", "Vira",
        "Harlan", "Zara", "Odric", "Tana", "Edwyn", "Rynn",
        "Fergus", "Mira", "Torben", "Lena", "Sigrid", "Oswin"
    };
    return FText::FromString(Names[FMath::RandRange(0, Names.Num() - 1)]);
}

FText UColonGenerator::GenerateLastName()
{
    TArray<FString> Names = {
        "Ironforge", "Ashwood", "Coldwater", "Stonefist", "Grimshaw",
        "Blackthorn", "Wraithwood", "Emberveil", "Ironwood", "Stormborn",
        "Dusthallow", "Bleakwood", "Steelhand", "Ravenscroft", "Goldvein"
    };
    return FText::FromString(Names[FMath::RandRange(0, Names.Num() - 1)]);
}

// ─────────────────────────────────────────────────────────────────────────────
//  BACKSTORY — basée sur la compétence principale
// ─────────────────────────────────────────────────────────────────────────────
FText UColonGenerator::GenerateBackStory(FColonSkills& Skills)
{
    // Trouve la compétence principale
    int32 MaxSkill = 0;
    int32 MainSkill = 0;

    TArray<int32> SkillValues = {
        Skills.Logging, Skills.Mining, Skills.Construction,
        Skills.Cooking, Skills.Medicine, Skills.Combat, Skills.Crafting
    };

    for (int32 i = 0; i < SkillValues.Num(); i++)
    {
        if (SkillValues[i] > MaxSkill)
        {
            MaxSkill = SkillValues[i];
            MainSkill = i;
        }
    }

    TArray<FString> Stories;

    switch (MainSkill)
    {
    case 0: // Logging
        Stories = {
            "Ancien bûcheron du Nord. Il connaît les forêts mieux que sa propre maison.",
            "A passé sa jeunesse à abattre des arbres pour payer les dettes de son père.",
            "Bûcheron de métier, fuyant une ville où il n'était plus le bienvenu."
        };
        break;
    case 1: // Mining
        Stories = {
            "Ancien mineur des mines de Valdrun. Ses mains racontent des années sous terre.",
            "A travaillé dans des mines jusqu'à ce qu'un effondrement l'oblige à partir.",
            "Chercheur de minerais reconverti, il sait flairer le bon filon."
        };
        break;
    case 2: // Construction
        Stories = {
            "Maçon itinérant qui a construit des bâtiments dans une dizaine de villes.",
            "Ancien apprenti architecte dont le maître est décédé avant la fin de sa formation.",
            "Bâtisseur autodidacte. Ses constructions sont solides même si pas toujours belles."
        };
        break;
    case 3: // Cooking
        Stories = {
            "Ancien cuisinier d'une auberge qui a brûlé dans un incendie mystérieux.",
            "Cuisinière de camp militaire. Elle sait nourrir une armée avec peu de ressources.",
            "A appris à cuisiner pour survivre. Maintenant c'est sa fierté."
        };
        break;
    case 4: // Medicine
        Stories = {
            "Guérisseur de campagne qui soignait les soldats des deux camps en temps de guerre.",
            "Ancienne apprentie chirurgienne chassée de l'académie pour ses méthodes non conventionnelles.",
            "Herboriste dont les remèdes sont réputés dans toute la région."
        };
        break;
    case 5: // Combat
        Stories = {
            "Ancien soldat qui a déserté après une bataille dont il ne veut pas parler.",
            "Mercenaire à la retraite. Il cherche une vie plus calme — mais ses poings ne le savent pas.",
            "Garde du corps licencié après la mort de son employeur."
        };
        break;
    default: // Crafting
        Stories = {
            "Artisan polyvalent qui peut fabriquer presque n'importe quoi avec presque rien.",
            "Ancien apprenti forgeron qui a préféré la liberté à la boutique familiale.",
            "Bricoleur de génie dont les inventions fonctionnent rarement du premier coup."
        };
        break;
    }

    return FText::FromString(Stories[FMath::RandRange(0, Stories.Num() - 1)]);
}

// ─────────────────────────────────────────────────────────────────────────────
//  GENERATE SKILLS
//  Principe RimWorld : chaque colon a UNE spécialité principale (6-12)
//  et des compétences secondaires faibles (0-4).
//  La réputation améliore légèrement tout mais ne change pas la spécialité.
// ─────────────────────────────────────────────────────────────────────────────
FColonSkills UColonGenerator::GenerateSkills(int32 Reputation)
{
    FColonSkills Skills;

    // Bonus de réputation — max +3 à la spécialité, +1 aux secondaires
    int32 RepBonus = FMath::Clamp(Reputation / 30, 0, 3);

    // Choisit une spécialité au hasard
    int32 Specialty = FMath::RandRange(0, 6);

    // Compétences de base — toutes faibles
    TArray<int32*> AllSkills = {
        &Skills.Logging, &Skills.Mining, &Skills.Construction,
        &Skills.Cooking, &Skills.Medicine, &Skills.Combat, &Skills.Crafting
    };

    // Toutes les compétences commencent à 0-4
    for (int32* Skill : AllSkills)
    {
        *Skill = FMath::RandRange(0, 4) + RepBonus;
    }

    // La spécialité est bien meilleure — 6 à 12
    *AllSkills[Specialty] = FMath::RandRange(6, 12) + RepBonus;

    // Une compétence secondaire est légèrement meilleure — 3 à 6
    int32 Secondary = FMath::RandRange(0, 6);
    while (Secondary == Specialty) Secondary = FMath::RandRange(0, 6);
    *AllSkills[Secondary] = FMath::RandRange(3, 6) + RepBonus;

    // Clamp tout entre 0 et 20
    for (int32* Skill : AllSkills)
    {
        *Skill = FMath::Clamp(*Skill, 0, 20);
    }

    return Skills;
}

// ─────────────────────────────────────────────────────────────────────────────
//  GENERATE TRAITS
//  1 à 3 traits — mix positifs/négatifs selon la réputation
// ─────────────────────────────────────────────────────────────────────────────
TArray<EAdventurerTrait> UColonGenerator::GenerateTraits(int32 Reputation)
{
    TArray<EAdventurerTrait> PositiveTraits = {
        EAdventurerTrait::Brave, EAdventurerTrait::Loyal,
        EAdventurerTrait::FastLearner, EAdventurerTrait::TeamPlayer
    };

    TArray<EAdventurerTrait> NegativeTraits = {
        EAdventurerTrait::Alcoholic, EAdventurerTrait::Greedy,
        EAdventurerTrait::Gambler, EAdventurerTrait::Arrogant,
        EAdventurerTrait::Traumatized
    };

    TArray<EAdventurerTrait> MixedTraits = {
        EAdventurerTrait::Reckless, EAdventurerTrait::Loner, EAdventurerTrait::Ambitious
    };

    TArray<EAdventurerTrait> Result;
    int32 NumTraits = FMath::RandRange(1, 3);

    // Probabilité de trait positif selon réputation
    int32 PositiveChance = FMath::Clamp(40 + Reputation / 3, 30, 70);

    for (int32 i = 0; i < NumTraits; i++)
    {
        EAdventurerTrait NewTrait;
        int32 Roll = FMath::RandRange(0, 99);

        if (Roll < PositiveChance)
            NewTrait = PositiveTraits[FMath::RandRange(0, PositiveTraits.Num() - 1)];
        else if (Roll < PositiveChance + 15)
            NewTrait = MixedTraits[FMath::RandRange(0, MixedTraits.Num() - 1)];
        else
            NewTrait = NegativeTraits[FMath::RandRange(0, NegativeTraits.Num() - 1)];

        if (!Result.Contains(NewTrait))
            Result.Add(NewTrait);
    }

    return Result;
}

// ─────────────────────────────────────────────────────────────────────────────
//  GENERATE AGE
//  Les jeunes ont moins de compétences mais apprennent plus vite.
//  Les vieux ont plus de compétences mais progressent moins.
// ─────────────────────────────────────────────────────────────────────────────
int32 UColonGenerator::GenerateAge()
{
    // Distribution réaliste — plus de jeunes adultes que de vieux
    int32 Roll = FMath::RandRange(0, 99);
    if (Roll < 30) return FMath::RandRange(18, 25);  // Jeune
    if (Roll < 60) return FMath::RandRange(26, 35);  // Adulte
    if (Roll < 85) return FMath::RandRange(36, 50);  // Expérimenté
    return FMath::RandRange(51, 65);                  // Vétéran
}

int32 UColonGenerator::ReputationScaledRandom(int32 Min, int32 Max, int32 RepBonus, int32 Reputation)
{
    float RepNormalized = FMath::Clamp((Reputation + 100.f) / 200.f, 0.f, 1.f);
    int32 Bonus = FMath::RoundToInt(RepBonus * RepNormalized);
    return FMath::RandRange(Min, Max) + Bonus;
}