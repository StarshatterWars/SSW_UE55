/*=============================================================================
    Project:        Starshatter Wars (nGenEx Unreal Port)
    Studio:         Fractal Dev Games
    Copyright:      (C) 2024–2026. All Rights Reserved.

    DIALOG:         PlayerDlg
    FILE:           PlayerDlg.cpp
    AUTHOR:         Carlos Bott

    OVERVIEW
    ========
    Player profile dialog (authoritative-only).

    - Reads/writes FS_PlayerGameInfo via UStarshatterPlayerSubsystem
    - DOES NOT use legacy PlayerCharacter
    - DOES NOT use lambdas
    - Fixes common “nothing shows up” issues:
        * edt_callsign was never created
        * empire row incorrectly used txt_rank
        * callsign was being written into edt_squadron
        * RootCanvas fallback robustness
=============================================================================*/

#include "PlayerDlg.h"

// UMG
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Components/SizeBox.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/Image.h"

// Helpers
#include "FormattingUtils.h"
#include "AwardInfoRegistry.h"

// Manager
#include "MenuScreen.h"

// Subsystem + data
#include "StarshatterPlayerSubsystem.h"
#include "GameStructs.h"

DEFINE_LOG_CATEGORY_STATIC(LogPlayerDlg, Log, All);

UPlayerDlg::UPlayerDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetDialogInputEnabled(true);
}

void UPlayerDlg::InitializeDlg(UMenuScreen* InManager)
{
    manager = InManager;
}

void UPlayerDlg::NativeOnInitialized()
{
    Super::NativeOnInitialized();
}

void UPlayerDlg::NativePreConstruct()
{
    Super::NativePreConstruct();

    EnsureLayout();
    EnsureScrollHostsStats();
}

void UPlayerDlg::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind base buttons (from BaseScreen)
    if (ApplyButton)
    {
        ApplyButton->OnClicked.RemoveAll(this);
        ApplyButton->OnClicked.AddUniqueDynamic(this, &UPlayerDlg::OnApply);
    }

    if (CancelButton)
    {
        CancelButton->OnClicked.RemoveAll(this);
        CancelButton->OnClicked.AddUniqueDynamic(this, &UPlayerDlg::OnCancel);
    }

    // Optional buttons (present in some blueprints)
    if (AddPlayerButton)
    {
        AddPlayerButton->OnClicked.RemoveAll(this);
        AddPlayerButton->OnClicked.AddUniqueDynamic(this, &UPlayerDlg::OnAdd);
    }

    if (DeletePlayerButton)
    {
        DeletePlayerButton->OnClicked.RemoveAll(this);
        DeletePlayerButton->OnClicked.AddUniqueDynamic(this, &UPlayerDlg::OnDel);
    }

    EnsureLayout();
    EnsureScrollHostsStats();

    BuildRosterPanel();
    BuildStatsRows();

    RefreshUIFromSubsystem();
}

void UPlayerDlg::NativeDestruct()
{
    if (AddPlayerButton)    AddPlayerButton->OnClicked.RemoveAll(this);
    if (DeletePlayerButton) DeletePlayerButton->OnClicked.RemoveAll(this);
    if (ApplyButton)        ApplyButton->OnClicked.RemoveAll(this);
    if (CancelButton)       CancelButton->OnClicked.RemoveAll(this);

    Super::NativeDestruct();
}

void UPlayerDlg::HandleAccept() { OnApply(); }
void UPlayerDlg::HandleCancel() { OnCancel(); }

void UPlayerDlg::ShowDlg()
{
    SetVisibility(ESlateVisibility::Visible);
    RefreshUIFromSubsystem();
}

void UPlayerDlg::HideDlg()
{
    SetVisibility(ESlateVisibility::Collapsed);
}

// ------------------------------------------------------------------------
// Subsystem access
// ------------------------------------------------------------------------

UStarshatterPlayerSubsystem* UPlayerDlg::GetPlayerSS() const
{
    if (!GetGameInstance())
        return nullptr;

    return GetGameInstance()->GetSubsystem<UStarshatterPlayerSubsystem>();
}

// ------------------------------------------------------------------------
// Layout
// ------------------------------------------------------------------------

