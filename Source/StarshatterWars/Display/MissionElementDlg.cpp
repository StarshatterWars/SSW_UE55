/*  Project Starshatter Wars
    Fractal Dev Studios LLC
    Copyright (c) 2025-2026

    SUBSYSTEM:    Stars.exe (Unreal Port)
    FILE:         MissionElementDlg.cpp
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC
*/

#include "MissionElementDlg.h"

// UMG
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"

// Starshatter
#include "Mission.h"
#include "MissionEvent.h"
#include "Instruction.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "StarSystem.h"
#include "Skin.h"
#include "Intel.h"

// NOTE:
// - Removed Game::GetText usage per project rules.
// - Removed (__FILE__, __LINE__) allocations per project rules.

static const TCHAR* MsnDefaultSkinLabel()
{
    return TEXT("DEFAULT");
}

UMissionElementDlg::UMissionElementDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Do not touch widgets or bind events here.
}

FString UMissionElementDlg::GetLegacyFormText() const
{
    return LegacyFormText;
}

void UMissionElementDlg::BindFormWidgets()
{
    // If you are parsing a legacy .frm, bind IDs here.
    // Leave empty if you are binding via UMG only.
    // Example:
    // BindButton(1, AcceptButton);
    // BindButton(2, CancelBtn);
}

void UMissionElementDlg::InitializeDlg(UMenuScreen* InManager)
{
    Manager = InManager;
}

void UMissionElementDlg::SetMission(Mission* InMission)
{
    MissionPtr = InMission;
}

void UMissionElementDlg::SetMissionElement(MissionElement* InElem)
{
    ElemPtr = InElem;
}

void UMissionElementDlg::NativeConstruct()
{
    Super::NativeConstruct();

    // BaseScreen Enter/Escape
    ApplyButton = AcceptButton;
    CancelButton = CancelBtn;

    if (AcceptButton)
    {
        AcceptButton->OnClicked.RemoveAll(this);
        AcceptButton->OnClicked.AddDynamic(this, &UMissionElementDlg::OnAcceptClicked);
    }

    if (CancelBtn)
    {
        CancelBtn->OnClicked.RemoveAll(this);
        CancelBtn->OnClicked.AddDynamic(this, &UMissionElementDlg::OnCancelClicked);
    }

    if (ClassCombo)
    {
        ClassCombo->OnSelectionChanged.RemoveAll(this);
        ClassCombo->OnSelectionChanged.AddDynamic(this, &UMissionElementDlg::OnClassChanged);
    }

    if (DesignCombo)
    {
        DesignCombo->OnSelectionChanged.RemoveAll(this);
        DesignCombo->OnSelectionChanged.AddDynamic(this, &UMissionElementDlg::OnDesignChanged);
    }

    if (ObjectiveCombo)
    {
        ObjectiveCombo->OnSelectionChanged.RemoveAll(this);
        ObjectiveCombo->OnSelectionChanged.AddDynamic(this, &UMissionElementDlg::OnObjectiveChanged);
    }

    if (IFFEdit)
    {
        IFFEdit->OnTextCommitted.RemoveAll(this);
        IFFEdit->OnTextCommitted.AddDynamic(this, &UMissionElementDlg::OnIFFCommitted);
    }
}

void UMissionElementDlg::ShowDlg()
{
    SetVisibility(ESlateVisibility::Visible);
    RebuildFromModel();
}

