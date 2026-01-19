/*  Project Starshatter Wars
    Fractal Dev Studios
    Copyright 2025-2026. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         SystemFont.cpp
    AUTHOR:       Carlos Bott

    ORIGINAL AUTHOR AND STUDIO:
    John DiCamillo, Destroyer Studios LLC


    OVERVIEW
    ========
    Font Resource class implementation
*/

#include "SystemFont.h"
#include "Bitmap.h"
#include "Polygon.h"
#include "DataLoader.h"
#include "ParseUtil.h"
#include "Video.h"

// Unreal (minimal) includes needed for UTexture2D usage:
#include "Engine/Texture2D.h"
#include "HAL/UnrealMemory.h"

DWORD GetRealTime();

// +--------------------------------------------------------------------+

SystemFont::SystemFont()
    : flags(0),
    height(0),
    baseline(0),
    interspace(0),
    spacewidth(0),
    expansion(0),
    alpha(1),
    blend(Video::BLEND_ALPHA),
    scale(1),
    caret_index(-1),
    caret_x(0),
    caret_y(0),
    imagewidth(0),
    image(nullptr),
    texture(nullptr),
    tgt_texture(nullptr),
    material(nullptr),
    vset(nullptr),
    polys(nullptr),
    npolys(0)
{
    FMemory::Memzero(name, sizeof(name));
    FMemory::Memzero(glyph, sizeof(glyph));
    FMemory::Memzero(kern, sizeof(kern));
}

SystemFont::SystemFont(const char* n)
    : flags(0),
    height(0),
    baseline(0),
    interspace(0),
    spacewidth(4),
    expansion(0),
    alpha(1),
    blend(Video::BLEND_ALPHA),
    scale(1),
    caret_index(-1),
    caret_x(0),
    caret_y(0),
    imagewidth(0),
    image(nullptr),
    texture(nullptr),
    tgt_texture(nullptr),
    material(nullptr),
    vset(nullptr),
    polys(nullptr),
    npolys(0)
{

    // ZeroMemory replacements
    FMemory::Memzero(glyph, sizeof(glyph));
    FMemory::Memzero(kern, sizeof(kern));

    // CopyMemory replacement
    FMemory::Memcpy(name, n, sizeof(name));

    if (!Load(name)) {
        flags = 0;
        height = 0;
        baseline = 0;
        interspace = 0;
        spacewidth = 0;
        imagewidth = 0;
        image = nullptr;

        texture = nullptr;
        
        FMemory::Memzero(glyph, sizeof(glyph));
        FMemory::Memzero(kern, sizeof(kern));
    }
}

// +--------------------------------------------------------------------+

SystemFont::~SystemFont()
{
    if (image)    delete[] image;
    if (vset)     delete vset;
    if (polys)    delete[] polys;
    if (material) delete material;

    image = nullptr;
    vset = nullptr;
    polys = nullptr;
    material = nullptr;
    texture = nullptr;
    tgt_texture = nullptr;
}

// +--------------------------------------------------------------------+

static char kern_tweak[256][256]; 

