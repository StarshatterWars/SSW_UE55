#include "TacRefDlg.h"

#include "MenuScreen.h"
#include "GameStructs.h"
#include "GameStructs_System.h"

// UMG:
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "Components/ComboBoxString.h"

DEFINE_LOG_CATEGORY_STATIC(LogTacRefDlg, Log, All);

// ------------------------------------------------------------
// Minimal weapon grouping helpers (optional)
// ------------------------------------------------------------
static FString NormalizeWeaponGroupLabel(const FShipWeapon& W)
{
    if (!W.GroupName.IsEmpty())
        return W.GroupName.ToUpper();

    const FString Type = W.DesignName.ToLower();
    const FString Wep = W.WeaponType.ToLower();

    if (Type.Contains(TEXT("beam")) || Wep.Contains(TEXT("laser")))
        return TEXT("LASER");

    if (Type.Contains(TEXT("missile")) || Wep.Contains(TEXT("mk")) || Wep.Contains(TEXT("nike")))
        return TEXT("MISSILE");

    if (Type.Contains(TEXT("bolt")) || Wep.Contains(TEXT("phalanx")) || Wep.Contains(TEXT("vanguard")))
        return TEXT("PDB");

    return TEXT("WEAPON");
}

// ------------------------------------------------------------
// NEW: classification derived from ShipClass string
// No DataTable/DEF changes needed.
// ------------------------------------------------------------
static bool ContainsAnyLower(const FString& Lower, std::initializer_list<const TCHAR*> Keys)
{
    for (const TCHAR* K : Keys)
    {
        if (Lower.Contains(K))
            return true;
    }
    return false;
}

static bool IsFighterClass(const FString& ShipClass)
{
    const FString C = ShipClass.TrimStartAndEnd().ToLower();
    return ContainsAnyLower(C, { TEXT("fighter"), TEXT("interceptor"), TEXT("bomber"), TEXT("strike") });
}

static bool IsStationClass(const FString& ShipClass)
{
    const FString C = ShipClass.TrimStartAndEnd().ToLower();
    return ContainsAnyLower(C, { TEXT("station"), TEXT("platform"), TEXT("outpost"), TEXT("base") });
}

// ------------------------------------------------------------

UTacRefDlg::UTacRefDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    Mode = 0;
    SelectedShipIndex = INDEX_NONE;
}

void UTacRefDlg::SetMenuManager(UMenuScreen* InManager)
{
    manager = InManager;
}

void UTacRefDlg::InitializeDlg(UMenuScreen* InManager)
{
    manager = InManager;

    UE_LOG(LogTacRefDlg, Warning, TEXT("[TacRefDlg] InitializeDlg: manager=%s ShipDesignTable=%s ShipCombo=%s"),
        manager ? TEXT("VALID") : TEXT("NULL"),
        ShipDesignTable ? TEXT("VALID") : TEXT("NULL"),
        ShipCombo ? TEXT("VALID") : TEXT("NULL"));
}

void UTacRefDlg::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // Buttons (your WBP names)
    if (CancelButton)   CancelButton->OnClicked.AddDynamic(this, &UTacRefDlg::HandleCloseClicked);

    if (StationButton)  StationButton->OnClicked.AddDynamic(this, &UTacRefDlg::HandleStationModeClicked);
    if (ShipButton)     ShipButton->OnClicked.AddDynamic(this, &UTacRefDlg::HandleShipModeClicked);
    if (FighterButton)  FighterButton->OnClicked.AddDynamic(this, &UTacRefDlg::HandleFighterModeClicked);
    if (WeaponButton)   WeaponButton->OnClicked.AddDynamic(this, &UTacRefDlg::HandleWeaponModeClicked);

    // Dropdown
    if (ShipCombo)
        ShipCombo->OnSelectionChanged.AddDynamic(this, &UTacRefDlg::HandleShipComboChanged);
}

void UTacRefDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    ExecFrame();
}

void UTacRefDlg::Show()
{
    UE_LOG(LogTacRefDlg, Warning, TEXT("[TacRefDlg] Show: BEGIN"));

    Mode = 0; // ships by default

    PopulateShipDropdown();

    if (SelectedShipIndex == INDEX_NONE && ShipRowNames.Num() > 0)
        SelectedShipIndex = 0;

    SelectShipByIndex(SelectedShipIndex);

    UE_LOG(LogTacRefDlg, Warning, TEXT("[TacRefDlg] Show: DONE ships=%d sel=%d"), ShipRowNames.Num(), SelectedShipIndex);
}

void UTacRefDlg::ExecFrame()
{
    // Per-frame logic optional
}

