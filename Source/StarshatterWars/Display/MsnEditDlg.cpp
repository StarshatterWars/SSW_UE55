/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         MsnEditDlg.cpp
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    Mission Editor Dialog (UMG UserWidget) — Unreal port of the legacy
    active window / form-driven mission editor screen.
*/

#include "GameStructs.h"

#include "MsnEditDlg.h"

// UMG:
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"

#include "Misc/DateTime.h"

// Legacy / Starshatter core:
#include "MenuScreen.h"
#include "Campaign.h"
#include "Mission.h"
#include "MissionEvent.h"
#include "ShipDesign.h"
#include "StarSystem.h"
#include "Galaxy.h"

// Other dialogs (UMG equivalents):
#include "MsnElemDlg.h"
#include "MsnEventDlg.h"
#include "NavDlg.h"

// Minimal logging category for this file:
DEFINE_LOG_CATEGORY_STATIC(LogMsnEditDlg, Log, All);

// +--------------------------------------------------------------------+

UMsnEditDlg::UMsnEditDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Defaults already set in header member initializers.
}

// +--------------------------------------------------------------------+

void UMsnEditDlg::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // Wire up button clicks if widgets exist:
    if (BtnAccept)    BtnAccept->OnClicked.AddDynamic(this, &UMsnEditDlg::HandleAcceptClicked);
    if (BtnCancel)    BtnCancel->OnClicked.AddDynamic(this, &UMsnEditDlg::HandleCancelClicked);

    if (BtnElemAdd)   BtnElemAdd->OnClicked.AddDynamic(this, &UMsnEditDlg::HandleElemAddClicked);
    if (BtnElemEdit)  BtnElemEdit->OnClicked.AddDynamic(this, &UMsnEditDlg::HandleElemEditClicked);
    if (BtnElemDel)   BtnElemDel->OnClicked.AddDynamic(this, &UMsnEditDlg::HandleElemDelClicked);
    if (BtnElemInc)   BtnElemInc->OnClicked.AddDynamic(this, &UMsnEditDlg::HandleElemIncClicked);
    if (BtnElemDec)   BtnElemDec->OnClicked.AddDynamic(this, &UMsnEditDlg::HandleElemDecClicked);

    if (BtnEventAdd)  BtnEventAdd->OnClicked.AddDynamic(this, &UMsnEditDlg::HandleEventAddClicked);
    if (BtnEventEdit) BtnEventEdit->OnClicked.AddDynamic(this, &UMsnEditDlg::HandleEventEditClicked);
    if (BtnEventDel)  BtnEventDel->OnClicked.AddDynamic(this, &UMsnEditDlg::HandleEventDelClicked);
    if (BtnEventInc)  BtnEventInc->OnClicked.AddDynamic(this, &UMsnEditDlg::HandleEventIncClicked);
    if (BtnEventDec)  BtnEventDec->OnClicked.AddDynamic(this, &UMsnEditDlg::HandleEventDecClicked);

    if (BtnSit)       BtnSit->OnClicked.AddDynamic(this, &UMsnEditDlg::HandleTabSitClicked);
    if (BtnPkg)       BtnPkg->OnClicked.AddDynamic(this, &UMsnEditDlg::HandleTabPkgClicked);
    if (BtnMap)       BtnMap->OnClicked.AddDynamic(this, &UMsnEditDlg::HandleTabMapClicked);

    // NOTE:
    // Combo/list selection events require specific UMG setup (ListView entry objects,
    // OnSelectionChanged delegates, etc.). Those are typically bound in BP or in code
    // once the list items are populated with UObject entries.

    exit_latch = true;
}

void UMsnEditDlg::NativeConstruct()
{
    Super::NativeConstruct();
}

void UMsnEditDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Legacy ExecFrame() behavior was keyboard-driven (RETURN/ESC).
    // In UE, you typically use focus/input routing. If you still want a polling approach:
    ExecFrame();
}

// +--------------------------------------------------------------------+

