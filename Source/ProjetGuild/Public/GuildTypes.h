#pragma once

// ─────────────────────────────────────────────────────────────────────────────
//  GUILDTYPES.H
//  Le fichier de définition de toutes les données du jeu.
//  Règle d'or : ce fichier ne FAIT rien. Il DÉCRIT.
//  Tout le reste du code va s'appuyer sur ces définitions.
// ─────────────────────────────────────────────────────────────────────────────

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GuildTypes.generated.h"


// ═════════════════════════════════════════════════════════════════════════════
//  1. RESSOURCES & OBJETS
//  Ce que ta guilde possède, achète, vend et utilise.
// ═════════════════════════════════════════════════════════════════════════════

// Les catégories d'objets dans le jeu
// UENUM = UE5 peut utiliser cet enum dans les Blueprints et DataTables
UENUM(BlueprintType)
enum class EItemCategory : uint8
{
    Weapon      UMETA(DisplayName = "Arme"),
    Armor       UMETA(DisplayName = "Armure"),
    Potion      UMETA(DisplayName = "Potion"),
    Food        UMETA(DisplayName = "Nourriture"),
    Material    UMETA(DisplayName = "Matériau"),
    Loot        UMETA(DisplayName = "Butin"),
    Decoration  UMETA(DisplayName = "Décoration"),
    Misc        UMETA(DisplayName = "Divers"),
};

// La rareté d'un objet — impacte le prix et les effets
UENUM(BlueprintType)
enum class EItemRarity : uint8
{
    Common      UMETA(DisplayName = "Commun"),
    Uncommon    UMETA(DisplayName = "Peu commun"),
    Rare        UMETA(DisplayName = "Rare"),
    Legendary   UMETA(DisplayName = "Légendaire"),
};

// FItemData = la fiche technique d'un objet
// Hérite de FTableRowBase → peut être stocké dans une DataTable UE5
// Tu définis tous tes objets dans l'éditeur UE5 sans toucher au code
USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    FText DisplayName;                          // Nom affiché en jeu

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    FText Description;                          // Description pour le joueur

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    EItemCategory Category = EItemCategory::Misc;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
    EItemRarity Rarity = EItemRarity::Common;

    // Prix de base en pièces d'or
    // Les marchands achètent à 60% de ce prix, vendent à 130%
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
    int32 BaseGoldValue = 10;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Economy")
    int32 MaxStackSize = 99;                    // Combien on peut empiler

    // Bonus pour les aventuriers qui l'utilisent (0 = pas d'effet direct)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    int32 StrengthBonus = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    int32 AgilityBonus = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    int32 IntelligenceBonus = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    int32 EnduranceBonus = 0;

    // Pour les potions : points de vie restaurés
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
    int32 HealAmount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
    TSoftObjectPtr<UTexture2D> Icon;            // Chargé seulement quand affiché
};

// Un objet dans l'inventaire = définition + quantité actuelle
// FItemData dit "qu'est-ce qu'une épée longue"
// FInventorySlot dit "j'ai 3 épées longues dans mon armurerie"
USTRUCT(BlueprintType)
struct FInventorySlot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FName ItemID = NAME_None;                   // Référence à la DataTable

    UPROPERTY(BlueprintReadWrite)
    int32 Quantity = 0;
};


// ═════════════════════════════════════════════════════════════════════════════
//  2. AVENTURIERS
//  Le cœur du jeu. Chaque aventurier est une personne avec une vie.
// ═════════════════════════════════════════════════════════════════════════════

// Les classes d'aventuriers — impacte les stats de base et les missions adaptées
UENUM(BlueprintType)
enum class EAdventurerClass : uint8
{
    Warrior     UMETA(DisplayName = "Guerrier"),    // Fort, résistant, lent
    Ranger      UMETA(DisplayName = "Rôdeur"),      // Agile, bon en exploration
    Mage        UMETA(DisplayName = "Mage"),        // Intelligent, fragile
    Rogue       UMETA(DisplayName = "Voleur"),      // Discret, risqué
    Cleric      UMETA(DisplayName = "Clerc"),       // Soigne, soutien
};

