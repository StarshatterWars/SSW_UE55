#include "MissionEditorDlg.h"

// UMG
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"
#include "Components/ListView.h"

// Starshatter
#include "MenuScreen.h"
#include "Campaign.h"
#include "Mission.h"
#include "MissionInfo.h"
#include "MissionEvent.h"
#include "Galaxy.h"
#include "StarSystem.h"
#include "OrbitalRegion.h"
#include "Game.h"
#include "Random.h"

UMissionEditorDlg::UMissionEditorDlg()
{
}

void UMissionEditorDlg::InitializeMissionEditor(UMenuScreen* InManager)
{
    Manager = InManager;
}

void UMissionEditorDlg::SetMission(Mission* InMission)
{
    mission = InMission;
    current_tab = 0;
}

void UMissionEditorDlg::SetMissionInfo(MissionInfo* InMissionInfo)
{
    mission_info = InMissionInfo;
    current_tab = 0;
}

void UMissionEditorDlg::NativeConstruct()
{
    Super::NativeConstruct();

    // BaseScreen Enter/Escape
    ApplyButton = BtnAccept;
    CancelButton = BtnCancel;

    if (BtnAccept) BtnAccept->OnClicked.AddDynamic(this, &UMissionEditorDlg::OnAcceptClicked);
    if (BtnCancel) BtnCancel->OnClicked.AddDynamic(this, &UMissionEditorDlg::OnCancelClicked);

    if (BtnSit) BtnSit->OnClicked.AddDynamic(this, &UMissionEditorDlg::OnTabSitClicked);
    if (BtnPkg) BtnPkg->OnClicked.AddDynamic(this, &UMissionEditorDlg::OnTabPkgClicked);
    if (BtnMap) BtnMap->OnClicked.AddDynamic(this, &UMissionEditorDlg::OnTabMapClicked);

    if (BtnElemAdd)  BtnElemAdd->OnClicked.AddDynamic(this, &UMissionEditorDlg::OnElemAddClicked);
    if (BtnElemEdit) BtnElemEdit->OnClicked.AddDynamic(this, &UMissionEditorDlg::OnElemEditClicked);
    if (BtnElemDel)  BtnElemDel->OnClicked.AddDynamic(this, &UMissionEditorDlg::OnElemDelClicked);
    if (BtnElemInc)  BtnElemInc->OnClicked.AddDynamic(this, &UMissionEditorDlg::OnElemIncClicked);
    if (BtnElemDec)  BtnElemDec->OnClicked.AddDynamic(this, &UMissionEditorDlg::OnElemDecClicked);

    if (BtnEventAdd)  BtnEventAdd->OnClicked.AddDynamic(this, &UMissionEditorDlg::OnEventAddClicked);
    if (BtnEventEdit) BtnEventEdit->OnClicked.AddDynamic(this, &UMissionEditorDlg::OnEventEditClicked);
    if (BtnEventDel)  BtnEventDel->OnClicked.AddDynamic(this, &UMissionEditorDlg::OnEventDelClicked);
    if (BtnEventInc)  BtnEventInc->OnClicked.AddDynamic(this, &UMissionEditorDlg::OnEventIncClicked);
    if (BtnEventDec)  BtnEventDec->OnClicked.AddDynamic(this, &UMissionEditorDlg::OnEventDecClicked);

    if (CmbSystem)
    {
        CmbSystem->OnSelectionChanged.AddDynamic(this, &UMissionEditorDlg::OnSystemSelectChanged);
    }

    // Initial show
    Show();
}

FString UMissionEditorDlg::GetLegacyFormText() const
{
    // If you assign in editor, this will be used.
    // If empty, you can paste the .frm contents into LegacyFormText.
    return LegacyFormText;
}

