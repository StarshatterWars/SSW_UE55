/*  Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO:
    John DiCamillo, Destroyer Studios LLC

    SUBSYSTEM:    nGenEx.lib
    FILE:         Surface.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Surface: a subset of a Model containing polys/verts and render segments
*/

#pragma once

#include "List.h"
#include "Polygon.h" // VertexSet, Poly, Material, Plane, etc.

// Minimal Unreal includes:
#include "Math/Vector.h"
#include "Math/Matrix.h"

// Forward declarations:
class Model;
class Segment;
class VideoPrivateData;
class OPCODE_data; // keep as opaque pointer for now (to delete later)

class Surface
{
    friend class Solid;
    friend class Model;

public:
    static const char* TYPENAME() { return "Surface"; }

    enum { NAMELEN = 64 };
    enum { HIDDEN = 1, LOCKED = 2, SIMPLE = 4, MAX_VERTS = 64000, MAX_POLYS = 16000 };

    Surface();
    ~Surface();

    int operator==(const Surface& s) const { return this == &s; }

    const char* Name()        const { return name; }
    int         NumVerts()    const { return vertex_set ? vertex_set->nverts : 0; }
    int         NumSegments() const { return segments.size(); }
    int         NumPolys()    const { return npolys; }
    int         NumIndices()  const { return nindices; }

    bool IsHidden()     const { return (state & HIDDEN) != 0; }
    bool IsLocked()     const { return (state & LOCKED) != 0; }
    bool IsSimplified() const { return (state & SIMPLE) != 0; }

    Model* GetModel() const { return model; }
    List<Segment>& GetSegments() { return segments; }

    const FVector& GetOffset() const { return offset; }
    const FMatrix& GetOrientation() const { return OrientationMatrix; }

    double     Radius() const { return radius; }
    VertexSet* GetVertexSet() const { return vertex_set; }
    FVector* GetVLoc() const { return vloc; }
    Poly* GetPolys() const { return polys; }

    void SetName(const char* n);
    void SetHidden(bool b);
    void SetLocked(bool b);
    void SetSimplified(bool b);

    void CreateVerts(int nverts);
    void CreatePolys(int npolys);
    void AddIndices(int n) { nindices += n; }
    Poly* AddPolys(int npolys, int nverts);

    VideoPrivateData* GetVideoPrivateData() const { return video_data; }
    void SetVideoPrivateData(VideoPrivateData* vpd) { video_data = vpd; }

    void ScaleBy(double factor);

    void BuildHull();
    void Normalize();
    void SelectPolys(List<Poly>&, Material* mtl);
    void SelectPolys(List<Poly>&, FVector loc);

    void InitializeCollisionHull(); // will be UE-replaced later
    void ComputeTangents();
    void CalcGradients(Poly& p, FVector& tangent, FVector& binormal);

    void Copy(Surface& s, Model* m);
    void OptimizeMesh();
    void ExplodeMesh();

private:
    char     name[NAMELEN]{};

    Model* model = nullptr;

    VertexSet* vertex_set = nullptr; // for rendering
    FVector* vloc = nullptr; // for hull/shadow
    float      radius = 0.0f;

    int   nhull = 0;
    int   npolys = 0;
    int   nindices = 0;
    int   state = 0;

    Poly* polys = nullptr;

    List<Segment> segments;

    FVector offset;
    FMatrix OrientationMatrix;

public:
    OPCODE_data* opcode = nullptr; // keep as opaque pointer for now

private:
    VideoPrivateData* video_data = nullptr;
};

