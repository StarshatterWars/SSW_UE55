/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         MsnElemDlg.cpp
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    Mission Editor Element Dialog (UMG UserWidget) — Unreal port of the legacy
    mission element editor screen.
*/

#include "GameStructs.h"

#include "MsnElemDlg.h"

// UMG:
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"

// Legacy / Starshatter core:
#include "MsnEditDlg.h"
#include "MenuScreen.h"
#include "Campaign.h"
#include "Mission.h"
#include "Instruction.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "StarSystem.h"
#include "Galaxy.h"
#include "Skin.h"
#include "Game.h"
#include "Random.h"

DEFINE_LOG_CATEGORY_STATIC(LogMsnElemDlg, Log, All);

// +--------------------------------------------------------------------+

UMsnElemDlg::UMsnElemDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    exit_latch = true;
}

// +--------------------------------------------------------------------+

void UMsnElemDlg::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // Wire buttons:
    if (BtnAccept)  BtnAccept->OnClicked.AddDynamic(this, &UMsnElemDlg::HandleAcceptClicked);
    if (BtnCancel)  BtnCancel->OnClicked.AddDynamic(this, &UMsnElemDlg::HandleCancelClicked);

    // Wire selection / change events (best-effort; safe if null):
    if (CmbClass)      CmbClass->OnSelectionChanged.AddDynamic(this, &UMsnElemDlg::HandleClassChanged);
    if (CmbDesign)     CmbDesign->OnSelectionChanged.AddDynamic(this, &UMsnElemDlg::HandleDesignChanged);
    if (CmbObjective)  CmbObjective->OnSelectionChanged.AddDynamic(this, &UMsnElemDlg::HandleObjectiveChanged);

    if (EdtIFF)        EdtIFF->OnTextChanged.AddDynamic(this, &UMsnElemDlg::HandleIFFTextChanged);

    exit_latch = true;
}

void UMsnElemDlg::NativeConstruct()
{
    Super::NativeConstruct();
}

void UMsnElemDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Legacy polled RETURN/ESC in ExecFrame(). In UE, proper input should be routed
    // via PlayerController / focus. Keeping ExecFrame() as a compatibility hook:
    ExecFrame();
}

// +--------------------------------------------------------------------+

void UMsnElemDlg::BindFormWidgets()
{
    // Optional: bind legacy FORM IDs to widgets if you want ID-based lookups.
    // BindButton(1, BtnAccept);
    // BindButton(2, BtnCancel);
    //
    // BindEdit(201, EdtName);
    // BindCombo(202, CmbClass);
    // BindCombo(203, CmbDesign);
    // BindCombo(213, CmbSkin);
    // BindEdit(204, EdtSize);
    // BindEdit(205, EdtIFF);
    // BindCombo(206, CmbRole);
    // BindCombo(207, CmbRegion);
    // BindEdit(208, EdtLocX);
    // BindEdit(209, EdtLocY);
    // BindEdit(210, EdtLocZ);
    // BindCombo(211, CmbHeading);
    // BindEdit(212, EdtHoldTime);
    //
    // BindButton(221, BtnPlayer);
    // BindButton(222, BtnAlert);
    // BindButton(223, BtnPlayable);
    // BindButton(224, BtnCommandAI);
    // BindEdit(225, EdtRespawns);
    // BindCombo(226, CmbCommander);
    // BindCombo(227, CmbCarrier);
    // BindCombo(228, CmbSquadron);
    // BindCombo(229, CmbIntel);
    // BindCombo(230, CmbLoadout);
    // BindCombo(231, CmbObjective);
    // BindCombo(232, CmbTarget);
}

// +--------------------------------------------------------------------+

void UMsnElemDlg::SetMission(Mission* inMission)
{
    mission = inMission;
}

void UMsnElemDlg::SetMissionElement(MissionElement* inElem)
{
    elem = inElem;
}

// +--------------------------------------------------------------------+