void UMissionEditorDlg::BindFormWidgets()
{
    // Tabs
    BindButton(301, BtnSit);
    BindButton(302, BtnPkg);
    BindButton(303, BtnMap);

    // OK/Cancel
    BindButton(1, BtnAccept);
    BindButton(2, BtnCancel);

    // Fields
    BindEdit(201, TxtName);
    BindCombo(202, CmbType);
    BindCombo(203, CmbSystem);
    BindCombo(204, CmbRegion);

    BindEdit(410, TxtDescription);
    BindEdit(411, TxtSituation);
    BindEdit(412, TxtObjective);

    // Lists
    BindList(510, LstElem);
    BindList(520, LstEvent);

    // Element buttons
    BindButton(501, BtnElemAdd);
    BindButton(505, BtnElemEdit);
    BindButton(502, BtnElemDel);
    BindButton(503, BtnElemInc);
    BindButton(504, BtnElemDec);

    // Event buttons
    BindButton(511, BtnEventAdd);
    BindButton(515, BtnEventEdit);
    BindButton(512, BtnEventDel);
    BindButton(513, BtnEventInc);
    BindButton(514, BtnEventDec);
}

void UMissionEditorDlg::Show()
{
    if (!TxtName || !CmbType)
        return;

    // Clear fields first
    TxtName->SetText(FText::GetEmpty());

    // Populate system combo from Galaxy
    if (CmbSystem)
    {
        CmbSystem->ClearOptions();

        Galaxy* galaxy = Galaxy::GetInstance();
        if (galaxy)
        {
            ListIter<StarSystem> iter = galaxy->GetSystemList();
            while (++iter)
            {
                StarSystem* sys = iter.value();
                if (sys && sys->Name())
                    CmbSystem->AddOption(ANSI_TO_TCHAR(sys->Name()));
            }
        }
    }

    // Populate from mission
    if (mission)
    {
        TxtName->SetText(FText::FromString(ANSI_TO_TCHAR(mission->Name())));

        if (CmbType)
            CmbType->SetSelectedIndex(mission->Type());

        StarSystem* sys = mission->GetStarSystem();
        if (sys && CmbSystem && CmbRegion)
        {
            // Select system by name
            const FString SysName = ANSI_TO_TCHAR(sys->Name());
            CmbSystem->SetSelectedOption(SysName);

            // Fill regions
            CmbRegion->ClearOptions();

            List<OrbitalRegion> regions;
            regions.append(sys->AllRegions());
            regions.sort();

            int32 sel_rgn = 0;
            int32 idx = 0;

            ListIter<OrbitalRegion> riter = regions;
            while (++riter)
            {
                OrbitalRegion* region = riter.value();
                if (!region || !region->Name())
                    continue;

                const FString RgnName = ANSI_TO_TCHAR(region->Name());
                CmbRegion->AddOption(RgnName);

                if (!FCStringAnsi::Strcmp(mission->GetRegion(), region->Name()))
                    sel_rgn = idx;

                idx++;
            }

            CmbRegion->SetSelectedIndex(sel_rgn);
        }

        if (TxtDescription && mission_info)
            TxtDescription->SetText(FText::FromString(ANSI_TO_TCHAR(mission_info->description)));

        if (TxtSituation)
            TxtSituation->SetText(FText::FromString(ANSI_TO_TCHAR(mission->Situation())));

        if (TxtObjective)
            TxtObjective->SetText(FText::FromString(ANSI_TO_TCHAR(mission->Objective())));

        DrawPackages();
    }

    ShowTab(current_tab);
}

void UMissionEditorDlg::ScrapeForm()
{
    if (!mission)
        return;

    if (TxtName)
        mission->SetName(TCHAR_TO_ANSI(*TxtName->GetText().ToString()));

    if (CmbType)
    {
        mission->SetType(CmbType->GetSelectedIndex());
        if (mission_info)
            mission_info->type = CmbType->GetSelectedIndex();
    }

    // System
    Galaxy* galaxy = Galaxy::GetInstance();
    StarSystem* system = 0;

    if (galaxy && CmbSystem)
    {
        const FString Selected = CmbSystem->GetSelectedOption();
        system = galaxy->GetSystem(TCHAR_TO_ANSI(*Selected));
    }

    if (system)
    {
        mission->ClearSystemList();
        mission->SetStarSystem(system);

        if (mission_info)
            mission_info->system = system->Name();
    }

    // Region
    if (CmbRegion)
    {
        const FString Region = CmbRegion->GetSelectedOption();
        mission->SetRegion(TCHAR_TO_ANSI(*Region));

        if (mission_info)
            mission_info->region = TCHAR_TO_ANSI(*Region);
    }

    if (TxtDescription && mission_info)
    {
        const FString Desc = TxtDescription->GetText().ToString();
        mission_info->description = TCHAR_TO_ANSI(*Desc);
        mission->SetDescription(TCHAR_TO_ANSI(*Desc));
    }

    if (TxtSituation)
        mission->SetSituation(TCHAR_TO_ANSI(*TxtSituation->GetText().ToString()));

    if (TxtObjective)
        mission->SetObjective(TCHAR_TO_ANSI(*TxtObjective->GetText().ToString()));
}

