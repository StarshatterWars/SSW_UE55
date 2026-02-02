#include "ExitDlg.h"

#include "MenuScreen.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Kismet/KismetSystemLibrary.h"

// ------------------------------------------------------------
// Construction
// ------------------------------------------------------------

UExitDlg::UExitDlg(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Constructor = defaults only
    bIsFocusable = true;
}

// ------------------------------------------------------------
// Setup / lifecycle
// ------------------------------------------------------------

void UExitDlg::SetManager(UMenuScreen* InManager)
{
    Manager = InManager;
}

void UExitDlg::NativeConstruct()
{
    Super::NativeConstruct();

    RegisterControls();

    bExitLatch = true;
    ScrollOffset = 0.0f;

    FString Credits;
    if (LoadCreditsFile(Credits))
    {
        ApplyCredits(Credits);
    }
}

void UExitDlg::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    ExecFrame(InDeltaTime);
}

// ------------------------------------------------------------
// Legacy form integration
// ------------------------------------------------------------

void UExitDlg::BindFormWidgets()
{
    if (ApplyButton)  BindButton(1, ApplyButton);
    if (CancelButton) BindButton(2, CancelButton);

    if (CreditsText)  BindText(201, CreditsText);
}

FString UExitDlg::GetLegacyFormText() const
{
    return FString(TEXT(R"FRM(
form: {
   rect:       (0,0,440,320)
   back_color: (0,0,0)
   fore_color: (255,255,255)
   font:       Limerick12

   texture:    "Message.pcx"
   margins:    (50,40,48,40)

   layout: {
      x_mins:     (20, 40, 100, 100, 20)
      x_weights:  ( 0,  0,   1,   1,  0)

      y_mins:     (44,  30, 30, 80, 35)
      y_weights:  ( 0,   0,  0,  1,  0)
   }

   ctrl: { id:100 type:label text:"Exit Starshatter?" align:center font:Limerick18 }
   ctrl: { id:101 type:label text:"Are you sure you want to exit Starshatter and return to Windows?" }
   ctrl: { id:201 type:text }

   ctrl: { id:1 type:button text:"Exit" }
   ctrl: { id:2 type:button text:"Cancel" }
}
)FRM"));
}

// ------------------------------------------------------------
// Control binding
// ------------------------------------------------------------

void UExitDlg::RegisterControls()
{
    if (ApplyButton)
    {
        ApplyButton->OnClicked.RemoveAll(this);
        ApplyButton->OnClicked.AddDynamic(this, &UExitDlg::HandleApplyClicked);
    }

    if (CancelButton)
    {
        CancelButton->OnClicked.RemoveAll(this);
        CancelButton->OnClicked.AddDynamic(this, &UExitDlg::HandleCancelClicked);
    }
}

// ------------------------------------------------------------
// Dialog control
// ------------------------------------------------------------

void UExitDlg::Show()
{
    bExitLatch = true;
    ScrollOffset = 0.0f;

    SetVisibility(ESlateVisibility::Visible);

    // Unreal modal behavior
    SetDialogInputEnabled(true);

    if (CreditsScroll)
    {
        CreditsScroll->SetScrollOffset(0.0f);
    }
}

void UExitDlg::ExecFrame(float DeltaTime)
{
    if (!CreditsScroll)
        return;

    ScrollOffset += ScrollPixelsPerSecond * DeltaTime;
    CreditsScroll->SetScrollOffset(ScrollOffset);

#if ENGINE_MAJOR_VERSION >= 5
    const float End = CreditsScroll->GetScrollOffsetOfEnd();
    if (End > 0.0f && ScrollOffset >= End)
    {
        ScrollOffset = 0.0f;
        CreditsScroll->SetScrollOffset(0.0f);
    }
#endif
}

// ------------------------------------------------------------
// Input handling
// ------------------------------------------------------------

void UExitDlg::HandleAccept()
{
    Super::HandleAccept();
    OnApply();
}

void UExitDlg::HandleCancel()
{
    if (bExitLatch)
    {
        bExitLatch = false;
        return;
    }

    Super::HandleCancel();
    OnCancel();
}

void UExitDlg::HandleApplyClicked()
{
    OnApply();
}

void UExitDlg::HandleCancelClicked()
{
    OnCancel();
}

// ------------------------------------------------------------
// Actions
// ------------------------------------------------------------

void UExitDlg::OnApply()
{
    SetDialogInputEnabled(false);
    UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, true);
}

void UExitDlg::OnCancel()
{
    SetDialogInputEnabled(false);

    if (Manager)
    {
        Manager->ShowMenuDlg();
    }

    SetVisibility(ESlateVisibility::Hidden);
}

// ------------------------------------------------------------
// Credits helpers
// ------------------------------------------------------------

bool UExitDlg::LoadCreditsFile(FString& OutText) const
{
    const TArray<FString> Paths =
    {
        FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Data/credits.txt")),
        FPaths::Combine(FPaths::ProjectDir(),        TEXT("Data/credits.txt"))
    };

    for (const FString& Path : Paths)
    {
        if (FPaths::FileExists(Path) &&
            FFileHelper::LoadFileToString(OutText, *Path))
        {
            return true;
        }
    }

    return false;
}

void UExitDlg::ApplyCredits(const FString& Text)
{
    if (URichTextBlock* T = GetText(201))
    {
        T->SetText(FText::FromString(Text));
    }
}