bool
SystemFont::Load(const char* loadname)
{
    if (!loadname || !loadname[0])
        return false;

    char imgname[256];
    char defname[256];
    sprintf_s(defname, "%s.def", loadname);
    sprintf_s(imgname, "%s.pcx", loadname);

    DataLoader* loader = DataLoader::GetLoader();
    if (!loader)
        return false;

    LoadDef(defname, imgname);

    for (int i = 0; i < 256; i++) {
        glyph[i].offset = GlyphOffset((BYTE)i);
        glyph[i].width = 0;
    }

    // NOTE: This keeps the original Starshatter DataLoader/Bitmap path for now,
    // but stores the render asset handle as UTexture2D* (not Bitmap) per UE port rules.
    // The CPU alpha mask is still extracted into `image[]` for width/kerning logic.
    Bitmap bmp;
    if (loader->LoadBitmap(imgname, bmp)) {
        if (!bmp.Pixels() && !bmp.HiPixels())
            return false;

        scale = bmp.Width() / 256;
        imagewidth = bmp.Width();

        if (height > bmp.Height())
            height = (BYTE)bmp.Height();

        const int imgsize = bmp.Width() * bmp.Height();

        if (image) {
            delete[] image;
            image = nullptr;
        }

        image = new BYTE[imgsize];

        if (image) {
            if (bmp.Pixels()) {
                FMemory::Memcpy(image, bmp.Pixels(), imgsize);
            }
            else {
                for (int i = 0; i < imgsize; i++)
                    image[i] = (BYTE)bmp.HiPixels()[i].Alpha();
            }
        }

        if (material)
            delete material;

        material = new Material;

        // UE PORT: do not keep a Bitmap* render dependency here.
        // If Material still expects `tex_diffuse` as a Bitmap*, remove/port that field
        // and bind `texture` via your UE render backend instead.
        //
        // material->tex_diffuse = &bmp; // ORIGINAL (removed)

        // If your loader can provide/resolve a UTexture2D*, assign it here.
        // texture = loader->LoadTexture2D(imgname); // example (implement in DataLoader)

        texture = nullptr;

        // Optional: warn once if texture binding is not implemented yet.
        UE_LOG(LogTemp, Warning, TEXT("SystemFont::Load('%hs'): Bitmap loaded; UE texture binding not implemented (texture=null)."), loadname);
    }
    else {
        return false;
    }

    for (int i = 0; i < 256; i++) {
        glyph[i].width = CalcWidth((BYTE)i);
    }

    color = Color::White;

    if (!(flags & (FONT_FIXED_PITCH | FONT_NO_KERN)))
        AutoKern();

    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            if (kern_tweak[i][j] < 100) {
                kern[i][j] = kern_tweak[i][j];
            }
        }
    }

    return true;
}

void
SystemFont::LoadDef(char* defname, char* imgname)
{
    for (int i = 0; i < 256; i++)
        for (int j = 0; j < 256; j++)
            kern_tweak[i][j] = 111;

    DataLoader* loader = DataLoader::GetLoader();
    if (!loader)
        return;

    BYTE* block = nullptr;
    int blocklen = loader->LoadBuffer(defname, block, true);

    if (!block || blocklen < 4)
        return;

    Parser parser(new BlockReader((const char*)block, blocklen));
    Term* term = parser.ParseTerm();

    if (!term) {
        UE_LOG(LogTemp, Warning, TEXT("WARNING: could not parse '%hs'"), defname);
        return;
    }
    else {
        TermText* file_type = term->isText();
        if (!file_type || file_type->value() != "FONT") {
            UE_LOG(LogTemp, Warning, TEXT("WARNING: invalid font def file '%hs'"), defname);
            return;
        }
    }

    do {
        delete term;

        term = parser.ParseTerm();

        if (term) {
            TermDef* def = term->isDef();
            if (def) {
                if (def->name()->value().indexOf("image") == 0) {
                    GetDefText(imgname, def, defname);
                }

                else if (def->name()->value() == "height") {
                    int h = 0;
                    GetDefNumber(h, def, defname);

                    if (h >= 0 && h <= 32)
                        height = (BYTE)h;
                }

                else if (def->name()->value() == "baseline") {
                    int b = 0;
                    GetDefNumber(b, def, defname);

                    if (b >= 0 && b <= 32)
                        baseline = (BYTE)b;
                }

                else if (def->name()->value() == "flags") {
                    if (def->term()->isText()) {
                        Text buf;
                        GetDefText(buf, def, defname);
                        buf.setSensitive(false);

                        flags = 0;

                        if (buf.contains("caps"))
                            flags = flags | FONT_ALL_CAPS;

                        if (buf.contains("kern"))
                            flags = flags | FONT_NO_KERN;

                        if (buf.contains("fixed"))
                            flags = flags | FONT_FIXED_PITCH;
                    }

                    else {
                        int f = 0;
                        GetDefNumber(f, def, defname);
                        flags = (WORD)f;
                    }
                }

                else if (def->name()->value() == "interspace") {
                    int n = 0;
                    GetDefNumber(n, def, defname);

                    if (n >= 0 && n <= 100)
                        interspace = (BYTE)n;
                }

                else if (def->name()->value() == "spacewidth") {
                    int n = 0;
                    GetDefNumber(n, def, defname);

                    if (n >= 0 && n <= 100)
                        spacewidth = (BYTE)n;
                }

                else if (def->name()->value() == "expansion") {
                    GetDefNumber(expansion, def, defname);
                }

                else if (def->name()->value() == "kern") {
                    TermStruct* val = def->term()->isStruct();

                    char a[8], b[8];
                    int  k = 111;

                    a[0] = 0;
                    b[0] = 0;

                    for (int i = 0; i < val->elements()->size(); i++) {
                        TermDef* pdef = val->elements()->at(i)->isDef();
                        if (pdef) {
                            if (pdef->name()->value() == "left" || pdef->name()->value() == "a")
                                GetDefText(a, pdef, defname);

                            else if (pdef->name()->value() == "right" || pdef->name()->value() == "b")
                                GetDefText(b, pdef, defname);

                            else if (pdef->name()->value() == "kern" || pdef->name()->value() == "k")
                                GetDefNumber(k, pdef, defname);
                        }
                    }

                    if (k < 100)
                        kern_tweak[(BYTE)a[0]][(BYTE)b[0]] = (char)k;
                }

                else {
                    UE_LOG(LogTemp, Warning, TEXT("WARNING: unknown object '%hs' in '%hs'"),
                        def->name()->value().data(), defname);
                }
            }
            else {
                UE_LOG(LogTemp, Warning, TEXT("WARNING: term ignored in '%hs'"), defname);
                term->print();
            }
        }
    } while (term);

    loader->ReleaseBuffer(block);
}