// L'état de santé d'un aventurier
UENUM(BlueprintType)
enum class EAdventurerStatus : uint8
{
    Available   UMETA(DisplayName = "Disponible"),  // Prêt pour une mission
    OnMission   UMETA(DisplayName = "En mission"),  // Parti, pas disponible
    Resting     UMETA(DisplayName = "En repos"),    // Récupère (blessé ou fatigué)
    Injured     UMETA(DisplayName = "Blessé"),      // Blessé, hors service X jours
    Dead        UMETA(DisplayName = "Mort"),        // Définitif. Toujours.
};

// Les traits de caractère — ce qui rend chaque aventurier unique
// Chaque trait a des effets positifs ou négatifs sur les missions et la vie en guilde
UENUM(BlueprintType)
enum class EAdventurerTrait : uint8
{
    // Traits positifs
    Brave           UMETA(DisplayName = "Courageux"),       // Meilleures perfs en danger
    Loyal           UMETA(DisplayName = "Loyal"),           // Ne déserte jamais
    Veteran         UMETA(DisplayName = "Vétéran"),         // Expérience compense stats
    FastLearner     UMETA(DisplayName = "Vif d'esprit"),    // Progresse plus vite
    TeamPlayer      UMETA(DisplayName = "Esprit d'équipe"), // Bonus en groupe

    // Traits négatifs
    Alcoholic       UMETA(DisplayName = "Alcoolique"),      // Besoin de taverne
    Greedy          UMETA(DisplayName = "Avide"),           // Demande des augmentations
    Traumatized     UMETA(DisplayName = "Traumatisé"),      // Craque sur certaines missions
    Gambler         UMETA(DisplayName = "Joueur"),          // Peut perdre son salaire
    Arrogant        UMETA(DisplayName = "Arrogant"),        // Conflits avec les autres

    // Traits mixtes
    Reckless        UMETA(DisplayName = "Téméraire"),       // Prend des risques, + butin ou mort
    Loner           UMETA(DisplayName = "Solitaire"),       // Mieux seul, pénalité en groupe
    Ambitious       UMETA(DisplayName = "Ambitieux"),       // Veut progresser vite ou part
};

// FAdventurerData = la fiche complète d'un aventurier vivant
// Pas dans une DataTable — c'est une instance unique générée procéduralement
USTRUCT(BlueprintType)
struct FAdventurerData
{
    GENERATED_BODY()

    // ── Identité ─────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadWrite)
    FGuid UniqueID;                             // ID unique — ne change jamais

    UPROPERTY(BlueprintReadWrite)
    FText FirstName;

    UPROPERTY(BlueprintReadWrite)
    FText LastName;

    UPROPERTY(BlueprintReadWrite)
    EAdventurerClass Class = EAdventurerClass::Warrior;

    UPROPERTY(BlueprintReadWrite)
    FText BackStory;                            // Une courte histoire générée

    // ── Stats (1-100) ─────────────────────────────────────────────────────────
    // Ces stats déterminent les résultats des missions
    // Elles progressent lentement avec l'expérience
    UPROPERTY(BlueprintReadWrite)
    int32 Strength = 50;        // Combat au corps à corps, porter du lourd

    UPROPERTY(BlueprintReadWrite)
    int32 Agility = 50;         // Esquive, discrétion, exploration

    UPROPERTY(BlueprintReadWrite)
    int32 Intelligence = 50;    // Magie, déchiffrer des indices, pièges

    UPROPERTY(BlueprintReadWrite)
    int32 Endurance = 50;       // Résistance aux blessures, longues missions

    UPROPERTY(BlueprintReadWrite)
    int32 Charisma = 50;        // Négociation, moral de l'équipe, contacts

    // ── État actuel ───────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadWrite)
    EAdventurerStatus Status = EAdventurerStatus::Available;

    UPROPERTY(BlueprintReadWrite)
    int32 CurrentMorale = 75;   // 0-100 : bas = mauvaises perfs, très bas = désertion

    UPROPERTY(BlueprintReadWrite)
    int32 Loyalty = 50;         // 0-100 : loyauté envers la guilde

