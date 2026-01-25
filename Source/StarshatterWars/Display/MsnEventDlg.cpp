/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         MsnEventDlg.cpp
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    OVERVIEW
    ========
    Mission Editor Event Dialog (UMG UserWidget) — Unreal port of the legacy
    mission event editor screen.
*/

#include "MsnEventDlg.h"
#include "GameStructs.h"

// UMG:
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"

// Legacy / Starshatter core:
#include "MsnEditDlg.h"
#include "MenuScreen.h"
#include "Campaign.h"
#include "Mission.h"
#include "MissionEvent.h"
#include "Instruction.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "StarSystem.h"
#include "Galaxy.h"

#include "Game.h"
#include "ParseUtil.h"

DEFINE_LOG_CATEGORY_STATIC(LogMsnEventDlg, Log, All);

// +--------------------------------------------------------------------+

UMsnEventDlg::UMsnEventDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

// +--------------------------------------------------------------------+

void UMsnEventDlg::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (BtnAccept) BtnAccept->OnClicked.AddDynamic(this, &UMsnEventDlg::HandleAcceptClicked);
    if (BtnCancel) BtnCancel->OnClicked.AddDynamic(this, &UMsnEventDlg::HandleCancelClicked);

    if (CmbEvent)  CmbEvent->OnSelectionChanged.AddDynamic(this, &UMsnEventDlg::HandleEventChanged);
}

void UMsnEventDlg::NativeConstruct()
{
    Super::NativeConstruct();
}

void UMsnEventDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    ExecFrame();
}

// +--------------------------------------------------------------------+

void UMsnEventDlg::BindFormWidgets()
{
    // Optional: bind legacy FORM IDs to widgets if you want ID-based lookups.
    // BindButton(1, BtnAccept);
    // BindButton(2, BtnCancel);
    //
    // BindLabel(201, LblId);
    // BindEdit(202, EdtTime);
    // BindEdit(203, EdtDelay);
    // BindCombo(204, CmbEvent);
    // BindCombo(205, CmbEventShip);
    // BindCombo(206, CmbEventSource);
    // BindCombo(207, CmbEventTarget);
    // BindEdit(208, EdtEventParam);
    // BindEdit(220, EdtEventChance);
    // BindEdit(209, EdtEventSound);
    // BindEdit(210, EdtEventMessage);
    //
    // BindCombo(221, CmbTrigger);
    // BindCombo(222, CmbTriggerShip);
    // BindCombo(223, CmbTriggerTarget);
    // BindEdit(224, EdtTriggerParam);
}

// +--------------------------------------------------------------------+

void UMsnEventDlg::SetMission(Mission* inMission)
{
    mission = inMission;
}

void UMsnEventDlg::SetMissionEvent(MissionEvent* inEvent)
{
    event = inEvent;
}

// +--------------------------------------------------------------------+

void UMsnEventDlg::Show()
{
    if (!event)
        return;

    FillShipList(CmbEventShip, event->EventShip());
    FillShipList(CmbEventSource, event->EventSource());

    if (event->Event() == MissionEvent::JUMP)
        FillRgnList(CmbEventTarget, event->EventTarget());
    else
        FillShipList(CmbEventTarget, event->EventTarget());

    FillShipList(CmbTriggerShip, event->TriggerShip());
    FillShipList(CmbTriggerTarget, event->TriggerTarget());

    if (LblId)
        LblId->SetText(FText::AsNumber(event->EventID()));

    if (EdtTime)
        EdtTime->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), (double)event->Time())));

    if (EdtDelay)
        EdtDelay->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), (double)event->Delay())));

    if (EdtEventChance)
        EdtEventChance->SetText(FText::AsNumber(event->EventChance()));

    if (EdtEventParam)
        EdtEventParam->SetText(FText::AsNumber(event->EventParam()));

    if (EdtTriggerParam)
        EdtTriggerParam->SetText(FText::FromString(FString(event->TriggerParamStr())));

    if (EdtEventMessage)
        EdtEventMessage->SetText(FText::FromString(FString(event->EventMessage())));

    if (EdtEventSound)
        EdtEventSound->SetText(FText::FromString(FString(event->EventSound())));

    if (CmbEvent)
    {
        CmbEvent->ClearOptions();

        for (int i = 0; i < MissionEvent::NUM_EVENTS; ++i)
            CmbEvent->AddOption(FString(MissionEvent::EventName(i)));

        CmbEvent->SetSelectedIndex(event->Event());
    }

    if (CmbTrigger)
    {
        CmbTrigger->ClearOptions();

        for (int i = 0; i < MissionEvent::NUM_TRIGGERS; ++i)
            CmbTrigger->AddOption(FString(MissionEvent::TriggerName(i)));

        CmbTrigger->SetSelectedIndex(event->Trigger());
    }
}

