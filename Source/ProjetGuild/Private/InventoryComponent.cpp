#include "InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    

    Slots.Empty();
    Slots.SetNum(SlotCount);

    UE_LOG(LogTemp, Log, TEXT("[Inventory] SlotCount: %d, Slots.Num(): %d"),
        SlotCount, Slots.Num());

    UE_LOG(LogTemp, Log, TEXT("[Inventory] Initialisé avec %d slots"), SlotCount);
}

// ─────────────────────────────────────────────────────────────────────────────
//  ADD ITEM
//  Ajoute un item dans l'inventaire.
//  Essaie d'abord de stacker sur un slot existant.
//  Si plein, cherche un slot vide.
//  Retourne la quantité qui n'a pas pu être ajoutée (0 = tout ajouté).
// ─────────────────────────────────────────────────────────────────────────────
int32 UInventoryComponent::AddItem(FName ItemID, int32 Quantity)
{
    
    UE_LOG(LogTemp, Log, TEXT("[Inventory] AddItem: ID='%s', Quantity=%d"), *ItemID.ToString(), Quantity);
    if (ItemID == NAME_None || Quantity <= 0) return Quantity;

    int32 Remaining = Quantity;

    // Essaie de stacker sur les slots existants du même item
    for (FInventoryItem& Slot : Slots)
    {
        if (Slot.ItemID != ItemID) continue;
        if (Remaining <= 0) break;

        int32 CanAdd = MaxStackSize - Slot.Quantity;
        if (CanAdd <= 0) continue;

        int32 Added = FMath::Min(CanAdd, Remaining);
        Slot.Quantity += Added;
        Remaining -= Added;
    }

    // Si il reste des items, cherche des slots vides
    while (Remaining > 0)
    {
        int32 EmptySlot = FindEmptySlot();
        if (EmptySlot == -1)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Inventory] Inventaire plein !"));
            break;
        }

        int32 Added = FMath::Min(MaxStackSize, Remaining);
        Slots[EmptySlot].ItemID = ItemID;
        Slots[EmptySlot].Quantity = Added;
        Remaining -= Added;
    }

    if (Remaining < Quantity)
    {
        OnInventoryChanged.Broadcast();
        UE_LOG(LogTemp, Log, TEXT("[Inventory] +%d %s"), Quantity - Remaining, *ItemID.ToString());
    }

    return Remaining;
}

// ─────────────────────────────────────────────────────────────────────────────
//  REMOVE ITEM
//  Retire une quantité d'un item de l'inventaire.
//  Retourne false si pas assez.
// ─────────────────────────────────────────────────────────────────────────────
bool UInventoryComponent::RemoveItem(FName ItemID, int32 Quantity)
{
    if (!HasItem(ItemID, Quantity)) return false;

    int32 Remaining = Quantity;

    for (FInventoryItem& Slot : Slots)
    {
        if (Slot.ItemID != ItemID) continue;
        if (Remaining <= 0) break;

        int32 Removed = FMath::Min(Slot.Quantity, Remaining);
        Slot.Quantity -= Removed;
        Remaining -= Removed;

        // Vide le slot si quantité = 0
        if (Slot.Quantity <= 0)
        {
            Slot.ItemID = NAME_None;
            Slot.Quantity = 0;
        }
    }

    OnInventoryChanged.Broadcast();
    UE_LOG(LogTemp, Log, TEXT("[Inventory] -%d %s"), Quantity, *ItemID.ToString());
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  GET ITEM COUNT
// ─────────────────────────────────────────────────────────────────────────────
int32 UInventoryComponent::GetItemCount(FName ItemID) const
{
    int32 Total = 0;
    for (const FInventoryItem& Slot : Slots)
    {
        if (Slot.ItemID == ItemID)
            Total += Slot.Quantity;
    }
    return Total;
}

bool UInventoryComponent::HasItem(FName ItemID, int32 Quantity) const
{
    return GetItemCount(ItemID) >= Quantity;
}

// ─────────────────────────────────────────────────────────────────────────────
//  MOVE ITEM
//  Déplace un item d'un slot à un autre.
//  Si le slot de destination a le même item — on stacke.
//  Sinon on échange les deux slots.
// ─────────────────────────────────────────────────────────────────────────────
bool UInventoryComponent::MoveItem(int32 FromSlot, int32 ToSlot)
{
    if (!Slots.IsValidIndex(FromSlot) || !Slots.IsValidIndex(ToSlot)) return false;
    if (Slots[FromSlot].IsEmpty()) return false;

    FInventoryItem& From = Slots[FromSlot];
    FInventoryItem& To = Slots[ToSlot];

    // Même item — on stacke
    if (To.ItemID == From.ItemID)
    {
        int32 CanAdd = MaxStackSize - To.Quantity;
        int32 Added = FMath::Min(CanAdd, From.Quantity);
        To.Quantity += Added;
        From.Quantity -= Added;

        if (From.Quantity <= 0)
        {
            From.ItemID = NAME_None;
            From.Quantity = 0;
        }
    }
    else
    {
        // Échange les deux slots
        FInventoryItem Temp = From;
        From = To;
        To = Temp;
    }

    OnInventoryChanged.Broadcast();
    return true;
}

void UInventoryComponent::ClearSlot(int32 SlotIndex)
{
    if (!Slots.IsValidIndex(SlotIndex)) return;
    Slots[SlotIndex].ItemID = NAME_None;
    Slots[SlotIndex].Quantity = 0;
    OnInventoryChanged.Broadcast();
}

FInventoryItem UInventoryComponent::GetSlot(int32 SlotIndex) const
{
    if (!Slots.IsValidIndex(SlotIndex)) return FInventoryItem();
    return Slots[SlotIndex];
}

bool UInventoryComponent::IsFull() const
{
    return FindEmptySlot() == -1;
}

int32 UInventoryComponent::FindItemSlot(FName ItemID) const
{
    for (int32 i = 0; i < Slots.Num(); i++)
    {
        if (Slots[i].ItemID == ItemID && Slots[i].Quantity < MaxStackSize)
            return i;
    }
    return -1;
}

int32 UInventoryComponent::FindEmptySlot() const
{
    for (int32 i = 0; i < Slots.Num(); i++)
    {
        if (Slots[i].IsEmpty()) return i;
    }
    return -1;
}