    UPROPERTY(BlueprintReadWrite)
    int32 Fatigue = 0;          // 0-100 : monte avec les missions, descend au repos

    UPROPERTY(BlueprintReadWrite)
    int32 InjuryDaysRemaining = 0;  // Jours avant de pouvoir repartir en mission

    // ── Traits ───────────────────────────────────────────────────────────────
    // Un aventurier a 2 à 4 traits tirés aléatoirement à la création
    UPROPERTY(BlueprintReadWrite)
    TArray<EAdventurerTrait> Traits;

    // ── Équipement ────────────────────────────────────────────────────────────
    // IDs des objets équipés (référence à FItemData dans la DataTable)
    UPROPERTY(BlueprintReadWrite)
    FName EquippedWeapon = NAME_None;

    UPROPERTY(BlueprintReadWrite)
    FName EquippedArmor = NAME_None;

    UPROPERTY(BlueprintReadWrite)
    TArray<FName> EquippedItems;    // Potions, accessoires portés en mission

    // ── Économie ──────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadWrite)
    int32 DailyWage = 10;           // Salaire par jour en pièces d'or

    UPROPERTY(BlueprintReadWrite)
    int32 MissionsCompleted = 0;    // Historique — les joueurs s'attachent aux vétérans

    UPROPERTY(BlueprintReadWrite)
    int32 MissionsFailed = 0;

    // ── Mission en cours ──────────────────────────────────────────────────────
    // Si l'aventurier est en mission, on stocke l'ID du contrat
    UPROPERTY(BlueprintReadWrite)
    FGuid CurrentContractID;        // Invalide si pas en mission

    // ── Utilitaires ───────────────────────────────────────────────────────────

    // Retourne le nom complet — utile pour l'affichage
    FText GetFullName() const
    {
        return FText::FromString(FirstName.ToString() + " " + LastName.ToString());
    }

    // Vérifie si l'aventurier a un trait particulier
    bool HasTrait(EAdventurerTrait Trait) const
    {
        return Traits.Contains(Trait);
    }

    // Score global de combat — utilisé pour calculer les résultats de mission
    // Chaque classe pondère les stats différemment
    int32 GetCombatScore() const
    {
        switch (Class)
        {
        case EAdventurerClass::Warrior:
            return (Strength * 3 + Endurance * 2 + Agility) / 6;
        case EAdventurerClass::Ranger:
            return (Agility * 3 + Strength + Endurance * 2) / 6;
        case EAdventurerClass::Mage:
            return (Intelligence * 3 + Agility * 2 + Strength) / 6;
        case EAdventurerClass::Rogue:
            return (Agility * 3 + Intelligence * 2 + Strength) / 6;
        case EAdventurerClass::Cleric:
            return (Intelligence * 2 + Endurance * 2 + Charisma * 2) / 6;
        default:
            return (Strength + Agility + Intelligence + Endurance) / 4;
        }
    }
};


// ═════════════════════════════════════════════════════════════════════════════
//  3. CONTRATS & MISSIONS
// ═════════════════════════════════════════════════════════════════════════════

// Le type de mission — détermine quelles stats sont importantes
UENUM(BlueprintType)
enum class EMissionType : uint8
{
    Combat      UMETA(DisplayName = "Combat"),          // Éliminer des ennemis
    Escort      UMETA(DisplayName = "Escorte"),         // Protéger quelqu'un
    Exploration UMETA(DisplayName = "Exploration"),     // Explorer une zone
    Retrieval   UMETA(DisplayName = "Récupération"),    // Ramener un objet
    Espionage   UMETA(DisplayName = "Espionnage"),      // Discrétion requise
    Negotiation UMETA(DisplayName = "Négociation"),     // Charisme requis
};

// La difficulté — impacte les risques et les récompenses
UENUM(BlueprintType)
enum class EMissionDifficulty : uint8
{
    Trivial     UMETA(DisplayName = "Triviale"),        // Zéro risque, peu de récompense
    Easy        UMETA(DisplayName = "Facile"),
    Medium      UMETA(DisplayName = "Modérée"),
    Hard        UMETA(DisplayName = "Difficile"),
    Deadly      UMETA(DisplayName = "Mortelle"),        // Risque élevé, gros butin
};

