#include "JoyDlg.h"

// Unreal
#include "Logging/LogMacros.h"

// UMG
#include "Components/Button.h"
#include "Components/TextBlock.h"

// Starshatter
#include "Starshatter.h"
#include "KeyMap.h"
#include "Joystick.h"
#include "Game.h"

// Manager:
#include "ControlOptionsDlg.h"

DEFINE_LOG_CATEGORY_STATIC(LogJoyDlg, Log, All);

// Legacy axis name tokens
static const char* JoyAxisNames[] =
{
    "JoyDlg.axis.0",
    "JoyDlg.axis.1",
    "JoyDlg.axis.2",
    "JoyDlg.axis.3",
    "JoyDlg.axis.4",
    "JoyDlg.axis.5",
    "JoyDlg.axis.6",
    "JoyDlg.axis.7"
};

UJoyDlg::UJoyDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UJoyDlg::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (ApplyButton)
        ApplyButton->OnClicked.AddDynamic(this, &UJoyDlg::OnApplyClicked);

    if (CancelButton)
        CancelButton->OnClicked.AddDynamic(this, &UJoyDlg::OnCancelClicked);

    if (AxisButton0) AxisButton0->OnClicked.AddDynamic(this, &UJoyDlg::OnAxis0Clicked);
    if (AxisButton1) AxisButton1->OnClicked.AddDynamic(this, &UJoyDlg::OnAxis1Clicked);
    if (AxisButton2) AxisButton2->OnClicked.AddDynamic(this, &UJoyDlg::OnAxis2Clicked);
    if (AxisButton3) AxisButton3->OnClicked.AddDynamic(this, &UJoyDlg::OnAxis3Clicked);
}

void UJoyDlg::NativeConstruct()
{
    Super::NativeConstruct();

    RefreshAxisButtonsFromCurrentBindings();

    SelectedAxis = -1;
    SampleAxis = -1;
    for (int i = 0; i < 8; ++i)
        Samples[i] = 10000000;
}

void UJoyDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    ExecFrame();
}

void UJoyDlg::SetManager(UControlOptionsDlg* InManager)
{
    Manager = InManager;
}

void UJoyDlg::ExecFrame()
{
    if (SelectedAxis < 0 || SelectedAxis >= 4)
        return;

    Joystick* Js = Joystick::GetInstance();
    if (!Js)
        return;

    Js->Acquire();

    int BestDelta = 1000;
    int BestAxis = -1;

    for (int i = 0; i < 8; ++i)
    {
        const int Raw = Joystick::ReadRawAxis(i + KEY_JOY_AXIS_X);
        const int Delta = FMath::Abs(Raw - Samples[i]);

        if (Delta > BestDelta && Samples[i] < 1000000)
        {
            BestDelta = Delta;
            BestAxis = i;
        }

        Samples[i] = Raw;
    }

    if (BestAxis >= 0)
    {
        SampleAxis = BestAxis;
        MapAxis[SelectedAxis] = BestAxis;
        UpdateAxisButtonText(SelectedAxis, JoyAxisNames[BestAxis]);
    }
}

void UJoyDlg::OnAxis0Clicked() { HandleAxisClicked(0); }
void UJoyDlg::OnAxis1Clicked() { HandleAxisClicked(1); }
void UJoyDlg::OnAxis2Clicked() { HandleAxisClicked(2); }
void UJoyDlg::OnAxis3Clicked() { HandleAxisClicked(3); }

void UJoyDlg::HandleAxisClicked(int AxisIndex)
{
    if (AxisIndex < 0 || AxisIndex >= 4)
        return;

    if (SelectedAxis == AxisIndex)
    {
        SelectedAxis = -1;

        const int Map = MapAxis[AxisIndex];
        if (Map >= 0 && Map < 8)
            UpdateAxisButtonText(AxisIndex, JoyAxisNames[Map]);
        else
            UpdateAxisButtonText(AxisIndex, "JoyDlg.unmapped");
    }
    else
    {
        for (int i = 0; i < 4; ++i)
        {
            const int Map = MapAxis[i];
            if (Map >= 0 && Map < 8)
                UpdateAxisButtonText(i, JoyAxisNames[Map]);
            else
                UpdateAxisButtonText(i, "JoyDlg.unmapped");
        }

        SelectedAxis = AxisIndex;
        SampleAxis = -1;
        UpdateAxisButtonText(AxisIndex, "JoyDlg.select");

        for (int i = 0; i < 8; ++i)
            Samples[i] = 10000000;
    }
}

