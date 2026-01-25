/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (c) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:    Stars.exe
    FILE:         MenuScreen.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Main Menu Screen (Unreal UUserWidget)
*/

#pragma once

// Minimal Unreal includes required by project conventions:
#include "Math/Vector.h"                // FVector
#include "Math/Color.h"                 // FColor
#include "Math/UnrealMathUtility.h"     // Math

// Base screen is a UUserWidget in this port:
#include "BaseScreen.h"

#include "MenuScreen.generated.h"

// Forward declarations (keep header light):
class UWidget;

class UMenuDlg;
class UAudioDlg;
class UVidDlg;
class UOptDlg;
class UCtlDlg;
class UJoyDlg;
class UKeyDlg;
class UExitDlg;
class UConfirmDlg;

class UFirstTimeDlg;
class UPlayerDlg;
class UAwardShowDlg;

class UMsnSelectDlg;
class UCmpSelectDlg;

class UMsnEditDlg;
class UMsnElemDlg;
class UMsnEventDlg;
class UNavDlg;

class ULoadDlg;
class UTacRefDlg;

UCLASS()
class UMenuScreen : public UBaseScreen
{
    GENERATED_BODY()

public:
    UMenuScreen(const FObjectInitializer& ObjectInitializer);

    // UUserWidget overrides:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // Display state:
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI")
    bool IsShown() const { return isShown; }

    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI")
    void Show();

    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI")
    void Hide();

    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI")
    bool CloseTopmost();

    // Dialog navigation:
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI") void ShowMenuDlg();
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI") void ShowCmpSelectDlg();
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI") void ShowMsnSelectDlg();
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI") void ShowMsnEditDlg();

    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI") void ShowFirstTimeDlg();
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI") void ShowPlayerDlg();
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI") void ShowTacRefDlg();
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI") void ShowAwardDlg();

    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI") void ShowAudDlg();
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI") void ShowVidDlg();
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI") void ShowOptDlg();
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI") void ShowCtlDlg();
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI") void ShowJoyDlg();
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI") void ShowKeyDlg();
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI") void ShowExitDlg();
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI") void ShowConfirmDlg();
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI") void HideConfirmDlg();
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI") void ShowLoadDlg();
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI") void HideLoadDlg();

    // Base screen interface (mission editor support):
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI") void ShowMsnElemDlg();
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI") void HideMsnElemDlg();
    UMsnElemDlg* GetMsnElemDlg() const { return msnElemDlg; }

    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI") void ShowMsnEventDlg();
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI") void HideMsnEventDlg();
    UMsnEventDlg* GetMsnEventDlg() const { return msnEventDlg; }

    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI") void ShowNavDlg();
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI") void HideNavDlg();
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI") bool IsNavShown() const;

    UNavDlg* GetNavDlg() const { return msnEditNavDlg; }

    // Getters (optional convenience):
    UMsnSelectDlg* GetMsnSelectDlg() const { return msnSelectDlg; }

    UMsnEditDlg* GetMsnEditDlg()   const { return msnEditDlg; }
    ULoadDlg* GetLoadDlg()      const { return loadDlg; }
    UTacRefDlg* GetTacRefDlg()    const { return tacRefDlg; }

    UAudioDlg* GetAudDlg()       const { return auddlg; }
    UVidDlg* GetVidDlg()       const { return viddlg; }
    UOptDlg* GetOptDlg()       const { return optdlg; }
    UCtlDlg* GetCtlDlg()       const { return ctldlg; }
    UJoyDlg* GetJoyDlg()       const { return joydlg; }
    UKeyDlg* GetKeyDlg()       const { return keydlg; }
    UExitDlg* GetExitDlg()      const { return exitdlg; }
    UFirstTimeDlg* GetFirstTimeDlg() const { return firstdlg; }
    UPlayerDlg* GetPlayerDlg()    const { return playdlg; }
    UAwardShowDlg* GetAwardDlg()     const { return awarddlg; }
    UConfirmDlg* GetConfirmDlg()   const { return confirmdlg; }

    // Apply / cancel option changes (invoked by dialogs):
    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI")
    void ApplyOptions();

    UFUNCTION(BlueprintCallable, Category = "StarshatterWars|UI")
    void CancelOptions();

private:
    void HideAll();

private:
    // Dialog widgets (UMG instances / children):
    UPROPERTY() UMenuDlg* menudlg = nullptr;

    UPROPERTY() UExitDlg* exitdlg = nullptr;
    UPROPERTY() UAudioDlg* auddlg = nullptr;
    UPROPERTY() UVidDlg* viddlg = nullptr;
    UPROPERTY() UOptDlg* optdlg = nullptr;
    UPROPERTY() UCtlDlg* ctldlg = nullptr;
    UPROPERTY() UJoyDlg* joydlg = nullptr;
    UPROPERTY() UKeyDlg* keydlg = nullptr;
    UPROPERTY() UConfirmDlg* confirmdlg = nullptr;

    UPROPERTY() UPlayerDlg* playdlg = nullptr;
    UPROPERTY() UAwardShowDlg* awarddlg = nullptr;

    UPROPERTY() UMsnSelectDlg* msnSelectDlg = nullptr;
    UPROPERTY() UMsnEditDlg* msnEditDlg = nullptr;
    UPROPERTY() UMsnElemDlg* msnElemDlg = nullptr;
    UPROPERTY() UMsnEventDlg* msnEventDlg = nullptr;
    UPROPERTY() UNavDlg* msnEditNavDlg = nullptr;

    UPROPERTY() ULoadDlg* loadDlg = nullptr;
    UPROPERTY() UTacRefDlg* tacRefDlg = nullptr;

    UPROPERTY() UCmpSelectDlg* cmpSelectDlg = nullptr;
    UPROPERTY() UFirstTimeDlg* firstdlg = nullptr;

    // Track the currently displayed dialog (optional):
    UPROPERTY() UWidget* current_dlg = nullptr;

    bool isShown = false;
};
