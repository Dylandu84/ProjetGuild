#include "GuildManager.h"
#include "Engine/GameInstance.h"

// ─────────────────────────────────────────────────────────────────────────────
//  INITIALIZE
//  UE5 appelle cette fonction automatiquement au démarrage du jeu.
//  C'est ici qu'on prépare l'état initial de la guilde.
// ─────────────────────────────────────────────────────────────────────────────
void UGuildManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // État de départ de la guilde
    GuildState.GuildName = FText::FromString("Guilde sans nom");
    GuildState.CurrentDay = 1;
    GuildState.Gold = 150;

    // Initialise la réputation à 0 pour chaque faction
    // On crée une entrée pour chaque faction dès le départ
    // pour éviter de vérifier "est-ce que cette clé existe ?" plus tard
    TArray<EFactionType> AllFactions = {
        EFactionType::Nobility,
        EFactionType::Merchants,
        EFactionType::Church,
        EFactionType::Underworld
    };

    for (EFactionType Faction : AllFactions)
    {
        FFactionReputation Rep;
        Rep.Faction = Faction;
        Rep.ReputationScore = 0;
        Rep.bIsAlly = false;
        Rep.bIsEnemy = false;
        GuildState.FactionReputations.Add(Rep);
    }

    UE_LOG(LogTemp, Log, TEXT("[GuildManager] Initialisé — Jour %d, Or: %d"),
        GuildState.CurrentDay, GuildState.Gold);
}

// ─────────────────────────────────────────────────────────────────────────────
//  DEINITIALIZE
//  Appelé quand le jeu se ferme. On nettoie proprement.
// ─────────────────────────────────────────────────────────────────────────────
void UGuildManager::Deinitialize()
{
    Super::Deinitialize();
    UE_LOG(LogTemp, Log, TEXT("[GuildManager] Arrêt propre."));
}

// ─────────────────────────────────────────────────────────────────────────────
//  GET — ACCÈS GLOBAL
//  Permet d'appeler UGuildManager::Get(this) depuis n'importe où.
// ─────────────────────────────────────────────────────────────────────────────
UGuildManager* UGuildManager::Get(const UObject* WorldContext)
{
    if (!IsValid(WorldContext)) return nullptr;

    UWorld* World = WorldContext->GetWorld();
    if (!IsValid(World)) return nullptr;

    UGameInstance* GI = World->GetGameInstance();
    if (!IsValid(GI)) return nullptr;

    return GI->GetSubsystem<UGuildManager>();
}

// ─────────────────────────────────────────────────────────────────────────────
//  GESTION DE L'OR
// ─────────────────────────────────────────────────────────────────────────────
void UGuildManager::AddGold(int32 Amount)
{
    GuildState.Gold += Amount;
    GuildState.TotalEarned += Amount;

    // On notifie l'UI — elle mettra à jour l'affichage automatiquement
    OnGoldChanged.Broadcast(GuildState.Gold);

    UE_LOG(LogTemp, Log, TEXT("[GuildManager] +%d or. Total: %d"),
        Amount, GuildState.Gold);
}

bool UGuildManager::SpendGold(int32 Amount)
{
    // Règle d'or : on ne dépense jamais ce qu'on n'a pas
    if (GuildState.Gold < Amount)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[GuildManager] Or insuffisant. Demandé: %d, Disponible: %d"),
            Amount, GuildState.Gold);
        return false;
    }

    GuildState.Gold -= Amount;
    GuildState.TotalSpent += Amount;

    OnGoldChanged.Broadcast(GuildState.Gold);
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  AVANCER D'UN JOUR
//  C'est la fonction centrale du jeu — appelée quand le joueur va dormir.
//  L'ordre des opérations est important :
//  1. Résoudre les contrats terminés (l'or rentre)
//  2. Payer les salaires (l'or sort)
//  3. Incrémenter le jour
//  4. Notifier tout le monde
// ─────────────────────────────────────────────────────────────────────────────
void UGuildManager::AdvanceDay()
{
    UE_LOG(LogTemp, Log, TEXT("[GuildManager] ═══ FIN DU JOUR %d ═══"),
        GuildState.CurrentDay);

    // 1. Les équipes qui devaient rentrer aujourd'hui rentrent
    ResolveFinishedContracts();

    // 2. On paie tout le monde
    ProcessDailyWages();

    // 3. Le jour avance
    GuildState.CurrentDay++;

    // 4. On informe tout le monde qu'un nouveau jour commence
    OnNewDay.Broadcast(GuildState.CurrentDay);

    UE_LOG(LogTemp, Log, TEXT("[GuildManager] ═══ DÉBUT DU JOUR %d — Or: %d ═══"),
        GuildState.CurrentDay, GuildState.Gold);
}

