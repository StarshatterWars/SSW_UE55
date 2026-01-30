/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    SUBSYSTEM:    Stars.exe
    FILE:         MissionWeaponDlg.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    MissionWeaponDlg (Unreal)
    - FORM-driven port of legacy MsnWepDlg.cpp
*/

#include "MissionWeaponDlg.h"

// Manager
#include "MissionPlanner.h"

// --- Sim includes (your ported equivalents) ---
#include "Campaign.h"
#include "Mission.h"
#include "MissionElement.h"
#include "ShipDesign.h"
#include "WeaponDesign.h"
#include "HardPoint.h"
#include "MissionLoad.h"
#include "ShipLoad.h"

// --- FORM/UI framework (your equivalents) ---
#include "FormLabel.h"
#include "FormButton.h"
#include "FormListBox.h"
#include "FormImageBox.h"

// --- Engine ---
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UMissionWeaponDlg::UMissionWeaponDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UMissionWeaponDlg::NativeConstruct()
{
    Super::NativeConstruct();

    // ------------------------------------------------------------
    // Initialize sim pointers (legacy: Campaign::GetCampaign())
    // ------------------------------------------------------------
    CampaignPtr = Campaign::GetCampaign();
    MissionPtr = CampaignPtr ? CampaignPtr->GetMission() : nullptr;

    // ------------------------------------------------------------
    // FORM init
    // NOTE: You likely do something like:
    //   InitFromForm("MsnWepDlg");
    // Keep it consistent with your UBaseScreen pattern.
    // ------------------------------------------------------------
    InitFromForm(TEXT("MsnWepDlg"));

    RegisterControls();
    RegisterEvents();

    // Start hidden by default; MissionPlanner will Show() when needed.
    Hide();
}

void UMissionWeaponDlg::NativeDestruct()
{
    Super::NativeDestruct();

    // Raw pointers are owned by widget tree; just null them.
    CampaignPtr = nullptr;
    MissionPtr = nullptr;
    PlayerElem = nullptr;
    Manager = nullptr;
}

void UMissionWeaponDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // ------------------------------------------------------------
    // Legacy behavior: if Enter pressed -> commit
    // ------------------------------------------------------------
    if (!bShown)
        return;

    APlayerController* PC = GetOwningPlayer();
    if (!PC)
        return;

    const bool bEnterDown =
        PC->IsInputKeyDown(EKeys::Enter) ||
        PC->IsInputKeyDown(EKeys::Virtual_Accept);

    if (bEnterDown && !bEnterWasDown)
    {
        OnCommit();
    }

    bEnterWasDown = bEnterDown;
}

bool UMissionWeaponDlg::IsShown() const
{
    return bShown;
}

void UMissionWeaponDlg::Show()
{
    bShown = true;
    SetVisibility(ESlateVisibility::Visible);

    // Legacy: ShowMsnDlg() (tabs + shared header already visible in parent)
    // Here we just refresh local view state:
    CampaignPtr = Campaign::GetCampaign();
    MissionPtr = CampaignPtr ? CampaignPtr->GetMission() : nullptr;

    FindPlayerElement();
    if (PlayerElem)
    {
        SetupControls();
    }
}

void UMissionWeaponDlg::Hide()
{
    bShown = false;
    SetVisibility(ESlateVisibility::Collapsed);
}

void UMissionWeaponDlg::RegisterControls()
{
    // ------------------------------------------------------------
    // Labels / list
    // ------------------------------------------------------------
    LblElement = Cast<UFormLabel>(FindControl(601));
    LblType = Cast<UFormLabel>(FindControl(602));
    LblWeight = Cast<UFormLabel>(FindControl(603));
    LoadoutList = Cast<UFormListBox>(FindControl(604));

    Beauty = Cast<UFormImageBox>(FindControl(300));
    PlayerDesc = Cast<UFormLabel>(FindControl(301));

    // ------------------------------------------------------------
    // Station headers 401..408
    // ------------------------------------------------------------
    for (int32 i = 0; i < 8; ++i)
    {
        LblStation[i] = Cast<UFormLabel>(FindControl(401 + i));
    }

    // ------------------------------------------------------------
    // Weapon row labels 500, 510, ... 570
    // ------------------------------------------------------------
    for (int32 i = 0; i < 8; ++i)
    {
        const int32 LabelId = 500 + i * 10;
        LblDesc[i] = Cast<UFormLabel>(FindControl(LabelId));
    }

    // ------------------------------------------------------------
    // Grid buttons 501..578 (row i, col n)
    // Legacy formula: 500 + i*10 + n + 1
    // ------------------------------------------------------------
    for (int32 i = 0; i < 8; ++i)
    {
        for (int32 n = 0; n < 8; ++n)
        {
            const int32 BtnId = 500 + i * 10 + n + 1;
            BtnLoad[i][n] = Cast<UFormButton>(FindControl(BtnId));
        }
    }
}

