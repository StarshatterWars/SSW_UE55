/*  Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO:
    John DiCamillo, Destroyer Studios LLC

    SUBSYSTEM:    nGenEx.lib
    FILE:         Segment.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Segment: contiguous poly range sharing a material (draw batch)
*/

#pragma once

#include "Polygon.h" // Material, Poly

// Forward declarations:
class Model;
class VideoPrivateData;

class Segment
{
public:
    static const char* TYPENAME() { return "Segment"; }

    Segment();
    Segment(int n, Poly* p, Material* mtl, Model* mod = nullptr);
    ~Segment();

    bool IsSolid()       const { return material ? material->IsSolid() : true; }
    bool IsTranslucent() const { return material ? material->IsTranslucent() : false; }
    bool IsGlowing()     const { return material ? material->IsGlowing() : false; }

    VideoPrivateData* GetVideoPrivateData() const { return video_data; }
    void SetVideoPrivateData(VideoPrivateData* vpd) { video_data = vpd; }

    int       npolys = 0;
    Poly* polys = nullptr;
    Material* material = nullptr;
    Model* model = nullptr;

    VideoPrivateData* video_data = nullptr;
};