void UMissionElementDlg::RebuildFromModel()
{
    if (!ElemPtr)
        return;

    // ----- CLASS -----
    if (ClassCombo)
    {
        ClassCombo->ClearOptions();

        ClassCombo->AddOption(UTF8_TO_TCHAR(Ship::GetShipClassName(CLASSIFICATION::DRONE)));
        ClassCombo->AddOption(UTF8_TO_TCHAR(Ship::GetShipClassName(CLASSIFICATION::FIGHTER)));
        ClassCombo->AddOption(UTF8_TO_TCHAR(Ship::GetShipClassName(CLASSIFICATION::ATTACK)));

        ClassCombo->AddOption(UTF8_TO_TCHAR(Ship::GetShipClassName(CLASSIFICATION::LCA)));
        ClassCombo->AddOption(UTF8_TO_TCHAR(Ship::GetShipClassName(CLASSIFICATION::COURIER)));
        ClassCombo->AddOption(UTF8_TO_TCHAR(Ship::GetShipClassName(CLASSIFICATION::CARGO)));
        ClassCombo->AddOption(UTF8_TO_TCHAR(Ship::GetShipClassName(CLASSIFICATION::CORVETTE)));
        ClassCombo->AddOption(UTF8_TO_TCHAR(Ship::GetShipClassName(CLASSIFICATION::FREIGHTER)));
        ClassCombo->AddOption(UTF8_TO_TCHAR(Ship::GetShipClassName(CLASSIFICATION::FRIGATE)));
        ClassCombo->AddOption(UTF8_TO_TCHAR(Ship::GetShipClassName(CLASSIFICATION::DESTROYER)));
        ClassCombo->AddOption(UTF8_TO_TCHAR(Ship::GetShipClassName(CLASSIFICATION::CRUISER)));
        ClassCombo->AddOption(UTF8_TO_TCHAR(Ship::GetShipClassName(CLASSIFICATION::BATTLESHIP)));
        ClassCombo->AddOption(UTF8_TO_TCHAR(Ship::GetShipClassName(CLASSIFICATION::CARRIER)));
        ClassCombo->AddOption(UTF8_TO_TCHAR(Ship::GetShipClassName(CLASSIFICATION::SWACS)));
        ClassCombo->AddOption(UTF8_TO_TCHAR(Ship::GetShipClassName(CLASSIFICATION::DREADNAUGHT)));
        ClassCombo->AddOption(UTF8_TO_TCHAR(Ship::GetShipClassName(CLASSIFICATION::STATION)));
        ClassCombo->AddOption(UTF8_TO_TCHAR(Ship::GetShipClassName(CLASSIFICATION::FARCASTER)));

        ClassCombo->AddOption(UTF8_TO_TCHAR(Ship::GetShipClassName(CLASSIFICATION::MINE)));
        ClassCombo->AddOption(UTF8_TO_TCHAR(Ship::GetShipClassName(CLASSIFICATION::COMSAT)));
        ClassCombo->AddOption(UTF8_TO_TCHAR(Ship::GetShipClassName(CLASSIFICATION::DEFSAT)));

        ClassCombo->AddOption(UTF8_TO_TCHAR(Ship::GetShipClassName(CLASSIFICATION::BUILDING)));
        ClassCombo->AddOption(UTF8_TO_TCHAR(Ship::GetShipClassName(CLASSIFICATION::FACTORY)));
        ClassCombo->AddOption(UTF8_TO_TCHAR(Ship::GetShipClassName(CLASSIFICATION::SAM)));
        ClassCombo->AddOption(UTF8_TO_TCHAR(Ship::GetShipClassName(CLASSIFICATION::EWR)));
        ClassCombo->AddOption(UTF8_TO_TCHAR(Ship::GetShipClassName(CLASSIFICATION::C3I)));
        ClassCombo->AddOption(UTF8_TO_TCHAR(Ship::GetShipClassName(CLASSIFICATION::STARBASE)));

        const ShipDesign* CurrentDesign = ElemPtr->GetDesign();

        if (CurrentDesign)
        {
            const int32 Count = ClassCombo->GetOptionCount();
            for (int32 i = 0; i < Count; ++i)
            {
                const FString Opt = ClassCombo->GetOptionAtIndex(i);
                const int32 ClassId = Ship::ClassForName(TCHAR_TO_ANSI(*Opt));
                if (ClassId == CurrentDesign->type)
                {
                    ClassCombo->SetSelectedIndex(i);
                    break;
                }
            }
        }
        else
        {
            ClassCombo->SetSelectedIndex(0);
        }
    }

    RebuildDesignListFromClass();
    RebuildSkinAndLoadoutFromDesign();

    // ----- ROLE -----
    if (RoleCombo)
    {
        RoleCombo->ClearOptions();
        for (int i = Mission::PATROL; i <= Mission::OTHER; ++i)
            RoleCombo->AddOption(UTF8_TO_TCHAR(Mission::RoleName(i)));

        const int32 RoleIndex = FMath::Max(0, ElemPtr->MissionRole() - Mission::PATROL);
        RoleCombo->SetSelectedIndex(RoleIndex);
    }

    // ----- REGION -----
    if (RegionCombo)
    {
        RegionCombo->ClearOptions();

        if (MissionPtr)
        {
            StarSystem* Sys = MissionPtr->GetStarSystem();
            if (Sys)
            {
                List<OrbitalRegion> Regions;
                Regions.append(Sys->AllRegions());
                Regions.sort();

                int32 Selected = 0;
                int32 Index = 0;

                ListIter<OrbitalRegion> Iter = Regions;
                while (++Iter)
                {
                    OrbitalRegion* R = Iter.value();
                    if (!R || !R->Name())
                        continue;

                    RegionCombo->AddOption(UTF8_TO_TCHAR(R->Name()));

                    if (ElemPtr->Region() && !FCStringAnsi::Strcmp(ElemPtr->Region(), R->Name()))
                        Selected = Index;

                    ++Index;
                }

                if (RegionCombo->GetOptionCount() > 0)
                    RegionCombo->SetSelectedIndex(Selected);
            }
        }
    }

    // ----- TEXT FIELDS -----
    if (NameEdit) NameEdit->SetText(FText::FromString(UTF8_TO_TCHAR(ElemPtr->Name())));
    if (SizeEdit) SizeEdit->SetText(FText::AsNumber(ElemPtr->Count()));
    if (IFFEdit)  IFFEdit->SetText(FText::AsNumber(ElemPtr->GetIFF()));

    if (LocXEdit) LocXEdit->SetText(FText::AsNumber((int32)(ElemPtr->Location().X / 1000)));
    if (LocYEdit) LocYEdit->SetText(FText::AsNumber((int32)(ElemPtr->Location().Y / 1000)));
    if (LocZEdit) LocZEdit->SetText(FText::AsNumber((int32)(ElemPtr->Location().Z / 1000)));

    if (RespawnsEdit) RespawnsEdit->SetText(FText::AsNumber(ElemPtr->RespawnCount()));
    if (HoldTimeEdit) HoldTimeEdit->SetText(FText::AsNumber(ElemPtr->HoldTime()));

    // ----- HEADING -----
    if (HeadingCombo)
    {
        if (HeadingCombo->GetOptionCount() == 0)
        {
            HeadingCombo->AddOption(TEXT("North"));
            HeadingCombo->AddOption(TEXT("East"));
            HeadingCombo->AddOption(TEXT("South"));
            HeadingCombo->AddOption(TEXT("West"));
        }

        double Heading = ElemPtr->Heading();
        while (Heading > 2 * PI) Heading -= 2 * PI;
        while (Heading < 0)      Heading += 2 * PI;

        int32 Sel = 0;
        if (Heading >= PI / 4 && Heading < 3 * PI / 4)            Sel = 1;
        else if (Heading >= 3 * PI / 4 && Heading < 5 * PI / 4)   Sel = 2;
        else if (Heading >= 5 * PI / 4 && Heading < 7 * PI / 4)   Sel = 3;

        HeadingCombo->SetSelectedIndex(Sel);
    }

    UpdateTeamInfo();

    // ----- INTEL -----
    if (IntelCombo)
    {
        IntelCombo->ClearOptions();

        int32 Sel = 0;
        int32 Index = 0;
        for (int i = Intel::RESERVE; i < Intel::TRACKED; ++i)
        {
            const char* Name = Intel::NameFromIntel(i);
            IntelCombo->AddOption(UTF8_TO_TCHAR(Name));

            if (ElemPtr->IntelLevel() == i)
                Sel = Index;

            ++Index;
        }

        if (IntelCombo->GetOptionCount() > 0)
            IntelCombo->SetSelectedIndex(Sel);
    }

    // ----- OBJECTIVE -----
    if (ObjectiveCombo)
    {
        ObjectiveCombo->ClearOptions();
        ObjectiveCombo->AddOption(TEXT(""));

        // Your port does not expose Instruction::NUM_ACTIONS / Action() / VECTOR.
        // Populate using ActionName(i) until it returns null/empty (with a hard safety cap).
        const int32 MaxScan = 128;
        for (int i = 0; i < MaxScan; ++i)
        {
            const char* ActionName = Instruction::ActionName(i);
            if (!ActionName || !ActionName[0])
                break;

            ObjectiveCombo->AddOption(UTF8_TO_TCHAR(ActionName));
        }

        ObjectiveCombo->SetSelectedIndex(0);
    }

    BuildObjectiveTargets();

    bExitLatch = true;
}

