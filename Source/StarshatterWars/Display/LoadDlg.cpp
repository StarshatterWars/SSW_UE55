/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         LoadDlg.cpp
    AUTHOR:       John DiCamillo

    UNREAL PORT:
    - Converted from FormWindow to UBaseScreen (UUserWidget).
    - Preserves original member names and intent.
    - Uses NativeTick to replicate ExecFrame polling.
    - Keeps original .frm content embedded for reference (UE-only runtime uses UMG).
*/

#include "LoadDlg.h"

// Unreal:
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

// Starshatter port (your project headers):
#include "Starshatter.h"
#include "Game.h"

ULoadDlg::ULoadDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void ULoadDlg::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    RegisterControls();
}

void ULoadDlg::NativeConstruct()
{
    Super::NativeConstruct();
}

void ULoadDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    ExecFrame();
}

void ULoadDlg::RegisterControls()
{
    // In UE, these are bound by name in the Widget Blueprint via BindWidget.
    // This method exists to preserve the original API surface and for any caching/validation you want later.
}

void ULoadDlg::ExecFrame()
{
    Starshatter* stars = Starshatter::GetInstance();
    if (!stars)
        return;

    // Title varies by game mode:
    if (title)
    {
        const int32 Mode = (int32)stars->GetGameMode();

        if (Mode == (int32)Starshatter::CLOD_MODE || Mode == (int32)Starshatter::CMPN_MODE)
        {
            title->SetText(FText::FromString(Game::GetText("LoadDlg.campaign").data()));
        }
        else if (Mode == (int32)Starshatter::MENU_MODE)
        {
            title->SetText(FText::FromString(Game::GetText("LoadDlg.tac-ref").data()));
        }
        else
        {
            title->SetText(FText::FromString(Game::GetText("LoadDlg.mission").data()));
        }
    }

    if (activity)
    {
        // Assuming GetLoadActivity() returns a Text/String type in your port.
        // If it returns std::string/Text wrapper, adjust accordingly.
        activity->SetText(FText::FromString(stars->GetLoadActivity().data()));
    }

    if (progress)
    {
        // Starshatter slider is 0..1 (typically). UProgressBar uses 0..1.
        const float P = (float)stars->GetLoadProgress();
        ProgressValue = P;
        progress->SetPercent(FMath::Clamp(P, 0.0f, 1.0f));
    }
}

/* --------------------------------------------------------------------
   FORM (REFERENCE ONLY — NOT PARSED AT RUNTIME IN BASIC UE PORT)
   --------------------------------------------------------------------

   +--------------------------------------------------------------------+
   |  Project:   Starshatter 4.5                                         |
   |  File:      ExitDlg.frm                                             |
   |                                                                    |
   |  Destroyer Studios LLC                                              |
   |  Copyright © 1997-2004. All Rights Reserved.                        |
   +--------------------------------------------------------------------+

form: {
   rect:       (0,0,440,320)
   back_color: (0,0,0)
   fore_color: (255,255,255)
   font:       Limerick12

   texture:    "Message.pcx"
   margins:    (50,40,48,40)

   layout: {
      x_mins:     (40, 40, 100, 100, 40)
      x_weights:  ( 0,  0,   1,   1,  0)

      y_mins:     (44, 30, 60, 35, 25)
      y_weights:  ( 0,  0,  0,  0,  0)
   }

   ctrl: {
      id:            100
      type:          label
      cells:         (1,1,3,1)
      text:          "Loading Mission"

      align:         center
      font:          Limerick18
      fore_color:    (255,255,255)
      transparent:   true
   },

   ctrl: {
      id:            101
      type:          label
      cells:         (1,3,3,1)
      text:          ""

      align:         center
      font:          Verdana
      fore_color:    (255,255,255)
      transparent:   true
   },

   ctrl: {
      id:            102,
      type:          slider,
      cells:         (1,4,3,1)
      fixed_height:  8

      active_color:  (250, 250, 100),
      back_color:    ( 21,  21,  21),
      border:        false,
      transparent:   false,
   },
}

-------------------------------------------------------------------- */
