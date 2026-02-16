// PlayerDlg.cpp

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
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Components/ListView.h"
#include "Components/SizeBox.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/Spacer.h"

// Input
#include "Input/Reply.h"

// Model
#include "PlayerRosterItem.h"
#include "PlayerCharacter.h"
#include "MenuScreen.h"

DEFINE_LOG_CATEGORY_STATIC(LogPlayerDlg, Log, All);

// -------------------- compile-time “feature detection” --------------------
template<typename T>
concept HasPasswordGet = requires(T * p) { { p->Password() }; };

template<typename T>
concept HasPasswordSet = requires(T * p, const FString & s) { p->SetPassword(s); };

template<typename T>
concept HasSquadGet = requires(T * p) { { p->Squadron() }; };

template<typename T>
concept HasSquadSet = requires(T * p, const FString & s) { p->SetSquadron(s); };

template<typename T>
concept HasSignatureGet = requires(T * p) { { p->Signature() }; };

template<typename T>
concept HasSignatureSet = requires(T * p, const FString & s) { p->SetSignature(s); };

template<typename T>
concept HasPointsGet = requires(T * p) { { p->Points() }; };

template<typename T>
concept HasChatMacroGet = requires(T * p, int32 i) { { p->GetChatMacro(i) }; };

template<typename T>
concept HasChatMacroSet = requires(T * p, int32 i, const FString & s) { p->SetChatMacro(i, s); };

// ------------------------------------------------------------------------

UPlayerDlg::UPlayerDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SelectedPlayer = nullptr;
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

    EnsureLayout();
    EnsureScrollHostsStats();

    BuildRosterPanel();
    BuildStatsRows();

    BuildRoster();
    RefreshUIFromPlayer();
}

void UPlayerDlg::NativeDestruct()
{
    if (lst_roster)
        lst_roster->OnItemSelectionChanged().RemoveAll(this);

    if (btn_add) btn_add->OnClicked.RemoveAll(this);
    if (btn_del) btn_del->OnClicked.RemoveAll(this);
    if (BtnApply) BtnApply->OnClicked.RemoveAll(this);
    if (BtnCancel) BtnCancel->OnClicked.RemoveAll(this);

    Super::NativeDestruct();
}

// BaseScreen central Enter/Escape routes here:
void UPlayerDlg::HandleAccept() { OnApply(); }
void UPlayerDlg::HandleCancel() { OnCancel(); }

void UPlayerDlg::ShowDlg()
{
    SetVisibility(ESlateVisibility::Visible);
    BuildRoster();
    RefreshUIFromPlayer();
}

void UPlayerDlg::HideDlg()
{
    SetVisibility(ESlateVisibility::Collapsed);
}

// ------------------------------------------------------------------------
// Layout (Runtime)
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
    }

    if (!RootCanvas)
    {
        UE_LOG(LogPlayerDlg, Error, TEXT("[PlayerDlg] EnsureLayout: RootCanvas is NULL"));
        return;
    }

    if (MainRow)
        return;

    // Main horizontal split
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

    // Left panel (roster)
    LeftPanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LeftPanel"));
    // Right scroll (stats)
    StatsScroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("StatsScroll"));
    StatsVBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("StatsVBox"));

    if (!LeftPanel || !StatsScroll || !StatsVBox)
        return;

    // Slot left (fixed-ish)
    if (UHorizontalBoxSlot* L = MainRow->AddChildToHorizontalBox(LeftPanel))
    {
        L->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
        L->SetHorizontalAlignment(HAlign_Left);
        L->SetVerticalAlignment(VAlign_Fill);
        L->SetPadding(FMargin(0.f, 0.f, 20.f, 0.f));
    }

    // Slot right (fills)
    if (UHorizontalBoxSlot* R = MainRow->AddChildToHorizontalBox(StatsScroll))
    {
        R->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        R->SetHorizontalAlignment(HAlign_Fill);
        R->SetVerticalAlignment(VAlign_Fill);
        R->SetPadding(FMargin(0.f));
    }

    // Scroll behavior like Options pages
    StatsScroll->SetConsumeMouseWheel(EConsumeMouseWheel::WhenScrollingPossible);
    StatsScroll->SetAnimateWheelScrolling(true);
    StatsScroll->SetScrollBarVisibility(ESlateVisibility::Visible);

    // IMPORTANT:
    // Use StatsVBox as the "AutoVBox" target so we can do Options-style rows
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
// Options-style row builders (local)
// ------------------------------------------------------------------------