void UMsnElemDlg::Show()
{
    if (!elem)
        return;

    // Populate Class combo:
    if (CmbClass)
    {
        CmbClass->ClearOptions();

        CmbClass->AddOption(FString(Ship::ClassName(Ship::DRONE)));
        CmbClass->AddOption(FString(Ship::ClassName(Ship::FIGHTER)));
        CmbClass->AddOption(FString(Ship::ClassName(Ship::ATTACK)));

        CmbClass->AddOption(FString(Ship::ClassName(Ship::LCA)));
        CmbClass->AddOption(FString(Ship::ClassName(Ship::COURIER)));
        CmbClass->AddOption(FString(Ship::ClassName(Ship::CARGO)));
        CmbClass->AddOption(FString(Ship::ClassName(Ship::CORVETTE)));
        CmbClass->AddOption(FString(Ship::ClassName(Ship::FREIGHTER)));
        CmbClass->AddOption(FString(Ship::ClassName(Ship::FRIGATE)));
        CmbClass->AddOption(FString(Ship::ClassName(Ship::DESTROYER)));
        CmbClass->AddOption(FString(Ship::ClassName(Ship::CRUISER)));
        CmbClass->AddOption(FString(Ship::ClassName(Ship::BATTLESHIP)));
        CmbClass->AddOption(FString(Ship::ClassName(Ship::CARRIER)));
        CmbClass->AddOption(FString(Ship::ClassName(Ship::SWACS)));
        CmbClass->AddOption(FString(Ship::ClassName(Ship::DREADNAUGHT)));
        CmbClass->AddOption(FString(Ship::ClassName(Ship::STATION)));
        CmbClass->AddOption(FString(Ship::ClassName(Ship::FARCASTER)));

        CmbClass->AddOption(FString(Ship::ClassName(Ship::MINE)));
        CmbClass->AddOption(FString(Ship::ClassName(Ship::COMSAT)));
        CmbClass->AddOption(FString(Ship::ClassName(Ship::DEFSAT)));

        CmbClass->AddOption(FString(Ship::ClassName(Ship::BUILDING)));
        CmbClass->AddOption(FString(Ship::ClassName(Ship::FACTORY)));
        CmbClass->AddOption(FString(Ship::ClassName(Ship::SAM)));
        CmbClass->AddOption(FString(Ship::ClassName(Ship::EWR)));
        CmbClass->AddOption(FString(Ship::ClassName(Ship::C3I)));
        CmbClass->AddOption(FString(Ship::ClassName(Ship::STARBASE)));

        // Select class based on element design type (legacy behavior):
        const ShipDesign* design = elem->GetDesign();
        if (design)
        {
            const int32 Num = CmbClass->GetOptionCount();
            for (int32 i = 0; i < Num; ++i)
            {
                const FString Opt = CmbClass->GetOptionAtIndex(i);
                const int ClassId = Ship::ClassForName(TCHAR_TO_ANSI(*Opt));

                if (ClassId == design->type)
                {
                    CmbClass->SetSelectedIndex(i);
                    break;
                }
            }
        }
        else
        {
            if (CmbClass->GetOptionCount() > 0)
                CmbClass->SetSelectedIndex(0);
        }
    }

    // Populate Design + dependent fields:
    if (CmbDesign)
    {
        HandleClassChanged(CmbClass ? CmbClass->GetSelectedOption() : FString(), ESelectInfo::Direct);
        HandleDesignChanged(CmbDesign->GetSelectedOption(), ESelectInfo::Direct);
    }

    // Populate Role:
    if (CmbRole)
    {
        CmbRole->ClearOptions();

        for (int i = Mission::PATROL; i <= Mission::OTHER; ++i)
        {
            CmbRole->AddOption(FString(Mission::RoleName(i)));

            if (i == 0)
                CmbRole->SetSelectedIndex(0);
            else if (elem->MissionRole() == i)
                CmbRole->SetSelectedIndex(CmbRole->GetOptionCount() - 1);
        }
    }

    // Populate Region:
    if (CmbRegion)
    {
        CmbRegion->ClearOptions();

        if (mission)
        {
            StarSystem* sys = mission->GetStarSystem();
            if (sys)
            {
                List<OrbitalRegion> regions;
                regions.append(sys->AllRegions());
                regions.sort();

                ListIter<OrbitalRegion> iter = regions;
                while (++iter)
                {
                    OrbitalRegion* region = iter.value();
                    if (!region)
                        continue;

                    CmbRegion->AddOption(FString(region->Name()));

                    if (elem->Region() && !FCStringAnsi::Strcmp(elem->Region(), region->Name()))
                        CmbRegion->SetSelectedIndex(CmbRegion->GetOptionCount() - 1);
                }
            }
        }
    }

    // Fill fields:
    if (EdtName)     EdtName->SetText(FText::FromString(FString(elem->Name())));
    if (EdtSize)     EdtSize->SetText(FText::AsNumber(elem->Count()));
    if (EdtIFF)      EdtIFF->SetText(FText::AsNumber(elem->GetIFF()));

    // Location (legacy was Point in meters; UI showed km):
    const FVector Loc = FVector(
        (float)elem->Location().x,
        (float)elem->Location().y,
        (float)elem->Location().z
    );

    if (EdtLocX) EdtLocX->SetText(FText::AsNumber((int32)(Loc.X / 1000.0f)));
    if (EdtLocY) EdtLocY->SetText(FText::AsNumber((int32)(Loc.Y / 1000.0f)));
    if (EdtLocZ) EdtLocZ->SetText(FText::AsNumber((int32)(Loc.Z / 1000.0f)));

    if (EdtRespawns) EdtRespawns->SetText(FText::AsNumber(elem->RespawnCount()));
    if (EdtHoldTime) EdtHoldTime->SetText(FText::AsNumber(elem->HoldTime()));

    // Buttons as toggles (requires your UMG logic; we keep Enable state only):
    if (BtnPlayer)    BtnPlayer->SetIsEnabled(true);
    if (BtnPlayable)  BtnPlayable->SetIsEnabled(true);
    if (BtnAlert)     BtnAlert->SetIsEnabled(true);
    if (BtnCommandAI) BtnCommandAI->SetIsEnabled(true);

    UpdateTeamInfo();

    // Intel:
    if (CmbIntel)
    {
        CmbIntel->ClearOptions();

        for (int i = Intel::RESERVE; i < Intel::TRACKED; ++i)
        {
            CmbIntel->AddOption(FString(Intel::NameFromIntel(i)));

            if (i == 0)
                CmbIntel->SetSelectedIndex(0);
            else if (elem->IntelLevel() == i)
                CmbIntel->SetSelectedIndex(CmbIntel->GetOptionCount() - 1);
        }
    }

    // Objective + target:
    Instruction* instr = nullptr;
    if (elem->Objectives().size() > 0)
        instr = elem->Objectives().at(0);

    if (CmbObjective)
    {
        CmbObjective->ClearOptions();
        CmbObjective->AddOption(TEXT(""));
        CmbObjective->SetSelectedIndex(0);

        for (int i = 0; i < Instruction::NUM_ACTIONS; ++i)
        {
            CmbObjective->AddOption(FString(Instruction::ActionName(i)));

            if (instr && instr->Action() == i)
                CmbObjective->SetSelectedIndex(i + 1);
        }
    }

    if (CmbTarget)
    {
        HandleObjectiveChanged(CmbObjective ? CmbObjective->GetSelectedOption() : FString(), ESelectInfo::Direct);
    }

    // Heading:
    if (CmbHeading)
    {
        double heading = elem->Heading();

        while (heading > 2 * PI) heading -= 2 * PI;
        while (heading < 0)      heading += 2 * PI;

        if (heading >= PI / 4 && heading < 3 * PI / 4)
            CmbHeading->SetSelectedIndex(1);
        else if (heading >= 3 * PI / 4 && heading < 5 * PI / 4)
            CmbHeading->SetSelectedIndex(2);
        else if (heading >= 5 * PI / 4 && heading < 7 * PI / 4)
            CmbHeading->SetSelectedIndex(3);
        else
            CmbHeading->SetSelectedIndex(0);
    }

    exit_latch = true;
}