void UMissionWeaponDlg::RegisterEvents()
{
    // ------------------------------------------------------------
    // Loadout list selection changed
    // ------------------------------------------------------------
    if (LoadoutList)
    {
        LoadoutList->OnSelectionChanged.BindUObject(
            this, &UMissionWeaponDlg::OnLoadoutSelectionChanged
        );
    }

    // ------------------------------------------------------------
    // Grid clicks
    // ------------------------------------------------------------
    for (int32 i = 0; i < 8; ++i)
    {
        for (int32 n = 0; n < 8; ++n)
        {
            if (BtnLoad[i][n])
            {
                BtnLoad[i][n]->OnClicked.BindLambda([this, Btn = BtnLoad[i][n]]()
                    {
                        this->OnMount(Btn);
                    });

                // Default LED off (legacy sets picture to led_off)
                BtnLoad[i][n]->SetPictureByName(LedOffName);
                BtnLoad[i][n]->SetPictureLocationCentered();
            }
        }
    }

    // ------------------------------------------------------------
    // Commit/Cancel/Tab buttons are usually registered by your shared
    // MissionBriefing base dialog (like legacy RegisterMsnControls()).
    // If your UBaseScreen provides those, wire them there.
    // Here we assume you have IDs like legacy:
    //   commit id 1, cancel id 2, tab ids 900..903
    // ------------------------------------------------------------
    if (UFormButton* CommitBtn = Cast<UFormButton>(FindControl(1)))
    {
        CommitBtn->OnClicked.BindUObject(this, &UMissionWeaponDlg::OnCommit);
    }

    if (UFormButton* CancelBtn = Cast<UFormButton>(FindControl(2)))
    {
        CancelBtn->OnClicked.BindUObject(this, &UMissionWeaponDlg::OnCancel);
    }

    // Tabs: 900 SIT, 901 PKG, 902 MAP, 903 WEP
    for (int32 TabId : { 900, 901, 902, 903 })
    {
        if (UFormButton* TabBtn = Cast<UFormButton>(FindControl(TabId)))
        {
            TabBtn->OnClicked.BindLambda([this, TabId]()
                {
                    this->OnTabButton(TabId);
                });
        }
    }
}

void UMissionWeaponDlg::FindPlayerElement()
{
    PlayerElem = nullptr;

    if (!MissionPtr)
        return;

    const auto& Elements = MissionPtr->GetElements();
    for (MissionElement* E : Elements)
    {
        if (E && E->Player())
        {
            PlayerElem = E;
            break;
        }
    }
}