void UMsnEditDlg::SetMission(Mission* m)
{
    mission = m;
    current_tab = 0;
}

void UMsnEditDlg::SetMissionInfo(MissionInfo* m)
{
    mission_info = m;
    current_tab = 0;
}

// +--------------------------------------------------------------------+
// Bind legacy FORM IDs to UMG widgets (optional)
// ---------------------------------------------------------------------+

void UMsnEditDlg::BindFormWidgets()
{
    // If you want legacy ID-based access, bind IDs here, e.g.:
    // BindButton(1, BtnAccept);
    // BindButton(2, BtnCancel);
    //
    // BindButton(301, BtnSit);
    // BindButton(302, BtnPkg);
    // BindButton(303, BtnMap);
    //
    // BindEdit(201, TxtName);
    // BindCombo(202, CmbType);
    // BindCombo(203, CmbSystem);
    // BindCombo(204, CmbRegion);
    //
    // BindEdit(410, TxtDescription);
    // BindEdit(411, TxtSituation);
    // BindEdit(412, TxtObjective);
    //
    // BindList(510, LstElem);
    // BindList(520, LstEvent);
}

// +--------------------------------------------------------------------+

void UMsnEditDlg::Show()
{
    // Legacy FormWindow::Show() equivalent: ensure widget is visible (handled by caller).
    // Here we just refresh all UI content from mission data.

    if (!TxtName || !CmbType)
        return;

    TxtName->SetText(FText::GetEmpty());

    if (CmbSystem)
    {
        CmbSystem->ClearOptions();

        Galaxy* galaxy = Galaxy::GetInstance();
        if (galaxy)
        {
            ListIter<StarSystem> iter = galaxy->GetSystemList();
            while (++iter)
            {
                CmbSystem->AddOption(FString(iter->Name()));
            }
        }
    }

    if (mission)
    {
        TxtName->SetText(FText::FromString(FString(mission->Name())));

        // Mission::Type() is an int index in the legacy code.
        if (CmbType)
        {
            // UComboBoxString does not have a "SetSelection(index)" API.
            // We attempt best-effort: select the option at that index if present.
            const int32 TypeIndex = mission->Type();
            if (TypeIndex >= 0 && TypeIndex < CmbType->GetOptionCount())
            {
                CmbType->SetSelectedIndex(TypeIndex);
            }
        }

        StarSystem* sys = mission->GetStarSystem();
        if (sys && CmbSystem && CmbRegion)
        {
            const FString SysName(sys->Name());
            CmbSystem->SetSelectedOption(SysName);

            CmbRegion->ClearOptions();
            int32 SelRgn = 0;

            List<OrbitalRegion> regions;
            regions.append(sys->AllRegions());
            regions.sort();

            int32 i = 0;
            ListIter<OrbitalRegion> riter = regions;
            while (++riter)
            {
                OrbitalRegion* region = riter.value();
                if (!region)
                    continue;

                const FString RegionName(region->Name());
                CmbRegion->AddOption(RegionName);

                if (mission->GetRegion() && !FCStringAnsi::Strcmp(mission->GetRegion(), region->Name()))
                    SelRgn = i;

                ++i;
            }

            if (SelRgn >= 0 && SelRgn < CmbRegion->GetOptionCount())
            {
                CmbRegion->SetSelectedIndex(SelRgn);
            }
        }

        if (TxtDescription && mission_info)
            TxtDescription->SetText(FText::FromString(FString(mission_info->description)));

        if (TxtSituation)
            TxtSituation->SetText(FText::FromString(FString(mission->Situation())));

        if (TxtObjective)
            TxtObjective->SetText(FText::FromString(FString(mission->Objective())));

        DrawPackages();
    }

    ShowTab(current_tab);

    exit_latch = true;
}

// +--------------------------------------------------------------------+
// NOTE: The legacy UI used ListBox with multi-column strings. UListView
// is row-object based and typically needs UObject items + entry widgets.
// This implementation keeps the control logic and button enable rules,
// and logs that list population should be implemented with list items.
// ---------------------------------------------------------------------+

