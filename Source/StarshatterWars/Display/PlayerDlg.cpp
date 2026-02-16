/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026.
    All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         PlayerDlg.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Player Logbook dialog (Options-subpanel row layout via BaseScreen)
*/

#include "PlayerDlg.h"

// UMG:
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/Image.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"

// Input:
#include "Input/Reply.h"
#include "InputCoreTypes.h"

// Game:
#include "PlayerRosterItem.h"
#include "PlayerCharacter.h"
#include "MenuScreen.h"

UPlayerDlg::UPlayerDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SelectedPlayer = nullptr;
}

void UPlayerDlg::InitializeDlg(UMenuScreen* InManager)
{
    manager = InManager;
}

void UPlayerDlg::NativeConstruct()
{
    Super::NativeConstruct();

    RegisterControls();

    // Build right-side “Options-style” rows into BaseScreen::AutoVBox:
    BuildFormRows();

    // Populate roster + selection:
    BuildRoster();

    // Push model -> UI:
    RebuildFromModel();
}

void UPlayerDlg::NativeDestruct()
{
    if (lst_roster)
        lst_roster->OnItemSelectionChanged().RemoveAll(this);

    if (btn_add) btn_add->OnClicked.RemoveAll(this);
    if (btn_del) btn_del->OnClicked.RemoveAll(this);

    if (BtnSave)   BtnSave->OnClicked.RemoveAll(this);
    if (BtnCancel) BtnCancel->OnClicked.RemoveAll(this);

    if (ApplyButton)  ApplyButton->OnClicked.RemoveAll(this);
    if (CancelButton) CancelButton->OnClicked.RemoveAll(this);

    Super::NativeDestruct();
}

