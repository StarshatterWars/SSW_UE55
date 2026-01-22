/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright (C) 2025-2026. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         Archiver.cpp
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO
    ==========================
    John DiCamillo / Destroyer Studios LLC
*/


#include "Archiver.h"
#include "Types.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>
#include <time.h>

#include "zlib.h"

// Unreal logging:
#include "Logging/LogMacros.h"
#include "GameStructs.h"

// +--------------------------------------------------------------------+

static int verbose = 1;
static int err = 0;

#define CHECK_ERR(errval, msg)                                     \
    {                                                              \
        if ((errval) != Z_OK) {                                    \
            UE_LOG(LogStarshatterWars, Error, TEXT("%s error: %d"), \
                   TEXT(msg), (int)(errval));                      \
            /* preserve legacy behavior */                         \
            exit(1);                                               \
        }                                                          \
    }

// +--------------------------------------------------------------------+

Archiver::Archiver(const char* name)
{
    ZeroMemory(this, sizeof(Archiver));

    if (name)
        LoadDatafile(name);
}

Archiver::~Archiver()
{
    delete[] block_map;
    delete[] directory;
}

// +--------------------------------------------------------------------+

void
Archiver::WriteEntry(int index, BYTE* buf)
{
    const int FileHandle = _open(datafile, _O_RDWR | _O_CREAT | _O_BINARY, _S_IREAD | _S_IWRITE);

    if (FileHandle != -1) {
        header.dir_size_comp = DirBlocks() * BLOCK_SIZE;
        dirbuf = new BYTE[header.dir_size_comp];

        if (!dirbuf) {
            err = Z_MEM_ERROR;
        }
        else {
            err = compress(dirbuf, &header.dir_size_comp,
                (BYTE*)directory, header.nfiles * sizeof(DataEntry));
            CHECK_ERR(err, "compress");

            header.dir_blocks = Blocks(header.dir_size_comp) * BLOCK_SIZE;

            _lseek(FileHandle, 0, SEEK_SET);
            _write(FileHandle, &header, sizeof(DataHeader));
            _lseek(FileHandle, sizeof(DataHeader) + header.dir_offset, SEEK_SET);
            _write(FileHandle, dirbuf, header.dir_blocks);

            delete[] dirbuf;
            dirbuf = nullptr;
        }

        if (buf && directory && directory[index].size_comp) {
            _lseek(FileHandle, sizeof(DataHeader) + directory[index].offset, SEEK_SET);
            _write(FileHandle, buf, directory[index].size_comp);
        }

        _close(FileHandle);
    }
    else {
        UE_LOG(LogStarshatterWars, Error, TEXT("Archiver::WriteEntry failed for '%hs'"), datafile);
    }
}

// +--------------------------------------------------------------------+

DWORD
Archiver::Blocks(DWORD raw_size)
{
    const int full_blocks = raw_size / BLOCK_SIZE;
    const int part_blocks = (raw_size % BLOCK_SIZE) > 0;

    return (DWORD)(full_blocks + part_blocks);
}

DWORD
Archiver::DirBlocks()
{
    DWORD result = Blocks(header.nfiles * sizeof(DataEntry));
    if (result == 0) result = 1;
    return result;
}

DWORD
Archiver::FileBlocks(int index)
{
    if (index >= 0 && index < (int)header.nfiles && directory)
        return Blocks(directory[index].size_comp);

    return 0;
}

// +--------------------------------------------------------------------+

void
Archiver::CreateBlockMap()
{
    delete[] block_map;
    block_map = nullptr;

    if (header.nfiles == 0)
        return;

    DWORD i, j;
    const DWORD dir_usage = header.dir_offset + DirBlocks() * BLOCK_SIZE;
    DWORD max_usage = dir_usage;

    for (i = 0; i < header.nfiles; i++) {
        const DWORD last_block = directory[i].offset + FileBlocks((int)i) * BLOCK_SIZE;
        if (last_block > max_usage)
            max_usage = last_block;
    }

    nblocks = max_usage / BLOCK_SIZE;
    block_map = new DWORD[nblocks];

    if (!block_map) {
        nblocks = 0;
    }
    else {
        ZeroMemory(block_map, nblocks * sizeof(DWORD));

        DWORD first_block = header.dir_offset / BLOCK_SIZE +
            (header.dir_offset % BLOCK_SIZE > 0);

        for (j = 0; j < DirBlocks(); j++)
            block_map[first_block + j] = 1;

        for (i = 0; i < header.nfiles; i++) {
            DWORD file_first_block = directory[i].offset / BLOCK_SIZE +
                (directory[i].offset % BLOCK_SIZE > 0);

            for (j = 0; j < FileBlocks((int)i); j++)
                block_map[file_first_block + j] = i + 2;
        }
    }
}

