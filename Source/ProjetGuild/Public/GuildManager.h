#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GuildTypes.h"
#include "GuildManager.generated.h"

UCLASS()
class PROJETGUILD_API UGuildManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:

    // ── CYCLE DE VIE ─────────────────────────────────────────────────────
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ── ACCÈS GLOBAL ─────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Guild",
        meta = (WorldContext = "WorldContext"))
    static UGuildManager* Get(const UObject* WorldContext);

    // ── OR ────────────────────────────────────────────────────────────────
    // Ajoute de l'or et notifie les abonnés
    UFUNCTION(BlueprintCallable, Category = "Guild|Economy")
    void AddGold(int32 Amount);

    // Retire de l'or — retourne false si pas assez
    UFUNCTION(BlueprintCallable, Category = "Guild|Economy")
    bool SpendGold(int32 Amount);

    // Lecture seule de l'or actuel
    UFUNCTION(BlueprintPure, Category = "Guild|Economy")
    int32 GetGold() const { return GuildState.Gold; }

    // ── JOURNÉE ───────────────────────────────────────────────────────────
    // Passe au jour suivant — appelé par le joueur quand il va dormir
    UFUNCTION(BlueprintCallable, Category = "Guild|Day")
    void AdvanceDay();

    // Numéro du jour actuel
    UFUNCTION(BlueprintPure, Category = "Guild|Day")
    int32 GetCurrentDay() const { return GuildState.CurrentDay; }

    // ── AVENTURIERS ───────────────────────────────────────────────────────
    // Ajoute un aventurier à la guilde
    UFUNCTION(BlueprintCallable, Category = "Guild|Adventurers")
    void RecruitAdventurer(FAdventurerData Adventurer);

    // Récupère tous les aventuriers disponibles
    UFUNCTION(BlueprintPure, Category = "Guild|Adventurers")
    TArray<FAdventurerData> GetAvailableAdventurers() const;

    // Nombre total d'aventuriers vivants
    UFUNCTION(BlueprintPure, Category = "Guild|Adventurers")
    int32 GetAdventurerCount() const { return Adventurers.Num(); }

    // ── CONTRATS ──────────────────────────────────────────────────────────
    // Ajoute un contrat au tableau de la guilde
    UFUNCTION(BlueprintCallable, Category = "Guild|Contracts")
    void AddContract(FContractData Contract);

    // Accepte un contrat et assigne des aventuriers
    UFUNCTION(BlueprintCallable, Category = "Guild|Contracts")
    bool AcceptContract(FGuid ContractID, TArray<FGuid> AdventurerIDs);

    // Tous les contrats en cours
    UFUNCTION(BlueprintPure, Category = "Guild|Contracts")
    TArray<FContractData> GetActiveContracts() const;

    // ── RÉPUTATION ────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Guild|Reputation")
    void ModifyReputation(EFactionType Faction, int32 Delta);

    UFUNCTION(BlueprintPure, Category = "Guild|Reputation")
    int32 GetReputation(EFactionType Faction) const;

    // ── INVENTAIRE DE RESSOURCES ──────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Guild|Resources")
    void AddResourceToInventory(EItemCategory ResourceType, int32 Amount);

    UFUNCTION(BlueprintPure, Category = "Guild|Resources")
    int32 GetResourceAmount(EItemCategory ResourceType) const;
    
    // ── DELEGATES ─────────────────────────────────────────────────────────
    // L'UI s'abonne à ces events pour se mettre à jour automatiquement
    UPROPERTY(BlueprintAssignable, Category = "Guild|Events")
    FOnNewDay OnNewDay;

    UPROPERTY(BlueprintAssignable, Category = "Guild|Events")
    FOnGoldChanged OnGoldChanged;

    UPROPERTY(BlueprintAssignable, Category = "Guild|Events")
    FOnContractResolved OnContractResolved;

    UPROPERTY(BlueprintAssignable, Category = "Guild|Events")
    FOnAdventurerDied OnAdventurerDied;

    UPROPERTY(BlueprintAssignable, Category = "Guild|Events")
    FOnGoldChanged OnAdventurerCountChanged;

    UPROPERTY(BlueprintAssignable, Category = "Guild|Events")
    FOnGoldChanged OnResourceChanged;

    UFUNCTION(BlueprintCallable, Category = "Guild|Resources")
    bool SpendResourceFromInventory(EItemCategory ResourceType, int32 Amount);
private:

    // ── DONNÉES INTERNES ──────────────────────────────────────────────────
    // L'état global de la guilde — finances, jour, réputation
    UPROPERTY()
    FGuildState GuildState;

    // Tous les aventuriers vivants de la guilde
    UPROPERTY()
    TArray<FAdventurerData> Adventurers;

    // Tous les contrats — en attente, en cours, terminés
    UPROPERTY()
    TArray<FContractData> Contracts;

    // ── MÉTHODES PRIVÉES ──────────────────────────────────────────────────
    // Vérifie les contrats en cours et résout ceux qui sont terminés
    void ResolveFinishedContracts();

    // Paie les salaires de tous les aventuriers
    void ProcessDailyWages();

    // Calcule le résultat d'une mission selon les stats de l'équipe
    EMissionOutcome CalculateMissionOutcome(const FContractData& Contract);

    // Trouve un aventurier par son ID unique
    FAdventurerData* FindAdventurer(FGuid AdventurerID);

    UPROPERTY()
    TMap<EItemCategory, int32> ResourceInventory;
};