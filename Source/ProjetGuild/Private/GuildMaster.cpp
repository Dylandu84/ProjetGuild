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

    // Caméra
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);
    Camera->bUsePawnControlRotation = false;

    // Rotation FPS
    bUseControllerRotationPitch = true;
    bUseControllerRotationYaw = true;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->MaxWalkSpeed = 300.f;

    // Crée le composant inventaire
    InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
}

void AGuildMaster::BeginPlay()
{
    Super::BeginPlay();

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

    // Cast vers Enhanced Input Component
    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGuildMaster::Move);
        EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGuildMaster::Look);
        EIC->BindAction(OpenRecruitmentAction, ETriggerEvent::Started, this, &AGuildMaster::OpenRecruitment);
    }
}

void AGuildMaster::Move(const FInputActionValue& Value)
{
    // Value est un Vector2D — X = droite/gauche, Y = avant/arrière
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
    // On laisse le Blueprint gérer l'affichage du panneau
    // On utilise un BlueprintImplementableEvent pour ça
}

// ─────────────────────────────────────────────────────────────────────────────
//  FIND NEAREST SNAP POINT — VERSION 3
//  Utilise le système de types compatibles pour trouver
//  le meilleur point de snap entre le ghost et les pièces posées.
//  Trouve le snap point du ghost le plus proche du snap point de la pièce
//  pour que l'alignement soit correct.
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

            // Pour l'axe Z on applique l'offset du ghost
// pour que son snap point s'aligne avec celui de la pièce
            FVector GhostSnapOffsetZ = FVector(0, 0, CompatibleGhostSP->LocalOffset.Z);
            FVector CandidateLocation = PieceSnapWorld - GhostSnapOffsetZ;

            float Distance = FVector::Dist(CurrentLocation, CandidateLocation);

            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                OutSnapLocation = CandidateLocation;
                // Rotation de la pièce + rotation du snap point
                OutSnapRotation = Piece->GetActorRotation() + PieceSP.SnapRotation;
                bFoundSnap = true;
            }
        }
    }

    return bFoundSnap;
}

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