void UMissionElementDlg::RebuildDesignListFromClass()
{
    if (!ClassCombo || !DesignCombo || !ElemPtr)
        return;

    const FString ClassName = ClassCombo->GetSelectedOption();
    const int32 ClassId = Ship::ClassForName(TCHAR_TO_ANSI(*ClassName));

    DesignCombo->ClearOptions();

    List<Text> Designs;
    ShipDesign::GetDesignList(ClassId, Designs);

    int32 Sel = 0;
    if (Designs.size() > 0)
    {
        const ShipDesign* CurrentDesign = ElemPtr->GetDesign();
        bool Found = false;

        for (int i = 0; i < Designs.size(); ++i)
        {
            const char* Dsn = Designs[i]->data();
            DesignCombo->AddOption(UTF8_TO_TCHAR(Dsn));

            if (CurrentDesign && !_stricmp(Dsn, CurrentDesign->name))
            {
                Sel = i;
                Found = true;
            }
        }

        if (!Found)
            Sel = 0;
    }
    else
    {
        DesignCombo->AddOption(TEXT(""));
        Sel = 0;
    }

    DesignCombo->SetSelectedIndex(Sel);
}

void UMissionElementDlg::RebuildSkinAndLoadoutFromDesign()
{
    if (!DesignCombo || !ElemPtr)
        return;

    const FString DesignName = DesignCombo->GetSelectedOption();
    ShipDesign* Design = nullptr;

    if (!DesignName.IsEmpty())
        Design = ShipDesign::Get(TCHAR_TO_ANSI(*DesignName));

    // LOADOUT
    if (LoadoutCombo)
    {
        LoadoutCombo->ClearOptions();
        int32 Sel = 0;

        if (Design)
        {
            MissionLoad* CurrentLoad = nullptr;
            if (ElemPtr->Loadouts().size() > 0)
                CurrentLoad = ElemPtr->Loadouts().at(0);

            const List<ShipLoad>& Loadouts = Design->loadouts;
            for (int i = 0; i < Loadouts.size(); ++i)
            {
                const ShipLoad* L = Loadouts[i];
                if (L && L->name[0])
                {
                    LoadoutCombo->AddOption(UTF8_TO_TCHAR(L->name));
                    if (CurrentLoad && CurrentLoad->GetName() == L->name)
                        Sel = LoadoutCombo->GetOptionCount() - 1;
                }
            }
        }

        if (LoadoutCombo->GetOptionCount() < 1)
            LoadoutCombo->AddOption(TEXT(""));

        LoadoutCombo->SetSelectedIndex(Sel);
    }

    // SKIN
    if (SkinCombo)
    {
        SkinCombo->ClearOptions();

        if (Design)
        {
            SkinCombo->AddOption(MsnDefaultSkinLabel());
            SkinCombo->SetSelectedIndex(0);

            int32 Sel = 0;
            int32 Index = 1;

            ListIter<Skin> Iter = Design->skins;
            while (++Iter)
            {
                Skin* S = Iter.value();
                if (!S) continue;

                SkinCombo->AddOption(UTF8_TO_TCHAR(S->Name()));

                if (ElemPtr->GetSkin() && !strcmp(S->Name(), ElemPtr->GetSkin()->Name()))
                    Sel = Index;

                ++Index;
            }

            SkinCombo->SetSelectedIndex(Sel);
        }
    }
}