// ─────────────────────────────────────────────────────────────────────────────
//  AVENTURIERS
// ─────────────────────────────────────────────────────────────────────────────
void UGuildManager::RecruitAdventurer(FAdventurerData Adventurer)
{
    // On génère un ID unique si ce n'est pas déjà fait
    if (!Adventurer.UniqueID.IsValid())
    {
        Adventurer.UniqueID = FGuid::NewGuid();
    }

    Adventurer.Status = EAdventurerStatus::Available;
    Adventurers.Add(Adventurer);

    OnAdventurerCountChanged.Broadcast(Adventurers.Num());

    UE_LOG(LogTemp, Log, TEXT("[GuildManager] Recruté: %s"),
        *Adventurer.GetFullName().ToString());
}

TArray<FAdventurerData> UGuildManager::GetAvailableAdventurers() const
{
    TArray<FAdventurerData> Available;
    for (const FAdventurerData& Adv : Adventurers)
    {
        // Disponible = vivant et pas en mission ou blessé
        if (Adv.Status == EAdventurerStatus::Available)
        {
            Available.Add(Adv);
        }
    }
    return Available;
}

// ─────────────────────────────────────────────────────────────────────────────
//  CONTRATS
// ─────────────────────────────────────────────────────────────────────────────
void UGuildManager::AddContract(FContractData Contract)
{
    if (!Contract.ContractID.IsValid())
    {
        Contract.ContractID = FGuid::NewGuid();
    }

    Contracts.Add(Contract);

    UE_LOG(LogTemp, Log, TEXT("[GuildManager] Nouveau contrat: %s"),
        *Contract.Title.ToString());
}

bool UGuildManager::AcceptContract(FGuid ContractID, TArray<FGuid> AdventurerIDs)
{
    // Trouve le contrat
    FContractData* Contract = nullptr;
    for (FContractData& C : Contracts)
    {
        if (C.ContractID == ContractID)
        {
            Contract = &C;
            break;
        }
    }

    if (!Contract)
    {
        UE_LOG(LogTemp, Warning, TEXT("[GuildManager] Contrat introuvable"));
        return false;
    }

    // Vérifie la taille de l'équipe
    if (AdventurerIDs.Num() < Contract->MinAdventurers ||
        AdventurerIDs.Num() > Contract->MaxAdventurers)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[GuildManager] Taille d'équipe incorrecte pour ce contrat"));
        return false;
    }

    // Marque les aventuriers comme partis en mission
    for (FGuid ID : AdventurerIDs)
    {
        if (FAdventurerData* Adv = FindAdventurer(ID))
        {
            Adv->Status = EAdventurerStatus::OnMission;
            Adv->CurrentContractID = ContractID;
        }
    }

    // Configure le contrat
    Contract->bIsAccepted = true;
    Contract->AssignedAdventurerIDs = AdventurerIDs;
    Contract->DepartureDay = GuildState.CurrentDay;
    Contract->ReturnDay = GuildState.CurrentDay + Contract->DurationDays;

    UE_LOG(LogTemp, Log,
        TEXT("[GuildManager] Contrat '%s' accepté — retour jour %d"),
        *Contract->Title.ToString(), Contract->ReturnDay);

    return true;
}

