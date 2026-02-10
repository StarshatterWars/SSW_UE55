/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025-2026.
    All Rights Reserved.

    SUBSYSTEM:    StarshatterWars (Unreal Engine)
    FILE:         StarshatterSystemDesignSubsystem.cpp
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Parses Content/GameData/Systems/sys.def (SYSTEM) and fills:
    - SystemsByName runtime cache
    - Optional SystemDesignDataTable via AddRow/RemoveRow
    - Optional legacy List<> catalog mirror
*/

#include "StarshatterSystemDesignSubsystem.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"

#include "Engine/DataTable.h"
#include "FormattingUtils.h"
#include "StarshatterAssetRegistrySubsystem.h"


void UStarshatterSystemDesignSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("[SYSTEM DESIGN] Initialize()"));

 /*------------------------------------------------------------------
     Resolve System Design DataTable via Asset Registry
 ------------------------------------------------------------------*/

    UStarshatterAssetRegistrySubsystem* Assets =
        GetGameInstance()->GetSubsystem<UStarshatterAssetRegistrySubsystem>();

    if (!Assets)
    {
        UE_LOG(LogTemp, Error, TEXT("[SYSTEMDESIGN] Asset Registry missing"));
        return;
    }

    SystemDesignDataTable =
        Assets->GetDataTable(TEXT("Data.SystemDesignTable"), true);

    if (!SystemDesignDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("[SYSTEMDESIGN] SystemDesignTable not found"));
        return;
    }

    if (bClearTables)
    {
        SystemDesignDataTable->EmptyTable();
    }
}

void UStarshatterSystemDesignSubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("[SYSTEM DESIGN] Deinitialize()"));
    DesignsByName.Empty();
    Super::Deinitialize();
}

void UStarshatterSystemDesignSubsystem::LoadAll(bool bLoaded)
{
    UE_LOG(LogTemp, Log, TEXT("UStarshatterShipDesignSubsystem::LoadAll()"));
    if (!bLoaded)
        return;

    LoadSystemDesigns();
    bLoaded = true;
}

void UStarshatterSystemDesignSubsystem::LoadSystemDesigns()
{
    const FString SysDefPath = FPaths::ProjectContentDir() / TEXT("GameData/Systems/sys.def");
    const FTCHARToUTF8 Utf8(*SysDefPath);
    LoadSystemDesign(Utf8.Get());
}