void UMissionElementDlg::BuildObjectiveTargets()
{
    if (!ObjectiveCombo || !TargetCombo || !MissionPtr || !ElemPtr)
        return;

    const int32 ObjIndex = ObjectiveCombo->GetSelectedIndex() - 1;

    TargetCombo->ClearOptions();
    TargetCombo->AddOption(TEXT(""));

    ListIter<MissionElement> Iter = MissionPtr->GetElements();
    while (++Iter)
    {
        MissionElement* E = Iter.value();
        if (!E || E == ElemPtr)
            continue;

        bool Add = false;

        // Keep original intent. Adjust threshold if your INSTRUCTION_ACTION differs.
        if (ObjIndex < (int)INSTRUCTION_ACTION::PATROL)
            Add = (E->GetIFF() == 0) || (E->GetIFF() == ElemPtr->GetIFF());
        else
            Add = (E->GetIFF() != ElemPtr->GetIFF());

        if (!Add)
            continue;

        TargetCombo->AddOption(UTF8_TO_TCHAR(E->Name()));
    }
}

void UMissionElementDlg::UpdateTeamInfo()
{
    if (!MissionPtr || !ElemPtr)
        return;

    if (CommanderCombo)
    {
        CommanderCombo->ClearOptions();
        CommanderCombo->AddOption(TEXT(""));
        CommanderCombo->SetSelectedIndex(0);

        ListIter<MissionElement> Iter = MissionPtr->GetElements();
        while (++Iter)
        {
            MissionElement* E = Iter.value();
            if (!E) continue;

            if (CanCommand(E, ElemPtr))
            {
                CommanderCombo->AddOption(UTF8_TO_TCHAR(E->Name()));
                if (ElemPtr->Commander() == E->Name())
                    CommanderCombo->SetSelectedIndex(CommanderCombo->GetOptionCount() - 1);
            }
        }
    }

    if (SquadronCombo)
    {
        SquadronCombo->ClearOptions();
        SquadronCombo->AddOption(TEXT(""));
        SquadronCombo->SetSelectedIndex(0);

        ListIter<MissionElement> Iter = MissionPtr->GetElements();
        while (++Iter)
        {
            MissionElement* E = Iter.value();
            if (!E) continue;

            if (E->GetIFF() == ElemPtr->GetIFF() && E != ElemPtr && E->IsSquadron())
            {
                SquadronCombo->AddOption(UTF8_TO_TCHAR(E->Name()));
                if (ElemPtr->Squadron() == E->Name())
                    SquadronCombo->SetSelectedIndex(SquadronCombo->GetOptionCount() - 1);
            }
        }
    }

    if (CarrierCombo)
    {
        CarrierCombo->ClearOptions();
        CarrierCombo->AddOption(TEXT(""));
        CarrierCombo->SetSelectedIndex(0);

        ListIter<MissionElement> Iter = MissionPtr->GetElements();
        while (++Iter)
        {
            MissionElement* E = Iter.value();
            if (!E) continue;

            if (E->GetIFF() == ElemPtr->GetIFF() && E != ElemPtr && E->GetDesign() && E->GetDesign()->flight_decks.size())
            {
                CarrierCombo->AddOption(UTF8_TO_TCHAR(E->Name()));
                if (ElemPtr->Carrier() == E->Name())
                    CarrierCombo->SetSelectedIndex(CarrierCombo->GetOptionCount() - 1);
            }
        }
    }
}