// Le résultat d'une mission — calculé quand l'équipe rentre
UENUM(BlueprintType)
enum class EMissionOutcome : uint8
{
    Pending         UMETA(DisplayName = "En cours"),        // Pas encore terminée
    FullSuccess     UMETA(DisplayName = "Succès total"),    // Objectif + bonus
    PartialSuccess  UMETA(DisplayName = "Succès partiel"),  // Objectif atteint, pertes
    Failure         UMETA(DisplayName = "Échec"),           // Objectif raté, pertes
    Disaster        UMETA(DisplayName = "Désastre"),        // Tout le monde mort
};

// FContractData = un contrat affiché au tableau de la guilde
USTRUCT(BlueprintType)
struct FContractData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FGuid ContractID;                           // ID unique de ce contrat

    // ── Description narrative ─────────────────────────────────────────────────
    UPROPERTY(BlueprintReadWrite)
    FText Title;                                // "Nettoyer les Caves de Valdrun"

    UPROPERTY(BlueprintReadWrite)
    FText Description;                          // Texte narratif pour le joueur

    UPROPERTY(BlueprintReadWrite)
    FText ClientName;                           // Qui a posté ce contrat

    // ── Paramètres de mission ─────────────────────────────────────────────────
    UPROPERTY(BlueprintReadWrite)
    EMissionType Type = EMissionType::Combat;

    UPROPERTY(BlueprintReadWrite)
    EMissionDifficulty Difficulty = EMissionDifficulty::Medium;

    UPROPERTY(BlueprintReadWrite)
    int32 DurationDays = 2;                     // Combien de jours l'équipe sera absente

    UPROPERTY(BlueprintReadWrite)
    int32 MinAdventurers = 1;                   // Taille minimale de l'équipe

    UPROPERTY(BlueprintReadWrite)
    int32 MaxAdventurers = 4;                   // Taille maximale

    // ── Récompenses ───────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadWrite)
    int32 GoldReward = 100;                     // Or garanti en cas de succès

    UPROPERTY(BlueprintReadWrite)
    int32 ReputationReward = 5;                 // Points de réputation gagnés

    // Butin possible — objets que l'équipe peut ramener
    // Ce sont des probabilités, pas des garanties
    UPROPERTY(BlueprintReadWrite)
    TArray<FName> PossibleLootIDs;

    // ── État du contrat ───────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadWrite)
    bool bIsAccepted = false;                   // Le joueur a accepté ce contrat

    UPROPERTY(BlueprintReadWrite)
    bool bIsCompleted = false;

    UPROPERTY(BlueprintReadWrite)
    EMissionOutcome Outcome = EMissionOutcome::Pending;

    // IDs des aventuriers envoyés sur ce contrat
    UPROPERTY(BlueprintReadWrite)
    TArray<FGuid> AssignedAdventurerIDs;

    // Jour de départ et jour de retour (en numéro de jour de jeu)
    UPROPERTY(BlueprintReadWrite)
    int32 DepartureDay = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 ReturnDay = 0;

    // ── Rapport de mission ────────────────────────────────────────────────────
    // Généré quand l'équipe rentre — texte narratif de ce qui s'est passé
    UPROPERTY(BlueprintReadWrite)
    FText MissionReport;

    // Butin réellement rapporté
    UPROPERTY(BlueprintReadWrite)
    TArray<FInventorySlot> LootReturned;

    // IDs des aventuriers morts pendant la mission
    UPROPERTY(BlueprintReadWrite)
    TArray<FGuid> AdventurerDeathIDs;
};


// ═════════════════════════════════════════════════════════════════════════════
//  4. LA GUILDE — PIÈCES ET ÉTAT
// ═════════════════════════════════════════════════════════════════════════════