void UMissionEditorDlg::ShowTab(int32 Tab)
{
    current_tab = Tab;
    if (current_tab < 0 || current_tab > 2)
        current_tab = 0;

    // In legacy: controls < 400 always show, controls in [400+tab*100 .. +100) show, else hide.
    // Here: we do the same with BaseScreen ID visibility helpers.
    const int32 low = 400 + current_tab * 100;
    const int32 high = low + 100;

    // Always-visible group: < 400
    // We do not have an enumeration of all IDs in this widget, so we use explicit list toggles:
    // Keep: tabs and info area live, and switch 400/500 blocks by ID ranges when those widgets are bound.
    //
    // Practical approach:
    // - If you bind additional widgets for ids 400..599 into BaseScreen maps, SetVisible() will work directly.
    //
    // For now, show/hide the two list blocks by tab:
    const bool bSit = (current_tab == 0);
    const bool bPkg = (current_tab == 1);
    const bool bMap = (current_tab == 2);

    // SIT tab: description/situation/objective (410-412) and possibly 400 panel widgets
    if (TxtDescription) TxtDescription->SetVisibility(bSit ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    if (TxtSituation)   TxtSituation->SetVisibility(bSit ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    if (TxtObjective)   TxtObjective->SetVisibility(bSit ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

    // PKG tab: element/event lists and buttons
    if (LstElem)  LstElem->SetVisibility(bPkg ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    if (LstEvent) LstEvent->SetVisibility(bPkg ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

    if (BtnElemAdd)  BtnElemAdd->SetVisibility(bPkg ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    if (BtnElemEdit) BtnElemEdit->SetVisibility(bPkg ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    if (BtnElemDel)  BtnElemDel->SetVisibility(bPkg ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    if (BtnElemInc)  BtnElemInc->SetVisibility(bPkg ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    if (BtnElemDec)  BtnElemDec->SetVisibility(bPkg ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

    if (BtnEventAdd)  BtnEventAdd->SetVisibility(bPkg ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    if (BtnEventEdit) BtnEventEdit->SetVisibility(bPkg ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    if (BtnEventDel)  BtnEventDel->SetVisibility(bPkg ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    if (BtnEventInc)  BtnEventInc->SetVisibility(bPkg ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    if (BtnEventDec)  BtnEventDec->SetVisibility(bPkg ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

    // MAP tab: in legacy, it pushes the NavDlg screen.
    if (bMap && Manager)
    {
        // Preserve original behavior: scrape edits before switching screens.
        ScrapeForm();

        // If your UMenuScreen already has Nav dialog methods, call them here.
        // Example (adjust to your actual API):
        // Manager->ShowNavDlg();
    }
}

void UMissionEditorDlg::DrawPackages()
{
    // This method controls enable/disable state similarly to legacy.
    // Actual list contents require a row widget + item data; not included here.

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
        // If you implement selection in your list view, update these.
        // For now, leave them false until you wire list selection.
    }

    if (BtnElemDel)  BtnElemDel->SetIsEnabled(elem_del);
    if (BtnElemEdit) BtnElemEdit->SetIsEnabled(elem_edit);
    if (BtnElemInc)  BtnElemInc->SetIsEnabled(elem_inc);
    if (BtnElemDec)  BtnElemDec->SetIsEnabled(elem_dec);

    if (BtnEventDel)  BtnEventDel->SetIsEnabled(event_del);
    if (BtnEventEdit) BtnEventEdit->SetIsEnabled(event_edit);
    if (BtnEventInc)  BtnEventInc->SetIsEnabled(event_inc);
    if (BtnEventDec)  BtnEventDec->SetIsEnabled(event_dec);
}

// ------------------------------------------------------------
// UI event handlers
// ------------------------------------------------------------

void UMissionEditorDlg::OnTabSitClicked()
{
    ShowTab(0);
}

void UMissionEditorDlg::OnTabPkgClicked()
{
    ShowTab(1);
}

void UMissionEditorDlg::OnTabMapClicked()
{
    ShowTab(2);
}

void UMissionEditorDlg::OnSystemSelectChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    // Fill regions for selected system
    StarSystem* sys = 0;

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
                if (s && s->Name() && Name.Equals(ANSI_TO_TCHAR(s->Name())))
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
            if (region && region->Name())
                CmbRegion->AddOption(ANSI_TO_TCHAR(region->Name()));
        }
    }

    ScrapeForm();

    // In legacy, it also updates NavDlg mission pointer.
    // If you have a Nav dialog instance in Manager, you can set it here.
}

void UMissionEditorDlg::OnAcceptClicked()
{
    if (mission)
    {
        ScrapeForm();

        if (mission_info)
        {
            mission_info->name = mission->Name();
        }

        mission->Save();
    }

    if (Manager)
    {
        // Legacy: manager->ShowMsnSelectDlg();
        // Adjust to your actual menu navigation API.
        // Manager->ShowMsnSelectDlg();
    }
}

void UMissionEditorDlg::OnCancelClicked()
{
    if (mission)
        mission->Load();

    if (Manager)
    {
        // Legacy: manager->ShowMsnSelectDlg();
        // Manager->ShowMsnSelectDlg();
    }
}

// ------------------------------------------------------------
// Element ops (call into your dialog manager if available)
// ------------------------------------------------------------

void UMissionEditorDlg::OnElemAddClicked()
{
    if (!mission)
        return;

    // Legacy: add element, random location if elements exist
    List<MissionElement>& elements = mission->GetElements();
    MissionElement* elem = new(__FILE__, __LINE__) MissionElement;

    if (elements.size() > 0)
        elem->SetLocation(RandomPoint());

    if (CmbRegion)
        elem->SetRegion(TCHAR_TO_ANSI(*CmbRegion->GetSelectedOption()));

    elements.append(elem);

    DrawPackages();

    if (Manager)
    {
        // Legacy: open element dialog
        // ScrapeForm();
        // Manager->ShowMsnElemDlg();
    }
}

void UMissionEditorDlg::OnElemDelClicked()
{
    if (!mission)
        return;

    // Requires selection wiring for UE list view.
    // When you implement selection: delete elements.removeIndex(SelectedIndex)
    DrawPackages();
}

void UMissionEditorDlg::OnElemEditClicked()
{
    if (!mission)
        return;

    // Requires selection wiring for UE list view.
    // When you implement selection: open element dialog on selected element.
}

void UMissionEditorDlg::OnElemIncClicked()
{
    if (!mission)
        return;

    // Requires selection wiring.
    DrawPackages();
}

void UMissionEditorDlg::OnElemDecClicked()
{
    if (!mission)
        return;

    // Requires selection wiring.
    DrawPackages();
}

// ------------------------------------------------------------
// Event ops (call into your dialog manager if available)
// ------------------------------------------------------------

void UMissionEditorDlg::OnEventAddClicked()
{
    if (!mission)
        return;

    List<MissionEvent>& events = mission->GetEvents();
    MissionEvent* event = new(__FILE__, __LINE__) MissionEvent;

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

    if (Manager)
    {
        // Legacy: open event dialog
        // ScrapeForm();
        // Manager->ShowMsnEventDlg();
    }
}

void UMissionEditorDlg::OnEventDelClicked()
{
    if (!mission)
        return;

    // Requires selection wiring.
    DrawPackages();
}

void UMissionEditorDlg::OnEventEditClicked()
{
    if (!mission)
        return;

    // Requires selection wiring.
}

void UMissionEditorDlg::OnEventIncClicked()
{
    if (!mission)
        return;

    // Requires selection wiring.
    DrawPackages();
}

void UMissionEditorDlg::OnEventDecClicked()
{
    if (!mission)
        return;

    // Requires selection wiring.
    DrawPackages();
}
