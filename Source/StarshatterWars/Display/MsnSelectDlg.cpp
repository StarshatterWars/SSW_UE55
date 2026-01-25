/*  Project Starshatter 5.0
    Destroyer Studios LLC
    Copyright © 1997-2007. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         MsnSelectDlg.cpp
    AUTHOR:       John DiCamillo

    UNREAL PORT:
    - Converted from FormWindow to UBaseScreen (UUserWidget).
    - Uses UComboBoxString + UListView + UButton + UTextBlock.
    - Preserves original control intent and state transitions.
*/

#include "MsnSelectDlg.h"

#include "MenuListItem.h"

// Your game-side singletons/models:
#include "Starshatter.h"
#include "Campaign.h"
#include "Mission.h"

// Optional manager screens/dialogs (stubs you likely have in your menu system):
// #include "MsnEditDlg.h"
// #include "MsnEditNavDlg.h"
// #include "ConfirmDlg.h"
// #include "MenuScreen.h"

#include "Components/ListView.h"
#include "Components/ComboBoxString.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Input/Reply.h"
#include "Engine/Engine.h"

// ---------------------------------------------------------------------
// Original static:
static Mission* GEditMission = nullptr;

// ---------------------------------------------------------------------
// Small formatting helpers (replace with your own FormatUtil equivalents)

static FString FormatDayTimeString(int32 StartSecondsOrWhatever)
{
    // TODO: Replace with your real time formatting.
    // Original uses FormatDayTime(time_buf, info->start);
    return FString::Printf(TEXT("%d"), StartSecondsOrWhatever);
}

static FString MissionRoleNameSafe(int32 Role)
{
    // TODO: Replace with Mission::RoleName(...) in your port
    // return UTF8_TO_TCHAR(Mission::RoleName(Role));
    return FString::Printf(TEXT("ROLE_%d"), Role);
}

// ---------------------------------------------------------------------

UMsnSelectDlg::UMsnSelectDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // mirrors old ctor initialization
    stars = nullptr;
    campaign = nullptr;
    selected_mission = -1;
    mission_id = 0;
    editable = false;
}

void UMsnSelectDlg::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // Cache model singletons:
    stars = Starshatter::GetInstance();
    campaign = Campaign::GetCampaign();
    GEditMission = nullptr;

    RegisterControls();
}

void UMsnSelectDlg::NativeConstruct()
{
    Super::NativeConstruct();

    // In Starshatter, Show() is invoked when dialog is presented.
    // In UE, you may call Show() from your menu manager when switching screens.
}

void UMsnSelectDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Prefer not to poll keys each tick.
    // ExecFrame() kept for parity; only call if you explicitly want it.
}