// Les pièces disponibles dans la guilde
UENUM(BlueprintType)
enum class ERoomType : uint8
{
    MainHall        UMETA(DisplayName = "Hall Principal"),
    Dormitory       UMETA(DisplayName = "Dortoir"),
    Armory          UMETA(DisplayName = "Armurerie"),
    Office          UMETA(DisplayName = "Bureau"),
    Tavern          UMETA(DisplayName = "Taverne"),
    PrivateRooms    UMETA(DisplayName = "Chambres Privées"),
    TrainingGround  UMETA(DisplayName = "Terrain d'entraînement"),
    Infirmary       UMETA(DisplayName = "Infirmerie"),
    SalesCounter    UMETA(DisplayName = "Comptoir de Vente"),
};

// L'état d'une pièce dans la guilde
USTRUCT(BlueprintType)
struct FRoomState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    ERoomType RoomType = ERoomType::MainHall;

    UPROPERTY(BlueprintReadWrite)
    bool bIsUnlocked = false;               // Le joueur a débloqué cette pièce

    UPROPERTY(BlueprintReadWrite)
    int32 Level = 1;                        // Niveau d'amélioration (1 à 3)

    // Score de décoration — impacte le moral et la réputation
    // Monte quand le joueur place des objets décoratifs dans la pièce
    UPROPERTY(BlueprintReadWrite)
    int32 DecorationScore = 0;

    // Condition de la pièce — descend avec le temps, nécessite entretien
    UPROPERTY(BlueprintReadWrite)
    float Condition = 1.0f;                 // 0.0 à 1.0 — en dessous de 0.3 : malus
};


// ═════════════════════════════════════════════════════════════════════════════
//  5. FACTIONS & RÉPUTATION
// ═════════════════════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class EFactionType : uint8
{
    Nobility    UMETA(DisplayName = "La Noblesse"),     // Contrats lucratifs, moralement douteux
    Merchants   UMETA(DisplayName = "Les Marchands"),   // Prix, fournisseurs, commerce
    Church      UMETA(DisplayName = "L'Église"),        // Soins, missions de protection
    Underworld  UMETA(DisplayName = "La Pègre"),        // Risqué, très rentable
};

USTRUCT(BlueprintType)
struct FFactionReputation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    EFactionType Faction = EFactionType::Merchants;

    // -100 (ennemi juré) à +100 (allié de confiance)
    // 0 = neutre, inconnu
    UPROPERTY(BlueprintReadWrite)
    int32 ReputationScore = 0;

    // Débloqué quand la réputation est assez haute avec cette faction
    UPROPERTY(BlueprintReadWrite)
    bool bIsAlly = false;

    UPROPERTY(BlueprintReadWrite)
    bool bIsEnemy = false;
};


// ═════════════════════════════════════════════════════════════════════════════
//  6. ÉVÉNEMENTS NARRATIFS
//  Les pauses avec choix qui ponctuent la gestion quotidienne
// ═════════════════════════════════════════════════════════════════════════════

// Un choix dans un événement narratif
USTRUCT(BlueprintType)
struct FEventChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText ChoiceText;                       // Le texte du choix affiché au joueur

    // Condition pour que ce choix soit disponible
    // Vide = toujours disponible
    // Exemple : "Charisma >= 60" — le choix n'apparaît que si stat suffisante
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText RequirementDescription;

    // Conséquences immédiates visibles par le joueur
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText ImmediateConsequenceText;

    // Effets concrets (valeurs modifiées dans le système)
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 GoldDelta = 0;                    // + ou - d'or

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 ReputationDelta = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MoraleDelta = 0;                  // Impact sur le moral général

    // Flag posé dans le World State pour les conséquences futures cachées
    // Exemple : "merchant_debt_unpaid" → dans 5 jours un événement se déclenche
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName WorldStateFlag = NAME_None;
};

// Un événement narratif complet
USTRUCT(BlueprintType)
struct FNarrativeEvent : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText NarrativeText;                    // Le texte de l'événement — ambiance, immersion

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FEventChoice> Choices;           // 2 à 4 choix possibles

    // Conditions de déclenchement
    // L'événement ne peut se déclencher que si ces conditions sont remplies
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MinDay = 1;                       // Pas avant ce jour de jeu

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MinAdventurers = 0;               // Pas sans ce nombre d'aventuriers

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bCanRepeat = false;                // Peut-il se déclencher plusieurs fois ?
};


