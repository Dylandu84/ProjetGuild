#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GuildTypes.h"
#include "InventoryComponent.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
//  INVENTORY COMPONENT
//  Composant attachable à n'importe quel Actor.
//  Gère un inventaire de slots avec items et quantités.
//  Utilisé par le joueur, les PNJ, les coffres, les marchands.
// ─────────────────────────────────────────────────────────────────────────────

// Un slot d'inventaire — un item avec sa quantité
USTRUCT(BlueprintType)
struct FInventoryItem
{
    GENERATED_BODY()

    // ID de l'item — référence à la DataTable des items
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FName ItemID = NAME_None;

    // Quantité dans ce slot
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 Quantity = 0;

    // Est-ce que ce slot est vide ?
    bool IsEmpty() const { return ItemID == NAME_None || Quantity <= 0; }
};

// Delegate — notifie quand l'inventaire change
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJETGUILD_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    UInventoryComponent();
    virtual void BeginPlay() override;

    // ── CONFIGURATION ─────────────────────────────────────────────────────
    // Nombre de slots dans cet inventaire
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    int32 SlotCount = 24;

    // ── DONNÉES ───────────────────────────────────────────────────────────
    // Les slots de l'inventaire
    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    TArray<FInventoryItem> Slots;

    // ── DELEGATE ──────────────────────────────────────────────────────────
    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryChanged OnInventoryChanged;

    // ── FONCTIONS PRINCIPALES ─────────────────────────────────────────────

    // Ajoute un item — retourne la quantité qui n'a pas pu être ajoutée
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 AddItem(FName ItemID, int32 Quantity);

    // Retire un item — retourne true si réussi
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItem(FName ItemID, int32 Quantity);

    // Combien d'un item on a en tout
    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetItemCount(FName ItemID) const;

    // Est-ce qu'on a assez d'un item
    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool HasItem(FName ItemID, int32 Quantity = 1) const;

    // Déplace un item d'un slot à un autre
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool MoveItem(int32 FromSlot, int32 ToSlot);

    // Vide un slot
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void ClearSlot(int32 SlotIndex);

    // Retourne les données d'un slot
    UFUNCTION(BlueprintPure, Category = "Inventory")
    FInventoryItem GetSlot(int32 SlotIndex) const;

    // Est-ce que l'inventaire est plein
    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool IsFull() const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetSlotCount() const { return Slots.Num(); }

private:

    // Trouve le premier slot avec cet item (pour stacker)
    int32 FindItemSlot(FName ItemID) const;

    // Trouve le premier slot vide
    int32 FindEmptySlot() const;

    // Taille max d'un stack — pour l'instant tous les items stackent à 99
    // Plus tard on lira ça depuis la DataTable
    const int32 MaxStackSize = 99;
};