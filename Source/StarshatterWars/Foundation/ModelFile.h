/*  Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025-2026. All Rights Reserved.

    ORIGINAL AUTHOR AND STUDIO:
    John DiCamillo, Destroyer Studios LLC

    SUBSYSTEM:    nGenEx.lib
    FILE:         ModelFile.h
    AUTHOR:       Carlos Bott

    OVERVIEW
    ========
    ModelFile: loader/saver interface for Model (MAG or other formats)
*/

#pragma once

// Forward declaration:
class Model;

class ModelFile
{
public:
    ModelFile(const char* fname);
    virtual ~ModelFile();

    virtual bool Load(Model* m, double scale = 1.0);
    virtual bool Save(Model* m);

protected:
    char  filename[256]{};
    Model* model = nullptr;

    // internal accessors (legacy pattern):
    char* pname = nullptr;
    int* pnverts = nullptr;
    int* pnpolys = nullptr;
    float* pradius = nullptr;
};

