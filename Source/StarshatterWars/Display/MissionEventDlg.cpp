/*  Project Starshatter Wars
    Fractal Dev Studios LLC
    Copyright (c) 2025-2026

    SUBSYSTEM:    Stars.exe (Unreal Port)
    FILE:         MissionEventDlg.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UMissionEventDlg
    - Unreal UUserWidget replacement for legacy MsnEventDlg.
    - Uses standard UMG widget bindings (BindWidgetOptional).
    - Bindings correspond to MsnEventDlg.frm control IDs.
*/

#include "MissionEventDlg.h"

#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Input/Reply.h"
#include "Framework/Application/SlateApplication.h"

#include "Keyboard.h"          
#include "MissionEvent.h"
#include "MissionElement.h"
#include "StarSystem.h"
#include "ParseUtil.h"        
#include "Game.h"

// NOTE:
// - This implementation preserves legacy behavior where possible.
// - Legacy array parsing for params "(1,2,3)" is preserved via ParseUtil.

UMissionEventDlg::UMissionEventDlg()
{
}

void UMissionEventDlg::InitializeDlg(UMenuScreen* InManager)
{
    Manager = InManager;
}

void UMissionEventDlg::SetMission(Mission* InMission)
{
    MissionPtr = InMission;
}

void UMissionEventDlg::SetMissionEvent(MissionEvent* InEvent)
{
    EventPtr = InEvent;
}

void UMissionEventDlg::NativeConstruct()
{
    Super::NativeConstruct();

    if (AcceptButton)
    {
        AcceptButton->OnClicked.Clear();
        AcceptButton->OnClicked.AddDynamic(this, &UMissionEventDlg::OnAcceptClicked);
    }

    if (CancelButton)
    {
        CancelButton->OnClicked.Clear();
        CancelButton->OnClicked.AddDynamic(this, &UMissionEventDlg::OnCancelClicked);
    }

    if (EventCombo)
    {
        EventCombo->OnSelectionChanged.Clear();
        EventCombo->OnSelectionChanged.AddDynamic(this, &UMissionEventDlg::OnEventSelectionChanged);
    }
}

FReply UMissionEventDlg::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey Key = InKeyEvent.GetKey();

    if (Key == EKeys::Enter || Key == EKeys::Virtual_Accept)
    {
        OnAcceptClicked();
        return FReply::Handled();
    }

    if (Key == EKeys::Escape)
    {
        OnCancelClicked();
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UMissionEventDlg::ShowDlg()
{
    SetVisibility(ESlateVisibility::Visible);
    RebuildFromModel();
}

bool UMissionEventDlg::ParseFloatBox(UEditableTextBox* Box, float& OutValue)
{
    if (!Box) return false;
    const FString S = Box->GetText().ToString().TrimStartAndEnd();
    if (S.IsEmpty()) return false;
    OutValue = FCString::Atof(*S);
    return true;
}

bool UMissionEventDlg::ParseIntBox(UEditableTextBox* Box, int32& OutValue)
{
    if (!Box) return false;
    const FString S = Box->GetText().ToString().TrimStartAndEnd();
    if (S.IsEmpty()) return false;

    // allow negative:
    if (!S.IsNumeric() && !(S.Len() > 1 && S[0] == '-' && S.Mid(1).IsNumeric()))
        return false;

    OutValue = FCString::Atoi(*S);
    return true;
}

void UMissionEventDlg::RebuildFromModel()
{
    if (!EventPtr)
        return;

    // Ship/source/target lists:
    FillShipList(EventShipCombo, EventPtr->EventShip());
    FillShipList(EventSourceCombo, EventPtr->EventSource());

    RefreshTargetListForSelectedEvent();

    FillShipList(TriggerShipCombo, EventPtr->TriggerShip());
    FillShipList(TriggerTargetCombo, EventPtr->TriggerTarget());

    // ID label:
    if (IdLabel)
    {
        IdLabel->SetText(FText::AsNumber(EventPtr->EventID()));
    }

    if (TimeEdit)
    {
        TimeEdit->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), EventPtr->Time())));
    }

    if (DelayEdit)
    {
        DelayEdit->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), EventPtr->Delay())));
    }

    if (ChanceEdit)
    {
        ChanceEdit->SetText(FText::AsNumber((int32)EventPtr->EventChance()));
    }

    if (EventParamEdit)
    {
        EventParamEdit->SetText(FText::AsNumber((int32)EventPtr->EventParam()));
    }

    if (TriggerParamEdit)
    {
        TriggerParamEdit->SetText(FText::FromString(UTF8_TO_TCHAR(EventPtr->TriggerParamStr())));
    }

    if (EventMessageEdit)
    {
        EventMessageEdit->SetText(FText::FromString(UTF8_TO_TCHAR(EventPtr->EventMessage())));
    }

    if (EventSoundEdit)
    {
        EventSoundEdit->SetText(FText::FromString(UTF8_TO_TCHAR(EventPtr->EventSound())));
    }

    // Event combo options:
    if (EventCombo)
    {
        EventCombo->ClearOptions();

        for (int i = 0; i < MissionEvent::NUM_EVENTS; ++i)
        {
            EventCombo->AddOption(UTF8_TO_TCHAR(MissionEvent::EventName(i)));
        }

        EventCombo->SetSelectedIndex(EventPtr->Event());
    }

    // Trigger combo options:
    if (TriggerCombo)
    {
        TriggerCombo->ClearOptions();

        for (int i = 0; i < MissionEvent::NUM_TRIGGERS; ++i)
        {
            TriggerCombo->AddOption(UTF8_TO_TCHAR(MissionEvent::TriggerName(i)));
        }

        TriggerCombo->SetSelectedIndex(EventPtr->Trigger());
    }
}