TArray<FContractData> UGuildManager::GetActiveContracts() const
{
    TArray<FContractData> Active;
    for (const FContractData& C : Contracts)
    {
        if (C.bIsAccepted && !C.bIsCompleted)
        {
            Active.Add(C);
        }
    }
    return Active;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RÉPUTATION
// ─────────────────────────────────────────────────────────────────────────────
void UGuildManager::ModifyReputation(EFactionType Faction, int32 Delta)
{
    for (FFactionReputation& Rep : GuildState.FactionReputations)
    {
        if (Rep.Faction == Faction)
        {
            Rep.ReputationScore = FMath::Clamp(
                Rep.ReputationScore + Delta, -100, 100);

            Rep.bIsAlly = Rep.ReputationScore >= 50;
            Rep.bIsEnemy = Rep.ReputationScore <= -50;

            UE_LOG(LogTemp, Log,
                TEXT("[GuildManager] Réputation faction %d : %d"),
                (int32)Faction, Rep.ReputationScore);
            return;
        }
    }
}

int32 UGuildManager::GetReputation(EFactionType Faction) const
{
    for (const FFactionReputation& Rep : GuildState.FactionReputations)
    {
        if (Rep.Faction == Faction)
        {
            return Rep.ReputationScore;
        }
    }
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  MÉTHODES PRIVÉES
// ─────────────────────────────────────────────────────────────────────────────
void UGuildManager::ResolveFinishedContracts()
{
    for (FContractData& Contract : Contracts)
    {
        // Ce contrat doit-il se terminer aujourd'hui ?
        if (!Contract.bIsAccepted || Contract.bIsCompleted) continue;
        if (Contract.ReturnDay != GuildState.CurrentDay) continue;

        // Calcule le résultat
        Contract.Outcome = CalculateMissionOutcome(Contract);
        Contract.bIsCompleted = true;

        // Succès — on encaisse la récompense
        if (Contract.Outcome == EMissionOutcome::FullSuccess ||
            Contract.Outcome == EMissionOutcome::PartialSuccess)
        {
            AddGold(Contract.GoldReward);
            GuildState.TotalMissionsCompleted++;
            ModifyReputation(EFactionType::Merchants, Contract.ReputationReward);
        }

        // Libère les aventuriers survivants
        for (FGuid AdvID : Contract.AssignedAdventurerIDs)
        {
            // Vérifie si cet aventurier est mort pendant la mission
            bool bDied = Contract.AdventurerDeathIDs.Contains(AdvID);

            if (FAdventurerData* Adv = FindAdventurer(AdvID))
            {
                if (bDied)
                {
                    // Mort permanente — on le déplace dans le registre
                    Adv->Status = EAdventurerStatus::Dead;
                    GuildState.DeceasedAdventurers.Add(*Adv);
                    GuildState.TotalAdventurerDeaths++;
                    OnAdventurerDied.Broadcast(*Adv);

                    UE_LOG(LogTemp, Warning,
                        TEXT("[GuildManager] ✝ %s est mort en mission."),
                        *Adv->GetFullName().ToString());
                }
                else
                {
                    Adv->Status = EAdventurerStatus::Available;
                    Adv->CurrentContractID = FGuid();
                    Adv->MissionsCompleted++;
                    Adv->Fatigue = FMath::Min(100,
                        Adv->Fatigue + Contract.DurationDays * 10);
                }
            }
        }

        // Retire les morts de la liste active
        Adventurers.RemoveAll([](const FAdventurerData& A)
            {
                return A.Status == EAdventurerStatus::Dead;
            });

        OnContractResolved.Broadcast(Contract);

        UE_LOG(LogTemp, Log,
            TEXT("[GuildManager] Contrat '%s' résolu — Résultat: %d"),
            *Contract.Title.ToString(), (int32)Contract.Outcome);
    }
}

void UGuildManager::ProcessDailyWages()
{
    int32 TotalWages = 0;
    for (const FAdventurerData& Adv : Adventurers)
    {
        // On paie même les aventuriers en mission
        if (Adv.Status != EAdventurerStatus::Dead)
        {
            TotalWages += Adv.DailyWage;
        }
    }

    if (TotalWages > 0)
    {
        if (!SpendGold(TotalWages))
        {
            // Pas assez d'or pour payer — moral en baisse
            UE_LOG(LogTemp, Warning,
                TEXT("[GuildManager] Salaires impayés ! (%d or manquant)"),
                TotalWages - GuildState.Gold);

            // Baisse le moral de tous les aventuriers présents
            for (FAdventurerData& Adv : Adventurers)
            {
                Adv.CurrentMorale = FMath::Max(0, Adv.CurrentMorale - 15);
                Adv.Loyalty = FMath::Max(0, Adv.Loyalty - 5);
            }
        }
        else
        {
            UE_LOG(LogTemp, Log,
                TEXT("[GuildManager] Salaires payés: %d or"), TotalWages);
        }
    }
}

EMissionOutcome UGuildManager::CalculateMissionOutcome(const FContractData& Contract)
{
    if (Contract.AssignedAdventurerIDs.Num() == 0)
        return EMissionOutcome::Failure;

    // Calcule le score moyen de l'équipe
    float TotalScore = 0.f;
    int32 ValidMembers = 0;

    for (FGuid AdvID : Contract.AssignedAdventurerIDs)
    {
        if (const FAdventurerData* Adv = FindAdventurer(AdvID))
        {
            TotalScore += Adv->GetCombatScore();
            ValidMembers++;
        }
    }

    if (ValidMembers == 0) return EMissionOutcome::Failure;

    float AverageScore = TotalScore / ValidMembers;

    // Seuils de difficulté — plus la mission est difficile,
    // plus le score requis est élevé
    float DifficultyModifier = 1.0f;
    switch (Contract.Difficulty)
    {
    case EMissionDifficulty::Trivial:  DifficultyModifier = 0.5f;  break;
    case EMissionDifficulty::Easy:     DifficultyModifier = 0.7f;  break;
    case EMissionDifficulty::Medium:   DifficultyModifier = 1.0f;  break;
    case EMissionDifficulty::Hard:     DifficultyModifier = 1.4f;  break;
    case EMissionDifficulty::Deadly:   DifficultyModifier = 1.8f;  break;
    }

    // Score effectif = score de l'équipe vs difficulté
    // + facteur aléatoire (10-20%) pour l'imprévu
    float RandomFactor = FMath::RandRange(0.85f, 1.15f);
    float EffectiveScore = (AverageScore * RandomFactor) / DifficultyModifier;

    // Résultat selon le score effectif
    if (EffectiveScore >= 80.f) return EMissionOutcome::FullSuccess;
    if (EffectiveScore >= 55.f) return EMissionOutcome::PartialSuccess;
    if (EffectiveScore >= 30.f) return EMissionOutcome::Failure;
    return EMissionOutcome::Disaster;
}

FAdventurerData* UGuildManager::FindAdventurer(FGuid AdventurerID)
{
    for (FAdventurerData& Adv : Adventurers)
    {
        if (Adv.UniqueID == AdventurerID)
        {
            return &Adv;
        }
    }
    return nullptr;
}

void UGuildManager::AddResourceToInventory(EItemCategory ResourceType, int32 Amount)
{
    if (int32* Current = ResourceInventory.Find(ResourceType))
    {
        *Current += Amount;
    }
    else
    {
        ResourceInventory.Add(ResourceType, Amount);
    }

    UE_LOG(LogTemp, Log, TEXT("[GuildManager] +%d ressource type %d. Total: %d"),
        Amount, (int32)ResourceType, ResourceInventory.FindRef(ResourceType));
}

int32 UGuildManager::GetResourceAmount(EItemCategory ResourceType) const
{
    return ResourceInventory.FindRef(ResourceType);
}