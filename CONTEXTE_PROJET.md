# CONTEXTE PROJET — PROJETGUILD — MIS À JOUR
## À donner à Claude au début de chaque conversation

---

## QUI JE SUIS
- Développeur débutant en C++ avec bases d'Unreal Engine
- Ma femme est artiste 3D — elle s'occupe des assets visuels
- On utilisera des assets du Marketplace/Fab pour les décors
- Moteur : Unreal Engine 5.6.1
- IDE : Visual Studio 2022
- GitHub : https://github.com/Dylandu84/ProjetGuild

---

## LE JEU — GUILD SIMULATOR

**Concept :**
Simulateur de guilde médiévale dark fantasy en vue première personne.
Inspiré de Graveyard Keeper + Supermarket Simulator + Darkest Dungeon.
Tu hérites d'une guilde en ruines. Tu recrutes des aventuriers, gères
les finances, décore ton espace, envoies des équipes en mission et
vis les conséquences dans un monde qui réagit à chaque décision.

**Structure des journées (comme Supermarket Simulator) :**
- Matin : consulter contrats, assigner équipes
- Journée : clients, fournisseurs, événements
- Soir : retour aventuriers, soins, comptes
- Nuit : écran bilan, nouveau jour

**Règles de game design :**
- Vue FPS — on vit dans la guilde, pas de vue du dessus
- Mort des aventuriers = définitive et permanente
- Missions de 1 à 7 jours selon difficulté
- Butin des missions = revendable aux marchands
- Pas de multijoueur
- Pas de monde ouvert

---

## FICHIERS C++ EXISTANTS

### GuildTypes.h (Public/)
Fichier de définition de toutes les données du jeu.
Contient :
- EItemCategory, EItemRarity, FItemData, FInventorySlot
- EAdventurerClass, EAdventurerStatus, EAdventurerTrait, FAdventurerData
- EMissionType, EMissionDifficulty, EMissionOutcome, FContractData
- ERoomType, FRoomState
- EFactionType, FFactionReputation
- FEventChoice, FNarrativeEvent
- EMerchantType, FMerchantData
- FGuildState
- Delegates : FOnNewDay, FOnGoldChanged, FOnContractResolved,
              FOnAdventurerDied, FOnNarrativeEvent, FOnReputationChanged

### GuildManager.h / GuildManager.cpp (Public/ et Private/)
UGameInstanceSubsystem — cerveau central du jeu.
Fonctions implémentées et testées :
- Initialize() / Deinitialize()
- Get() — accès global depuis n'importe où
- AddGold() / SpendGold() / GetGold()
- AdvanceDay() — résout contrats + paie salaires + incrémente jour
- RecruitAdventurer() / GetAvailableAdventurers() / GetAdventurerCount()
- AddContract() / AcceptContract() / GetActiveContracts()
- ModifyReputation() / GetReputation()
- Privées : ResolveFinishedContracts(), ProcessDailyWages(),
            CalculateMissionOutcome(), FindAdventurer()

### GuildMaster.h / GuildMaster.cpp (Public/ et Private/)
ACharacter — le personnage joueur en vue FPS.
Contient :
- SpringArm (longueur 0 = FPS pur)
- UCameraComponent attachée au SpringArm
- Mouvement : MoveForward, MoveRight, LookUp, LookRight
- Input bindé sur axes : MoveForward, MoveRight, LookUp, LookRight

---

## BLUEPRINTS EXISTANTS

### BP_GuildTest
Blueprint de test dans Content/BP/.
Branche : BeginPlay → GetGuildManager → AddGold(100) → PrintString(GetGold())
Résultat vérifié : affiche 250 (150 départ + 100 ajoutés) ✅
Test AdvanceDay() vérifié : affiche jour 2 ✅

### BP_GuildMaster
Blueprint du personnage joueur basé sur AGuildMaster.
À configurer avec le bon mesh et les axes d'input.

---

## ÉTAT ACTUEL DU PROJET

### Systèmes terminés et testés ✅
- GuildTypes.h — toutes les données définies
- GuildManager — or, journées, aventuriers, contrats, réputation
- GuildMaster — personnage FPS de base
- GitHub configuré et à jour

### En cours ⚙️
- Configuration du personnage FPS dans UE5
  - Axes d'input à configurer (MoveForward, MoveRight, LookUp, LookRight)
  - Game Mode BP_GuildGameMode à créer avec Default Pawn = BP_GuildMaster
  - World Settings du niveau MainWorld à configurer

### À faire ensuite 📋
1. Tester le déplacement FPS dans la scène de test
2. Créer le HUD de base (or, jour, aventuriers)
3. Système de génération procédurale d'aventuriers
4. Panneau de recrutement UI
5. Système de décoration FPS (placer des objets)
6. Marchands qui passent à la guilde
7. Événements narratifs avec choix

---

## RÈGLES DE TRAVAIL AVEC CLAUDE

- Toujours donner le .h ET le .cpp ensemble
- Toujours compiler depuis UE5 (Tools → Compile), pas depuis Visual Studio
- Ignorer les erreurs IntelliSense en rouge dans VS — seul UE5 dit la vérité
- Tester chaque système dès qu'il est codé
- Mettre à jour ce fichier et GitHub à la fin de chaque session

---

## COMMANDES GIT DE FIN DE SESSION

```bash
git add -A
git commit -m "Description de ce qu'on a fait"
git push origin main
```