// ------------------------------------------------------------
// NEW: filter predicate used by PopulateShipDropdown()
// Mode meanings:
// 0=ship, 1=weapon, 2=fighter, 3=station
// ------------------------------------------------------------
bool UTacRefDlg::PassShipFilter(const FShipDesign& Row) const
{
    // Weapon mode doesn't display ships (we'll still let ship list show ships if you want,
    // but per your request, Weapon button should NOT show ships).
    // We'll handle weapon mode by just changing the text panel for now (since your weapon mode isn't wired).
    if (Mode == 1)
        return false;

    const bool bIsFighter = IsFighterClass(Row.ShipClass);
    const bool bIsStation = IsStationClass(Row.ShipClass);

    if (Mode == 2) // fighters only
        return bIsFighter;

    if (Mode == 3) // stations only
        return bIsStation;

    // Mode 0 (ships): ships only = NOT fighter AND NOT station
    return (!bIsFighter && !bIsStation);
}

// ------------------------------------------------------------
// Dropdown population (MODIFIED to apply filter, otherwise same)
// ------------------------------------------------------------

void UTacRefDlg::PopulateShipDropdown()
{
    ShipRowNames.Reset();

    if (!ShipCombo)
    {
        UE_LOG(LogTacRefDlg, Error, TEXT("[TacRefDlg] PopulateShipDropdown: ShipCombo is NULL (BindWidget failed)"));
        return;
    }

    ShipCombo->ClearOptions();

    if (!ShipDesignTable)
    {
        UE_LOG(LogTacRefDlg, Error, TEXT("[TacRefDlg] PopulateShipDropdown: ShipDesignTable is NULL"));
        return;
    }

    TArray<FName> RowNames = ShipDesignTable->GetRowNames();
    if (RowNames.Num() == 0)
    {
        UE_LOG(LogTacRefDlg, Warning, TEXT("[TacRefDlg] PopulateShipDropdown: no rows"));
        return;
    }

    // Sort by DisplayName (unchanged)
    RowNames.Sort([this](const FName& A, const FName& B)
        {
            const FShipDesign* RA = ShipDesignTable->FindRow<FShipDesign>(A, TEXT("Sort"), false);
            const FShipDesign* RB = ShipDesignTable->FindRow<FShipDesign>(B, TEXT("Sort"), false);

            const FString SA = RA ? RA->DisplayName : A.ToString();
            const FString SB = RB ? RB->DisplayName : B.ToString();
            return SA < SB;
        });

    for (const FName& RowName : RowNames)
    {
        const FShipDesign* Row = ShipDesignTable->FindRow<FShipDesign>(RowName, TEXT("PopulateShipDropdown"), false);
        if (!Row)
            continue;

        // NEW: apply filter based on Mode and ShipClass
        if (!PassShipFilter(*Row))
            continue;

        // Label format: "CR COURAGEOUS"
        const FString Label =
            Row->Abrv.IsEmpty()
            ? Row->DisplayName
            : FString::Printf(TEXT("%s %s"), *Row->Abrv, *Row->DisplayName);

        ShipCombo->AddOption(Label);
        ShipRowNames.Add(RowName);
    }

    if (ShipRowNames.Num() > 0)
    {
        // If current index is out of range, snap to 0
        if (!ShipRowNames.IsValidIndex(SelectedShipIndex))
            SelectedShipIndex = 0;

        ShipCombo->SetSelectedIndex(SelectedShipIndex);
    }
    else
    {
        SelectedShipIndex = INDEX_NONE;
    }

    UE_LOG(LogTacRefDlg, Log, TEXT("[TacRefDlg] PopulateShipDropdown: %d entries (Mode=%d)"), ShipRowNames.Num(), Mode);
}

void UTacRefDlg::HandleShipComboChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    (void)SelectedItem;
    (void)SelectionType;

    if (!ShipCombo)
        return;

    const FString Current = ShipCombo->GetSelectedOption();
    const int32 Index = ShipCombo->FindOptionIndex(Current);

    if (Index == INDEX_NONE)
        return;

    SelectedShipIndex = Index;

    // In weapon mode, we don't drive ship selection (since it is not wired)
    if (Mode != 1)
        SelectShipByIndex(Index);
}

void UTacRefDlg::SelectShipByIndex(int32 Index)
{
    if (!ShipDesignTable)
        return;

    if (!ShipRowNames.IsValidIndex(Index))
        return;

    const FName RowName = ShipRowNames[Index];
    const FShipDesign* Row = ShipDesignTable->FindRow<FShipDesign>(RowName, TEXT("SelectShipByIndex"), false);
    if (!Row)
        return;

    FString Caption, Stats, Desc;
    BuildShipTexts(*Row, Caption, Stats, Desc);

    if (TxtCaption)     TxtCaption->SetText(FText::FromString(Caption));
    if (TxtStats)       TxtStats->SetText(FText::FromString(Stats));
    if (TxtDescription) TxtDescription->SetText(FText::FromString(Desc));
}