void UStarshatterSystemDesignSubsystem::LoadSystemDesign(const char* Filename)
{
    if (!Filename || !*Filename)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SYSTEM] LoadSystemDesign: null/empty filename"));
        return;
    }

    const FString FilePath = ANSI_TO_TCHAR(Filename);
    UE_LOG(LogTemp, Log, TEXT("[SYSTEM] Loading System Designs '%s'"), *FilePath);

    if (!FPaths::FileExists(FilePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("[SYSTEM] file not found: %s"), *FilePath);
        return;
    }

    // Optional clear:
    if (bClearTables)
    {
        DesignsByName.Empty();
        // NOTE: We can't safely clear a DataTable without editor helpers unless we know row names.
        // We'll overwrite existing rows as we parse (RemoveRow/AddRow per system).
        UE_LOG(LogTemp, Log, TEXT("[SYSTEM] bClearTables=true: cleared in-memory cache (DT rows will be overwritten as parsed)"));
    }

    // Load file into bytes for legacy parser (char* buffer expected)
    TArray<uint8> Bytes;
    if (!FFileHelper::LoadFileToArray(Bytes, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("[SYSTEM] failed to read: %s"), *FilePath);
        return;
    }
    Bytes.Add(0);

    // Stable UTF-8 filename for legacy helpers
    const FTCHARToUTF8 Utf8Path(*FilePath);
    const char* fn = Utf8Path.Get();

    Parser ParserObj(new BlockReader(reinterpret_cast<const char*>(Bytes.GetData())));
    Term* TermObj = ParserObj.ParseTerm();

    if (!TermObj)
    {
        UE_LOG(LogTemp, Error, TEXT("[SYSTEM] ERROR: could not parse '%s'"), *FilePath);
        return;
    }

    // Header check
    {
        TermText* FileType = TermObj->isText();
        if (!FileType || FileType->value() != "SYSTEM")
        {
            UE_LOG(LogTemp, Error, TEXT("[SYSTEM] ERROR: invalid system design file '%s' (expected SYSTEM)"), *FilePath);
            delete TermObj;
            return;
        }
    }

    int32 ParsedSystems = 0;

    do
    {
        delete TermObj;
        TermObj = ParserObj.ParseTerm();

        if (!TermObj)
            break;

        TermDef* Def = TermObj->isDef();
        if (!Def)
            continue;

        const Text& DefName = Def->name()->value();

        if (DefName == "system")
        {
            if (!Def->term() || !Def->term()->isStruct())
            {
                UE_LOG(LogTemp, Warning, TEXT("[SYSTEM] WARNING: system structure missing in '%s'"), *FilePath);
                continue;
            }

            TermStruct* SystemStruct = Def->term()->isStruct();

            FSystemDesign NewSystem;
            NewSystem.SourceFile = FilePath;
            NewSystem.Components.Empty();

            Text SystemNameText = "";

            // Parse system { ... }
            for (int32 i = 0; i < (int32)SystemStruct->elements()->size(); ++i)
            {
                TermDef* ParamDef = SystemStruct->elements()->at(i)->isDef();
                if (!ParamDef)
                    continue;

                const Text& Key = ParamDef->name()->value();

                if (Key == "name")
                {
                    GetDefText(SystemNameText, ParamDef, fn);
                    NewSystem.Name = FString(SystemNameText);
                }
                else if (Key == "component")
                {
                    if (!ParamDef->term() || !ParamDef->term()->isStruct())
                    {
                        UE_LOG(LogTemp, Warning,
                            TEXT("[SYSTEM] WARNING: component struct missing in system '%s' in '%s'"),
                            *NewSystem.Name, *FilePath);
                        continue;
                    }

                    TermStruct* CompStruct = ParamDef->term()->isStruct();

                    FComponentDesign NewComp;

                    Text  CompNameText = "";
                    Text  CompAbrvText = "";
                    float RepairTime = 0.0f;
                    float ReplaceTime = 0.0f;
                    int   Spares = 0;
                    int   Affects = 0;

                    // Parse component { ... }
                    for (int32 j = 0; j < (int32)CompStruct->elements()->size(); ++j)
                    {
                        TermDef* CDef = CompStruct->elements()->at(j)->isDef();
                        if (!CDef)
                            continue;

                        const Text& CKey = CDef->name()->value();

                        if (CKey == "name")
                        {
                            GetDefText(CompNameText, CDef, fn);
                            NewComp.Name = FString(CompNameText);
                        }
                        else if (CKey == "abrv")
                        {
                            GetDefText(CompAbrvText, CDef, fn);
                            NewComp.Abbrev = FString(CompAbrvText);
                        }
                        else if (CKey == "repair_time")
                        {
                            GetDefNumber(RepairTime, CDef, fn);
                            NewComp.RepairTime = RepairTime;
                        }
                        else if (CKey == "replace_time")
                        {
                            GetDefNumber(ReplaceTime, CDef, fn);
                            NewComp.ReplaceTime = ReplaceTime;
                        }
                        else if (CKey == "spares")
                        {
                            GetDefNumber(Spares, CDef, fn);
                            NewComp.Spares = (int32)Spares;
                        }
                        else if (CKey == "affects")
                        {
                            GetDefNumber(Affects, CDef, fn);
                            NewComp.Affects = (int32)Affects;
                        }
                        else
                        {
                            UE_LOG(LogTemp, Verbose,
                                TEXT("[SYSTEM] component param '%s' ignored in '%s'"),
                                ANSI_TO_TCHAR(CKey.data()), *FilePath);
                        }
                    }

                    NewSystem.Components.Add(NewComp);
                }
                else
                {
                    UE_LOG(LogTemp, Verbose,
                        TEXT("[SYSTEM] system param '%s' ignored in '%s'"),
                        ANSI_TO_TCHAR(Key.data()), *FilePath);
                }
            }

            if (NewSystem.Name.IsEmpty())
            {
                UE_LOG(LogTemp, Warning, TEXT("[SYSTEM] WARNING: system with missing name ignored in '%s'"), *FilePath);
                continue;
            }

            const FName RowName(*NewSystem.Name);

            // Cache:
            DesignsByName.Add(RowName, NewSystem);

            // Optional DT write:
            if (SystemDesignDataTable)
            {
                if (SystemDesignDataTable->FindRow<FSystemDesign>(RowName, TEXT("LoadSystemDesign"), false))
                {
                    SystemDesignDataTable->RemoveRow(RowName);
                }
                SystemDesignDataTable->AddRow(RowName, NewSystem);
            }

            ++ParsedSystems;
        }
        else
        {
            UE_LOG(LogTemp, Verbose,
                TEXT("[SYSTEM] WARNING: unknown definition '%s' in '%s'"),
                ANSI_TO_TCHAR(DefName.data()), *FilePath);
        }

    } while (TermObj);

    if (TermObj)
    {
        delete TermObj;
        TermObj = nullptr;
    }

    UE_LOG(LogTemp, Log, TEXT("[SYSTEM] Loaded %d system designs (cache=%d) from '%s'"),
        ParsedSystems, DesignsByName.Num(), *FilePath);
}