// +--------------------------------------------------------------------+

static const int pipe_width = 16;
static const int char_width = 16;
static const int char_height = 16;
static const int row_items = 16;
static const int row_width = row_items * char_width;
static const int row_size = char_height * row_width;

int
SystemFont::GlyphOffset(BYTE c) const
{
    if (flags & FONT_ALL_CAPS)
        if (islower(c))
            c = (BYTE)toupper(c);

    return (c / row_items * row_size * scale * scale +
        c % row_items * char_width * scale);
}

int
SystemFont::GlyphLocationX(BYTE c) const
{
    if (flags & FONT_ALL_CAPS)
        if (islower(c))
            c = (BYTE)toupper(c);

    return c % row_items * char_width;
}

int
SystemFont::GlyphLocationY(BYTE c) const
{
    if (flags & FONT_ALL_CAPS)
        if (islower(c))
            c = (BYTE)toupper(c);

    return c / row_items * char_height;
}

// +--------------------------------------------------------------------+

int
SystemFont::CalcWidth(BYTE c) const
{
    if (c >= PIPE_NBSP && c <= ARROW_RIGHT)
        return pipe_width;

    if (c >= 128 || !image)
        return 0;

    // all digits should be same size:
    if (isdigit(c))
        c = '0';

    int result = 0;
    int w = 16 * scale;
    int h = 16 * scale;

    BYTE* src = image + GlyphOffset(c);

    for (int y = 0; y < h; y++) {
        BYTE* pleft = src;

        for (int x = 0; x < w; x++) {
            if (*pleft++ > 0 && x > result)
                result = x;
        }

        src += imagewidth;
    }

    return result + 2;
}

// +--------------------------------------------------------------------+

struct FontKernData
{
    double l[32];
    double r[32];
};

void
SystemFont::FindEdges(BYTE c, double* l, double* r)
{
    if (!image)
        return;

    int w = glyph[c].width;
    int h = height;

    if (h > 32)
        h = 32;

    BYTE* src = image + GlyphOffset(c);

    for (int y = 0; y < h; y++) {
        BYTE* pleft = src;
        BYTE* pright = src + w - 1;

        *l = -1;
        *r = -1;

        for (int x = 0; x < w; x++) {
            if (*l == -1 && *pleft != 0)
                *l = x + 1 - (double)(*pleft) / 255.0;
            if (*r == -1 && *pright != 0)
                *r = x + 1 - (double)(*pright) / 255.0;

            pleft++;
            pright--;
        }

        src += imagewidth;
        l++;
        r++;
    }
}

static bool nokern(char c)
{
    if ((unsigned char)c <= SystemFont::ARROW_RIGHT)
        return true;

    const char* nokernchars = "0123456789+=<>-.,:;?'\"";

    if (strchr(nokernchars, c))
        return true;

    return false;
}