void UMissionWeaponDlg::SetupControls()
{
    if (!PlayerElem)
        return;

    ShipDesign* Design = (ShipDesign*)PlayerElem->GetDesign();
    if (!Design)
        return;

    // ------------------------------------------------------------
    // Top labels
    // ------------------------------------------------------------
    if (LblElement) LblElement->SetText(PlayerElem->Name());
    if (LblType)    LblType->SetText(Design->name);

    // ------------------------------------------------------------
    // Build design/mount matrix + station headers + Loads mapping
    // ------------------------------------------------------------
    BuildLists();

    // ------------------------------------------------------------
    // Fill weapon rows + show/hide grid cells
    // ------------------------------------------------------------
    for (int32 i = 0; i < 8; ++i)
    {
        if (!LblDesc[i])
            continue;

        if (Designs[i])
        {
            LblDesc[i]->Show();
            LblDesc[i]->SetText(Designs[i]->group + TEXT(" ") + Designs[i]->name);

            for (int32 n = 0; n < 8; ++n)
            {
                if (!BtnLoad[i][n]) continue;

                if (Mounts[i][n])
                {
                    BtnLoad[i][n]->Show();
                    const bool bSelected = (Loads[n] == i);
                    BtnLoad[i][n]->SetPictureByName(bSelected ? LedOnName : LedOffName);
                }
                else
                {
                    BtnLoad[i][n]->Hide();
                }
            }
        }
        else
        {
            LblDesc[i]->Hide();
            for (int32 n = 0; n < 8; ++n)
            {
                if (BtnLoad[i][n]) BtnLoad[i][n]->Hide();
            }
        }
    }

    // ------------------------------------------------------------
    // Standard loadouts list
    // ------------------------------------------------------------
    double LoadedMass = 0.0;

    if (LoadoutList)
    {
        LoadoutList->ClearItems();

        // Legacy iterates Design->loadouts
        const auto& Loadouts = Design->loadouts;

        for (const ShipLoad& Load : Loadouts)
        {
            const int32 Row = LoadoutList->AddItem(Load.name);

            const int32 Kg = (int32)((Design->mass + Load.mass) * 1000.0);
            LoadoutList->SetItemText(Row, 1, FString::Printf(TEXT("%d kg"), Kg));
            LoadoutList->SetItemData(Row, 1, (int32)(Load.mass * 1000.0));

            // Select current named loadout if matches
            if (PlayerElem->Loadouts().Num() > 0)
            {
                MissionLoad* ML = PlayerElem->Loadouts()[0];
                if (ML && ML->GetName().Len() > 0 && ML->GetName() == Load.name)
                {
                    LoadoutList->SetSelected(Row, true);
                    LoadedMass = Design->mass + Load.mass;
                }
            }
        }
    }

    // ------------------------------------------------------------
    // Weight label
    // ------------------------------------------------------------
    if (LblWeight)
    {
        if (LoadedMass < 1.0)
            LoadedMass = Design->mass;

        const int32 Kg = (int32)(LoadedMass * 1000.0);
        LblWeight->SetText(FString::Printf(TEXT("%d kg"), Kg));
    }

    // ------------------------------------------------------------
    // Optional beauty + player desc (commented out in FORM, but supported)
    // ------------------------------------------------------------
    if (Beauty && Design)
    {
        Beauty->SetPictureByName(Design->beauty);
    }

    if (PlayerDesc && Design)
    {
        FString Txt;
        if (Design->type <= Ship::ATTACK)
            Txt = FString::Printf(TEXT("%s %s"), *Design->abrv, *Design->display_name);
        else
            Txt = FString::Printf(TEXT("%s %s"), *Design->abrv, *PlayerElem->Name());

        PlayerDesc->SetText(Txt);
    }
}

void UMissionWeaponDlg::BuildLists()
{
    // Clear arrays
    for (int32 i = 0; i < 8; ++i)
    {
        Designs[i] = nullptr;
        for (int32 n = 0; n < 8; ++n)
            Mounts[i][n] = false;
    }

    for (int32 i = 0; i < 8; ++i)
        Loads[i] = -1;

    if (!PlayerElem)
        return;

    ShipDesign* D = (ShipDesign*)PlayerElem->GetDesign();
    if (!D)
        return;

    const int32 NStations = D->hard_points.Num();

    // Center stations like legacy: first_station = (8 - nstations) / 2;
    FirstStation = (8 - NStations) / 2;

    // Clear station header labels
    for (int32 s = 0; s < 8; ++s)
    {
        if (LblStation[s])
            LblStation[s]->SetText(TEXT(""));
    }

    // Build unique weapon design list + mount matrix
    int32 Index = 0;
    int32 Station = FirstStation;

    for (int32 HpIndex = 0; HpIndex < NStations && Station < 8; ++HpIndex, ++Station)
    {
        HardPoint* Hp = D->hard_points[HpIndex];
        if (!Hp)
            continue;

        if (LblStation[Station])
            LblStation[Station]->SetText(Hp->GetAbbreviation());

        for (int32 n = 0; n < HardPoint::MAX_DESIGNS; ++n)
        {
            WeaponDesign* W = Hp->GetWeaponDesign(n);
            if (!W)
                continue;

            bool bFound = false;
            for (int32 i = 0; i < 8 && !bFound; ++i)
            {
                if (Designs[i] == W)
                {
                    bFound = true;
                    Mounts[i][Station] = true;
                }
            }

            if (!bFound && Index < 8)
            {
                Designs[Index] = W;
                Mounts[Index][Station] = true;
                ++Index;
            }
        }
    }

    // Map existing mission loadout to Loads[] (station->designIdx)
    if (PlayerElem->Loadouts().Num() > 0)
    {
        MissionLoad* MsnLoad = PlayerElem->Loadouts()[0];
        if (!MsnLoad)
            return;

        int32* Loadout = nullptr;

        if (MsnLoad->GetName().Len() > 0)
        {
            // Named loadout from ship design
            for (ShipLoad& SL : D->loadouts)
            {
                if (SL.name.Equals(MsnLoad->GetName(), ESearchCase::IgnoreCase))
                {
                    Loadout = SL.load; // matches legacy int* load
                    break;
                }
            }
        }
        else
        {
            // Custom station array from mission load
            Loadout = MsnLoad->GetStations();
        }

        if (!Loadout)
            return;

        // Apply to centered stations (loads[i + first_station] = loadout[i];)
        for (int32 i = 0; i < NStations; ++i)
        {
            const int32 StationIdx = i + FirstStation;
            if (StationIdx >= 0 && StationIdx < 8)
                Loads[StationIdx] = Loadout[i];
        }
    }
}