// +--------------------------------------------------------------------+

int
Archiver::FindDataBlocks(int blocks_needed)
{
    if ((int)(nblocks)-blocks_needed > 0) {
        DWORD start;
        int i;

        for (start = 0; start < nblocks - (DWORD)blocks_needed; start++) {
            for (i = 0; block_map[start + i] == 0 && i < blocks_needed; i++)
                ;

            if (i == blocks_needed)
                return (int)(start * BLOCK_SIZE);

            start += (DWORD)i;
        }
    }

    return (int)(nblocks * BLOCK_SIZE);
}

// +--------------------------------------------------------------------+

void
Archiver::LoadDatafile(const char* name)
{
    if (!name) return;

    delete[] directory;
    delete[] block_map;

    ZeroMemory(this, sizeof(Archiver));
    strncpy_s(datafile, name, NAMELEN - 1);

    FILE* f = nullptr;
    fopen_s(&f, datafile, "rb");
    if (f) {
        fread(&header, sizeof(DataHeader), 1, f);

        if (header.version != VERSION) {
            UE_LOG(LogStarshatterWars, Error,
                TEXT("ERROR: datafile '%hs' invalid version '%u'"),
                datafile, (unsigned)header.version);
            fclose(f);
            ZeroMemory(&header, sizeof(header));
            return;
        }

        DWORD len = DirBlocks() * BLOCK_SIZE;
        DWORD dirsize = header.nfiles + 64;

        dirbuf = new BYTE[len];
        directory = new DataEntry[dirsize];

        if (!dirbuf || !directory) {
            err = Z_MEM_ERROR;
        }
        else {
            ZeroMemory(directory, sizeof(DataEntry) * dirsize);

            fseek(f, sizeof(DataHeader) + header.dir_offset, SEEK_SET);
            fread(dirbuf, header.dir_size_comp, 1, f);

            int UncompressErr = uncompress((BYTE*)directory, &len,
#pragma warning(suppress : 6029)
                dirbuf, header.dir_size_comp);

            if (UncompressErr != Z_OK)
                ZeroMemory(directory, sizeof(DataEntry) * dirsize);

            delete[] dirbuf;
            dirbuf = nullptr;

            CreateBlockMap();
        }

        fclose(f);
    }
    else {
        UE_LOG(LogStarshatterWars, Log, TEXT("Creating Archive '%hs'..."), datafile);

        header.version = VERSION;
        header.nfiles = 0;
        header.dir_blocks = 0;
        header.dir_size_comp = 0;
        header.dir_offset = 0;

        nblocks = DirBlocks();

        delete[] block_map;
        block_map = nullptr;
    }
}

// +--------------------------------------------------------------------+

int
Archiver::FindEntry(const char* req_name)
{
    int entry = -1;

    if (req_name && *req_name && directory) {
        char path[256];
        const int len = (int)strlen(req_name);

        ZeroMemory(path, sizeof(path));

        for (int c = 0; c < len; c++) {
            if (req_name[c] == '\\')
                path[c] = '/';
            else
                path[c] = req_name[c];
        }

        for (DWORD i = 0; i < header.nfiles; i++) {
            if (!_stricmp(directory[i].name, path))
                return (int)i;
        }
    }

    return entry;
}

// +--------------------------------------------------------------------+