void
SystemFont::AutoKern()
{
    FontKernData* data = new FontKernData[256];

    if (!data)
        return;

    int h = height;
    if (h > 32)
        h = 32;

    // first, compute row edges for each glyph:
    for (int i = 0; i < 256; i++) {
        ZeroMemory(&data[i], sizeof(FontKernData));

        char c = (char)i;

        if ((flags & FONT_ALL_CAPS) && islower((unsigned char)c))
            c = (char)toupper((unsigned char)c);

        if (glyph[(BYTE)c].width > 0) {
            FindEdges((BYTE)c, data[i].l, data[i].r);
        }
    }

    // then, compute the appropriate kern for each pair.
    // use a desired average distance of one pixel,
    // with a desired minimum distance of more than half a pixel:
    double desired_avg = 2.5 + expansion;
    double desired_min = 1;

    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            // no kerning between digits or dashes:
            if (nokern((char)i) || nokern((char)j)) {
                kern[i][j] = (char)0;
            }
            else {
                double delta = 0;
                double avg = 0;
                double minv = 2500;
                int    n = 0;

                for (int y = 0; y < h; y++) {
                    if (data[i].r[y] >= 0 && data[j].l[y] >= 0) {
                        delta = data[i].r[y] + data[j].l[y];
                        avg += delta;
                        if (delta < minv)
                            minv = delta;
                        n++;
                    }
                }

                if (n > 0) {
                    avg /= n;

                    delta = desired_avg - avg;

                    if (delta < desired_min - minv) {
                        delta = ceil(desired_min - minv);

                        if (i == 'T' && islower((unsigned char)j) && !(flags & FONT_ALL_CAPS))
                            delta += 1;
                    }
                }
                else {
                    delta = 0;
                }

                kern[i][j] = (char)delta;
            }
        }
    }

    delete[] data;
}

// +--------------------------------------------------------------------+

int
SystemFont::CharWidth(char c) const
{
    if (flags & FONT_ALL_CAPS)
        if (islower((unsigned char)c))
            c = (char)toupper((unsigned char)c);

    int result = 0;

    if ((unsigned char)c >= PIPE_NBSP && (unsigned char)c <= ARROW_RIGHT)
        result = pipe_width;

    else if ((signed char)c < 0 || isspace((unsigned char)c))
        result = spacewidth;

    else
        result = glyph[(BYTE)c].width + interspace;

    return result;
}

int
SystemFont::SpaceWidth() const
{
    return spacewidth;
}

int
SystemFont::KernWidth(char a, char b) const
{
    if (flags & FONT_ALL_CAPS) {
        if (islower((unsigned char)a)) a = (char)toupper((unsigned char)a);
        if (islower((unsigned char)b)) b = (char)toupper((unsigned char)b);
    }

    return kern[(BYTE)a][(BYTE)b];
}

void
SystemFont::SetKern(char a, char b, int k)
{
    if (k < -100 || k > 100)
        return;

    if (flags & FONT_ALL_CAPS) {
        if (islower((unsigned char)a)) a = (char)toupper((unsigned char)a);
        if (islower((unsigned char)b)) b = (char)toupper((unsigned char)b);
    }

    kern[(BYTE)a][(BYTE)b] = (char)k;
}

// +--------------------------------------------------------------------+

int
SystemFont::StringWidth(const char* str, int len) const
{
    int result = 0;

    if (!str)
        return result;

    if (!len)
        len = (int)strlen(str);

    const char* c = str;
    for (int i = 0; i < len; i++) {
        if (isspace((unsigned char)*c) && ((unsigned char)*c < PIPE_NBSP || (unsigned char)*c > ARROW_RIGHT))
            result += spacewidth;
        else {
            int cc = (unsigned char)*c;
            if (flags & FONT_ALL_CAPS)
                if (islower((unsigned char)cc))
                    cc = toupper((unsigned char)cc);

            int k = 0;
            if (i < len - 1)
                k = kern[cc][(unsigned char)str[i + 1]];

            result += glyph[cc].width + interspace + k;
        }
        c++;
    }

    return result;
}