bool UMissionElementDlg::CanCommand(const MissionElement* Commander, const MissionElement* Subordinate) const
{
    if (MissionPtr && Commander && Subordinate && Commander != Subordinate)
    {
        if (Commander->GetIFF() != Subordinate->GetIFF())
            return false;

        if (Commander->IsSquadron())
            return false;

        if (Commander->Commander().length() == 0)
            return true;

        if (Subordinate->Name() == Commander->Commander())
            return false;

        MissionElement* Up = MissionPtr->FindElement(Commander->Commander());
        if (Up)
            return CanCommand(Up, Subordinate);
    }

    return false;
}

void UMissionElementDlg::OnClassChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    (void)SelectedItem;
    (void)SelectionType;

    RebuildDesignListFromClass();
    RebuildSkinAndLoadoutFromDesign();
}

void UMissionElementDlg::OnDesignChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    (void)SelectedItem;
    (void)SelectionType;

    RebuildSkinAndLoadoutFromDesign();
}

void UMissionElementDlg::OnObjectiveChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    (void)SelectedItem;
    (void)SelectionType;

    BuildObjectiveTargets();
}

void UMissionElementDlg::OnIFFCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    (void)CommitMethod;

    if (!ElemPtr)
        return;

    const FString S = Text.ToString();
    int32 NewIFF = 0;

    if (S.IsNumeric() || (S.Len() > 1 && S[0] == '-' && S.Mid(1).IsNumeric()))
        NewIFF = FCString::Atoi(*S);

    if (ElemPtr->GetIFF() == NewIFF)
        return;

    ElemPtr->SetIFF(NewIFF);

    UpdateTeamInfo();
    BuildObjectiveTargets();
}

