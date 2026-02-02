/*  Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO:
    John DiCamillo, Destroyer Studios LLC

    SUBSYSTEM:    nGenEx.lib
    FILE:         Model.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    Model data: surfaces + materials + mag loading
*/

#pragma once

#include "List.h"
#include "Polygon.h" // Material, Poly, VertexSet, Plane, etc.

// Forward declarations:
class Surface;
class ModelFile;
class UTexture2D;

class Model
{
    friend class Solid;
    friend class ModelFile;

public:
    static const char* TYPENAME() { return "Model"; }

    enum { NAMELEN = 64 };
    enum { MAX_VERTS = 64000, MAX_POLYS = 16000 };

    Model();
    Model(const Model& m);
    ~Model();

    Model& operator=(const Model& m);
    int    operator==(const Model& that) const { return this == &that; }

    bool   Load(const char* mag_file, double scale = 1.0);
    bool   Load(ModelFile* loader, double scale = 1.0);

    const char* Name()         const { return name; }
    int         NumVerts()     const { return nverts; }
    int         NumSurfaces()  const { return surfaces.size(); }
    int         NumMaterials() const { return materials.size(); }
    int         NumPolys()     const { return npolys; }
    int         NumSegments()  const;
    double      Radius()       const { return radius; }

    bool  IsDynamic()  const { return dynamic; }
    void  SetDynamic(bool d) { dynamic = d; }

    bool  IsLuminous() const { return luminous; }
    void  SetLuminous(bool l) { luminous = l; }

    List<Surface>& GetSurfaces() { return surfaces; }
    List<Material>& GetMaterials() { return materials; }

    const Material* FindMaterial(const char* mtl_name) const;
    const Material* ReplaceMaterial(const Material* mtl);

    void   GetAllTextures(List<UTexture2D*>& textures);

    Poly* AddPolys(int nsurf, int npolys, int nverts);
    void   ExplodeMesh();
    void   OptimizeMesh();
    void   OptimizeMaterials();
    void   ScaleBy(double factor);

    void   Normalize();
    void   SelectPolys(List<Poly>&, Material* mtl);
    void   SelectPolys(List<Poly>&, FVector loc);

    void   AddSurface(Surface* s);
    void   ComputeTangents();

    // buffer management
    void   DeletePrivateData();

private:
    bool   LoadMag5(BYTE* block, int len, double scale);
    bool   LoadMag6(BYTE* block, int len, double scale);

private:
    char          name[NAMELEN]{};
    List<Surface> surfaces;
    List<Material> materials;

    int   nverts = 0;
    int   npolys = 0;
    float radius = 0.0f;
    float extents[6]{};

    bool  luminous = false;
    bool  dynamic = false;
};