// +--------------------------------------------------------------------+

void
SystemFont::DrawText(const char* text, int count, Rect& text_rect, DWORD textflags, UTexture2D* tgt)
{
    Rect clip_rect = text_rect;

    if (clip_rect.w < 1 || clip_rect.h < 1)
        return;

    tgt_texture = tgt;

    if (text && text[0]) {
        if (count < 1)
            count = (int)strlen(text);

        // single line:
        if (flags & DT_SINGLELINE) {
            DrawTextSingle(text, count, text_rect, clip_rect);
        }

        // multi-line with word wrap:
        else if (flags & DT_WORDBREAK) {
            DrawTextWrap(text, count, text_rect, clip_rect);
        }

        // multi-line with clip:
        else {
            DrawTextMulti(text, count, text_rect, clip_rect);
        }
    }
    else {
        caret_x = text_rect.x + 2;
        caret_y = text_rect.y + 2;
    }

    // if calc only, update the rectangle:
    if (flags & DT_CALCRECT) {
        text_rect.h = clip_rect.h;
        text_rect.w = clip_rect.w;
    }

    // otherwise, draw caret if requested:
    else if (caret_index >= 0 && caret_y >= text_rect.y && caret_y <= text_rect.y + text_rect.h) {
        Video* video = Video::GetInstance();

        if (video && (GetRealTime() / 500) & 1) {
            float v[4];
            v[0] = (float)(caret_x + 1);
            v[1] = (float)(caret_y);
            v[2] = (float)(caret_x + 1);
            v[3] = (float)(caret_y + height);

            video->DrawScreenLines(1, v, color, blend);
        }

        caret_index = -1;
    }

    tgt_texture = nullptr;
}

// +--------------------------------------------------------------------+

static int find_next_word_start(const char* text, int index)
{
    // step through intra-word space:
    while (text[index] && isspace((unsigned char)text[index]) && text[index] != '\n')
        index++;

    return index;
}

static int find_next_word_end(const char* text, int index)
{
    if (index < 0)
        return index;

    // check for leading newline:
    if (text[index] == '\n')
        return index;

    // step through intra-word space:
    while (text[index] && isspace((unsigned char)text[index]))
        index++;

    // step through word:
    while (text[index] && !isspace((unsigned char)text[index]))
        index++;

    return index - 1;
}

// +--------------------------------------------------------------------+

void
SystemFont::DrawTextSingle(const char* text, int count, const Rect& text_rect, Rect& clip_rect)
{
    // Use the member/global flags (SystemFont::flags)
    const uint32 f = (uint32)flags;

    const bool nodraw = (f & DT_CALCRECT) != 0;

    if (!text) {
        caret_x = text_rect.x;
        caret_y = text_rect.y;

        if (nodraw) {
            clip_rect.h = Height();
            clip_rect.w = 0;
        }
        return;
    }

    if (count < 0) {
        count = 0;
    }
    if (count == 0) {
        count = (int)strlen(text);
    }

    int align = DT_LEFT;
    if (f & DT_RIGHT)       align = DT_RIGHT;
    else if (f & DT_CENTER) align = DT_CENTER;

    int valign = DT_TOP;
    if (f & DT_BOTTOM)       valign = DT_BOTTOM;
    else if (f & DT_VCENTER) valign = DT_VCENTER;

    int xoffset = 0;
    int yoffset = 0;

    const int length = StringWidth(text, count);

    if (length < text_rect.w) {
        switch (align) {
        default:
        case DT_LEFT:   break;
        case DT_RIGHT:  xoffset = text_rect.w - length; break;
        case DT_CENTER: xoffset = (text_rect.w - length) / 2; break;
        }
    }

    if (Height() < text_rect.h) {
        switch (valign) {
        default:
        case DT_TOP:     break;
        case DT_BOTTOM:  yoffset = text_rect.h - Height(); break;
        case DT_VCENTER: yoffset = (text_rect.h - Height()) / 2; break;
        }
    }

    if (nodraw) {
        clip_rect.h = Height();
        clip_rect.w = length;
    }
    else {
        const int x1 = text_rect.x + xoffset;
        const int y1 = text_rect.y + yoffset;
        DrawString(text, count, x1, y1, text_rect);
    }

    if (caret_index >= 0 && caret_index <= count) {
        caret_x = text_rect.x + xoffset;
        caret_y = text_rect.y + yoffset;

        if (caret_index > 0) {
            caret_x += StringWidth(text, caret_index);
        }
    }
    else {
        caret_x = text_rect.x;
        caret_y = text_rect.y;
    }
}

