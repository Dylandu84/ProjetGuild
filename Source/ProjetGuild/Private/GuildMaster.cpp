#include "GuildMaster.h"
#include "BuildingPiece.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

AGuildMaster::AGuildMaster()
{
    PrimaryActorTick.bCanEverTick = true;

    // Spring Arm — longueur 0 pour vue FPS pure
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(GetRootComponent());
    SpringArm->TargetArmLength = 0.f;
    SpringArm->bUsePawnControlRotation = true;
    SpringArm->SetRelativeLocation(FVector(0.f, 0.f, 70.f)); // Hauteur des yeux

    // Caméra attachée au SpringArm
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);
    Camera->bUsePawnControlRotation = false;

    // Rotation FPS — le joueur tourne avec la caméra
    bUseControllerRotationPitch = true;
    bUseControllerRotationYaw = true;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->MaxWalkSpeed = 300.f;

    // Inventaire du joueur
    InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
}

void AGuildMaster::BeginPlay()
{
    Super::BeginPlay();

    // Initialise la vie au maximum
    CurrentHealth = MaxHealth;

    // Notifie le HUD de la vie initiale
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

    // Ajoute le contexte d'input au joueur
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(MappingContext, 0);
        }
    }
}

void AGuildMaster::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AGuildMaster::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGuildMaster::Move);
        EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGuildMaster::Look);
        EIC->BindAction(OpenRecruitmentAction, ETriggerEvent::Started, this, &AGuildMaster::OpenRecruitment);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  TAKE DAMAGE — override UE5, appelé par ApplyDamage() ou les projectiles
// ─────────────────────────────────────────────────────────────────────────────
float AGuildMaster::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    ApplyDamage(DamageAmount);

    return DamageAmount;
}

// ─────────────────────────────────────────────────────────────────────────────
//  APPLY DAMAGE — réduit la vie et notifie le HUD
// ─────────────────────────────────────────────────────────────────────────────
void AGuildMaster::ApplyDamage(float Amount)
{
    if (IsDead()) return;

    // Réduit la vie sans descendre sous 0
    CurrentHealth = FMath::Max(0.f, CurrentHealth - Amount);

    UE_LOG(LogTemp, Log, TEXT("[GuildMaster] Dégâts : -%.0f | Vie : %.0f/%.0f"),
        Amount, CurrentHealth, MaxHealth);

    // Notifie le HUD
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

    // Vérifie si le joueur est mort
    if (IsDead())
    {
        Die();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  HEAL — soigne le joueur sans dépasser MaxHealth
// ─────────────────────────────────────────────────────────────────────────────
void AGuildMaster::Heal(float Amount)
{
    if (IsDead()) return;

    CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + Amount);

    UE_LOG(LogTemp, Log, TEXT("[GuildMaster] Soin : +%.0f | Vie : %.0f/%.0f"),
        Amount, CurrentHealth, MaxHealth);

    // Notifie le HUD
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

// ─────────────────────────────────────────────────────────────────────────────
//  DIE — appelé quand la vie tombe à 0
// ─────────────────────────────────────────────────────────────────────────────
void AGuildMaster::Die()
{
    UE_LOG(LogTemp, Warning, TEXT("[GuildMaster] Le joueur est mort !"));

    // Notifie le Blueprint pour l'écran de mort
    OnDeath();
}

void AGuildMaster::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    AddMovementInput(GetActorForwardVector(), MovementVector.Y);
    AddMovementInput(GetActorRightVector(), MovementVector.X);
}

void AGuildMaster::Look(const FInputActionValue& Value)
{
    FVector2D LookVector = Value.Get<FVector2D>();
    AddControllerYawInput(LookVector.X);
    AddControllerPitchInput(LookVector.Y);
}

void AGuildMaster::OpenRecruitment(const FInputActionValue& Value)
{
    // Géré dans le Blueprint
}

// ─────────────────────────────────────────────────────────────────────────────
//  FIND NEAREST SNAP POINT — VERSION 3
// ─────────────────────────────────────────────────────────────────────────────
bool AGuildMaster::FindNearestSnapPoint(FVector CurrentLocation, FVector& OutSnapLocation, FRotator& OutSnapRotation)
{
    const float DetectionRadius = 600.f;

    if (!CurrentGhost) return false;
    if (CurrentGhost->SnapPoints.IsEmpty()) return false;

    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(
        GetWorld(), ABuildingPiece::StaticClass(), NearbyActors);

    float NearestDistance = DetectionRadius;
    bool bFoundSnap = false;

    for (AActor* Actor : NearbyActors)
    {
        ABuildingPiece* Piece = Cast<ABuildingPiece>(Actor);
        if (!Piece || Piece->bIsGhost) continue;

        float DistanceToPiece = FVector::Dist(CurrentLocation, Piece->GetActorLocation());
        if (DistanceToPiece > DetectionRadius) continue;

        for (const FSnapPointData& PieceSP : Piece->SnapPoints)
        {
            FVector PieceSnapWorld = Piece->GetActorTransform()
                .TransformPosition(PieceSP.LocalOffset);

            const FSnapPointData* CompatibleGhostSP = nullptr;
            float NearestGhostSnapDist = FLT_MAX;

            for (const FSnapPointData& GhostSP : CurrentGhost->SnapPoints)
            {
                if (!PieceSP.CompatibleTypes.Contains(GhostSP.SnapType)) continue;

                FVector GhostSnapWorld = CurrentGhost->GetActorTransform()
                    .TransformPosition(GhostSP.LocalOffset);

                float Dist = FVector::Dist(PieceSnapWorld, GhostSnapWorld);
                if (Dist < NearestGhostSnapDist)
                {
                    NearestGhostSnapDist = Dist;
                    CompatibleGhostSP = &GhostSP;
                }
            }

            if (!CompatibleGhostSP) continue;

            FVector GhostSnapOffsetZ = FVector(0, 0, CompatibleGhostSP->LocalOffset.Z);
            FVector CandidateLocation = PieceSnapWorld - GhostSnapOffsetZ;

            float Distance = FVector::Dist(CurrentLocation, CandidateLocation);

            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                OutSnapLocation = CandidateLocation;
                OutSnapRotation = Piece->GetActorRotation() + PieceSP.SnapRotation;
                bFoundSnap = true;
            }
        }
    }

    return bFoundSnap;
}

// ─────────────────────────────────────────────────────────────────────────────
//  GET GROUND POSITION — line trace vers le sol
// ─────────────────────────────────────────────────────────────────────────────
FVector AGuildMaster::GetGroundPosition(FVector StartLocation, AActor* IgnoredActor)
{
    FVector Start = StartLocation + FVector(0, 0, 1000.f);
    FVector End = StartLocation - FVector(0, 0, 1000.f);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    if (IgnoredActor)
    {
        Params.AddIgnoredActor(IgnoredActor);
    }

    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    {
        return Hit.ImpactPoint;
    }

    return StartLocation;
}