// +--------------------------------------------------------------------+

void UMsnElemDlg::ExecFrame()
{
    // Legacy used Keyboard::KeyDown(VK_RETURN/VK_ESCAPE).
    // In UE, implement input handling in the owning PlayerController and call:
    // HandleAcceptClicked() / HandleCancelClicked().
    if (exit_latch)
        exit_latch = false;
}

// +--------------------------------------------------------------------+

void UMsnElemDlg::UpdateTeamInfo()
{
    if (CmbCommander)
    {
        CmbCommander->ClearOptions();
        CmbCommander->AddOption(TEXT(""));
        CmbCommander->SetSelectedIndex(0);

        if (mission && elem)
        {
            ListIter<MissionElement> iter = mission->GetElements();
            while (++iter)
            {
                MissionElement* e = iter.value();
                if (!e)
                    continue;

                if (CanCommand(e, elem))
                {
                    CmbCommander->AddOption(FString(e->Name()));

                    if (elem->Commander() == e->Name())
                        CmbCommander->SetSelectedIndex(CmbCommander->GetOptionCount() - 1);
                }
            }
        }
    }

    if (CmbSquadron)
    {
        CmbSquadron->ClearOptions();
        CmbSquadron->AddOption(TEXT(""));
        CmbSquadron->SetSelectedIndex(0);

        if (mission && elem)
        {
            ListIter<MissionElement> iter = mission->GetElements();
            while (++iter)
            {
                MissionElement* e = iter.value();
                if (!e)
                    continue;

                if (e->GetIFF() == elem->GetIFF() && e != elem && e->IsSquadron())
                {
                    CmbSquadron->AddOption(FString(e->Name()));

                    if (elem->Squadron() == e->Name())
                        CmbSquadron->SetSelectedIndex(CmbSquadron->GetOptionCount() - 1);
                }
            }
        }
    }

    if (CmbCarrier)
    {
        CmbCarrier->ClearOptions();
        CmbCarrier->AddOption(TEXT(""));
        CmbCarrier->SetSelectedIndex(0);

        if (mission && elem)
        {
            ListIter<MissionElement> iter = mission->GetElements();
            while (++iter)
            {
                MissionElement* e = iter.value();
                if (!e)
                    continue;

                if (e->GetIFF() == elem->GetIFF() && e != elem && e->GetDesign() && e->GetDesign()->flight_decks.size())
                {
                    CmbCarrier->AddOption(FString(e->Name()));

                    if (elem->Carrier() == e->Name())
                        CmbCarrier->SetSelectedIndex(CmbCarrier->GetOptionCount() - 1);
                }
            }
        }
    }
}