// +--------------------------------------------------------------------+

void UMsnEventDlg::FillShipList(UComboBoxString* Combo, const char* SelectedName)
{
    if (!Combo)
        return;

    Combo->ClearOptions();

    if (!mission)
        return;

    int index = 1;
    int selected_index = 0;

    Combo->AddOption(TEXT(""));

    List<MissionElement>& list = mission->GetElements();
    for (int i = 0; i < list.size(); ++i)
    {
        MissionElement* elem = list[i];
        if (!elem)
            continue;

        if (elem->IsSquadron())
            continue;

        if (elem->Count() == 1)
        {
            const FString Name = FString(elem->Name());
            Combo->AddOption(Name);

            if (SelectedName && elem->Name() == SelectedName)
                selected_index = index;

            index++;
        }
        else
        {
            for (int n = 0; n < elem->Count(); ++n)
            {
                const FString ShipName = FString::Printf(TEXT("%s %d"), ANSI_TO_TCHAR(elem->Name().data()), n + 1);
                Combo->AddOption(ShipName);

                if (SelectedName && !_stricmp(TCHAR_TO_ANSI(*ShipName), SelectedName))
                    selected_index = index;

                index++;
            }
        }
    }

    Combo->SetSelectedIndex(selected_index);
}

void UMsnEventDlg::FillRgnList(UComboBoxString* Combo, const char* SelectedName)
{
    if (!Combo)
        return;

    Combo->ClearOptions();

    if (!mission)
        return;

    int selected_index = 0;
    int i = 0;

    ListIter<StarSystem> iter = mission->GetSystemList();
    while (++iter)
    {
        StarSystem* sys = iter.value();
        if (!sys)
            continue;

        ListIter<OrbitalRegion> iter2 = sys->AllRegions();
        while (++iter2)
        {
            OrbitalRegion* region = iter2.value();
            if (!region)
                continue;

            const char* RgnName = region->Name();

            if (SelectedName && RgnName && !strcmp(RgnName, SelectedName))
                selected_index = i;

            Combo->AddOption(FString(RgnName));
            i++;
        }
    }

    Combo->SetSelectedIndex(selected_index);
}

// +--------------------------------------------------------------------+

void UMsnEventDlg::ExecFrame()
{
    // Legacy polled VK_RETURN. In UE, route input via PlayerController / focus
    // and call HandleAcceptClicked(). This remains as a compatibility hook.
}

// +--------------------------------------------------------------------+

void UMsnEventDlg::HandleEventChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (!CmbEventTarget || !event)
        return;

    if (CmbEvent && CmbEvent->GetSelectedIndex() == MissionEvent::JUMP)
        FillRgnList(CmbEventTarget, event->EventTarget());
    else
        FillShipList(CmbEventTarget, event->EventTarget());
}

// +--------------------------------------------------------------------+