void UMissionEventDlg::FillShipList(UComboBoxString* Combo, const char* SelectedAnsi)
{
    if (!Combo)
        return;

    Combo->ClearOptions();

    if (!MissionPtr)
        return;

    // legacy: first item is blank
    Combo->AddOption(TEXT(""));

    int32 SelectedIndex = 0;
    int32 Index = 1;

    List<MissionElement>& ListRef = MissionPtr->GetElements();
    for (int i = 0; i < ListRef.size(); ++i)
    {
        MissionElement* Elem = ListRef[i];
        if (!Elem) continue;

        if (Elem->IsSquadron())
            continue;

        if (Elem->Count() == 1)
        {
            const FString Name = UTF8_TO_TCHAR(Elem->Name());
            Combo->AddOption(Name);

            if (SelectedAnsi && (Elem->Name() == SelectedAnsi))
                SelectedIndex = Index;

            ++Index;
        }
        else
        {
            for (int n = 0; n < Elem->Count(); ++n)
            {
                const FString ShipName = FString::Printf(TEXT("%s %d"), UTF8_TO_TCHAR(Elem->Name().data()), n + 1);
                Combo->AddOption(ShipName);

                if (SelectedAnsi && !_stricmp(TCHAR_TO_UTF8(*ShipName), SelectedAnsi))
                    SelectedIndex = Index;

                ++Index;
            }
        }
    }

    Combo->SetSelectedIndex(SelectedIndex);
}

void UMissionEventDlg::FillRgnList(UComboBoxString* Combo, const char* SelectedAnsi)
{
    if (!Combo)
        return;

    Combo->ClearOptions();

    if (!MissionPtr)
        return;

    int32 SelectedIndex = 0;
    int32 i = 0;

    ListIter<StarSystem> SysIter = MissionPtr->GetSystemList();
    while (++SysIter)
    {
        StarSystem* Sys = SysIter.value();
        if (!Sys) continue;

        ListIter<OrbitalRegion> RgnIter = Sys->AllRegions();
        while (++RgnIter)
        {
            OrbitalRegion* R = RgnIter.value();
            if (!R) continue;

            const FString Name = UTF8_TO_TCHAR(R->Name());
            Combo->AddOption(Name);

            if (SelectedAnsi && !strcmp(R->Name(), SelectedAnsi))
                SelectedIndex = i;

            ++i;
        }
    }

    Combo->SetSelectedIndex(SelectedIndex);
}

void UMissionEventDlg::RefreshTargetListForSelectedEvent()
{
    if (!EventPtr)
        return;

    // If Event is JUMP, target is a REGION list; otherwise ship list
    const int32 EventId = (EventCombo) ? EventCombo->GetSelectedIndex() : EventPtr->Event();

    if (EventId == MissionEvent::JUMP)
        FillRgnList(EventTargetCombo, EventPtr->EventTarget());
    else
        FillShipList(EventTargetCombo, EventPtr->EventTarget());
}

void UMissionEventDlg::OnEventSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    RefreshTargetListForSelectedEvent();
}

