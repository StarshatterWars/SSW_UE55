/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         KeyDlg.h
    AUTHOR:       John DiCamillo

    UNREAL PORT:
    - Converted from FormWindow to UUserWidget.
    - Preserves original member names and intent.
*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KeyDlg.generated.h"

class UButton;
class UTextBlock;

/**
 * Navigation Active Window class (UE UUserWidget port)
 */
UCLASS()
class STARSHATTERWARS_API UKeyDlg : public UUserWidget
{
    GENERATED_BODY()

public:
    UKeyDlg(const FObjectInitializer& ObjectInitializer);

    // Original API surface (ported):
    virtual void      RegisterControls();   // bind widget events, cache pointers if needed
    virtual void      Show();               // make visible / focus
    virtual void      ExecFrame();          // optional per-frame logic (usually avoid; see .cpp)

    // UI callbacks (ported from AWEvent style; UE will bind to these):
    UFUNCTION()
    virtual void      OnApply();

    UFUNCTION()
    virtual void      OnCancel();

    UFUNCTION()
    virtual void      OnClear();

    int               GetKeyMapIndex() const { return key_index; }
    void              SetKeyMapIndex(int i);

protected:
    // UUserWidget lifecycle:
    virtual void      NativeOnInitialized() override;
    virtual void      NativeConstruct() override;
    virtual void      NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    // Starshatter: BaseScreen* manager;
    // UE: keep as UObject* so you can point at HUD/Controller/SubSystem without hard dependency.
    UPROPERTY(BlueprintReadWrite, Category = "KeyDlg")
    UObject* manager = nullptr;

    // Preserved member names:
    UPROPERTY(BlueprintReadOnly, Category = "KeyDlg")
    int32             key_index = 0;

    UPROPERTY(BlueprintReadOnly, Category = "KeyDlg")
    int32             key_key = 0;

    UPROPERTY(BlueprintReadOnly, Category = "KeyDlg")
    int32             key_shift = 0;

    UPROPERTY(BlueprintReadOnly, Category = "KeyDlg")
    int32             key_joy = 0;

    UPROPERTY(BlueprintReadOnly, Category = "KeyDlg")
    int32             key_clear = 0;

    // Starshatter: Button* clear/apply/cancel;
    // UE: UButton* with BindWidget (names must match in the Widget Blueprint).
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UButton* clear = nullptr;

    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UButton* apply = nullptr;

    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UButton* cancel = nullptr;

    // Starshatter: ActiveWindow* command/current_key/new_key;
    // UE: display fields as TextBlocks (or swap to UEditableTextBox if needed).
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UTextBlock* command = nullptr;

    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UTextBlock* current_key = nullptr;

    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UTextBlock* new_key = nullptr;
};