// +--------------------------------------------------------------------+

void UMsnElemDlg::HandleClassChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (!CmbClass || !CmbDesign || !elem)
        return;

    const FString ClassName = CmbClass->GetSelectedOption();
    const int classid = Ship::ClassForName(TCHAR_TO_ANSI(*ClassName));

    CmbDesign->ClearOptions();

    List<Text> designs;
    ShipDesign::GetDesignList(classid, designs);

    if (designs.size() > 0)
    {
        const ShipDesign* cur = elem->GetDesign();
        bool found = false;

        for (int i = 0; i < designs.size(); ++i)
        {
            const char* dsn = designs[i]->data();
            CmbDesign->AddOption(FString(dsn));

            if (cur && !_stricmp(dsn, cur->name))
            {
                CmbDesign->SetSelectedIndex(i);
                found = true;
            }
        }

        if (!found)
            CmbDesign->SetSelectedIndex(0);
    }
    else
    {
        CmbDesign->AddOption(TEXT(""));
        CmbDesign->SetSelectedIndex(0);
    }

    HandleDesignChanged(CmbDesign->GetSelectedOption(), ESelectInfo::Direct);
}

// +--------------------------------------------------------------------+

void UMsnElemDlg::HandleDesignChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (!CmbDesign)
        return;

    ShipDesign* design = nullptr;

    const FString DName = CmbDesign->GetSelectedOption();
    if (!DName.IsEmpty())
        design = ShipDesign::Get(TCHAR_TO_ANSI(*DName));

    int load_index = 0;

    if (CmbLoadout)
    {
        CmbLoadout->ClearOptions();

        if (design)
        {
            MissionLoad* mload = nullptr;
            if (elem && elem->Loadouts().size() > 0)
                mload = elem->Loadouts().at(0);

            const List<ShipLoad>& loadouts = design->loadouts;

            if (loadouts.size() > 0)
            {
                for (int i = 0; i < loadouts.size(); ++i)
                {
                    const ShipLoad* load = loadouts[i];
                    if (load && load->name[0])
                    {
                        CmbLoadout->AddOption(FString(load->name));

                        if (mload && mload->GetName() == load->name)
                            load_index = CmbLoadout->GetOptionCount() - 1;
                    }
                }
            }
        }

        if (CmbLoadout->GetOptionCount() < 1)
            CmbLoadout->AddOption(TEXT(""));

        CmbLoadout->SetSelectedIndex(load_index);
    }

    if (CmbSkin)
    {
        CmbSkin->ClearOptions();

        if (design)
        {
            CmbSkin->AddOption(FString(Game::GetText("MsnDlg.default").data()));
            CmbSkin->SetSelectedIndex(0);

            ListIter<Skin> iter = design->skins;
            while (++iter)
            {
                Skin* s = iter.value();
                if (!s)
                    continue;

                CmbSkin->AddOption(FString(s->Name()));

                if (elem && elem->GetSkin() && elem->GetSkin()->Name() && !FCStringAnsi::Strcmp(s->Name(), elem->GetSkin()->Name()))
                    CmbSkin->SetSelectedIndex(CmbSkin->GetOptionCount() - 1);
            }
        }
    }
}