void UMissionElementDlg::OnAcceptClicked()
{
    if (!MissionPtr || !ElemPtr)
    {
        if (Manager)
        {
            // Manager->ShowMsnEditDlg();
        }
        return;
    }

    auto ReadInt = [](UEditableTextBox* Box, int32 DefaultVal) -> int32
        {
            if (!Box) return DefaultVal;
            const FString S = Box->GetText().ToString();
            if (S.IsEmpty()) return DefaultVal;
            return FCString::Atoi(*S);
        };

    if (NameEdit)
        ElemPtr->SetName(TCHAR_TO_ANSI(*NameEdit->GetText().ToString()));

    ElemPtr->SetCount(FMath::Max(1, ReadInt(SizeEdit, ElemPtr->Count())));
    ElemPtr->SetIFF(ReadInt(IFFEdit, ElemPtr->GetIFF()));

    // Location (km -> meters)
    {
        FVector Loc = ElemPtr->Location();
        Loc.X = ReadInt(LocXEdit, (int32)(Loc.X / 1000)) * 1000;
        Loc.Y = ReadInt(LocYEdit, (int32)(Loc.Y / 1000)) * 1000;
        Loc.Z = ReadInt(LocZEdit, (int32)(Loc.Z / 1000)) * 1000;
        ElemPtr->SetLocation(Loc);
    }

    ElemPtr->SetRespawnCount(FMath::Max(0, ReadInt(RespawnsEdit, ElemPtr->RespawnCount())));
    ElemPtr->SetHoldTime(FMath::Max(0, ReadInt(HoldTimeEdit, ElemPtr->HoldTime())));

    // Design + Skin
    if (DesignCombo)
    {
        const FString DesignName = DesignCombo->GetSelectedOption();
        ShipDesign* D = DesignName.IsEmpty() ? nullptr : ShipDesign::Get(TCHAR_TO_ANSI(*DesignName));

        if (D)
        {
            ElemPtr->SetDesign(D);

            if (SkinCombo)
            {
                const FString SkinName = SkinCombo->GetSelectedOption();
                const FString DefaultName = MsnDefaultSkinLabel();

                if (!SkinName.IsEmpty() && SkinName != DefaultName)
                    ElemPtr->SetSkin(D->FindSkin(TCHAR_TO_ANSI(*SkinName)));
                else
                    ElemPtr->SetSkin(0);
            }
        }
    }

    if (RoleCombo)
        ElemPtr->SetMissionRole(RoleCombo->GetSelectedIndex() + Mission::PATROL);

    if (RegionCombo)
    {
        const FString RegionName = RegionCombo->GetSelectedOption();
        ElemPtr->SetRegion(TCHAR_TO_ANSI(*RegionName));

        if (ElemPtr->Player() > 0)
            MissionPtr->SetRegion(TCHAR_TO_ANSI(*RegionName));
    }

    if (HeadingCombo)
    {
        switch (HeadingCombo->GetSelectedIndex())
        {
        default:
        case 0: ElemPtr->SetHeading(0);           break;
        case 1: ElemPtr->SetHeading(PI / 2);      break;
        case 2: ElemPtr->SetHeading(PI);          break;
        case 3: ElemPtr->SetHeading(3 * PI / 2);  break;
        }
    }

    if (CommanderCombo) ElemPtr->SetCommander(TCHAR_TO_ANSI(*CommanderCombo->GetSelectedOption()));
    if (SquadronCombo)  ElemPtr->SetSquadron(TCHAR_TO_ANSI(*SquadronCombo->GetSelectedOption()));
    if (CarrierCombo)   ElemPtr->SetCarrier(TCHAR_TO_ANSI(*CarrierCombo->GetSelectedOption()));

    if (IntelCombo)
        ElemPtr->SetIntelLevel(Intel::IntelFromName(TCHAR_TO_ANSI(*IntelCombo->GetSelectedOption())));

    if (LoadoutCombo)
    {
        ElemPtr->Loadouts().destroy();

        const FString LoadName = LoadoutCombo->GetSelectedOption();
        if (!LoadName.IsEmpty())
        {
            MissionLoad* MLoad = new MissionLoad(-1, TCHAR_TO_ANSI(*LoadName));
            ElemPtr->Loadouts().append(MLoad);
        }
    }

    // Objective/Target: build Instruction using ctor(action, targetName)
    if (ObjectiveCombo && TargetCombo)
    {
        List<Instruction>& Objectives = ElemPtr->Objectives();
        Objectives.destroy();

        const int32 ActionIndex = ObjectiveCombo->GetSelectedIndex() - 1;
        const FString Target = TargetCombo->GetSelectedOption();

        if (ActionIndex >= 0 && !Target.IsEmpty())
        {
            const INSTRUCTION_ACTION Action = (INSTRUCTION_ACTION)ActionIndex;
            Instruction* Obj = new Instruction(Action, TCHAR_TO_ANSI(*Target));
            Objectives.append(Obj);
        }
    }

    if (ElemPtr->Player())
        MissionPtr->SetTeam(ElemPtr->GetIFF());

    if (Manager)
    {
        // Manager->ShowMsnEditDlg();
    }
}

void UMissionElementDlg::OnCancelClicked()
{
    if (Manager)
    {
        // Manager->ShowMsnEditDlg();
    }
}
