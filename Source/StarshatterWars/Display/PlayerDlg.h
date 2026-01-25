/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004.

    SUBSYSTEM:    Stars.exe
    FILE:         PlayerDlg.h
    AUTHOR:       John DiCamillo

    UNREAL PORT:
    - Converted from FormWindow to UBaseScreen (UUserWidget-derived).
    - Preserves original member names and intent.
*/

#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"
#include "PlayerDlg.generated.h"

class UButton;
class UEditableTextBox;
class UListView;
class UTextBlock;
class UImage;

// Forward declarations for your ported gameplay/menu classes:
class MenuScreen;
class Player;

/**
 * Main Menu Dialog Active Window class (UE UBaseScreen port)
 */
UCLASS()
class STARSHATTERWARS_API UPlayerDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UPlayerDlg(const FObjectInitializer& ObjectInitializer);

    // Original API surface (ported):
    virtual void      RegisterControls();
    virtual void      Show();
    virtual void      ExecFrame();

    // Operations (ported from AWEvent style; UE binds directly):
    UFUNCTION()
    virtual void      OnApply();

    UFUNCTION()
    virtual void      OnCancel();

    UFUNCTION()
    virtual void      OnSelectPlayer();

    UFUNCTION()
    virtual void      OnAdd();

    UFUNCTION()
    virtual void      OnDel();

    UFUNCTION()
    virtual void      OnDelConfirm();

    UFUNCTION()
    virtual void      OnRank();

    UFUNCTION()
    virtual void      OnMedal();

    virtual void      UpdatePlayer();
    virtual void      ShowPlayer();

protected:
    // UUserWidget lifecycle:
    virtual void      NativeOnInitialized() override;
    virtual void      NativeConstruct() override;
    virtual void      NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    // Starshatter: MenuScreen* manager;
    // UE: keep generic until you have a UMenuScreen/UMenuRoot type to cast to.
    UPROPERTY(BlueprintReadWrite, Category = "PlayerDlg")
    UObject* manager = nullptr;

    // ListBox* lst_roster;
    // UE: prefer UListView (Object list). If you use UListView, entries are UObjects.
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UListView* lst_roster = nullptr;

    // Button* btn_add/btn_del;
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UButton* btn_add = nullptr;

    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UButton* btn_del = nullptr;

    // EditBox* fields:
    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UEditableTextBox* txt_name = nullptr;

    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UEditableTextBox* txt_password = nullptr;

    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UEditableTextBox* txt_squadron = nullptr;

    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UEditableTextBox* txt_signature = nullptr;

    // EditBox* txt_chat[10];
    // UE: bind individually (txt_chat_0 ... txt_chat_9) since UPROPERTY cannot BindWidget an array.
    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UEditableTextBox* txt_chat_0 = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UEditableTextBox* txt_chat_1 = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UEditableTextBox* txt_chat_2 = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UEditableTextBox* txt_chat_3 = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UEditableTextBox* txt_chat_4 = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UEditableTextBox* txt_chat_5 = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UEditableTextBox* txt_chat_6 = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UEditableTextBox* txt_chat_7 = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UEditableTextBox* txt_chat_8 = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UEditableTextBox* txt_chat_9 = nullptr;

    // ActiveWindow* labels -> UTextBlock*
    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UTextBlock* lbl_createdate = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UTextBlock* lbl_rank = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UTextBlock* lbl_flighttime = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UTextBlock* lbl_missions = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UTextBlock* lbl_kills = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UTextBlock* lbl_losses = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UTextBlock* lbl_points = nullptr;

    // ImageBox* -> UImage*
    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UImage* img_rank = nullptr;

    // ImageBox* img_medals[15];
    // UE: bind individually (img_medals_0 ... img_medals_14)
    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UImage* img_medals_0 = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UImage* img_medals_1 = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UImage* img_medals_2 = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UImage* img_medals_3 = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UImage* img_medals_4 = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UImage* img_medals_5 = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UImage* img_medals_6 = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UImage* img_medals_7 = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UImage* img_medals_8 = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UImage* img_medals_9 = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UImage* img_medals_10 = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UImage* img_medals_11 = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UImage* img_medals_12 = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UImage* img_medals_13 = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UImage* img_medals_14 = nullptr;

    // int medals[15]; keep as TArray for UE reflection safety, but preserve name.
    UPROPERTY(BlueprintReadOnly, Category = "PlayerDlg")
    TArray<int32>      medals;

    // Buttons:
    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UButton* apply = nullptr;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UButton* cancel = nullptr;
};