// +--------------------------------------------------------------------+

void UMsnElemDlg::HandleObjectiveChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (!CmbObjective || !CmbTarget || !elem)
        return;

    Instruction* instr = nullptr;
    if (elem->Objectives().size() > 0)
        instr = elem->Objectives().at(0);

    const int objid = CmbObjective->GetSelectedIndex() - 1;

    CmbTarget->ClearOptions();
    CmbTarget->AddOption(TEXT(""));

    if (mission)
    {
        ListIter<MissionElement> iter = mission->GetElements();
        while (++iter)
        {
            MissionElement* e = iter.value();
            if (!e || e == elem)
                continue;

            bool add = false;

            if (objid < Instruction::PATROL)
                add = e->GetIFF() == 0 || e->GetIFF() == elem->GetIFF();
            else
                add = e->GetIFF() != elem->GetIFF();

            if (add)
            {
                CmbTarget->AddOption(FString(e->Name()));

                if (instr && !_stricmp(instr->TargetName(), e->Name()))
                    CmbTarget->SetSelectedIndex(CmbTarget->GetOptionCount() - 1);
            }
        }
    }
}

// +--------------------------------------------------------------------+

void UMsnElemDlg::HandleIFFTextChanged(const FText& NewText)
{
    if (!elem)
        return;

    const FString S = NewText.ToString();
    const int32 NewIFF = FCString::Atoi(*S);

    if (elem->GetIFF() == NewIFF)
        return;

    elem->SetIFF(NewIFF);

    UpdateTeamInfo();

    if (CmbTarget)
        HandleObjectiveChanged(CmbObjective ? CmbObjective->GetSelectedOption() : FString(), ESelectInfo::Direct);
}

// +--------------------------------------------------------------------+