BYTE*
Archiver::CompressEntry(int i)
{
    if (directory && i >= 0 && i < (int)header.nfiles) {
        char* name = directory[i].name;

        FILE* f = nullptr;
        fopen_s(&f, name, "rb");

        if (f) {
            fseek(f, 0, SEEK_END);
            DWORD len = (DWORD)ftell(f);
            fseek(f, 0, SEEK_SET);

            BYTE* buf = new BYTE[len];

            if (!buf) {
                err = Z_MEM_ERROR;
            }
            else {
                fread(buf, len, 1, f);
                fclose(f);

                directory[i].size_orig = len;

                directory[i].size_comp = (DWORD)(len * 1.1);
                BYTE* cbuf = new BYTE[directory[i].size_comp];

                if (!cbuf) {
                    err = Z_MEM_ERROR;
                }
                else {
                    err = compress(cbuf, &directory[i].size_comp, buf, len);
                    CHECK_ERR(err, "compress");
                }

                delete[] buf;
                return cbuf;
            }
        }
    }

    return nullptr;
}

// +--------------------------------------------------------------------+

int
Archiver::ExpandEntry(int i, BYTE*& buf, bool null_terminate)
{
    DWORD len = 0;

    if (directory && i >= 0 && i < (int)header.nfiles) {
        FILE* f = nullptr;
        fopen_s(&f, datafile, "rb");

        if (f) {
            const DWORD clen = directory[i].size_comp;
            BYTE* cbuf = new BYTE[clen];

            if (!cbuf) {
                err = Z_MEM_ERROR;
            }
            else {
                fseek(f, sizeof(DataHeader) + directory[i].offset, SEEK_SET);
                fread(cbuf, clen, 1, f);

                len = directory[i].size_orig;

                if (null_terminate) {
                    buf = new BYTE[len + 1];
                    if (buf) buf[len] = 0;
                }
                else {
                    buf = new BYTE[len];
                }

                if (!buf) {
                    err = Z_MEM_ERROR;
                }
                else {
                    err = uncompress(buf, &len, cbuf, clen);
                    if (err != Z_OK) {
                        delete[] buf;
                        buf = nullptr;
                    }
                }

                delete[] cbuf;
                fclose(f);
            }
        }
    }

    return (int)len;
}

// +--------------------------------------------------------------------+

int
Archiver::InsertEntry(const char* name)
{
    if (name && *name) {
        char  path[256];
        DWORD len = (DWORD)strlen(name);

        ZeroMemory(path, sizeof(path));

        for (DWORD c = 0; c < len; c++) {
            if (name[c] == '\\')
                path[c] = '/';
            else
                path[c] = name[c];
        }

        const int dirsize = (int)header.nfiles + 64;

        if (directory && dirsize) {
            for (int i = 0; i < dirsize; i++) {
                if (directory[i].size_orig == 0) {
                    ZeroMemory(directory[i].name, NAMELEN);
                    strncpy_s(directory[i].name, path, NAMELEN);
                    directory[i].name[NAMELEN - 1] = '\0';
                    directory[i].size_orig = 1;
                    return i;
                }
            }
        }

        DataEntry* dir = new DataEntry[dirsize + 64];

        if (directory && dirsize) {
            ZeroMemory(dir, (dirsize + 64) * sizeof(DataEntry));
            CopyMemory(dir, directory, dirsize * sizeof(DataEntry));
        }

        delete[] directory;

        header.nfiles = (DWORD)(dirsize + 64);
        directory = dir;

        ZeroMemory(directory[dirsize].name, NAMELEN);
        strncpy_s(directory[dirsize].name, path, NAMELEN);
        directory[dirsize].name[NAMELEN - 1] = '\0';
        directory[dirsize].size_orig = 1;

        return dirsize;
    }

    return -1;
}

// +--------------------------------------------------------------------+

void
Archiver::RemoveEntry(int index)
{
    if (directory && index >= 0 && index < (int)header.nfiles)
        ZeroMemory(&directory[index], sizeof(DataEntry));
}

// +--------------------------------------------------------------------+

