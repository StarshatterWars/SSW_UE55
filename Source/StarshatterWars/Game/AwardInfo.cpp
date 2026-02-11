/*  Project Starshatter Wars
    Fractal Dev Games
    Copyright (C) 2024. All Rights Reserved.

    SUBSYSTEM:    Game
    FILE:         AwardInfo.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Player Awards class
*/

/*=============================================================================
    Project:        Starshatter Wars (nGenEx Unreal Port)
    Studio:         Fractal Dev Games
    Copyright:      (C) 2024–2026. All Rights Reserved.

    ORIGINAL DESIGN:
    John DiCamillo / Destroyer Studios LLC

    SUBSYSTEM:      Player / Awards
    FILE:           AwardInfo.h
    AUTHOR:         Carlos Bott

    OVERVIEW
    ========
    AwardInfo

    Legacy-compatible descriptor for Rank and Medal definitions
    loaded from awards.def.

    Responsibilities:
    - Defines rank progression thresholds
    - Defines medal eligibility criteria
    - Holds insignia references
    - Stores award sound and ceremony metadata

    This is a pure C++ data container (not a UObject).
    It intentionally mirrors the original Starshatter structure
    to allow safe incremental migration to UE-native DataTables
    in a later phase.

    NOTE:
    - This class owns no dynamic memory.
    - Bitmap* pointers are managed externally by DataLoader.
=============================================================================*/


#include "AwardInfo.h"