FReply UPlayerDlg::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey Key = InKeyEvent.GetKey();

    if (Key == EKeys::Enter || Key == EKeys::Virtual_Accept)
    {
        OnApply();
        return FReply::Handled();
    }

    if (Key == EKeys::Escape || Key == EKeys::Virtual_Back)
    {
        OnCancel();
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UPlayerDlg::ShowDlg()
{
    SetVisibility(ESlateVisibility::Visible);
    BuildRoster();
    RebuildFromModel();
}

void UPlayerDlg::HideDlg()
{
    SetVisibility(ESlateVisibility::Collapsed);
}

// ------------------------------------------------------------
// Controls
// ------------------------------------------------------------

void UPlayerDlg::RegisterControls()
{
    if (lst_roster)
    {
        lst_roster->OnItemSelectionChanged().RemoveAll(this);
        lst_roster->OnItemSelectionChanged().AddUObject(this, &UPlayerDlg::OnRosterSelectionChanged);
    }

    if (btn_add)
    {
        btn_add->OnClicked.RemoveAll(this);
        btn_add->OnClicked.AddDynamic(this, &UPlayerDlg::OnAdd);
    }

    if (btn_del)
    {
        btn_del->OnClicked.RemoveAll(this);
        btn_del->OnClicked.AddDynamic(this, &UPlayerDlg::OnDel);
    }

    // Save/Cancel prefer local, fallback BaseScreen ApplyButton/CancelButton:
    if (BtnSave)
    {
        BtnSave->OnClicked.RemoveAll(this);
        BtnSave->OnClicked.AddDynamic(this, &UPlayerDlg::OnApply);
    }
    else if (ApplyButton)
    {
        ApplyButton->OnClicked.RemoveAll(this);
        ApplyButton->OnClicked.AddDynamic(this, &UPlayerDlg::OnApply);
    }

    if (BtnCancel)
    {
        BtnCancel->OnClicked.RemoveAll(this);
        BtnCancel->OnClicked.AddDynamic(this, &UPlayerDlg::OnCancel);
    }
    else if (CancelButton)
    {
        CancelButton->OnClicked.RemoveAll(this);
        CancelButton->OnClicked.AddDynamic(this, &UPlayerDlg::OnCancel);
    }
}

// ------------------------------------------------------------
// Build the legacy FORM controls as Options-style rows
// ------------------------------------------------------------

void UPlayerDlg::BuildFormRows()
{
    // Use BaseScreen canvas overlay + offsets, same pattern as Options panels:
    // (Tune offsets if you want the panel inset differently)
    EnsureAutoVerticalBoxWithOffsets(FMargin(32.f, 64.f, 32.f, 32.f));

    UVerticalBox* VBox = EnsureAutoVerticalBox();
    if (!VBox || !WidgetTree)
    {
        UE_LOG(LogTemp, Error, TEXT("[PlayerDlg] BuildFormRows: AutoVBox/WidgetTree missing. Ensure RootCanvas is bound."));
        return;
    }

    VBox->ClearChildren();

    // ----- Edits -----
    edt_name = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("edt_name"));
    AddLabeledRow(TEXT("NAME:"), edt_name, 360.f, 0.f, 6.f, 24.f);

    edt_password = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("edt_password"));
    edt_password->SetIsPassword(true);
    AddLabeledRow(TEXT("PASSWORD:"), edt_password, 360.f, 0.f, 6.f, 24.f);

    edt_squadron = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("edt_squadron"));
    AddLabeledRow(TEXT("SQUADRON:"), edt_squadron, 360.f, 0.f, 6.f, 24.f);

    edt_signature = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("edt_signature"));
    AddLabeledRow(TEXT("SIGNATURE:"), edt_signature, 360.f, 0.f, 6.f, 24.f);

    // ----- Stats labels -----
    txt_created = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("txt_created"));
    AddLabeledRow(TEXT("CREATED:"), txt_created, 360.f, 0.f, 6.f, 24.f);

    txt_flighttime = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("txt_flighttime"));
    AddLabeledRow(TEXT("FLIGHT TIME:"), txt_flighttime, 360.f, 0.f, 6.f, 24.f);

    txt_missions = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("txt_missions"));
    AddLabeledRow(TEXT("MISSIONS:"), txt_missions, 360.f, 0.f, 6.f, 24.f);

    txt_kills = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("txt_kills"));
    AddLabeledRow(TEXT("KILLS:"), txt_kills, 360.f, 0.f, 6.f, 24.f);

    txt_losses = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("txt_losses"));
    AddLabeledRow(TEXT("LOSSES:"), txt_losses, 360.f, 0.f, 6.f, 24.f);

    txt_points = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("txt_points"));
    AddLabeledRow(TEXT("POINTS:"), txt_points, 360.f, 0.f, 6.f, 24.f);

    // ----- Rank name + insignia -----
    txt_rankname = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("txt_rankname"));
    AddLabeledRow(TEXT("RANK:"), txt_rankname, 360.f, 0.f, 6.f, 24.f);

    img_rank = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("img_rank"));
    AddLabeledRow(TEXT("INSIGNIA:"), img_rank, 140.f, 0.f, 6.f, 24.f);

    // ----- Medals (3x5) -----
    medals_grid = WidgetTree->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass(), TEXT("medals_grid"));
    AddLabeledRow(TEXT("MEDALS:"), medals_grid, 440.f, 0.f, 6.f, 24.f);

    MedalImages.Reset();
    MedalImages.SetNum(15);

    for (int32 r = 0; r < 3; ++r)
    {
        for (int32 c = 0; c < 5; ++c)
        {
            const int32 idx = r * 5 + c;
            UImage* Img = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), *FString::Printf(TEXT("medal_%02d"), idx));
            MedalImages[idx] = Img;

            USizeBox* Wrapper = WidgetTree->ConstructWidget<USizeBox>(
                USizeBox::StaticClass(),
                *FString::Printf(TEXT("medal_wrap_%02d"), idx)
            );

            // Optional: enforce consistent medal size
            Wrapper->SetWidthOverride(82.f);
            Wrapper->SetHeightOverride(24.f);

            Wrapper->AddChild(Img);

            if (UUniformGridSlot* GSlot = medals_grid->AddChildToUniformGrid(Wrapper, r, c))
            {
                GSlot->SetHorizontalAlignment(HAlign_Center);
                GSlot->SetVerticalAlignment(VAlign_Center);
            }
        }
    }

    // ----- Chat macros (1..9,0) -----
    MacroEdits.Reset();
    MacroEdits.SetNum(10);

    // Header row like the legacy "Chat Macros:" label:
    {
        UTextBlock* Header = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("txt_macros_header"));
        Header->SetText(FText::FromString(TEXT("CHAT MACROS:")));
        VBox->AddChildToVerticalBox(Header);
    }

    for (int32 i = 0; i < 10; ++i)
    {
        const int32 Display = (i == 9) ? 0 : (i + 1);

        UEditableTextBox* E = WidgetTree->ConstructWidget<UEditableTextBox>(
            UEditableTextBox::StaticClass(),
            *FString::Printf(TEXT("macro_%d"), Display)
        );

        MacroEdits[i] = E;

        // Label is the macro index (1..9,0)
        AddLabeledRow(FString::FromInt(Display), E, 440.f, 0.f, 6.f, 24.f);
    }
}