void
Archiver::Insert(const char* name)
{
    DWORD old_blocks = 0, old_offset = 0, new_blocks = 0;
    DWORD old_dir_blocks = 0, old_dir_offset = 0, new_dir_blocks = 0;
    int   added = 0;

    int index = FindEntry(name);

    if (index < 0) {
        old_dir_blocks = DirBlocks();
        old_dir_offset = header.dir_offset;

        index = InsertEntry(name);

        if (index >= (int)header.nfiles) {
            header.nfiles = (DWORD)(index + 1);
            added = 1;
        }

        new_dir_blocks = DirBlocks();

        if (new_dir_blocks > old_dir_blocks) {
            header.dir_offset = (DWORD)FindDataBlocks((int)new_dir_blocks);
            CreateBlockMap();
        }
    }
    else {
        old_blocks = FileBlocks(index);
        old_offset = directory[index].offset;
    }

    if (index >= 0) {
        DataEntry& e = directory[index];

        if (verbose)
            UE_LOG(LogStarshatterWars, Log, TEXT("   Inserting: %-16hs "), e.name);

        BYTE* buf = CompressEntry(index);

        if (!buf) {
            UE_LOG(LogStarshatterWars, Error, TEXT("ERROR: Could not compress %d:%hs"), index, directory[index].name);
            exit(1);
        }

        new_blocks = FileBlocks(index);

        // the file is new, or got bigger, need to find room for the data:
        if (new_blocks > old_blocks) {
            directory[index].offset = (DWORD)FindDataBlocks((int)new_blocks);
            CreateBlockMap();
        }

        WriteEntry(index, buf);
        delete[] buf;

        if (verbose) {
            const int ratio = (int)(100.0 * (double)e.size_comp / (double)e.size_orig);
            UE_LOG(LogStarshatterWars, Log, TEXT("%9u => %9u (%2d%%)"),
                (unsigned)e.size_orig, (unsigned)e.size_comp, ratio);
        }
    }
    else if (added) {
        header.nfiles--;
    }
}

// +--------------------------------------------------------------------+

void
Archiver::Extract(const char* name)
{
    int index = FindEntry(name);

    if (!directory || index < 0 || index >= (int)header.nfiles) {
        UE_LOG(LogStarshatterWars, Warning, TEXT("Could not extract '%hs', not found"), name);
        return;
    }

    BYTE* buf = nullptr;
    ExpandEntry(index, buf);

    FILE* f = nullptr;
    fopen_s(&f, directory[index].name, "wb");
    if (f) {
        fwrite(buf, directory[index].size_orig, 1, f);
        fclose(f);
    }
    else {
        UE_LOG(LogStarshatterWars, Warning, TEXT("Could not extract '%hs', could not open file for writing"), name);
    }

    delete[] buf;

    if (verbose)
        UE_LOG(LogStarshatterWars, Log, TEXT("   Extracted: %hs"), name);
}

// +--------------------------------------------------------------------+

void
Archiver::Remove(const char* name)
{
    int index = FindEntry(name);

    if (!directory || index < 0 || index >= (int)header.nfiles) {
        UE_LOG(LogStarshatterWars, Warning, TEXT("Could not remove '%hs', not found"), name);
        return;
    }

    RemoveEntry(index);
    WriteEntry(index, nullptr);

    if (verbose)
        UE_LOG(LogStarshatterWars, Log, TEXT("   Removed: %hs"), name);
}

// +--------------------------------------------------------------------+

void
Archiver::List()
{
    int total_orig = 0;
    int total_comp = 0;

    printf("DATAFILE: %s\n", datafile);
    printf("Files:    %u\n", (unsigned)header.nfiles);
    printf("\n");

    if (directory && header.nfiles) {
        printf("Index  Orig Size  Comp Size  Ratio  Name\n");
        printf("-----  ---------  ---------  -----  ----------------\n");

        for (DWORD i = 0; i < header.nfiles; i++) {
            DataEntry& e = directory[i];
            const int ratio = (int)(100.0 * (double)e.size_comp / (double)e.size_orig);

            printf("%5u  %9u  %9u   %2d%%   %s\n",
                (unsigned)(i + 1),
                (unsigned)e.size_orig,
                (unsigned)e.size_comp,
                ratio,
                e.name);

            total_orig += (int)e.size_orig;
            total_comp += (int)e.size_comp;
        }

        const int total_ratio = (int)(100.0 * (double)total_comp / (double)total_orig);

        printf("-----  ---------  ---------  -----\n");
        printf("TOTAL  %9d  %9d   %2d%%\n\n", total_orig, total_comp, total_ratio);
    }
}