void UMsnElemDlg::HandleAcceptClicked()
{
    if (!mission || !elem)
    {
        if (manager)
            manager->ShowMsnEditDlg();
        return;
    }

    // Name:
    if (EdtName)
    {
        const FString Name = EdtName->GetText().ToString();
        elem->SetName(TCHAR_TO_ANSI(*Name));
    }

    // Size:
    if (EdtSize)
    {
        const int32 Val = FMath::Max(1, FCString::Atoi(*EdtSize->GetText().ToString()));
        elem->SetCount(Val);
    }

    // IFF:
    if (EdtIFF)
    {
        const int32 Val = FMath::Max(0, FCString::Atoi(*EdtIFF->GetText().ToString()));
        elem->SetIFF(Val);
    }

    // Location (UI is km, internal is meters):
    if (EdtLocX && EdtLocY && EdtLocZ)
    {
        const int32 Xkm = FCString::Atoi(*EdtLocX->GetText().ToString());
        const int32 Ykm = FCString::Atoi(*EdtLocY->GetText().ToString());
        const int32 Zkm = FCString::Atoi(*EdtLocZ->GetText().ToString());

        const FVector NewLoc((float)Xkm * 1000.0f, (float)Ykm * 1000.0f, (float)Zkm * 1000.0f);

        // MissionElement legacy expects Point. If you've converted it to FVector in your port,
        // SetLocation should accept FVector. If not, add a shim in MissionElement.
        elem->SetLocation(NewLoc);
    }

    // Respawns:
    if (EdtRespawns)
    {
        const int32 Val = FMath::Max(0, FCString::Atoi(*EdtRespawns->GetText().ToString()));
        elem->SetRespawnCount(Val);
    }

    // Hold time:
    if (EdtHoldTime)
    {
        const int32 Val = FMath::Max(0, FCString::Atoi(*EdtHoldTime->GetText().ToString()));
        elem->SetHoldTime(Val);
    }

    // Player / playable / alert / command AI:
    // NOTE: UButton is not a toggle by default. Replace with UCheckBox or maintain state separately.
    // Here we leave behavior to your widget logic. Keep calls in place if you provide toggle state APIs.

    // Design + skin:
    if (CmbDesign)
    {
        const FString DName = CmbDesign->GetSelectedOption();
        ShipDesign* d = DName.IsEmpty() ? nullptr : ShipDesign::Get(TCHAR_TO_ANSI(*DName));

        if (d)
        {
            elem->SetDesign(d);

            if (CmbSkin)
            {
                const FString SkinName = CmbSkin->GetSelectedOption();
                const FString DefaultSkin = FString(Game::GetText("MsnDlg.default").data());

                if (!SkinName.IsEmpty() && SkinName != DefaultSkin)
                    elem->SetSkin(d->FindSkin(TCHAR_TO_ANSI(*SkinName)));
                else
                    elem->SetSkin(nullptr);
            }
        }
    }

    // Role:
    if (CmbRole)
        elem->SetMissionRole(CmbRole->GetSelectedIndex());

    // Region:
    if (CmbRegion)
    {
        const FString Region = CmbRegion->GetSelectedOption();
        elem->SetRegion(TCHAR_TO_ANSI(*Region));

        if (elem->Player() > 0)
            mission->SetRegion(TCHAR_TO_ANSI(*Region));
    }

    // Heading:
    if (CmbHeading)
    {
        switch (CmbHeading->GetSelectedIndex())
        {
        default:
        case 0: elem->SetHeading(0);      break;
        case 1: elem->SetHeading(PI / 2);   break;
        case 2: elem->SetHeading(PI);     break;
        case 3: elem->SetHeading(3 * PI / 2); break;
        }
    }

    // Commander / squadron / carrier:
    if (CmbCommander) elem->SetCommander(TCHAR_TO_ANSI(*CmbCommander->GetSelectedOption()));
    if (CmbSquadron)  elem->SetSquadron(TCHAR_TO_ANSI(*CmbSquadron->GetSelectedOption()));
    if (CmbCarrier)   elem->SetCarrier(TCHAR_TO_ANSI(*CmbCarrier->GetSelectedOption()));

    // Intel:
    if (CmbIntel)
        elem->SetIntelLevel(Intel::IntelFromName(TCHAR_TO_ANSI(*CmbIntel->GetSelectedOption())));

    // Loadout:
    if (CmbLoadout && CmbLoadout->GetOptionCount() > 0)
    {
        elem->Loadouts().destroy();

        const FString LoadName = CmbLoadout->GetSelectedOption();
        if (!LoadName.IsEmpty())
        {
            MissionLoad* mload = new MissionLoad(-1, TCHAR_TO_ANSI(*LoadName)); // removed (__FILE__,__LINE__)
            elem->Loadouts().append(mload);
        }
    }

    // Objective + target:
    if (CmbObjective && CmbTarget)
    {
        List<Instruction>& objectives = elem->Objectives();
        objectives.destroy();

        const int action = CmbObjective->GetSelectedIndex() - 1;
        const FString Target = CmbTarget->GetSelectedOption();

        if (action >= Instruction::VECTOR)
        {
            Instruction* obj = new Instruction(action, TCHAR_TO_ANSI(*Target)); // removed (__FILE__,__LINE__)
            objectives.append(obj);
        }
    }

    if (elem->Player())
        mission->SetTeam(elem->GetIFF());

    if (manager)
        manager->ShowMsnEditDlg();
}

// +--------------------------------------------------------------------+

void UMsnElemDlg::HandleCancelClicked()
{
    if (manager)
        manager->ShowMsnEditDlg();
}

// +--------------------------------------------------------------------+

bool UMsnElemDlg::CanCommand(const MissionElement* commander, const MissionElement* subordinate) const
{
    if (mission && commander && subordinate && commander != subordinate)
    {
        if (commander->GetIFF() != subordinate->GetIFF())
            return false;

        if (commander->IsSquadron())
            return false;

        if (commander->Commander().length() == 0)
            return true;

        if (subordinate->Name() == commander->Commander())
            return false;

        MissionElement* e = mission->FindElement(commander->Commander());
        if (e)
            return CanCommand(e, subordinate);
    }

    return false;
}