void UPlayerDlg::EnsureLayout()
{
    if (!WidgetTree)
        return;

    // Prefer BaseScreen::RootCanvas if available; else attempt alternate bind.
    if (!RootCanvas)
    {
        RootCanvas = RootCanvasPlayer;

        if (!RootCanvas)
            RootCanvas = Cast<UCanvasPanel>(GetRootWidget());

        if (!RootCanvas)
            RootCanvas = Cast<UCanvasPanel>(WidgetTree->FindWidget(TEXT("RootCanvas")));

        if (!RootCanvas)
            RootCanvas = Cast<UCanvasPanel>(WidgetTree->FindWidget(TEXT("RootCanvasPlayer")));
    }

    if (!RootCanvas)
    {
        UE_LOG(LogPlayerDlg, Error, TEXT("[PlayerDlg] EnsureLayout: RootCanvas is NULL"));
        return;
    }

    if (MainRow)
        return;

    MainRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("MainRow"));
    if (!MainRow)
        return;

    if (UCanvasPanelSlot* S = RootCanvas->AddChildToCanvas(MainRow))
    {
        S->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
        S->SetAlignment(FVector2D(0.f, 0.f));
        S->SetOffsets(FMargin(24.f, 64.f, 24.f, 24.f));
        S->SetAutoSize(false);
        S->SetZOrder(100);
    }

    LeftPanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LeftPanel"));
    StatsScroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("StatsScroll"));
    StatsVBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("StatsVBox"));

    if (!LeftPanel || !StatsScroll || !StatsVBox)
        return;

    if (UHorizontalBoxSlot* L = MainRow->AddChildToHorizontalBox(LeftPanel))
    {
        L->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
        L->SetHorizontalAlignment(HAlign_Left);
        L->SetVerticalAlignment(VAlign_Fill);
        L->SetPadding(FMargin(0.f, 0.f, 20.f, 0.f));
    }

    if (UHorizontalBoxSlot* R = MainRow->AddChildToHorizontalBox(StatsScroll))
    {
        R->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        R->SetHorizontalAlignment(HAlign_Fill);
        R->SetVerticalAlignment(VAlign_Fill);
        R->SetPadding(FMargin(0.f));
    }

    StatsScroll->SetConsumeMouseWheel(EConsumeMouseWheel::WhenScrollingPossible);
    StatsScroll->SetAnimateWheelScrolling(true);
    StatsScroll->SetScrollBarVisibility(ESlateVisibility::Visible);

    // BaseScreen helper target (if you use it elsewhere)
    AutoVBox = StatsVBox;
}

void UPlayerDlg::EnsureScrollHostsStats()
{
    if (!StatsScroll || !StatsVBox)
        return;

    if (StatsVBox->GetParent())
        StatsVBox->RemoveFromParent();

    StatsScroll->ClearChildren();
    StatsScroll->AddChild(StatsVBox);

    StatsVBox->SetVisibility(ESlateVisibility::Visible);
}

// ------------------------------------------------------------------------
// Options-style row builders
// ------------------------------------------------------------------------

UTextBlock* UPlayerDlg::MakeValueText()
{
    if (!WidgetTree)
        return nullptr;

    UTextBlock* T = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    if (!T)
        return nullptr;

    T->SetJustification(ETextJustify::Right);
    return T;
}

UEditableTextBox* UPlayerDlg::MakeEditBox(bool bPassword)
{
    if (!WidgetTree)
        return nullptr;

    UEditableTextBox* E = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass());
    if (!E)
        return nullptr;

    if (bPassword)
        E->SetIsPassword(true);

    return E;
}

UHorizontalBox* UPlayerDlg::AddStatRow(const FText& Label, UWidget* RightWidget, float RightWidth, float RowPadY)
{
    if (!WidgetTree || !StatsVBox || !RightWidget)
        return nullptr;

    UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
    if (!Row)
        return nullptr;

    if (UVerticalBoxSlot* VSlot = StatsVBox->AddChildToVerticalBox(Row))
    {
        VSlot->SetHorizontalAlignment(HAlign_Fill);
        VSlot->SetVerticalAlignment(VAlign_Top);
        VSlot->SetPadding(FMargin(0.f, RowPadY, 0.f, RowPadY));
    }

    UTextBlock* Lbl = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    if (Lbl)
    {
        Lbl->SetText(Label);
        Lbl->SetJustification(ETextJustify::Left);

        if (UHorizontalBoxSlot* LS = Row->AddChildToHorizontalBox(Lbl))
        {
            LS->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
            LS->SetHorizontalAlignment(HAlign_Left);
            LS->SetVerticalAlignment(VAlign_Center);
            LS->SetPadding(FMargin(0.f, 0.f, 12.f, 0.f));
        }
    }

    USizeBox* Wrap = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
    if (Wrap)
    {
        Wrap->SetWidthOverride(RightWidth);
        Wrap->AddChild(RightWidget);

        if (UHorizontalBoxSlot* RS = Row->AddChildToHorizontalBox(Wrap))
        {
            RS->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
            RS->SetHorizontalAlignment(HAlign_Right);
            RS->SetVerticalAlignment(VAlign_Center);
        }
    }

    return Row;
}