UTextBlock* UPlayerDlg::MakeValueText()
{
    if (!WidgetTree) return nullptr;

    UTextBlock* T = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    if (!T) return nullptr;

    T->SetJustification(ETextJustify::Right);
    return T;
}

UEditableTextBox* UPlayerDlg::MakeEditBox(bool bPassword)
{
    if (!WidgetTree) return nullptr;

    UEditableTextBox* E = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass());
    if (!E) return nullptr;

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

    // Label (left)
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

    // Right side (fixed width wrapper like Options)
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
// Left panel (roster)
// ------------------------------------------------------------------------

void UPlayerDlg::BuildRosterPanel()
{
    if (!WidgetTree || !LeftPanel)
        return;

    LeftPanel->ClearChildren();

    // Roster list
    lst_roster = WidgetTree->ConstructWidget<UListView>(UListView::StaticClass(), TEXT("RosterList"));
    btn_add = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("BtnCreate"));
    btn_del = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("BtnDelete"));

    if (!lst_roster || !btn_add || !btn_del)
        return;

    // Simple button text
    auto MakeBtnLabel = [&](UButton* B, const TCHAR* Txt)
        {
            UTextBlock* T = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
            if (T) T->SetText(FText::FromString(Txt));
            if (T) B->AddChild(T);
        };
    MakeBtnLabel(btn_add, TEXT("CREATE"));
    MakeBtnLabel(btn_del, TEXT("DELETE"));

    // List slot
    if (UVerticalBoxSlot* S = LeftPanel->AddChildToVerticalBox(lst_roster))
    {
        S->SetHorizontalAlignment(HAlign_Fill);
        S->SetVerticalAlignment(VAlign_Fill);
        S->SetPadding(FMargin(0.f, 0.f, 0.f, 10.f));
    }

    // Buttons row
    UHorizontalBox* BtnRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("RosterBtnRow"));
    if (BtnRow)
    {
        if (UVerticalBoxSlot* VS = LeftPanel->AddChildToVerticalBox(BtnRow))
        {
            VS->SetHorizontalAlignment(HAlign_Fill);
            VS->SetVerticalAlignment(VAlign_Top);
        }

        if (UHorizontalBoxSlot* A = BtnRow->AddChildToHorizontalBox(btn_add))
        {
            A->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
            A->SetPadding(FMargin(0.f, 0.f, 6.f, 0.f));
        }

        if (UHorizontalBoxSlot* D = BtnRow->AddChildToHorizontalBox(btn_del))
        {
            D->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
            D->SetPadding(FMargin(6.f, 0.f, 0.f, 0.f));
        }
    }

    // Bind delegates once
    lst_roster->OnItemSelectionChanged().RemoveAll(this);
    lst_roster->OnItemSelectionChanged().AddUObject(this, &UPlayerDlg::OnRosterSelectionChanged);

    btn_add->OnClicked.RemoveAll(this);
    btn_add->OnClicked.AddDynamic(this, &UPlayerDlg::OnAdd);

    btn_del->OnClicked.RemoveAll(this);
    btn_del->OnClicked.AddDynamic(this, &UPlayerDlg::OnDel);
}

// ------------------------------------------------------------------------
// Right panel (stats) — everything inside scroll, rows like Options
// ------------------------------------------------------------------------