void UMsnEventDlg::HandleAcceptClicked()
{
    if (mission && event)
    {
        // Time:
        if (EdtTime)
        {
            const float t = (float)FCString::Atof(*EdtTime->GetText().ToString());
            event->time = t;
        }

        // Delay:
        if (EdtDelay)
        {
            const float t = (float)FCString::Atof(*EdtDelay->GetText().ToString());
            event->delay = t;
        }

        // Event param (supports integer or "( ... )" array syntax):
        if (EdtEventParam)
        {
            const FString S = EdtEventParam->GetText().ToString();
            const FString Trim = S.TrimStartAndEnd();

            if (Trim.Len() > 0 && FChar::IsDigit(Trim[0]))
            {
                const int32 val = FCString::Atoi(*Trim);
                event->event_param[0] = val;
                event->event_nparams = 1;
            }
            else if (Trim.Len() > 0 && Trim[0] == TCHAR('('))
            {
                // Keep Starshatter parser types:
                Parser parser(new BlockReader(TCHAR_TO_ANSI(*Trim))); // removed (__FILE__,__LINE__)
                Term* term = parser.ParseTerm();

                if (term && term->isArray())
                {
                    TermArray* arr = term->isArray();
                    if (arr)
                    {
                        int nelem = arr->elements()->size();
                        if (nelem > 10) nelem = 10;

                        for (int i = 0; i < nelem; ++i)
                            event->event_param[i] = (int)(arr->elements()->at(i)->isNumber()->value());

                        event->event_nparams = nelem;
                    }
                }
            }
        }

        // Event chance:
        if (EdtEventChance)
        {
            const FString S = EdtEventChance->GetText().ToString().TrimStartAndEnd();
            const int32 val = (S.Len() > 0 && FChar::IsDigit(S[0])) ? FCString::Atoi(*S) : 0;
            event->event_chance = val;
        }

        // Message / sound:
        if (EdtEventMessage)
            event->event_message = TCHAR_TO_ANSI(*EdtEventMessage->GetText().ToString());

        if (EdtEventSound)
            event->event_sound = TCHAR_TO_ANSI(*EdtEventSound->GetText().ToString());

        // Trigger params:
        if (EdtTriggerParam)
        {
            const FString S = EdtTriggerParam->GetText().ToString();
            const FString Trim = S.TrimStartAndEnd();

            FMemory::Memzero(event->trigger_param, sizeof(event->trigger_param));

            if (Trim.Len() > 0 && FChar::IsDigit(Trim[0]))
            {
                const int32 val = FCString::Atoi(*Trim);
                event->trigger_param[0] = val;
                event->trigger_nparams = 1;
            }
            else if (Trim.Len() > 0 && Trim[0] == TCHAR('('))
            {
                Parser parser(new BlockReader(TCHAR_TO_ANSI(*Trim))); // removed (__FILE__,__LINE__)
                Term* term = parser.ParseTerm();

                if (term && term->isArray())
                {
                    TermArray* arr = term->isArray();
                    if (arr)
                    {
                        int nelem = arr->elements()->size();
                        if (nelem > 10) nelem = 10;

                        for (int i = 0; i < nelem; ++i)
                            event->trigger_param[i] = (int)(arr->elements()->at(i)->isNumber()->value());

                        event->trigger_nparams = nelem;
                    }
                }
            }
        }

        // Combo selections:
        if (CmbEvent)
            event->event = CmbEvent->GetSelectedIndex();

        if (CmbEventShip)
            event->event_ship = TCHAR_TO_ANSI(*CmbEventShip->GetSelectedOption());

        if (CmbEventSource)
            event->event_source = TCHAR_TO_ANSI(*CmbEventSource->GetSelectedOption());

        if (CmbEventTarget)
            event->event_target = TCHAR_TO_ANSI(*CmbEventTarget->GetSelectedOption());

        if (CmbTrigger)
            event->trigger = CmbTrigger->GetSelectedIndex();

        if (CmbTriggerShip)
            event->trigger_ship = TCHAR_TO_ANSI(*CmbTriggerShip->GetSelectedOption());

        if (CmbTriggerTarget)
            event->trigger_target = TCHAR_TO_ANSI(*CmbTriggerTarget->GetSelectedOption());
    }

    if (manager)
        manager->ShowMsnEditDlg();
}

void UMsnEventDlg::HandleCancelClicked()
{
    if (manager)
        manager->ShowMsnEditDlg();
}