// ------------------------------------------------------------------------
// Left panel
// ------------------------------------------------------------------------

void UPlayerDlg::BuildRosterPanel()
{
    if (!WidgetTree || !LeftPanel)
        return;

    LeftPanel->ClearChildren();

    UTextBlock* Hdr = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ProfileHdr"));
    if (Hdr)
    {
        Hdr->SetText(FText::FromString(TEXT("CURRENT PROFILE")));
        if (UVerticalBoxSlot* S = LeftPanel->AddChildToVerticalBox(Hdr))
            S->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
    }

    txt_profile_name = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ProfileName"));
    if (txt_profile_name)
    {
        txt_profile_name->SetText(FText::FromString(TEXT("")));
        if (UVerticalBoxSlot* S = LeftPanel->AddChildToVerticalBox(txt_profile_name))
            S->SetPadding(FMargin(0.f, 0.f, 0.f, 12.f));
    }
}

// ------------------------------------------------------------------------
// Right panel
// ------------------------------------------------------------------------

void UPlayerDlg::BuildStatsRows()
{
    if (!WidgetTree || !StatsVBox)
        return;

    StatsVBox->ClearChildren();
    MedalImages.Reset();
    MacroEdits.Reset();

    // EDITABLES (FIXED: Callsign edit is created)
    edt_name = MakeEditBox(false);
    edt_password = MakeEditBox(true);
    edt_callsign = MakeEditBox(false);
    edt_squadron = MakeEditBox(false);
    edt_signature = MakeEditBox(false);

    AddStatRow(FText::FromString(TEXT("NAME")), edt_name);
    AddStatRow(FText::FromString(TEXT("PASSWORD")), edt_password);
    AddStatRow(FText::FromString(TEXT("CALLSIGN")), edt_callsign);
    AddStatRow(FText::FromString(TEXT("SQUADRON")), edt_squadron);
    AddStatRow(FText::FromString(TEXT("SIGNATURE")), edt_signature);


    ApplyDefaultEditBoxStyle(edt_name, 18);
    ApplyDefaultEditBoxStyle(edt_password, 18);
    ApplyDefaultEditBoxStyle(edt_callsign, 18);
    ApplyDefaultEditBoxStyle(edt_squadron, 18);
    ApplyDefaultEditBoxStyle(edt_signature, 18);

    // READ-ONLY FIELDS
    txt_created = MakeValueText();
    txt_flighttime = MakeValueText();
    txt_missions = MakeValueText();
    txt_kills = MakeValueText();
    txt_losses = MakeValueText();
    txt_points = MakeValueText();
    txt_rank = MakeValueText();
    txt_empire = MakeValueText();

    AddStatRow(FText::FromString(TEXT("CREATED")), txt_created);
    AddStatRow(FText::FromString(TEXT("FLIGHT TIME")), txt_flighttime);
    AddStatRow(FText::FromString(TEXT("MISSIONS")), txt_missions);
    AddStatRow(FText::FromString(TEXT("KILLS")), txt_kills);
    AddStatRow(FText::FromString(TEXT("LOSSES")), txt_losses);
    AddStatRow(FText::FromString(TEXT("POINTS")), txt_points);
    AddStatRow(FText::FromString(TEXT("RANK")), txt_rank);

    // FIXED: EMPIRE row uses txt_empire, not txt_rank
    AddStatRow(FText::FromString(TEXT("EMPIRE")), txt_empire);

    img_rank = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("RankImage"));
    if (img_rank)
        AddStatRow(FText::FromString(TEXT("INSIGNIA")), img_rank, 160.f);

    // MEDALS
    {
        UTextBlock* MedalsHdr = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MedalsHdr"));
        if (MedalsHdr)
        {
            MedalsHdr->SetText(FText::FromString(TEXT("MEDALS")));
            if (UVerticalBoxSlot* S = StatsVBox->AddChildToVerticalBox(MedalsHdr))
                S->SetPadding(FMargin(0.f, 18.f, 0.f, 8.f));
        }

        medals_grid = WidgetTree->ConstructWidget<UUniformGridPanel>(UUniformGridPanel::StaticClass(), TEXT("MedalsGrid"));
        if (medals_grid)
        {
            if (UVerticalBoxSlot* S = StatsVBox->AddChildToVerticalBox(medals_grid))
                S->SetPadding(FMargin(0.f, 0.f, 0.f, 18.f));
        }

        BuildMedalsGrid(5, 3);
    }

    // CHAT MACROS
    {
        UTextBlock* MacroHdr = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MacroHdr"));
        if (MacroHdr)
        {
            MacroHdr->SetText(FText::FromString(TEXT("CHAT MACROS")));
            if (UVerticalBoxSlot* S = StatsVBox->AddChildToVerticalBox(MacroHdr))
                S->SetPadding(FMargin(0.f, 8.f, 0.f, 8.f));
        }

        BuildChatMacrosRows();
    }
}