int32 UMissionWeaponDlg::LoadToPointIndex(int32 HardPointIndex) const
{
    const int32 Station = HardPointIndex + FirstStation;

    if (!PlayerElem || Station < 0 || Station >= 8 || Loads[Station] == -1)
        return -1;

    WeaponDesign* WepDesign = Designs[Loads[Station]];
    ShipDesign* Design = (ShipDesign*)PlayerElem->GetDesign();
    if (!Design || !Design->hard_points.IsValidIndex(HardPointIndex))
        return -1;

    HardPoint* Hp = Design->hard_points[HardPointIndex];
    if (!Hp)
        return -1;

    for (int32 i = 0; i < 8; ++i)
    {
        if (Hp->GetWeaponDesign(i) == WepDesign)
            return i;
    }

    return -1;
}

int32 UMissionWeaponDlg::PointIndexToLoad(int32 HardPointIndex, int32 DesignIndexInHardPoint) const
{
    const int32 Station = HardPointIndex + FirstStation;

    if (!PlayerElem || Station < 0 || Station >= 8)
        return -1;

    ShipDesign* Design = (ShipDesign*)PlayerElem->GetDesign();
    if (!Design || !Design->hard_points.IsValidIndex(HardPointIndex))
        return -1;

    HardPoint* Hp = Design->hard_points[HardPointIndex];
    if (!Hp)
        return -1;

    WeaponDesign* W = Hp->GetWeaponDesign(DesignIndexInHardPoint);
    if (!W)
        return -1;

    for (int32 i = 0; i < 8; ++i)
    {
        if (Designs[i] == W)
            return i;
    }

    return -1;
}

void UMissionWeaponDlg::OnMount(UFormButton* ClickedButton)
{
    if (!ClickedButton)
        return;

    // Find station + item index from BtnLoad matrix
    int32 Station = -1;
    int32 Item = -1;

    for (int32 i = 0; i < 8 && Item < 0; ++i)
    {
        for (int32 n = 0; n < 8 && Station < 0; ++n)
        {
            if (BtnLoad[i][n] == ClickedButton)
            {
                Station = n;
                Item = i;
            }
        }
    }

    if (Item < 0 || Station < 0)
        return;

    // Toggle (legacy: if loads[station]==item -> item=-1)
    if (Loads[Station] == Item)
        Item = -1;

    Loads[Station] = Item;

    // Update pictures for that station column
    UpdateGridPicturesForStation(Station);

    // Persist into MissionLoad (legacy logic)
    if (PlayerElem)
    {
        ShipDesign* D = (ShipDesign*)PlayerElem->GetDesign();
        if (D)
        {
            const int32 NStations = D->hard_points.Num();

            if (PlayerElem->Loadouts().Num() < 1)
            {
                MissionLoad* L = new MissionLoad();
                PlayerElem->Loadouts().Add(L);

                for (int32 n = 0; n < NStations; ++n)
                    L->SetStation(n, LoadToPointIndex(n));
            }
            else
            {
                for (MissionLoad* L : PlayerElem->Loadouts())
                {
                    if (!L) continue;

                    // Custom loadout: clear name so sim loader doesn't use named ship loadout
                    L->SetName(TEXT(""));

                    for (int32 n = 0; n < NStations; ++n)
                        L->SetStation(n, LoadToPointIndex(n));
                }
            }
        }
    }

    // Clear standard selection (legacy)
    ClearLoadoutSelection();

    // Recompute mass
    UpdateWeightLabelFromLoads();
}