void UJoyDlg::OnApplyClicked()
{
    Starshatter* Stars = Starshatter::GetInstance();
    if (Stars)
    {
        KeyMap& KM = Stars->GetKeyMap();

        KM.Bind(KEY_AXIS_YAW, MapAxis[0] + KEY_JOY_AXIS_X, 0);
        KM.Bind(KEY_AXIS_PITCH, MapAxis[1] + KEY_JOY_AXIS_X, 0);
        KM.Bind(KEY_AXIS_ROLL, MapAxis[2] + KEY_JOY_AXIS_X, 0);
        KM.Bind(KEY_AXIS_THROTTLE, MapAxis[3] + KEY_JOY_AXIS_X, 0);

        KM.Bind(KEY_AXIS_YAW_INVERT, GetInvertButtonState(Invert0) ? 1 : 0, 0);
        KM.Bind(KEY_AXIS_PITCH_INVERT, GetInvertButtonState(Invert1) ? 1 : 0, 0);
        KM.Bind(KEY_AXIS_ROLL_INVERT, GetInvertButtonState(Invert2) ? 1 : 0, 0);
        KM.Bind(KEY_AXIS_THROTTLE_INVERT, GetInvertButtonState(Invert3) ? 1 : 0, 0);

        KM.SaveKeyMap("key.cfg", 256);
        Stars->MapKeys();
    }

    // Return to Control Options (not MenuScreen):
    if (Manager)
        Manager->Show();   // or Manager->SetVisibility(Visible) if you prefer
    SetVisibility(ESlateVisibility::Collapsed);
}

void UJoyDlg::OnCancelClicked()
{
    if (Manager)
        Manager->Show();
    SetVisibility(ESlateVisibility::Collapsed);
}

void UJoyDlg::RefreshAxisButtonsFromCurrentBindings()
{
    for (int i = 0; i < 4; ++i)
    {
        const int Map = Joystick::GetAxisMap(i) - KEY_JOY_AXIS_X;
        const int Inv = Joystick::GetAxisInv(i);

        if (Map >= 0 && Map < 8)
        {
            MapAxis[i] = Map;
            UpdateAxisButtonText(i, JoyAxisNames[Map]);
        }
        else
        {
            MapAxis[i] = -1;
            UpdateAxisButtonText(i, "JoyDlg.unmapped");
        }

        if (i == 0) SetInvertButtonState(Invert0, Inv != 0);
        if (i == 1) SetInvertButtonState(Invert1, Inv != 0);
        if (i == 2) SetInvertButtonState(Invert2, Inv != 0);
        if (i == 3) SetInvertButtonState(Invert3, Inv != 0);
    }
}

void UJoyDlg::UpdateAxisButtonText(int AxisIndex, const char* TextId)
{
    UButton* Target = nullptr;
    switch (AxisIndex)
    {
    case 0: Target = AxisButton0; break;
    case 1: Target = AxisButton1; break;
    case 2: Target = AxisButton2; break;
    case 3: Target = AxisButton3; break;
    default: break;
    }

    if (!Target || !TextId)
        return;

    // NOTE: This assumes TextBlock is child(0). If your UMG hierarchy differs,
    // bind a UTextBlock* instead (recommended).
    UTextBlock* Label = Cast<UTextBlock>(Target->GetChildAt(0));
    if (!Label)
        return;

    const Text T = Game::GetText(TextId);
    Label->SetText(FText::FromString(UTF8_TO_TCHAR(T.data())));
}

void UJoyDlg::SetInvertButtonState(UButton* Button, bool bChecked)
{
    if (Button)
        Button->SetIsEnabled(bChecked);
}

bool UJoyDlg::GetInvertButtonState(const UButton* Button) const
{
    return Button && Button->GetIsEnabled();
}
