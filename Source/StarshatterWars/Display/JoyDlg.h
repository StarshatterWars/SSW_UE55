#pragma once

#include "CoreMinimal.h"
#include "BaseScreen.h"

// Minimal Unreal includes requested for headers:
#include "Math/Vector.h"
#include "Math/Color.h"
#include "Math/UnrealMathUtility.h"

#include "JoyDlg.generated.h"

// Forward declarations:
class UButton;
class UTextBlock;
class UControlOptionsDlg;

UCLASS()
class STARSHATTERWARS_API UJoyDlg : public UBaseScreen
{
    GENERATED_BODY()

public:
    UJoyDlg(const FObjectInitializer& ObjectInitializer);

    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // Legacy parity:
    void ExecFrame();

    // ? Manager is ControlOptionsDlg (NOT MenuScreen):
    void SetManager(UControlOptionsDlg* InManager);

protected:
    UFUNCTION() void OnApplyClicked();
    UFUNCTION() void OnCancelClicked();
    UFUNCTION() void OnAxis0Clicked();
    UFUNCTION() void OnAxis1Clicked();
    UFUNCTION() void OnAxis2Clicked();
    UFUNCTION() void OnAxis3Clicked();

protected:
    UPROPERTY(meta = (BindWidgetOptional)) UTextBlock* MessageText = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UButton* AxisButton0 = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* AxisButton1 = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* AxisButton2 = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* AxisButton3 = nullptr;

    UPROPERTY(meta = (BindWidgetOptional)) UButton* Invert0 = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* Invert1 = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* Invert2 = nullptr;
    UPROPERTY(meta = (BindWidgetOptional)) UButton* Invert3 = nullptr;

protected:
    // ? Match your error log: Manager is a TObjectPtr<UControlOptionsDlg>
    UPROPERTY(Transient)
    TObjectPtr<UControlOptionsDlg> Manager = nullptr;

    int SelectedAxis = -1;
    int SampleAxis = -1;
    int Samples[8] = { 0,0,0,0,0,0,0,0 };
    int MapAxis[4] = { -1,-1,-1,-1 };

private:
    void HandleAxisClicked(int AxisIndex);
    void RefreshAxisButtonsFromCurrentBindings();
    void UpdateAxisButtonText(int AxisIndex, const char* TextId);
    void SetInvertButtonState(UButton* InvertButton, bool bChecked);
    bool GetInvertButtonState(const UButton* InvertButton) const;
};