void UTacRefDlg::BuildShipTexts(const FShipDesign& Dsn, FString& OutCaption, FString& OutStats, FString& OutDesc) const
{
    OutCaption = Dsn.Abrv.IsEmpty()
        ? Dsn.DisplayName
        : FString::Printf(TEXT("%s %s"), *Dsn.Abrv, *Dsn.DisplayName);

    OutDesc = !Dsn.Description.IsEmpty() ? Dsn.Description : TEXT("NO DESCRIPTION AVAILABLE.");

    auto Num0 = [](double V) { return FString::Printf(TEXT("%.0f"), V); };
    auto Num1 = [](double V) { return FString::Printf(TEXT("%.1f"), V); };

    FString S;
    S += FString::Printf(TEXT("CLASS:\t%s\n"), *Dsn.ShipClass);
    S += FString::Printf(TEXT("MASS:\t%s\n"), *Num0(Dsn.Mass));
    S += FString::Printf(TEXT("SCALE:\t%s\n"), *Num1(Dsn.Scale));
    S += FString::Printf(TEXT("V LIMIT:\t%s\n"), *Num0(Dsn.Vlimit));
    S += FString::Printf(TEXT("AGILITY:\t%s\n"), *Num1(Dsn.Agility));
    S += FString::Printf(TEXT("DETECT:\t%s\n"), *Num0(Dsn.Detet));
    S += FString::Printf(TEXT("REPAIR TEAMS:\t%d\n"), Dsn.RepairTeams);

    // Weapons summary (optional)
    if (Dsn.Weapon.Num() > 0)
    {
        TMap<FString, int32> GroupCounts;

        for (const FShipWeapon& W : Dsn.Weapon)
        {
            const FString G = NormalizeWeaponGroupLabel(W);
            GroupCounts.FindOrAdd(G)++;
        }

        S += TEXT("\nWEAPONS:\n");
        for (const auto& KVP : GroupCounts)
        {
            S += FString::Printf(TEXT("%s:\t%d\n"), *KVP.Key, KVP.Value);
        }
    }

    OutStats = MoveTemp(S);
}

// ------------------------------------------------------------
// Mode buttons (MODIFIED: ship/fighter/station all repopulate)
// ------------------------------------------------------------

void UTacRefDlg::HandleStationModeClicked()
{
    Mode = 3; // station
    SelectedShipIndex = 0;
    PopulateShipDropdown();

    if (ShipRowNames.Num() > 0)
        SelectShipByIndex(0);
    else
    {
        if (TxtCaption)     TxtCaption->SetText(FText::FromString(TEXT("STATION REFERENCE")));
        if (TxtStats)       TxtStats->SetText(FText::FromString(TEXT("NO STATIONS FOUND.")));
        if (TxtDescription) TxtDescription->SetText(FText::GetEmpty());
    }
}

void UTacRefDlg::HandleShipModeClicked()
{
    Mode = 0; // ships
    SelectedShipIndex = 0;
    PopulateShipDropdown();

    if (ShipRowNames.Num() > 0)
        SelectShipByIndex(0);
    else
    {
        if (TxtCaption)     TxtCaption->SetText(FText::FromString(TEXT("SHIP REFERENCE")));
        if (TxtStats)       TxtStats->SetText(FText::FromString(TEXT("NO SHIPS FOUND.")));
        if (TxtDescription) TxtDescription->SetText(FText::GetEmpty());
    }
}

void UTacRefDlg::HandleFighterModeClicked()
{
    Mode = 2; // fighters
    SelectedShipIndex = 0;
    PopulateShipDropdown();

    if (ShipRowNames.Num() > 0)
        SelectShipByIndex(0);
    else
    {
        if (TxtCaption)     TxtCaption->SetText(FText::FromString(TEXT("FIGHTER REFERENCE")));
        if (TxtStats)       TxtStats->SetText(FText::FromString(TEXT("NO FIGHTERS FOUND.")));
        if (TxtDescription) TxtDescription->SetText(FText::GetEmpty());
    }
}

void UTacRefDlg::HandleWeaponModeClicked()
{
    Mode = 1; // weapon

    // Per your request: Weapon button should list weapons only.
    // Your weapon mode isn't wired yet; we keep behavior stable and simply:
    // - clear the dropdown
    // - show a placeholder in the panel
    if (ShipCombo)
        ShipCombo->ClearOptions();

    ShipRowNames.Reset();
    SelectedShipIndex = INDEX_NONE;

    if (TxtCaption)     TxtCaption->SetText(FText::FromString(TEXT("WEAPON REFERENCE")));
    if (TxtStats)       TxtStats->SetText(FText::FromString(TEXT("WEAPON DROPDOWN NOT WIRED YET.")));
    if (TxtDescription) TxtDescription->SetText(FText::GetEmpty());
}

// ------------------------------------------------------------

void UTacRefDlg::HandleCloseClicked()
{
    if (manager)
    {
        manager->ShowMenuDlg();
        return;
    }

    RemoveFromParent();
}