void UPlayerDlg::BuildStatsRows()
{
    if (!WidgetTree || !StatsVBox)
        return;

    StatsVBox->ClearChildren();
    MedalImages.Reset();
    MacroEdits.Reset();

    // Title
    {
        UTextBlock* TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Title"));
        if (TitleText)
        {
            TitleText->SetText(FText::FromString(TEXT("PLAYER LOGBOOK")));
            TitleText->SetJustification(ETextJustify::Left);

            if (UVerticalBoxSlot* S = StatsVBox->AddChildToVerticalBox(Title))
            {
                S->SetPadding(FMargin(0.f, 0.f, 0.f, 14.f));
                S->SetHorizontalAlignment(HAlign_Left);
            }
        }
    }

    // Editable fields
    edt_name = MakeEditBox(false);
    edt_password = MakeEditBox(true);
    edt_squadron = MakeEditBox(false);
    edt_signature = MakeEditBox(false);

    AddStatRow(FText::FromString(TEXT("NAME")), edt_name);
    AddStatRow(FText::FromString(TEXT("PASSWORD")), edt_password);
    AddStatRow(FText::FromString(TEXT("SQUADRON")), edt_squadron);
    AddStatRow(FText::FromString(TEXT("SIGNATURE")), edt_signature);

    // Read-only stats
    txt_created = MakeValueText();
    txt_flighttime = MakeValueText();
    txt_missions = MakeValueText();
    txt_kills = MakeValueText();
    txt_losses = MakeValueText();
    txt_points = MakeValueText();
    txt_rank = MakeValueText();

    AddStatRow(FText::FromString(TEXT("CREATED")), txt_created);
    AddStatRow(FText::FromString(TEXT("FLIGHT TIME")), txt_flighttime);
    AddStatRow(FText::FromString(TEXT("MISSIONS")), txt_missions);
    AddStatRow(FText::FromString(TEXT("KILLS")), txt_kills);
    AddStatRow(FText::FromString(TEXT("LOSSES")), txt_losses);
    AddStatRow(FText::FromString(TEXT("POINTS")), txt_points);
    AddStatRow(FText::FromString(TEXT("RANK")), txt_rank);

    // Rank insignia image row (right aligned)
    img_rank = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("RankImage"));
    if (img_rank)
    {
        // Add as its own row, label on left, image on right
        AddStatRow(FText::FromString(TEXT("INSIGNIA")), img_rank, 160.f);
    }

    // Medals grid
    {
        UTextBlock* MedalsHdr = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MedalsHdr"));
        if (MedalsHdr)
        {
            MedalsHdr->SetText(FText::FromString(TEXT("MEDALS")));
            MedalsHdr->SetJustification(ETextJustify::Left);

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

    // Chat macros
    {
        UTextBlock* MacroHdr = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("MacroHdr"));
        if (MacroHdr)
        {
            MacroHdr->SetText(FText::FromString(TEXT("CHAT MACROS")));
            MacroHdr->SetJustification(ETextJustify::Left);

            if (UVerticalBoxSlot* S = StatsVBox->AddChildToVerticalBox(MacroHdr))
                S->SetPadding(FMargin(0.f, 8.f, 0.f, 8.f));
        }

        BuildChatMacrosRows();
    }

    // Bottom Apply/Cancel
    {
        UHorizontalBox* Bottom = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("BottomButtons"));
        if (Bottom)
        {
            if (UVerticalBoxSlot* S = StatsVBox->AddChildToVerticalBox(Bottom))
            {
                S->SetPadding(FMargin(0.f, 18.f, 0.f, 0.f));
                S->SetHorizontalAlignment(HAlign_Fill);
            }

            BtnApply = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("BtnApply"));
            BtnCancel = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("BtnCancel"));

            if (BtnApply)
            {
                UTextBlock* T = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
                if (T) T->SetText(FText::FromString(TEXT("APPLY")));
                if (T) BtnApply->AddChild(T);

                BtnApply->OnClicked.RemoveAll(this);
                BtnApply->OnClicked.AddDynamic(this, &UPlayerDlg::OnApply);
            }

            if (BtnCancel)
            {
                UTextBlock* T = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
                if (T) T->SetText(FText::FromString(TEXT("CANCEL")));
                if (T) BtnCancel->AddChild(T);

                BtnCancel->OnClicked.RemoveAll(this);
                BtnCancel->OnClicked.AddDynamic(this, &UPlayerDlg::OnCancel);
            }

            // Hook BaseScreen routing to these
            ApplyButton = BtnApply;
            CancelButton = BtnCancel;

            if (BtnApply)
            {
                if (UHorizontalBoxSlot* A = Bottom->AddChildToHorizontalBox(BtnApply))
                {
                    A->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
                    A->SetPadding(FMargin(0.f, 0.f, 8.f, 0.f));
                }
            }

            if (BtnCancel)
            {
                if (UHorizontalBoxSlot* C = Bottom->AddChildToHorizontalBox(BtnCancel))
                {
                    C->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
                    C->SetPadding(FMargin(8.f, 0.f, 0.f, 0.f));
                }
            }
        }
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

        // Wrap (fixed cell size like legacy: 82x21-ish)
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

    // Legacy labels are 1..9 and 0
    auto LabelFor = [](int32 i) -> FString
        {
            if (i >= 1 && i <= 9) return FString::FromInt(i);
            return TEXT("0");
        };

    // Build rows 1..9 then 0 (to match legacy visual order)
    for (int32 ui = 1; ui <= 9; ++ui)
    {
        UEditableTextBox* E = MakeEditBox(false);
        MacroEdits[ui] = E;
        AddStatRow(FText::FromString(LabelFor(ui)), E);
    }

    {
        UEditableTextBox* E = MakeEditBox(false);
        MacroEdits[0] = E;
        AddStatRow(FText::FromString(LabelFor(0)), E);
    }
}

