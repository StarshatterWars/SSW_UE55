/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         FirstTimeDlg.h
    AUTHOR:       John DiCamillo

    UNREAL PORT:
    - Converted from FormWindow to UBaseScreen (UUserWidget-derived).
    - Preserves original member names and intent.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "FirstTimeDlg.generated.h"

class UButton;
class UComboBoxString;
class UEditableTextBox;

/**
 * Main Menu Dialog Active Window class (UE UBaseScreen port)
 */
UCLASS()
class STARSHATTERWARS_API UFirstTimeDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UFirstTimeDlg(const FObjectInitializer& ObjectInitializer);

    // Original API surface (ported):
    virtual void      RegisterControls();   // bind widget events, cache pointers if needed
    virtual void      Show();               // make visible / focus
    virtual void      ExecFrame();          // optional per-frame logic (usually avoid)

    // Operations:
    UFUNCTION()
    virtual void      OnApply();

protected:
    // UUserWidget lifecycle:
    virtual void      NativeOnInitialized() override;
    virtual void      NativeConstruct() override;
    virtual void      NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    // Starshatter: MenuScreen* manager;
    // UE: BaseScreen is the shared base; keep manager generic but strongly typed to your base.
    UPROPERTY(BlueprintReadWrite, Category = "FirstTimeDlg")
    UBaseScreen* manager = nullptr;

    // Starshatter: EditBox* edt_name;
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UEditableTextBox* edt_name = nullptr;

    // Starshatter: ComboBox* cmb_playstyle;
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UComboBoxString* cmb_playstyle = nullptr;

    // Starshatter: ComboBox* cmb_experience;
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UComboBoxString* cmb_experience = nullptr;

    // Starshatter: Button* btn_apply;
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UButton* btn_apply = nullptr;
};