// ------------------------------------------------------------
// Roster list
// ------------------------------------------------------------

void UPlayerDlg::BuildRoster()
{
    if (!lst_roster)
        return;

    const int32 PrevId = (selected_item && selected_item->GetPlayer())
        ? selected_item->GetPlayer()->GetIdentity()
        : 0;

    lst_roster->ClearListItems();
    selected_item = nullptr;
    SelectedPlayer = nullptr;

    List<PlayerCharacter>& roster = PlayerCharacter::GetRoster();
    if (roster.size() < 1)
    {
        RebuildFromModel();
        return;
    }

    UPlayerRosterItem* FirstItem = nullptr;
    UPlayerRosterItem* MatchItem = nullptr;

    for (int i = 0; i < roster.size(); ++i)
    {
        PlayerCharacter* player = roster[i];
        if (!player)
            continue;

        UPlayerRosterItem* item = NewObject<UPlayerRosterItem>(this);
        item->Initialize(player);

        if (!FirstItem)
            FirstItem = item;

        if (PrevId != 0 && player->GetIdentity() == PrevId)
            MatchItem = item;

        lst_roster->AddItem(item);
    }

    selected_item = MatchItem ? MatchItem : FirstItem;

    if (selected_item)
    {
        lst_roster->SetSelectedItem(selected_item);
        SelectedPlayer = selected_item->GetPlayer();

        if (SelectedPlayer)
            PlayerCharacter::SetCurrentPlayer(SelectedPlayer);
    }

    RebuildFromModel();
}

void UPlayerDlg::RefreshRoster()
{
    BuildRoster();
}

void UPlayerDlg::OnRosterSelectionChanged(UObject* SelectedItem)
{
    UPlayerRosterItem* Item = Cast<UPlayerRosterItem>(SelectedItem);

    selected_item = Item;
    SelectedPlayer = Item ? Item->GetPlayer() : nullptr;

    if (SelectedPlayer)
        PlayerCharacter::SetCurrentPlayer(SelectedPlayer);

    // Don’t commit on selection change; just show:
    RebuildFromModel();
}

PlayerCharacter* UPlayerDlg::GetSelectedPlayer() const
{
    return selected_item ? selected_item->GetPlayer() : nullptr;
}

// ------------------------------------------------------------
// Model <-> UI
// ------------------------------------------------------------

void UPlayerDlg::CommitToModel()
{
    PlayerCharacter* P = GetSelectedPlayer();
    if (!P)
        return;

    // SAFE: you already have SetName()/Name() in your codebase.
    if (edt_name)
        P->SetName(edt_name->GetText().ToString());

    // TODO: wire these once you confirm PlayerCharacter API:
    // if (edt_password)  P->SetPassword(edt_password->GetText().ToString());
    // if (edt_squadron)  P->SetSquadron(edt_squadron->GetText().ToString());
    // if (edt_signature) P->SetSignature(edt_signature->GetText().ToString());
    // for (int32 i=0;i<10;++i) P->SetChatMacro(i, MacroEdits[i]->GetText().ToString());
}