void UPlayerDlg::BuildMedalsGrid(int32 Columns, int32 Rows)
{
    if (!WidgetTree || !medals_grid)
        return;

    medals_grid->ClearChildren();
    MedalImages.Reset();

    const int32 Total = Columns * Rows;

    for (int32 idx = 0; idx < Total; ++idx)
    {
        const int32 r = idx / Columns;
        const int32 c = idx % Columns;

        UImage* Img = WidgetTree->ConstructWidget<UImage>(
            UImage::StaticClass(),
            *FString::Printf(TEXT("Medal_%02d"), idx)
        );

        USizeBox* Wrap = WidgetTree->ConstructWidget<USizeBox>(
            USizeBox::StaticClass(),
            *FString::Printf(TEXT("MedalWrap_%02d"), idx)
        );

        if (!Img || !Wrap)
            continue;

        Wrap->SetWidthOverride(82.f);
        Wrap->SetHeightOverride(21.f);
        Wrap->AddChild(Img);

        if (UUniformGridSlot* CSlot = medals_grid->AddChildToUniformGrid(Wrap, r, c))
        {
            CSlot->SetHorizontalAlignment(HAlign_Center);
            CSlot->SetVerticalAlignment(VAlign_Center);
        }

        MedalImages.Add(Img);
    }
}

void UPlayerDlg::BuildChatMacrosRows()
{
    if (!WidgetTree || !StatsVBox)
        return;

    MacroEdits.SetNum(10);

    // 1..9 then 0 (NO LAMBDAS)
    for (int32 i = 1; i <= 9; ++i)
    {
        UEditableTextBox* E = MakeEditBox(false);
        MacroEdits[i] = E;

        AddStatRow(FText::FromString(FString::FromInt(i)), E);
    }

    {
        UEditableTextBox* E = MakeEditBox(false);
        MacroEdits[0] = E;

        AddStatRow(FText::FromString(TEXT("0")), E);
    }
}

// ------------------------------------------------------------------------
// Model <-> UI (SUBSYSTEM)
// ------------------------------------------------------------------------

void UPlayerDlg::UpdatePlayerFromUI_Subsystem()
{
    UStarshatterPlayerSubsystem* SS = GetPlayerSS();
    if (!SS)
        return;

    // Make sure it has loaded at least once
    if (!SS->HasLoaded())
        SS->LoadPlayer();

    FS_PlayerGameInfo& Info = SS->GetMutablePlayerInfo();

    if (edt_name)
    {
        const FString NewName = edt_name->GetText().ToString().TrimStartAndEnd();
        if (!NewName.IsEmpty())
            Info.Name = NewName;
    }

    if (edt_callsign)
        Info.Callsign = edt_callsign->GetText().ToString().TrimStartAndEnd();

    if (edt_squadron)
        Info.Squadron = edt_squadron->GetText().ToString().TrimStartAndEnd();

    if (edt_signature)
        Info.Signature = edt_signature->GetText().ToString().TrimStartAndEnd();

    // Chat macros (10)
    if (MacroEdits.Num() == 10)
    {
        Info.ChatMacros.SetNum(10);
        for (int32 i = 0; i < 10; ++i)
        {
            if (MacroEdits[i])
                Info.ChatMacros[i] = MacroEdits[i]->GetText().ToString();
        }
    }

    SS->SavePlayer(true);
}