FReply UMsnSelectDlg::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey Key = InKeyEvent.GetKey();

    if (Key == EKeys::Enter || Key == EKeys::Virtual_Accept)
    {
        if (btn_accept && btn_accept->GetIsEnabled())
        {
            OnAccept();
            return FReply::Handled();
        }
    }

    if (Key == EKeys::Escape)
    {
        OnCancel();
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

// ---------------------------------------------------------------------
// Starshatter-like methods (ported API surface)

void UMsnSelectDlg::RegisterControls()
{
    // Buttons
    if (btn_accept)
    {
        btn_accept->SetIsEnabled(false);
        btn_accept->OnClicked.Clear();
        btn_accept->OnClicked.AddDynamic(this, &UMsnSelectDlg::OnAccept);
    }

    if (btn_cancel)
    {
        btn_cancel->OnClicked.Clear();
        btn_cancel->OnClicked.AddDynamic(this, &UMsnSelectDlg::OnCancel);
    }

    if (btn_mod)
    {
        btn_mod->OnClicked.Clear();
        btn_mod->OnClicked.AddDynamic(this, &UMsnSelectDlg::OnMod);
    }

    if (btn_new)
    {
        btn_new->OnClicked.Clear();
        btn_new->OnClicked.AddDynamic(this, &UMsnSelectDlg::OnNew);
    }

    if (btn_edit)
    {
        btn_edit->OnClicked.Clear();
        btn_edit->OnClicked.AddDynamic(this, &UMsnSelectDlg::OnEdit);
    }

    if (btn_del)
    {
        btn_del->OnClicked.Clear();
        btn_del->OnClicked.AddDynamic(this, &UMsnSelectDlg::OnDel);

        // “User_1” confirm path in classic UI becomes a separate confirm callback.
        // Your confirm dialog should call OnDelConfirm() if user accepts.
        // (No binding here; your Confirm UI should bind to this.)
    }

    // Campaign selectors:
    if (cmb_campaigns)
    {
        cmb_campaigns->OnSelectionChanged.Clear();
        cmb_campaigns->OnSelectionChanged.AddDynamic(
            this, &UMsnSelectDlg::OnCampaignSelect
        );
    }

    if (lst_campaigns)
    {
        lst_campaigns->OnItemSelectionChanged().AddUObject(
            this, &UMsnSelectDlg::OnCampaignSelect
        );
    }

    // Mission selector:
    if (lst_missions)
    {
        lst_missions->OnItemSelectionChanged().AddUObject(
            this, &UMsnSelectDlg::OnMissionSelect
        );
    }
}

void UMsnSelectDlg::Show()
{
    // Equivalent of FormWindow::Show(); in UE you’d SetVisibility + focus.
    SetVisibility(ESlateVisibility::Visible);
    SetKeyboardFocus();

    campaign = Campaign::GetCampaign();

    // Populate campaign list (ComboBoxString OR ListView)
    if (cmb_campaigns)
    {
        int32 N = 0;
        cmb_campaigns->ClearOptions();

        for (Campaign* C : Campaign::GetAllCampaignsArray())
        {
            if (!C) continue;

            if (C->GetCampaignId() >= Campaign::SINGLE_MISSIONS)
            {
                cmb_campaigns->AddOption(C->Name());

                if (campaign && campaign->GetCampaignId() < Campaign::SINGLE_MISSIONS)
                {
                    campaign = Campaign::SelectCampaign(C->Name());
                    cmb_campaigns->SetSelectedIndex(N);
                }
                else if (campaign && campaign->GetCampaignId() == C->GetCampaignId())
                {
                    cmb_campaigns->SetSelectedIndex(N);
                }

                ++N;
            }
        }
    }
    else if (lst_campaigns)
    {
        TArray<UObject*> Items;
        int32 N = 0;

        for (Campaign* C : Campaign::GetAllCampaignsArray())
        {
            if (!C) continue;

            if (C->GetCampaignId() >= Campaign::SINGLE_MISSIONS)
            {
                Items.Add(UMenuListItem::Make(this, C->Name(), C->GetCampaignId()));
                if (campaign && campaign->GetCampaignId() < Campaign::SINGLE_MISSIONS)
                {
                    campaign = Campaign::SelectCampaign(C->Name());
                    // select after list set
                }
                ++N;
            }
        }

        lst_campaigns->ClearListItems();
        lst_campaigns->SetListItems(Items);

        // Select current campaign:
        if (campaign)
        {
            for (UObject* Obj : Items)
            {
                if (const UMenuListItem* Item = Cast<UMenuListItem>(Obj))
                {
                    if (Item->Label == campaign->Name())
                    {
                        lst_campaigns->SetSelectedItem(Obj);
                        break;
                    }
                }
            }
        }
    }

    // Editable flags (same logic):
    if (campaign)
    {
        const int32 Id = campaign->GetCampaignId();
        editable = (Id >= Campaign::MULTIPLAYER_MISSIONS && Id <= Campaign::CUSTOM_MISSIONS);

        if (btn_new)  btn_new->SetIsEnabled(editable);
        if (btn_edit) btn_edit->SetIsEnabled(false);
        if (btn_del)  btn_del->SetIsEnabled(false);
    }

    if (description)
    {
        description->SetText(FText::FromString(Game::GetText(TEXT("MsnSelectDlg.choose"))));
    }

    // Populate missions:
    if (lst_missions)
    {
        TArray<UObject*> MissionItems;
        if (campaign)
        {
            const TArray<MissionInfo*>& List = campaign->GetMissionListArray();
            for (MissionInfo* Info : List)
            {
                if (!Info) continue;

                MissionItems.Add(UMenuListItem::Make(this, Info->name, Info->id));

                if (Info->mission && Info->mission == GEditMission)
                {
                    // select later
                }
            }
        }

        lst_missions->ClearListItems();
        lst_missions->SetListItems(MissionItems);

        // Reselect edit mission if any:
        if (GEditMission && campaign)
        {
            const TArray<MissionInfo*>& List = campaign->GetMissionListArray();
            for (int32 i = 0; i < List.Num(); ++i)
            {
                MissionInfo* Info = List[i];
                if (Info && Info->mission == GEditMission)
                {
                    if (MissionItems.IsValidIndex(i))
                    {
                        lst_missions->SetSelectedItem(MissionItems[i]);
                        break;
                    }
                }
            }
        }

        // Reselect previous selection if still valid:
        if (selected_mission >= 0 && selected_mission < MissionItems.Num() && lst_missions->GetSelectedItems().Num() == 0)
        {
            lst_missions->SetSelectedItem(MissionItems[selected_mission]);
        }

        OnMissionSelect(nullptr);
        GEditMission = nullptr;
    }
}

void UMsnSelectDlg::ExecFrame()
{
    // Avoid per-frame key polling; kept for parity if you insist on calling it.
}

// ---------------------------------------------------------------------
// Event handlers

void UMsnSelectDlg::OnCampaignSelect()
{
    FString SelectedCampaign;

    if (cmb_campaigns)
    {
        SelectedCampaign = cmb_campaigns->GetSelectedOption();
    }
    else if (lst_campaigns)
    {
        UObject* Sel = lst_campaigns->GetSelectedItem();
        if (const UMenuListItem* Item = Cast<UMenuListItem>(Sel))
        {
            SelectedCampaign = Item->Label;
        }
    }

    if (SelectedCampaign.IsEmpty())
        return;

    Campaign* C = Campaign::SelectCampaign(SelectedCampaign);
    if (!C)
        return;

    campaign = C;

    // Populate missions:
    if (lst_missions)
    {
        TArray<UObject*> MissionItems;
        const TArray<MissionInfo*>& List = campaign->GetMissionListArray();

        for (MissionInfo* Info : List)
        {
            if (!Info) continue;
            MissionItems.Add(UMenuListItem::Make(this, Info->name, Info->id));
        }

        lst_missions->ClearListItems();
        lst_missions->SetListItems(MissionItems);
        lst_missions->ScrollToTop();
    }

    if (btn_accept)
        btn_accept->SetIsEnabled(false);

    if (description)
        description->SetText(FText::FromString(Game::GetText(TEXT("MsnSelectDlg.choose"))));

    const int32 Id = C->GetCampaignId();
    editable = (Id >= Campaign::MULTIPLAYER_MISSIONS && Id <= Campaign::CUSTOM_MISSIONS);

    if (btn_new)  btn_new->SetIsEnabled(editable);
    if (btn_edit) btn_edit->SetIsEnabled(false);
    if (btn_del)  btn_del->SetIsEnabled(false);
}

void UMsnSelectDlg::OnMissionSelect()
{
    selected_mission = -1;

    if (!lst_missions)
        return;

    UObject* Sel = lst_missions->GetSelectedItem();
    const UMenuListItem* SelItem = Cast<UMenuListItem>(Sel);

    if (SelItem)
    {
        // Determine index:
        const TArray<UObject*>& Items = lst_missions->GetListItems();
        selected_mission = Items.IndexOfByKey(Sel);
    }

    if (!(btn_accept && description && campaign))
        return;

    const TArray<MissionInfo*>& MissionList = campaign->GetMissionListArray();

    if (selected_mission >= 0 && selected_mission < MissionList.Num())
    {
        MissionInfo* Info = MissionList[selected_mission];
        if (Info)
        {
            mission_id = Info->id;

            const FString TimeBuf = FormatDayTimeString(Info->start);

            // Original built a markup-rich Text block; in UE keep it simple unless you have rich text.
            FString D;
            D += Info->name;
            D += TEXT("\n\n");
            D += Game::GetText(TEXT("MsnSelectDlg.mission-type"));
            D += TEXT("\n    ");
            D += MissionRoleNameSafe(Info->type);
            D += TEXT("\n\n");
            D += Game::GetText(TEXT("MsnSelectDlg.scenario"));
            D += TEXT("\n    ");
            D += Info->description;
            D += TEXT("\n\n");
            D += Game::GetText(TEXT("MsnSelectDlg.location"));
            D += TEXT("\n    ");
            D += Info->region;
            D += TEXT(" ");
            D += Game::GetText(TEXT("MsnSelectDlg.sector"));
            D += TEXT(" / ");
            D += Info->system;
            D += TEXT(" ");
            D += Game::GetText(TEXT("MsnSelectDlg.system"));
            D += TEXT("\n\n");
            D += Game::GetText(TEXT("MsnSelectDlg.start-time"));
            D += TEXT("\n    ");
            D += TimeBuf;

            description->SetText(FText::FromString(D));
            btn_accept->SetIsEnabled(true);

            if (btn_edit) btn_edit->SetIsEnabled(editable);
            if (btn_del)  btn_del->SetIsEnabled(editable);

            return;
        }
    }

    // Nothing valid selected:
    description->SetText(FText::FromString(Game::GetText(TEXT("MsnSelectDlg.choose"))));
    btn_accept->SetIsEnabled(false);

    if (btn_edit) btn_edit->SetIsEnabled(false);
    if (btn_del)  btn_del->SetIsEnabled(false);
}

void UMsnSelectDlg::OnMod()
{
    // TODO: call your manager to show Mod screen
    // manager->ShowModDlg();
}

void UMsnSelectDlg::OnNew()
{
    FString CampaignName;

    if (cmb_campaigns)
        CampaignName = cmb_campaigns->GetSelectedOption();
    else if (lst_campaigns)
    {
        if (const UMenuListItem* Item = Cast<UMenuListItem>(lst_campaigns->GetSelectedItem()))
            CampaignName = Item->Label;
    }

    Campaign* C = Campaign::SelectCampaign(CampaignName);
    if (!C)
        return;

    MissionInfo* Info = C->CreateNewMission();
    if (!Info || !Info->mission)
        return;

    mission_id = Info->id;

    // TODO: show mission editor screen
    // MsnEditDlg* Editor = manager->GetMsnEditDlg();
    // if (Editor) { ... }

    GEditMission = Info->mission;
}

void UMsnSelectDlg::OnEdit()
{
    FString CampaignName;

    if (cmb_campaigns)
        CampaignName = cmb_campaigns->GetSelectedOption();
    else if (lst_campaigns)
    {
        if (const UMenuListItem* Item = Cast<UMenuListItem>(lst_campaigns->GetSelectedItem()))
            CampaignName = Item->Label;
    }

    Campaign* C = Campaign::SelectCampaign(CampaignName);
    if (!C)
        return;

    Mission* M = C->GetMission(mission_id);
    if (!M)
        return;

    // TODO: show mission editor
    // GEditMission = M;
}

void UMsnSelectDlg::OnDel()
{
    FString CampaignName;

    if (cmb_campaigns)
        CampaignName = cmb_campaigns->GetSelectedOption();
    else if (lst_campaigns)
    {
        if (const UMenuListItem* Item = Cast<UMenuListItem>(lst_campaigns->GetSelectedItem()))
            CampaignName = Item->Label;
    }

    Campaign* C = Campaign::SelectCampaign(CampaignName);
    if (!C)
        return;

    Mission* M = C->GetMission(mission_id);
    if (!M)
        return;

    // UE: open your confirm dialog, then call OnDelConfirm() on accept.
    // If you do not have a confirm dialog yet, call OnDelConfirm() directly:
    //
    // OnDelConfirm();
}

void UMsnSelectDlg::OnDelConfirm()
{
    FString CampaignName;

    if (cmb_campaigns)
        CampaignName = cmb_campaigns->GetSelectedOption();
    else if (lst_campaigns)
    {
        if (const UMenuListItem* Item = Cast<UMenuListItem>(lst_campaigns->GetSelectedItem()))
            CampaignName = Item->Label;
    }

    Campaign* C = Campaign::SelectCampaign(CampaignName);
    if (!C)
        return;

    GEditMission = nullptr;
    C->DeleteMission(mission_id);
    Show();
}

void UMsnSelectDlg::OnAccept()
{
    if (selected_mission < 0 || !campaign || !stars)
        return;

    // Original: Mouse::Show(false);
    // UE: set input mode / cursor visibility via PlayerController if needed:
    // if (APlayerController* PC = GetWorld()->GetFirstPlayerController()) { PC->bShowMouseCursor = false; }

    const TArray<MissionInfo*>& List = campaign->GetMissionListArray();
    if (!List.IsValidIndex(selected_mission) || !List[selected_mission])
        return;

    const int32 Id = List[selected_mission]->id;
    campaign->SetMissionId(Id);
    campaign->ReloadMission(Id);

    stars->SetGameMode(Starshatter::PREP_MODE);
}

void UMsnSelectDlg::OnCancel()
{
    // TODO: manager->ShowMenuDlg();
    SetVisibility(ESlateVisibility::Hidden);
}