void UMsnEditDlg::DrawPackages()
{
    bool elem_del = false;
    bool elem_edit = false;
    bool elem_inc = false;
    bool elem_dec = false;

    bool event_del = false;
    bool event_edit = false;
    bool event_inc = false;
    bool event_dec = false;

    if (mission)
    {
        // Elements list population:
        if (LstElem)
        {
            // TODO (UMG): Populate LstElem with UObject entries representing MissionElement rows.
            // This requires:
            //  - a UObject row item type (e.g., UMissionElementRowItem)
            //  - EntryWidgetClass for ListView
            //  - Assigning items via LstElem->SetListItems(...)
            //
            // Until that exists, we can still enable/disable buttons using mission data only.
            const int32 NumElems = mission->GetElements().size();
            const int32 Index = (NumElems > 0) ? 0 : -1;

            if (Index >= 0)
            {
                elem_del = true;
                elem_edit = true;
                elem_inc = Index > 0;
                elem_dec = Index < NumElems - 1;
            }
        }

        // Events list population:
        if (LstEvent)
        {
            // TODO (UMG): Populate LstEvent with UObject entries representing MissionEvent rows.
            const int32 NumEvents = mission->GetEvents().size();
            const int32 Index = (NumEvents > 0) ? 0 : -1;

            if (Index >= 0)
            {
                event_del = true;
                event_edit = true;
                event_inc = Index > 0;
                event_dec = Index < NumEvents - 1;
            }
        }
    }

    if (BtnElemDel)   BtnElemDel->SetIsEnabled(elem_del);
    if (BtnElemEdit)  BtnElemEdit->SetIsEnabled(elem_edit);
    if (BtnElemInc)   BtnElemInc->SetIsEnabled(elem_inc);
    if (BtnElemDec)   BtnElemDec->SetIsEnabled(elem_dec);

    if (BtnEventDel)  BtnEventDel->SetIsEnabled(event_del);
    if (BtnEventEdit) BtnEventEdit->SetIsEnabled(event_edit);
    if (BtnEventInc)  BtnEventInc->SetIsEnabled(event_inc);
    if (BtnEventDec)  BtnEventDec->SetIsEnabled(event_dec);
}

// +--------------------------------------------------------------------+

void UMsnEditDlg::ExecFrame()
{
    // Legacy code polled Keyboard::KeyDown(VK_RETURN / VK_ESCAPE).
    // In UE, proper input is usually handled via PlayerController / Enhanced Input.
    // If you need legacy behavior, implement it in the owning PlayerController and call
    // HandleAcceptClicked/HandleCancelClicked.
    //
    // We keep the latch semantics to preserve behavior if wired externally.
    if (!exit_latch)
    {
        // Placeholder: No direct polling here.
    }
    else
    {
        exit_latch = false;
    }
}

// +--------------------------------------------------------------------+

void UMsnEditDlg::ScrapeForm()
{
    if (!mission)
        return;

    if (TxtName)
    {
        const FString NewName = TxtName->GetText().ToString();
        mission->SetName(TCHAR_TO_ANSI(*NewName));
    }

    if (CmbType)
    {
        const int32 TypeIndex = CmbType->GetSelectedIndex();
        mission->SetType(TypeIndex);

        if (mission_info)
            mission_info->type = TypeIndex;
    }

    Galaxy* galaxy = Galaxy::GetInstance();
    StarSystem* system = nullptr;

    if (galaxy && CmbSystem)
    {
        const FString SelectedSystem = CmbSystem->GetSelectedOption();
        system = galaxy->GetSystem(TCHAR_TO_ANSI(*SelectedSystem));
    }

    if (system)
    {
        mission->ClearSystemList();
        mission->SetStarSystem(system);

        if (mission_info)
            mission_info->system = system->Name();
    }

    if (CmbRegion)
    {
        const FString SelectedRegion = CmbRegion->GetSelectedOption();
        mission->SetRegion(TCHAR_TO_ANSI(*SelectedRegion));

        if (mission_info)
            mission_info->region = TCHAR_TO_ANSI(*SelectedRegion);
    }

    if (TxtDescription && mission_info)
    {
        const FString Desc = TxtDescription->GetText().ToString();
        mission_info->description = TCHAR_TO_ANSI(*Desc);
        mission->SetDescription(TCHAR_TO_ANSI(*Desc));
    }

    if (TxtSituation)
    {
        const FString Sit = TxtSituation->GetText().ToString();
        mission->SetSituation(TCHAR_TO_ANSI(*Sit));
    }

    if (TxtObjective)
    {
        const FString Obj = TxtObjective->GetText().ToString();
        mission->SetObjective(TCHAR_TO_ANSI(*Obj));
    }
}