void UMissionEventDlg::OnAcceptClicked()
{
    if (MissionPtr && EventPtr)
    {
        // time
        if (TimeEdit)
        {
            float T = 0.0f;
            if (ParseFloatBox(TimeEdit, T))
                EventPtr->time = T;
        }

        // delay
        if (DelayEdit)
        {
            float D = 0.0f;
            if (ParseFloatBox(DelayEdit, D))
                EventPtr->delay = D;
        }

        // event param (supports numeric or "(a,b,c)" array)
        if (EventParamEdit)
        {
            const FString S = EventParamEdit->GetText().ToString().TrimStartAndEnd();

            if (!S.IsEmpty())
            {
                const char* Buf = TCHAR_TO_UTF8(*S);

                if (isdigit(*Buf) || *Buf == '-')
                {
                    int32 Val = FCString::Atoi(*S);
                    EventPtr->event_param[0] = Val;
                    EventPtr->event_nparams = 1;
                }
                else if (*Buf == '(')
                {
                    Parser ParserObj(new BlockReader(Buf));
                    Term* TermObj = ParserObj.ParseTerm();

                    if (TermObj && TermObj->isArray())
                    {
                        TermArray* Arr = TermObj->isArray();
                        if (Arr)
                        {
                            const int N = Arr->elements()->size();
                            const int NUse = (N > 10) ? 10 : N;

                            for (int i = 0; i < NUse; ++i)
                                EventPtr->event_param[i] = (int)(Arr->elements()->at(i)->isNumber()->value());

                            EventPtr->event_nparams = NUse;
                        }
                    }
                }
            }
        }

        // chance
        if (ChanceEdit)
        {
            int32 Val = 0;
            if (!ParseIntBox(ChanceEdit, Val))
                Val = 0;

            EventPtr->event_chance = Val;
        }

        // message/sound
        if (EventMessageEdit)
            EventPtr->event_message = TCHAR_TO_UTF8(*EventMessageEdit->GetText().ToString());

        if (EventSoundEdit)
            EventPtr->event_sound = TCHAR_TO_UTF8(*EventSoundEdit->GetText().ToString());

        // trigger param (numeric or "(a,b,c)" array)
        if (TriggerParamEdit)
        {
            const FString S = TriggerParamEdit->GetText().ToString().TrimStartAndEnd();
            const char* Buf = TCHAR_TO_UTF8(*S);

            ZeroMemory(EventPtr->trigger_param, sizeof(EventPtr->trigger_param));
            EventPtr->trigger_nparams = 0;

            if (!S.IsEmpty())
            {
                if (isdigit(*Buf) || *Buf == '-')
                {
                    int32 Val = FCString::Atoi(*S);
                    EventPtr->trigger_param[0] = Val;
                    EventPtr->trigger_nparams = 1;
                }
                else if (*Buf == '(')
                {
                    Parser ParserObj(new(__FILE__, __LINE__) BlockReader(Buf));
                    Term* TermObj = ParserObj.ParseTerm();

                    if (TermObj && TermObj->isArray())
                    {
                        TermArray* Arr = TermObj->isArray();
                        if (Arr)
                        {
                            const int N = Arr->elements()->size();
                            const int NUse = (N > 10) ? 10 : N;

                            for (int i = 0; i < NUse; ++i)
                                EventPtr->trigger_param[i] = (int)(Arr->elements()->at(i)->isNumber()->value());

                            EventPtr->trigger_nparams = NUse;
                        }
                    }
                }
            }
        }

        // combos -> event fields
        if (EventCombo)
            EventPtr->event = EventCombo->GetSelectedIndex();

        if (EventShipCombo)
            EventPtr->event_ship = TCHAR_TO_UTF8(*EventShipCombo->GetSelectedOption());

        if (EventSourceCombo)
            EventPtr->event_source = TCHAR_TO_UTF8(*EventSourceCombo->GetSelectedOption());

        if (EventTargetCombo)
            EventPtr->event_target = TCHAR_TO_UTF8(*EventTargetCombo->GetSelectedOption());

        if (TriggerCombo)
            EventPtr->trigger = TriggerCombo->GetSelectedIndex();

        if (TriggerShipCombo)
            EventPtr->trigger_ship = TCHAR_TO_UTF8(*TriggerShipCombo->GetSelectedOption());

        if (TriggerTargetCombo)
            EventPtr->trigger_target = TCHAR_TO_UTF8(*TriggerTargetCombo->GetSelectedOption());
    }

    if (Manager)
    {
        // Manager->ShowMsnEditDlg();
    }
}

void UMissionEventDlg::OnCancelClicked()
{
    if (Manager)
    {
        // Manager->ShowMsnEditDlg();
    }
}