void UPlayerDlg::RebuildFromModel()
{
    PlayerCharacter* P = GetSelectedPlayer();

    if (!P)
    {
        if (edt_name)      edt_name->SetText(FText::GetEmpty());
        if (edt_password)  edt_password->SetText(FText::GetEmpty());
        if (edt_squadron)  edt_squadron->SetText(FText::GetEmpty());
        if (edt_signature) edt_signature->SetText(FText::GetEmpty());

        if (txt_created)    txt_created->SetText(FText::GetEmpty());
        if (txt_flighttime) txt_flighttime->SetText(FText::GetEmpty());
        if (txt_missions)   txt_missions->SetText(FText::GetEmpty());
        if (txt_kills)      txt_kills->SetText(FText::GetEmpty());
        if (txt_losses)     txt_losses->SetText(FText::GetEmpty());
        if (txt_points)     txt_points->SetText(FText::GetEmpty());
        if (txt_rankname)   txt_rankname->SetText(FText::GetEmpty());

        for (UEditableTextBox* E : MacroEdits)
            if (E) E->SetText(FText::GetEmpty());

        return;
    }

    if (edt_name)
        edt_name->SetText(FText::FromString(P->Name()));

    // TODO: fill these once API exists:
    // if (edt_password)  edt_password->SetText(FText::FromString(P->Password()));
    // if (edt_squadron)  edt_squadron->SetText(FText::FromString(P->Squadron()));
    // if (edt_signature) edt_signature->SetText(FText::FromString(P->Signature()));

    if (txt_created)
        txt_created->SetText(FText::FromString(FormatDateFromUnixSeconds((int64)P->CreateDate())));

    if (txt_flighttime)
        txt_flighttime->SetText(FText::FromString(FormatTimeHMS(P->FlightTime())));

    if (txt_missions)
        txt_missions->SetText(FText::AsNumber(P->Missions()));

    if (txt_kills)
        txt_kills->SetText(FText::AsNumber(P->Kills()));

    // Legacy "Losses" — map to Deaths() in your current model:
    if (txt_losses)
        txt_losses->SetText(FText::AsNumber(P->Deaths()));

    // Legacy "Points" — placeholder until you hook the real stat:
    if (txt_points)
        txt_points->SetText(FText::AsNumber(P->Kills() * 10)); // TODO: replace with real points

    if (txt_rankname)
    {
        // If you have a RankName helper, plug it here; otherwise show numeric rank:
        txt_rankname->SetText(FText::AsNumber(P->GetRank()));
        // Example if you have this:
        // txt_rankname->SetText(FText::FromString(FString(PlayerCharacter::RankName(P->GetRank()))));
    }

    // TODO: rank/medals images once your texture pipeline is ready:
    // img_rank->SetBrushFromTexture(...);
    // MedalImages[i]->SetBrushFromTexture(...);

    // TODO: macros:
    // for (int32 i=0;i<10;++i) MacroEdits[i]->SetText(FText::FromString(P->ChatMacro(i)));
}

// ------------------------------------------------------------
// Actions
// ------------------------------------------------------------

void UPlayerDlg::OnAdd()
{
    CommitToModel();

    PlayerCharacter* NewPlayer = PlayerCharacter::CreateDefault();
    if (!NewPlayer)
        return;

    PlayerCharacter::AddToRoster(NewPlayer);
    PlayerCharacter::SetCurrentPlayer(NewPlayer);
    PlayerCharacter::Save();

    selected_item = nullptr;
    RefreshRoster();
}

void UPlayerDlg::OnDel()
{
    PlayerCharacter* P = GetSelectedPlayer();
    if (!P)
        return;

    PlayerCharacter::RemoveFromRoster(P);
    PlayerCharacter::Save();

    selected_item = nullptr;
    SelectedPlayer = nullptr;
    RefreshRoster();
}

void UPlayerDlg::OnApply()
{
    CommitToModel();
    PlayerCharacter::Save();

    if (manager)
        manager->ShowMenuDlg();
    else
        HideDlg();
}

void UPlayerDlg::OnCancel()
{
    RebuildFromModel();

    if (manager)
        manager->ShowMenuDlg();
    else
        HideDlg();
}

// ------------------------------------------------------------
// Formatting helpers
// ------------------------------------------------------------

FString UPlayerDlg::FormatTimeHMS(double Seconds)
{
    if (Seconds < 0.0)
        Seconds = 0.0;

    const int64 TotalSeconds = (int64)Seconds;
    const int64 H = TotalSeconds / 3600;
    const int64 M = (TotalSeconds % 3600) / 60;
    const int64 S = (TotalSeconds % 60);

    return FString::Printf(TEXT("%lld:%02lld:%02lld"), H, M, S);
}

FString UPlayerDlg::FormatDateFromUnixSeconds(int64 UnixSeconds)
{
    if (UnixSeconds <= 0)
        return TEXT("");

    const FDateTime DT = FDateTime::FromUnixTimestamp(UnixSeconds);
    return DT.ToString(TEXT("%Y-%m-%d"));
}