void UPlayerDlg::RefreshUIFromSubsystem()
{
    UStarshatterPlayerSubsystem* SS = GetPlayerSS();
    if (!SS)
    {
        UE_LOG(LogPlayerDlg, Warning, TEXT("[PlayerDlg] RefreshUIFromSubsystem: PlayerSubsystem is NULL"));
        return;
    }

    if (!SS->HasLoaded())
        SS->LoadPlayer();

    const FS_PlayerGameInfo& Info = SS->GetPlayerInfo();

    UE_LOG(LogPlayerDlg, Warning, TEXT("[PlayerDlg] Refresh: Name='%s' Callsign='%s' Empire=%d Rank=%d"),
        *Info.Name, *Info.Callsign, Info.Empire, Info.Rank);

    // Left panel
    if (txt_profile_name)
        txt_profile_name->SetText(FText::FromString(Info.Name.IsEmpty() ? TEXT("<UNNAMED>") : Info.Name));

    // Editable fields (FIXED: callsign goes into edt_callsign)
    if (edt_name)      edt_name->SetText(FText::FromString(Info.Name));
    if (edt_callsign)  edt_callsign->SetText(FText::FromString(Info.Callsign));
    if (edt_squadron)  edt_squadron->SetText(FText::FromString(Info.Squadron));
    if (edt_signature) edt_signature->SetText(FText::FromString(Info.Signature));

    // Password not in FS_PlayerGameInfo (leave blank)
    if (edt_password) edt_password->SetText(FText::GetEmpty());

    // Read-only
    if (txt_created)    txt_created->SetText(FText::FromString(UFormattingUtils::FormatDateFromUnixSeconds(Info.CreateTime)));
    if (txt_flighttime) txt_flighttime->SetText(FText::FromString(UFormattingUtils::FormatTimeHMS((double)Info.FlightTime)));
    if (txt_missions)   txt_missions->SetText(FText::AsNumber(Info.PlayerMissions));
    if (txt_kills)      txt_kills->SetText(FText::AsNumber(Info.PlayerKills));
    if (txt_losses)     txt_losses->SetText(FText::AsNumber(Info.PlayerLosses));
    if (txt_points)     txt_points->SetText(FText::AsNumber(Info.PlayerPoints));

    // Rank text
    if (txt_rank)
    {
        const int32 RankId = Info.Rank;
        const TCHAR* RankName = UAwardInfoRegistry::RankName(RankId);

        if (RankName && FCString::Strlen(RankName) > 0)
            txt_rank->SetText(FText::FromString(RankName));
        else
            txt_rank->SetText(FText::FromString(TEXT("UNKNOWN RANK")));
    }

    // Empire text
    if (txt_empire)
    {
        const UEnum* Enum = StaticEnum<EEMPIRE_NAME>();
        if (Enum && Enum->IsValidEnumValue(Info.Empire))
            txt_empire->SetText(Enum->GetDisplayNameTextByValue((int64)Info.Empire));
        else
            txt_empire->SetText(FText::FromString(TEXT("UNKNOWN EMPIRE")));
    }

    // Macros
    if (MacroEdits.Num() == 10)
    {
        for (int32 i = 0; i < 10; ++i)
        {
            if (!MacroEdits[i])
                continue;

            const FString V = Info.ChatMacros.IsValidIndex(i) ? Info.ChatMacros[i] : FString();
            MacroEdits[i]->SetText(FText::FromString(V));
        }
    }

    // Medal imagery can be hooked later using Info.MedalsMask
}

// ------------------------------------------------------------------------
// Actions
// ------------------------------------------------------------------------

void UPlayerDlg::OnAdd()
{
    UE_LOG(LogPlayerDlg, Warning, TEXT("[PlayerDlg] Add is disabled for single-profile subsystem flow."));
}

void UPlayerDlg::OnDel()
{
    UE_LOG(LogPlayerDlg, Warning, TEXT("[PlayerDlg] Delete is disabled for single-profile subsystem flow."));
}

void UPlayerDlg::OnApply()
{
    UpdatePlayerFromUI_Subsystem();

    if (manager)
        manager->ShowMenuDlg();
    else
        HideDlg();
}

void UPlayerDlg::OnCancel()
{
    RefreshUIFromSubsystem();

    if (manager)
        manager->ShowMenuDlg();
    else
        HideDlg();
}
