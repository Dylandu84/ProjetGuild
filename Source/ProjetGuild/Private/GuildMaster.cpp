#include "GuildMaster.h"
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