// +--------------------------------------------------------------------+

void UMsnEditDlg::ShowTab(int tab)
{
    current_tab = tab;
    if (current_tab < 0 || current_tab > 2)
        current_tab = 0;

    // In UMG, you typically show/hide panels. This port keeps the intent and
    // delegates layout to the widget hierarchy (BP or named panels).
    // You can implement actual switching by binding three panels and toggling visibility.

    if (current_tab == 2)
    {
        if (!manager)
        {
            UE_LOG(LogMsnEditDlg, Warning, TEXT("ShowTab(map): manager is null."));
            return;
        }

        NavDlg* navdlg = manager->GetNavDlg();
        if (navdlg)
        {
            navdlg->SetMission(mission);
            navdlg->SetEditorMode(true);
        }

        manager->ShowNavDlg();
    }
    else
    {
        if (manager)
            manager->HideNavDlg();
    }
}

// +--------------------------------------------------------------------+

void UMsnEditDlg::HandleTabSitClicked() { ShowTab(0); }
void UMsnEditDlg::HandleTabPkgClicked() { ShowTab(1); }
void UMsnEditDlg::HandleTabMapClicked() { ShowTab(2); }

// +--------------------------------------------------------------------+
// System selection:
// In UMG, bind CmbSystem->OnSelectionChanged to call this method.
// ---------------------------------------------------------------------+

void UMsnEditDlg::OnSystemSelect()
{
    StarSystem* sys = nullptr;

    if (CmbSystem)
    {
        const FString Name = CmbSystem->GetSelectedOption();
        Galaxy* galaxy = Galaxy::GetInstance();

        if (galaxy)
        {
            ListIter<StarSystem> iter = galaxy->GetSystemList();
            while (++iter)
            {
                StarSystem* s = iter.value();
                if (!s)
                    continue;

                if (!FCStringAnsi::Strcmp(s->Name(), TCHAR_TO_ANSI(*Name)))
                {
                    sys = s;
                    break;
                }
            }
        }
    }

    if (sys && CmbRegion)
    {
        CmbRegion->ClearOptions();

        List<OrbitalRegion> regions;
        regions.append(sys->AllRegions());
        regions.sort();

        ListIter<OrbitalRegion> iter = regions;
        while (++iter)
        {
            OrbitalRegion* region = iter.value();
            if (region)
                CmbRegion->AddOption(FString(region->Name()));
        }
    }

    ScrapeForm();

    if (manager)
    {
        NavDlg* navdlg = manager->GetNavDlg();
        if (navdlg)
            navdlg->SetMission(mission);
    }
}

// +--------------------------------------------------------------------+
// Element button handlers (UMG-driven)
// NOTE: List selection index retrieval requires your UListView item model.
// This port keeps behavior but logs when selection is not available.
// ---------------------------------------------------------------------+

