#include "GuildMaster.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// ─────────────────────────────────────────────────────────────────────────────
//  CONSTRUCTEUR
//  Appelé une seule fois quand la classe est créée.
//  On y configure les composants — caméra, mouvement, etc.
// ─────────────────────────────────────────────────────────────────────────────
AGuildMaster::AGuildMaster()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Spring Arm ────────────────────────────────────────────────────────
    // Le Spring Arm est un bras invisible qui tient la caméra.
    // En FPS on le met à la position de la tête avec une longueur de 0
    // pour que la caméra soit exactement aux yeux du personnage.
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(GetMesh(), FName("head"));
    SpringArm->TargetArmLength = 0.f;          // FPS = longueur 0
    SpringArm->bUsePawnControlRotation = true;  // La caméra suit la souris

    // ── Caméra ────────────────────────────────────────────────────────────
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);
    Camera->bUsePawnControlRotation = false;    // Le SpringArm gère déjà ça

    // ── Mouvement ─────────────────────────────────────────────────────────
    // On désactive la rotation automatique vers la direction de mouvement
    // En FPS le personnage tourne avec la souris, pas avec le déplacement
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = true;         // Gauche/droite suit la souris
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->MaxWalkSpeed = 300.f;   // Vitesse de marche
}

// ─────────────────────────────────────────────────────────────────────────────
//  BEGIN PLAY
//  Appelé quand le jeu démarre. On cache le mesh du personnage
//  car en FPS on ne voit pas son propre corps.
// ─────────────────────────────────────────────────────────────────────────────
void AGuildMaster::BeginPlay()
{
    Super::BeginPlay();

    // Cache le mesh en FPS — on ne voit pas son propre corps
    if (GetMesh())
    {
        GetMesh()->SetOwnerNoSee(true);
    }
}

void AGuildMaster::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// ─────────────────────────────────────────────────────────────────────────────
//  INPUT
//  On lie les touches aux fonctions de mouvement.
//  "MoveForward" et "MoveRight" sont des noms d'axes qu'on va
//  configurer dans Project Settings → Input.
// ─────────────────────────────────────────────────────────────────────────────
void AGuildMaster::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Axes de mouvement
    PlayerInputComponent->BindAxis("MoveForward", this, &AGuildMaster::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AGuildMaster::MoveRight);

    // Axes de caméra (souris)
    PlayerInputComponent->BindAxis("LookUp", this, &AGuildMaster::LookUp);
    PlayerInputComponent->BindAxis("LookRight", this, &AGuildMaster::LookRight);
}

// ─────────────────────────────────────────────────────────────────────────────
//  FONCTIONS DE MOUVEMENT
// ─────────────────────────────────────────────────────────────────────────────
void AGuildMaster::MoveForward(float Value)
{
    if (Value == 0.f) return;

    // GetActorForwardVector() = la direction vers laquelle le personnage regarde
    // On ajoute un mouvement dans cette direction selon la valeur de l'axe
    // Value > 0 = avancer, Value < 0 = reculer
    AddMovementInput(GetActorForwardVector(), Value);
}

void AGuildMaster::MoveRight(float Value)
{
    if (Value == 0.f) return;

    // GetActorRightVector() = la direction à droite du personnage
    AddMovementInput(GetActorRightVector(), Value);
}

void AGuildMaster::LookUp(float Value)
{
    // AddControllerPitchInput = rotation verticale (haut/bas)
    AddControllerPitchInput(Value);
}

void AGuildMaster::LookRight(float Value)
{
    // AddControllerYawInput = rotation horizontale (gauche/droite)
    AddControllerYawInput(Value);
}