// +--------------------------------------------------------------------+

void
SystemFont::DrawTextWrap(const char* text, int count, const Rect& text_rect, Rect& clip_rect)
{
    // Use the member/global flags (SystemFont::flags)
    const uint32 f = (uint32)flags;

    const bool nodraw = (f & DT_CALCRECT) != 0;

    if (!text) {
        caret_x = -1;
        caret_y = -1;

        if (nodraw) {
            clip_rect.h = 0;
            clip_rect.w = 0;
        }
        return;
    }

    if (count < 0) {
        count = 0;
    }
    if (count == 0) {
        count = (int)strlen(text);
    }

    int align = DT_LEFT;
    if (f & DT_RIGHT)       align = DT_RIGHT;
    else if (f & DT_CENTER) align = DT_CENTER;

    int nlines = 0;
    int max_width = 0;

    int line_start = 0;
    int line_count = 0;
    int count_remaining = count;
    int curr_word_end = -1;
    int next_word_end = 0;
    int eol_index = 0;

    int xoffset = 0;
    int yoffset = 0;

    caret_x = -1;
    caret_y = -1;

    // repeat for each line of text:
    while (count_remaining > 0) {
        int length = 0;

        // find the end of the last whole word that fits on the line:
        for (;;) {
            next_word_end = find_next_word_end(text, curr_word_end + 1);

            if (next_word_end < 0 || next_word_end == curr_word_end)
                break;

            if (text[next_word_end] == '\n') {
                eol_index = curr_word_end = next_word_end;
                break;
            }

            const int word_len = next_word_end - line_start + 1;
            length = StringWidth(text + line_start, word_len);

            if (length < text_rect.w) {
                curr_word_end = next_word_end;

                // check for a newline in the next block of white space:
                eol_index = 0;
                const char* eol = &text[curr_word_end + 1];
                while (*eol && isspace((unsigned char)*eol) && *eol != '\n')
                    eol++;

                if (*eol == '\n') {
                    eol_index = (int)(eol - text);
                    break;
                }
            }
            else {
                break;
            }
        }

        line_count = curr_word_end - line_start + 1;

        if (line_count > 0) {
            length = StringWidth(text + line_start, line_count);
        }
        // there was a single word longer than the entire line:
        else {
            line_count = next_word_end - line_start + 1;
            length = StringWidth(text + line_start, line_count);
            curr_word_end = next_word_end;
        }

        xoffset = 0;
        if (length < text_rect.w) {
            switch (align) {
            default:
            case DT_LEFT:   break;
            case DT_RIGHT:  xoffset = text_rect.w - length; break;
            case DT_CENTER: xoffset = (text_rect.w - length) / 2; break;
            }
        }

        if (length > max_width)
            max_width = length;

        if (eol_index > 0)
            curr_word_end = eol_index;

        const int next_line_start = find_next_word_start(text, curr_word_end + 1);

        if (length > 0 && !nodraw) {
            const int x1 = text_rect.x + xoffset;
            const int y1 = text_rect.y + yoffset;

            DrawString(text + line_start, line_count, x1, y1, text_rect);

            if (caret_index == line_start) {
                caret_x = x1 - 2;
                caret_y = y1;
            }
            else if (caret_index > line_start && caret_index < next_line_start) {
                caret_x = text_rect.x + xoffset +
                    StringWidth(text + line_start, caret_index - line_start) - 2;
                caret_y = text_rect.y + yoffset;
            }
            else if (caret_index == count) {
                if (count > 0 && text[count - 1] == '\n') {
                    caret_x = x1 - 2;
                    caret_y = y1 + height;
                }
                else {
                    caret_x = text_rect.x + xoffset +
                        StringWidth(text + line_start, caret_index - line_start) - 2;
                    caret_y = text_rect.y + yoffset;
                }
            }
        }

        nlines++;
        yoffset += Height();

        if (eol_index > 0)
            curr_word_end = eol_index;

        line_start = find_next_word_start(text, curr_word_end + 1);
        count_remaining = count - line_start;
    }

    // if calc only, update the rectangle:
    if (nodraw) {
        clip_rect.h = nlines * Height();
        clip_rect.w = max_width;
    }
}