// ═════════════════════════════════════════════════════════════════════════════
//  7. MARCHANDS
// ═════════════════════════════════════════════════════════════════════════════

UENUM(BlueprintType)
enum class EMerchantType : uint8
{
    Weaponsmith     UMETA(DisplayName = "Marchand d'armes"),
    Alchemist       UMETA(DisplayName = "Alchimiste"),
    Supplier        UMETA(DisplayName = "Fournisseur"),
    Fence           UMETA(DisplayName = "Receleur"),            // Achète tout, prix bas
    RareTraveler    UMETA(DisplayName = "Voyageur rare"),       // Objets uniques
};

USTRUCT(BlueprintType)
struct FMerchantData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText Name;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EMerchantType Type = EMerchantType::Supplier;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText Description;

    // Les IDs des objets qu'il vend (référence à FItemData)
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FName> ItemsForSale;

    // Modificateur de prix — 1.0 = prix normal, 1.3 = 30% plus cher
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float PriceMultiplier = 1.0f;

    // Réputation minimum pour qu'il accepte de commercer
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 RequiredReputationScore = 0;

    // Probabilité qu'il passe chaque jour (0.0 à 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float DailyAppearanceChance = 0.3f;
};


// ═════════════════════════════════════════════════════════════════════════════
//  8. ÉTAT GLOBAL DU JEU
//  Tout ce qui doit persister entre les sessions
// ═════════════════════════════════════════════════════════════════════════════

USTRUCT(BlueprintType)
struct FGuildState
{
    GENERATED_BODY()

    // ── Identité de la guilde ─────────────────────────────────────────────────
    UPROPERTY(BlueprintReadWrite)
    FText GuildName = FText::FromString("Guilde sans nom");

    UPROPERTY(BlueprintReadWrite)
    int32 CurrentDay = 1;

    // ── Finances ──────────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadWrite)
    int32 Gold = 150;                       // L'or du joueur au démarrage

    UPROPERTY(BlueprintReadWrite)
    int32 TotalEarned = 0;                  // Historique — revenus cumulés

    UPROPERTY(BlueprintReadWrite)
    int32 TotalSpent = 0;                   // Historique — dépenses cumulées

    // ── Réputation globale ────────────────────────────────────────────────────
    // Score général de réputation de la guilde dans la ville
    UPROPERTY(BlueprintReadWrite)
    int32 GlobalReputation = 0;             // -100 à +100

    // Réputation avec chaque faction
    UPROPERTY(BlueprintReadWrite)
    TArray<FFactionReputation> FactionReputations;

    // ── Statistiques ──────────────────────────────────────────────────────────
    UPROPERTY(BlueprintReadWrite)
    int32 TotalMissionsCompleted = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 TotalAdventurerDeaths = 0;        // Le livre des morts

    // ── World State Flags ─────────────────────────────────────────────────────
    // Les flags posés par les événements narratifs
    // Clé = nom du flag, Valeur = jour où il a été posé
    // Exemple : "merchant_debt_unpaid" → jour 5
    UPROPERTY(BlueprintReadWrite)
    TMap<FName, int32> WorldStateFlags;

    // ── Aventuriers décédés ───────────────────────────────────────────────────
    // On les garde en mémoire pour le registre des morts dans le bureau
    UPROPERTY(BlueprintReadWrite)
    TArray<FAdventurerData> DeceasedAdventurers;
};


// ═════════════════════════════════════════════════════════════════════════════
//  9. DELEGATES — LE SYSTÈME NERVEUX
//  Permettent aux différents systèmes de communiquer sans se connaître
// ═════════════════════════════════════════════════════════════════════════════

// Un nouveau jour commence
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewDay, int32, DayNumber);

// L'or du joueur a changé
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGoldChanged, int32, NewAmount);

// Un contrat vient d'être résolu (équipe rentrée)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnContractResolved, FContractData, Contract);

// Un aventurier est mort
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAdventurerDied, FAdventurerData, Adventurer);

// Un événement narratif doit s'afficher
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNarrativeEvent, FNarrativeEvent, Event);

// La réputation a changé
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReputationChanged, EFactionType, Faction, int32, NewScore);