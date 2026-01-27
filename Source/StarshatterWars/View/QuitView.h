/*  Project STARSHATTER WARS
    Fractal Dev Studios
    Copyright (c) 2025-2026.

    ORIGINAL AUTHOR: John DiCamillo
    ORIGINAL STUDIO: Destroyer Studios

    SUBSYSTEM:    Stars.exe
    FILE:         QuitView.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    UQuitView
    - Unreal (UMG) port of legacy QuitView (End Mission menu).
    - Inherits from UView (which is a UUserWidget).
    - Keeps all legacy game logic:
      * CanAccept() threat/time checks
      * Accept (exit + keep results)
      * Discard (rollback/unload)
      * Resume
      * Controls (delegates to GameScreen)
      * Pause/unpause + mouse cursor/input mode toggles
*/

#pragma once

#include "CoreMinimal.h"

// IMPORTANT: Inherit from the new View base (UView):
#include "View.h"

#include "QuitView.generated.h"

class UButton;
class UTextBlock;

class Sim;

UCLASS()
class STARSHATTERWARS_API UQuitView : public UView
{
    GENERATED_BODY()

public:
    UQuitView(const FObjectInitializer& ObjectInitializer);

    // Legacy-style API (kept):
    bool  IsMenuShown() const;
    void  ShowMenu();
    void  CloseMenu();
    bool  CanAccept();

    virtual void ExecFrame(float DeltaTime);

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;

    // Button handlers:
    UFUNCTION() void OnAcceptClicked();
    UFUNCTION() void OnDiscardClicked();
    UFUNCTION() void OnResumeClicked();
    UFUNCTION() void OnControlsClicked();

    void ApplyMenuInputMode(bool bEnableMenu);
    void SetMessageText(const FString& InText);

protected:
    // UMG widgets (must exist in the Widget Blueprint with these exact names):
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> BtnAccept = nullptr;     // "ACCEPT / EXIT"
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> BtnDiscard = nullptr;    // "DISCARD"
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> BtnResume = nullptr;     // "RESUME"
    UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> BtnControls = nullptr;   // "CONTROLS"

    // Optional message label for errors like "too soon" / "threats present":
    UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> TxtMessage = nullptr;

private:
    bool bMenuShown = false;
    bool bPrevShowMouseCursor = false;

    Sim* sim = nullptr; // raw pointer per your direction
};