// ------------------------------------------------------------------------
// Roster list population
// ------------------------------------------------------------------------

void UPlayerDlg::BuildRoster()
{
    if (!lst_roster)
        return;

    const int32 PrevId =
        (selected_item && selected_item->GetPlayer())
        ? selected_item->GetPlayer()->GetIdentity()
        : 0;

    lst_roster->ClearListItems();
    selected_item = nullptr;
    SelectedPlayer = nullptr;

    List<PlayerCharacter>& roster = PlayerCharacter::GetRoster();
    if (roster.size() < 1)
    {
        RefreshUIFromPlayer();
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

    RefreshUIFromPlayer();
}

void UPlayerDlg::RefreshRoster()
{
    BuildRoster();
}

void UPlayerDlg::OnRosterSelectionChanged(UObject* SelectedItem)
{
    UPlayerRosterItem* Item = Cast<UPlayerRosterItem>(SelectedItem);
    PlayerCharacter* player = Item ? Item->GetPlayer() : nullptr;

    selected_item = Item;
    SelectedPlayer = player;

    if (player)
        PlayerCharacter::SetCurrentPlayer(player);

    RefreshUIFromPlayer();
}

PlayerCharacter* UPlayerDlg::GetSelectedPlayer() const
{
    if (selected_item)
        return selected_item->GetPlayer();
    return nullptr;
}

// ------------------------------------------------------------------------
// Model <-> UI
// ------------------------------------------------------------------------

void UPlayerDlg::UpdatePlayerFromUI()
{
    PlayerCharacter* player = GetSelectedPlayer();
    if (!player)
        return;

    // Name
    if (edt_name)
    {
        const FString NewName = edt_name->GetText().ToString();
        if (!NewName.IsEmpty())
            player->SetName(NewName);
    }

    // Password (only if your PlayerCharacter supports it)
    if constexpr (HasPasswordSet<PlayerCharacter>)
    {
        if (edt_password)
            player->SetPassword(edt_password->GetText().ToString());
    }

    // Squadron
    if constexpr (HasSquadSet<PlayerCharacter>)
    {
        if (edt_squadron)
            player->SetSquadron(edt_squadron->GetText().ToString());
    }

    // Signature
    if constexpr (HasSignatureSet<PlayerCharacter>)
    {
        if (edt_signature)
            player->SetSignature(edt_signature->GetText().ToString());
    }

    // Chat macros (only if available)
    if constexpr (HasChatMacroSet<PlayerCharacter>)
    {
        if (MacroEdits.Num() == 10)
        {
            for (int32 i = 0; i < 10; ++i)
            {
                if (MacroEdits[i])
                    player->SetChatMacro(i, MacroEdits[i]->GetText().ToString());
            }
        }
    }
}

void UPlayerDlg::RefreshUIFromPlayer()
{
    PlayerCharacter* player = GetSelectedPlayer();

    if (!player)
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
        if (txt_rank)       txt_rank->SetText(FText::GetEmpty());

        if (MacroEdits.Num() == 10)
            for (UEditableTextBox* E : MacroEdits) if (E) E->SetText(FText::GetEmpty());

        return;
    }

    if (edt_name)
        edt_name->SetText(FText::FromString(player->Name()));

    if constexpr (HasPasswordGet<PlayerCharacter>)
    {
        if (edt_password)
            edt_password->SetText(FText::FromString(player->Password()));
    }

    if constexpr (HasSquadGet<PlayerCharacter>)
    {
        if (edt_squadron)
            edt_squadron->SetText(FText::FromString(player->Squadron()));
    }

    if constexpr (HasSignatureGet<PlayerCharacter>)
    {
        if (edt_signature)
            edt_signature->SetText(FText::FromString(player->Signature()));
    }

    if (txt_rank)
        txt_rank->SetText(FText::AsNumber(player->GetRank()));

    if (txt_flighttime)
        txt_flighttime->SetText(FText::FromString(FormatTimeHMS(player->FlightTime())));

    if (txt_created)
        txt_created->SetText(FText::FromString(FormatDateFromUnixSeconds((int64)player->CreateDate())));

    if (txt_kills)
        txt_kills->SetText(FText::AsNumber(player->Kills()));

    // legacy "Losses" maps cleanly to Deaths() in your port
    if (txt_losses)
        txt_losses->SetText(FText::AsNumber(player->Deaths()));

    if (txt_missions)
        txt_missions->SetText(FText::AsNumber(player->Missions()));

    if constexpr (HasPointsGet<PlayerCharacter>)
    {
        if (txt_points)
            txt_points->SetText(FText::AsNumber(player->Points()));
    }
    else
    {
        if (txt_points)
            txt_points->SetText(FText::GetEmpty());
    }

    if constexpr (HasChatMacroGet<PlayerCharacter>)
    {
        if (MacroEdits.Num() == 10)
        {
            for (int32 i = 0; i < 10; ++i)
            {
                //if (MacroEdits[i])
                //    MacroEdits[i]->SetText(FText::FromString(player->GetChatMacro(i)));
            }
        }
    }

    // Rank/medal imagery: hook your texture pipeline here later.
    // img_rank->SetBrushFromTexture(...)
    // MedalImages[idx]->SetBrushFromTexture(...)
}

// ------------------------------------------------------------------------
// Actions
// ------------------------------------------------------------------------

void UPlayerDlg::OnAdd()
{
    UpdatePlayerFromUI();

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
    PlayerCharacter* player = GetSelectedPlayer();
    if (!player)
        return;

    PlayerCharacter::RemoveFromRoster(player);
    PlayerCharacter::Save();

    selected_item = nullptr;
    SelectedPlayer = nullptr;
    RefreshRoster();
}

void UPlayerDlg::OnApply()
{
    UpdatePlayerFromUI();
    PlayerCharacter::Save();

    if (manager)
        manager->ShowMenuDlg();
    else
        HideDlg();
}

void UPlayerDlg::OnCancel()
{
    RefreshUIFromPlayer();

    if (manager)
        manager->ShowMenuDlg();
    else
        HideDlg();
}

// ------------------------------------------------------------------------
// Formatting helpers
// ------------------------------------------------------------------------

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