// +--------------------------------------------------------------------+

void
SystemFont::DrawTextMulti(const char* text, int count, const Rect& text_rect, Rect& clip_rect)
{
    // Use the member/global flags (SystemFont::flags)
    const uint32 f = (uint32)flags;

    const bool nodraw = (f & DT_CALCRECT) != 0;

    if (!text) {
        caret_x = -1;
        caret_y = -1;

        if (nodraw) {
            clip_rect.h = 0;
            clip_rect.w = 0;
        }
        return;
    }

    if (count < 0) {
        count = 0;
    }
    if (count == 0) {
        count = (int)strlen(text);
    }

    int align = DT_LEFT;
    if (f & DT_RIGHT)       align = DT_RIGHT;
    else if (f & DT_CENTER) align = DT_CENTER;

    int max_width = 0;
    int line_start = 0;
    int count_remaining = count;

    int xoffset = 0;
    int yoffset = 0;
    int nlines = 0;

    // repeat for each line of text:
    while (count_remaining > 0) {
        int length = 0;
        int line_count = 0;

        // find the end of line:
        while (line_count < count_remaining) {
            const char c = text[line_start + line_count];
            if (!c || c == '\n')
                break;

            line_count++;
        }

        if (line_count > 0) {
            length = StringWidth(text + line_start, line_count);
        }

        xoffset = 0;
        if (length < text_rect.w) {
            switch (align) {
            default:
            case DT_LEFT:   break;
            case DT_RIGHT:  xoffset = text_rect.w - length; break;
            case DT_CENTER: xoffset = (text_rect.w - length) / 2; break;
            }
        }

        if (length > max_width)
            max_width = length;

        if (length > 0 && !nodraw) {
            const int x1 = text_rect.x + xoffset;
            const int y1 = text_rect.y + yoffset;

            DrawString(text + line_start, line_count, x1, y1, text_rect);
        }

        nlines++;
        yoffset += Height();

        if (line_start + line_count + 1 < count) {
            line_start = find_next_word_start(text, line_start + line_count + 1);
            count_remaining = count - line_start;
        }
        else {
            count_remaining = 0;
        }
    }

    // if calc only, update the rectangle:
    if (nodraw) {
        clip_rect.h = nlines * Height();
        clip_rect.w = max_width;
    }
}

// +--------------------------------------------------------------------+