void UMissionWeaponDlg::OnLoadoutSelectionChanged(int32 SelectedIndex)
{
    if (!PlayerElem)
        return;

    ShipDesign* Design = (ShipDesign*)PlayerElem->GetDesign();
    if (!Design || !LoadoutList)
        return;

    const FString LoadName = LoadoutList->GetItemText(SelectedIndex);

    ShipLoad* SelectedLoad = nullptr;
    for (ShipLoad& SL : Design->loadouts)
    {
        if (SL.name == LoadName)
        {
            SelectedLoad = &SL;
            break;
        }
    }

    if (!SelectedLoad)
        return;

    // Update weight label (design.mass + shipload.mass)
    if (LblWeight)
    {
        const int32 Kg = (int32)((Design->mass + SelectedLoad->mass) * 1000.0);
        LblWeight->SetText(FString::Printf(TEXT("%d kg"), Kg));
    }

    // Persist chosen named loadout into mission loadouts
    if (PlayerElem->Loadouts().Num() < 1)
    {
        MissionLoad* L = new MissionLoad(-1, SelectedLoad->name);
        PlayerElem->Loadouts().Add(L);
    }
    else
    {
        for (MissionLoad* L : PlayerElem->Loadouts())
        {
            if (!L) continue;
            L->SetName(SelectedLoad->name);
        }
    }

    // Apply station mapping
    const int32 NStations = Design->hard_points.Num();
    int32* Loadout = SelectedLoad->load;

    for (int32 i = 0; i < 8; ++i)
        Loads[i] = -1;

    for (int32 i = 0; i < NStations; ++i)
        Loads[i + FirstStation] = PointIndexToLoad(i, Loadout[i]);

    UpdateAllGridPictures();
}

void UMissionWeaponDlg::UpdateGridPicturesForStation(int32 Station)
{
    for (int32 n = 0; n < 8; ++n)
    {
        if (!BtnLoad[n][Station]) continue;
        const bool bOn = (n == Loads[Station]);
        BtnLoad[n][Station]->SetPictureByName(bOn ? LedOnName : LedOffName);
    }
}

void UMissionWeaponDlg::UpdateAllGridPictures()
{
    for (int32 Station = 0; Station < 8; ++Station)
    {
        UpdateGridPicturesForStation(Station);
    }
}

void UMissionWeaponDlg::ClearLoadoutSelection()
{
    if (LoadoutList)
        LoadoutList->ClearSelection();
}

void UMissionWeaponDlg::UpdateWeightLabelFromLoads()
{
    if (!LblWeight || !PlayerElem)
        return;

    ShipDesign* D = (ShipDesign*)PlayerElem->GetDesign();
    if (!D)
        return;

    const int32 NStations = D->hard_points.Num();
    double Mass = D->mass;

    for (int32 n = 0; n < NStations; ++n)
    {
        const int32 StationIdx = n + FirstStation;
        if (StationIdx < 0 || StationIdx >= 8)
            continue;

        const int32 Item = Loads[StationIdx];
        Mass += D->hard_points[n]->GetCarryMass(Item);
    }

    const int32 Kg = (int32)(Mass * 1000.0);
    LblWeight->SetText(FString::Printf(TEXT("%d kg"), Kg));
}

void UMissionWeaponDlg::OnCommit()
{
    // Legacy: MsnDlg::OnCommit -> MissionPlanner handles transition
    if (Manager)
    {
        Manager->ShowNavDlg(); // typical flow after Accept; adjust if your legacy differs
    }
}

void UMissionWeaponDlg::OnCancel()
{
    if (Manager)
    {
        Manager->Hide(); // or return to prior screen; keep consistent with your flow
    }
}

void UMissionWeaponDlg::OnTabButton(int32 TabId)
{
    if (!Manager)
        return;

    // 900 SIT, 901 PKG, 902 MAP, 903 WEP
    switch (TabId)
    {
    case 900: Manager->ShowMsnDlg();     break; // situation/objectives in your mapping
    case 901: Manager->ShowMsnPkgDlg();  break;
    case 902: Manager->ShowNavDlg();     break;
    case 903: Manager->ShowMsnWepDlg();  break;
    default: break;
    }
}