void UMsnEditDlg::HandleElemAddClicked()
{
    if (!mission)
        return;

    List<MissionElement>& elements = mission->GetElements();
    MissionElement* elem = new MissionElement; // removed (__FILE__,__LINE__)

    if (elements.size() > 0)
        elem->SetLocation(RandomPoint());

    if (CmbRegion)
    {
        const FString Region = CmbRegion->GetSelectedOption();
        elem->SetRegion(TCHAR_TO_ANSI(*Region));
    }

    elements.append(elem);
    DrawPackages();

    if (manager)
    {
        MsnElemDlg* msn_elem_dlg = manager->GetMsnElemDlg();
        if (msn_elem_dlg)
        {
            ScrapeForm();
            msn_elem_dlg->SetMission(mission);
            msn_elem_dlg->SetMissionElement(elem);
            manager->ShowMsnElemDlg();
        }
    }
}

void UMsnEditDlg::HandleElemDelClicked()
{
    if (!mission)
        return;

    UE_LOG(LogMsnEditDlg, Warning, TEXT("ElemDel: selection requires UListView item model; implement selection index mapping."));
}

void UMsnEditDlg::HandleElemEditClicked()
{
    if (!mission)
        return;

    UE_LOG(LogMsnEditDlg, Warning, TEXT("ElemEdit: selection requires UListView item model; implement selection index mapping."));
}

void UMsnEditDlg::HandleElemIncClicked()
{
    if (!mission)
        return;

    UE_LOG(LogMsnEditDlg, Warning, TEXT("ElemInc: selection requires UListView item model; implement selection index mapping."));
}

void UMsnEditDlg::HandleElemDecClicked()
{
    if (!mission)
        return;

    UE_LOG(LogMsnEditDlg, Warning, TEXT("ElemDec: selection requires UListView item model; implement selection index mapping."));
}

// +--------------------------------------------------------------------+
// Event button handlers
// ---------------------------------------------------------------------+

void UMsnEditDlg::HandleEventAddClicked()
{
    if (!mission)
        return;

    List<MissionEvent>& events = mission->GetEvents();
    MissionEvent* event = new MissionEvent; // removed (__FILE__,__LINE__)

    int id = 1;
    for (int i = 0; i < events.size(); i++)
    {
        MissionEvent* e = events[i];
        if (e && e->EventID() >= id)
            id = e->EventID() + 1;
    }

    event->id = id;

    events.append(event);
    DrawPackages();

    if (manager)
    {
        MsnEventDlg* msn_event_dlg = manager->GetMsnEventDlg();
        if (msn_event_dlg)
        {
            ScrapeForm();
            msn_event_dlg->SetMission(mission);
            msn_event_dlg->SetMissionEvent(event);
            manager->ShowMsnEventDlg();
        }
    }
}

void UMsnEditDlg::HandleEventDelClicked()
{
    if (!mission)
        return;

    UE_LOG(LogMsnEditDlg, Warning, TEXT("EventDel: selection requires UListView item model; implement selection index mapping."));
}

void UMsnEditDlg::HandleEventEditClicked()
{
    if (!mission)
        return;

    UE_LOG(LogMsnEditDlg, Warning, TEXT("EventEdit: selection requires UListView item model; implement selection index mapping."));
}

void UMsnEditDlg::HandleEventIncClicked()
{
    if (!mission)
        return;

    UE_LOG(LogMsnEditDlg, Warning, TEXT("EventInc: selection requires UListView item model; implement selection index mapping."));
}

void UMsnEditDlg::HandleEventDecClicked()
{
    if (!mission)
        return;

    UE_LOG(LogMsnEditDlg, Warning, TEXT("EventDec: selection requires UListView item model; implement selection index mapping."));
}

// +--------------------------------------------------------------------+

void UMsnEditDlg::HandleAcceptClicked()
{
    if (mission)
    {
        ScrapeForm();

        if (!mission_info)
        {
            UE_LOG(LogMsnEditDlg, Warning, TEXT("Accept: mission_info is null."));
        }
        else
        {
            mission_info->name = mission->Name();
        }

        mission->Save();
    }

    if (manager)
        manager->ShowMsnSelectDlg();
}

void UMsnEditDlg::HandleCancelClicked()
{
    if (mission)
        mission->Load();

    if (manager)
        manager->ShowMsnSelectDlg();
}
