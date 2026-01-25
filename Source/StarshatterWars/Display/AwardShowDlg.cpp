/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         AwardShowDlg.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Award / Rank display dialog Unreal UUserWidget implementation.
    Port of Starshatter 4.5 AwardShowDlg (FormWindow) to UMG + UBaseScreen.
*/

#include "AwardShowDlg.h"

// IMPORTANT: full definition required to call manager methods:
#include "GameScreen.h"

// UMG:
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

// Input:
#include "Input/Reply.h"
#include "InputCoreTypes.h"

// Starshatter:
#include "PlayerCharacter.h"

UAwardShowDlg::UAwardShowDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UAwardShowDlg::BindFormWidgets()
{
    // Map FORM IDs to widgets:
    BindLabel(201, lbl_info);
    BindImage(202, img_rank);
    BindLabel(203, lbl_name);
    BindButton(1, btn_close);
}

void UAwardShowDlg::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind close:
    if (btn_close)
        btn_close->OnClicked.AddDynamic(this, &UAwardShowDlg::OnCloseClicked);

    // Match legacy Show():
    ShowAward();

    // Legacy latch resets on show:
    exit_latch = true;
}

void UAwardShowDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // Legacy behavior: after one frame of "idle", allow close on key
    // (original code latched until no key pressed)
    // Here we release latch after first tick.
    if (exit_latch)
        exit_latch = false;
}

FReply UAwardShowDlg::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey Key = InKeyEvent.GetKey();

    if (!exit_latch)
    {
        if (Key == EKeys::Enter || Key == EKeys::Virtual_Accept)
        {
            OnCloseClicked();
            return FReply::Handled();
        }

        if (Key == EKeys::Escape || Key == EKeys::Virtual_Back)
        {
            OnCloseClicked();
            return FReply::Handled();
        }
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

// ---------------------------------------------------------------------
// Legacy operations
// ---------------------------------------------------------------------

void UAwardShowDlg::SetRank(int32 InRank)
{
    rank = InRank;
    medal = -1;
}

void UAwardShowDlg::SetMedal(int32 InMedal)
{
    rank = -1;
    medal = InMedal;
}

void UAwardShowDlg::ShowAward()
{
    // Rank:
    if (rank >= 0)
    {
        if (lbl_name)
        {
            const FString NameText = FString(TEXT("Rank of ")) + FString(PlayerCharacter::RankName(rank));
            lbl_name->SetText(FText::FromString(NameText));
        }

        if (lbl_info)
        {
            lbl_info->SetText(FText::FromString(FString(PlayerCharacter::RankDescription(rank))));
        }

        if (img_rank)
        {
            // NOTE:
            // Player::RankInsignia returns a Bitmap* in legacy code. Your Unreal port likely wraps this.
            // If you have a UTexture2D* equivalent, set it here.
            //
            // Example (if you have a helper):
            // UTexture2D* Tex = Player::RankInsigniaTexture(rank, 1);
            // img_rank->SetBrushFromTexture(Tex, true);

            img_rank->SetVisibility(ESlateVisibility::Visible);
        }

        return;
    }

    // Medal:
    if (medal >= 0)
    {
        if (lbl_name)
        {
            lbl_name->SetText(FText::FromString(FString(PlayerCharacter::MedalName(medal))));
        }

        if (lbl_info)
        {
            lbl_info->SetText(FText::FromString(FString(PlayerCharacter::MedalDescription(medal))));
        }

        if (img_rank)
        {
            // NOTE: same texture binding note as above.
            img_rank->SetVisibility(ESlateVisibility::Visible);
        }

        return;
    }

    // None:
    if (lbl_name) lbl_name->SetText(FText::GetEmpty());
    if (lbl_info) lbl_info->SetText(FText::GetEmpty());
    if (img_rank) img_rank->SetVisibility(ESlateVisibility::Collapsed);
}

// ---------------------------------------------------------------------
// Close
// ---------------------------------------------------------------------

void UAwardShowDlg::OnCloseClicked()
{
    if (manager)
        manager->ShowPlayerDlg();
    else
        UE_LOG(LogTemp, Warning, TEXT("AwardShowDlg: manager is null (OnCloseClicked)."));
}