int
SystemFont::DrawString(const char* str, int len, int x1, int y1, const Rect& clip, UTexture2D* tgt)
{
    Video* video = Video::GetInstance();
    int    count = 0;
    int    maxw = clip.w;

    if (len < 1 || !video)
        return count;

    // vertical clip
    if ((y1 < clip.y) || (y1 > clip.y + clip.h))
        return count;

    // Starshatter Wars UE port:
    // The original code could render into a CPU Bitmap (BitBlt / alpha copy).
    // UE does not support direct software writes to UTexture2D without an explicit update pipeline.
    // For now, we only support the Video path; if a target texture is provided, we log once and ignore it.
    if (tgt) {
        UE_LOG(LogTemp, Warning, TEXT("SystemFont::DrawString: CPU render-to-texture path is not implemented; rendering via Video instead."));
    }

    // RENDER TO VIDEO

    // allocate verts, if necessary
    int nverts = 4 * len;
    if (!vset) {
        vset = new VertexSet(nverts);

        if (!vset)
            return count;

        vset->space = VertexSet::SCREEN_SPACE;

        for (int v = 0; v < vset->nverts; v++) {
            vset->s_loc[v].Z = 0.0f;
            vset->rw[v] = 1.0f;
        }
    }
    else if (vset->nverts < nverts) {
        vset->Resize(nverts);

        for (int v = 0; v < vset->nverts; v++) {
            vset->s_loc[v].Z = 0.0f;
            vset->rw[v] = 1.0f;
        }
    }

    if (vset->nverts < nverts)
        return count;

    if (alpha < 1)
        color.SetAlpha((BYTE)(alpha * 255.0f));
    else
        color.SetAlpha(255);

    for (int i = 0; i < len; i++) {
        char c = str[i];

        if ((flags & FONT_ALL_CAPS) && islower((unsigned char)c))
            c = (char)toupper((unsigned char)c);

        int cw = glyph[(BYTE)c].width + interspace;
        int k = 0;

        if (i < len - 1)
            k = kern[(BYTE)c][(BYTE)str[i + 1]];

        // horizontal clip:
        if (x1 < clip.x) {
            if (isspace((unsigned char)c) && ((unsigned char)c < PIPE_NBSP || (unsigned char)c > ARROW_RIGHT)) {
                x1 += spacewidth;
                maxw -= spacewidth;
            }
            else {
                x1 += cw + k;
                maxw -= cw + k;
            }
        }
        else if (x1 + cw > clip.x + clip.w) {
            break;
        }
        else {
            if (isspace((unsigned char)c) && ((unsigned char)c < PIPE_NBSP || (unsigned char)c > ARROW_RIGHT)) {
                x1 += spacewidth;
                maxw -= spacewidth;
            }
            else {
                // create four verts for this character:
                int    v = count * 4;
                double char_x = (double)GlyphLocationX((BYTE)c);
                double char_y = (double)GlyphLocationY((BYTE)c);
                double char_w = (double)glyph[(BYTE)c].width;
                double char_h = (double)height;

                if (y1 + (int)char_h > clip.y + clip.h) {
                    char_h = (double)(clip.y + clip.h - y1);
                }

                vset->s_loc[v + 0].X = (float)(x1 - 0.5);
                vset->s_loc[v + 0].Y = (float)(y1 - 0.5);
                vset->tu[v + 0] = (float)(char_x / 256.0);
                vset->tv[v + 0] = (float)(char_y / 256.0);
                vset->diffuse[v + 0] = color.Value();

                vset->s_loc[v + 1].X = (float)(x1 + char_w - 0.5);
                vset->s_loc[v + 1].Y = (float)(y1 - 0.5);
                vset->tu[v + 1] = (float)(char_x / 256.0 + char_w / 256.0);
                vset->tv[v + 1] = (float)(char_y / 256.0);
                vset->diffuse[v + 1] = color.Value();

                vset->s_loc[v + 2].X = (float)(x1 + char_w - 0.5);
                vset->s_loc[v + 2].Y = (float)(y1 + char_h - 0.5);
                vset->tu[v + 2] = (float)(char_x / 256.0 + char_w / 256.0);
                vset->tv[v + 2] = (float)(char_y / 256.0 + char_h / 256.0);
                vset->diffuse[v + 2] = color.Value();

                vset->s_loc[v + 3].X = (float)(x1 - 0.5);
                vset->s_loc[v + 3].Y = (float)(y1 + char_h - 0.5);
                vset->tu[v + 3] = (float)(char_x / 256.0);
                vset->tv[v + 3] = (float)(char_y / 256.0 + char_h / 256.0);
                vset->diffuse[v + 3] = color.Value();

                x1 += cw + k;
                maxw -= cw + k;

                count++;
            }
        }
    }

    if (count) {
        // this small hack is an optimization to reduce the
        // size of vertex buffer needed for font rendering:
        int old_nverts = vset->nverts;
        vset->nverts = 4 * count;

        // create a larger poly array, if necessary:
        if (count > npolys) {
            if (polys)
                delete[] polys;

            npolys = count;
            polys = new Poly[npolys];
            Poly* p = polys;
            int   index = 0;

            for (int i = 0; i < npolys; i++) {
                p->nverts = 4;
                p->vertex_set = vset;
                p->material = material;
                p->verts[0] = index++;
                p->verts[1] = index++;
                p->verts[2] = index++;
                p->verts[3] = index++;

                p++;
            }
        }

        video->DrawScreenPolys(count, polys, blend);

        // remember to restore the proper size of the vertex set:
        vset->nverts = old_nverts;
    }

    return count;
}
