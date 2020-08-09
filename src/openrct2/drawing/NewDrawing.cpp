/*****************************************************************************
 * Copyright (c) 2014-2020 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "NewDrawing.h"

#include "../Context.h"
#include "../config/Config.h"
#include "../drawing/Drawing.h"
#include "../interface/Screenshot.h"
#include "../localisation/StringIds.h"
#include "../paint/Painter.h"
#include "../ui/UiContext.h"
#include "../world/Location.hpp"
#include "IDrawingContext.h"
#include "IDrawingEngine.h"

#include <cmath>

using namespace OpenRCT2;
using namespace OpenRCT2::Drawing;
using namespace OpenRCT2::Paint;
using namespace OpenRCT2::Ui;

rct_string_id DrawingEngineStringIds[] = {
    STR_DRAWING_ENGINE_SOFTWARE,
    STR_DRAWING_ENGINE_SOFTWARE_WITH_HARDWARE_DISPLAY,
    STR_DRAWING_ENGINE_OPENGL,
};

int32_t drawing_engine_get_type()
{
    auto context = GetContext();
    return context->GetDrawingEngineType();
}

static IDrawingEngine* GetDrawingEngine()
{
    IDrawingEngine* result = nullptr;
    auto context = GetContext();
    if (context != nullptr)
    {
        result = context->GetDrawingEngine();
    }
    return result;
}

bool drawing_engine_requires_new_window(int32_t srcEngine, int32_t dstEngine)
{
#ifdef _WIN32
    if (srcEngine != DRAWING_ENGINE_OPENGL && dstEngine != DRAWING_ENGINE_OPENGL)
    {
        // Windows is apparently able to switch to hardware rendering on the fly although
        // using the same window in an unaccelerated and accelerated context is unsupported by SDL2
        return false;
    }
#endif

    return true;
}

void drawing_engine_init()
{
    auto context = GetContext();
    if (context != nullptr)
    {
        context->InitialiseDrawingEngine();
    }
}

void drawing_engine_resize()
{
    auto context = GetContext();
    if (context != nullptr)
    {
        auto drawingEngine = context->GetDrawingEngine();
        if (drawingEngine != nullptr)
        {
            auto uiContext = context->GetUiContext();
            drawingEngine->Resize(uiContext->GetWidth(), uiContext->GetHeight());
        }
    }
}

void drawing_engine_set_palette(const GamePalette& colours)
{
    auto context = GetContext();
    if (context != nullptr)
    {
        auto drawingEngine = context->GetDrawingEngine();
        if (drawingEngine != nullptr)
        {
            drawingEngine->SetPalette(colours);
        }
    }
}

void drawing_engine_copy_rect(int32_t x, int32_t y, int32_t width, int32_t height, int32_t dx, int32_t dy)
{
    auto context = GetContext();
    if (context != nullptr)
    {
        auto drawingEngine = context->GetDrawingEngine();
        if (drawingEngine != nullptr)
        {
            drawingEngine->CopyRect(x, y, width, height, dx, dy);
        }
    }
}

void drawing_engine_dispose()
{
    auto context = GetContext();
    if (context != nullptr)
    {
        context->DisposeDrawingEngine();
    }
}

rct_drawpixelinfo* drawing_engine_get_dpi()
{
    auto context = GetContext();
    auto drawingEngine = context->GetDrawingEngine();
    return drawingEngine->GetDrawingPixelInfo();
}

bool drawing_engine_has_dirty_optimisations()
{
    bool result = false;
    auto drawingEngine = GetDrawingEngine();
    if (drawingEngine != nullptr)
    {
        result = (drawingEngine->GetFlags() & DEF_DIRTY_OPTIMISATIONS);
    }
    return result;
}

void drawing_engine_invalidate_image(uint32_t image)
{
    auto drawingEngine = GetDrawingEngine();
    if (drawingEngine != nullptr)
    {
        drawingEngine->InvalidateImage(image);
    }
}

void drawing_engine_set_vsync(bool vsync)
{
    auto drawingEngine = GetDrawingEngine();
    if (drawingEngine != nullptr)
    {
        drawingEngine->SetVSync(vsync);
    }
}

void gfx_set_dirty_blocks(const ScreenRect& rect)
{
    auto drawingEngine = GetDrawingEngine();
    if (drawingEngine != nullptr)
    {
        drawingEngine->Invalidate(rect.GetLeft(), rect.GetTop(), rect.GetRight(), rect.GetBottom());
    }
}

void gfx_draw_all_dirty_blocks()
{
}

void gfx_clear(rct_drawpixelinfo* dpi, uint8_t paletteIndex)
{
    auto drawingEngine = dpi->DrawingEngine;
    if (drawingEngine != nullptr)
    {
        IDrawingContext* dc = drawingEngine->GetDrawingContext(dpi);
        dc->Clear(paletteIndex);
    }
}

void gfx_fill_rect(rct_drawpixelinfo* dpi, const ScreenRect& rect, int32_t colour)
{
    auto drawingEngine = dpi->DrawingEngine;
    if (drawingEngine != nullptr)
    {
        IDrawingContext* dc = drawingEngine->GetDrawingContext(dpi);
        dc->FillRect(colour, rect.GetLeft(), rect.GetTop(), rect.GetRight(), rect.GetBottom());
    }
}

void gfx_filter_rect(
    rct_drawpixelinfo* dpi, int32_t left, int32_t top, int32_t right, int32_t bottom, FILTER_PALETTE_ID palette)
{
    gfx_filter_rect(dpi, { left, top, right, bottom }, palette);
}

void gfx_filter_rect(rct_drawpixelinfo* dpi, const ScreenRect& rect, FILTER_PALETTE_ID palette)
{
    auto drawingEngine = dpi->DrawingEngine;
    if (drawingEngine != nullptr)
    {
        IDrawingContext* dc = drawingEngine->GetDrawingContext(dpi);
        dc->FilterRect(palette, rect.GetLeft(), rect.GetTop(), rect.GetRight(), rect.GetBottom());
    }
}

void gfx_draw_line(rct_drawpixelinfo* dpi, const ScreenLine& line, int32_t colour)
{
    auto drawingEngine = dpi->DrawingEngine;
    if (drawingEngine != nullptr)
    {
        IDrawingContext* dc = drawingEngine->GetDrawingContext(dpi);
        dc->DrawLine(colour, line.GetX1(), line.GetY1(), line.GetX2(), line.GetY2());
    }
}

void gfx_draw_dashed_line(
    rct_drawpixelinfo* dpi, const ScreenLine& screenLine, const int32_t dashedLineSegmentLength, const int32_t color)
{
    assert(dashedLineSegmentLength > 0);

    const auto drawingEngine = dpi->DrawingEngine;
    if (drawingEngine != nullptr)
    {
        constexpr int32_t precisionFactor = 1000;

        const int32_t dashedLineLength = std::hypot(
            screenLine.GetX2() - screenLine.GetX1(), screenLine.GetY2() - screenLine.GetY1());
        const int32_t lineSegmentCount = dashedLineLength / dashedLineSegmentLength / 2;
        if (lineSegmentCount == 0)
        {
            return;
        }

        const int32_t lineXDist = std::abs(screenLine.GetX2() - screenLine.GetX1());
        const int32_t lineYDist = std::abs(screenLine.GetY2() - screenLine.GetY1());
        const int32_t dxPrecise = precisionFactor * lineXDist / lineSegmentCount / 2;
        const int32_t dyPrecise = precisionFactor * lineYDist / lineSegmentCount / 2;
        IDrawingContext* dc = drawingEngine->GetDrawingContext(dpi);

        for (int32_t i = 0, x, y; i < lineSegmentCount; ++i)
        {
            x = screenLine.GetX1() + dxPrecise * i * 2 / precisionFactor;
            y = screenLine.GetY1() + dyPrecise * i * 2 / precisionFactor;
            dc->DrawLine(color, x, y, x + dxPrecise / precisionFactor, y + dyPrecise / precisionFactor);
        }
    }
}

void FASTCALL gfx_draw_sprite(rct_drawpixelinfo* dpi, int32_t image, const ScreenCoordsXY& coords, uint32_t tertiary_colour)
{
    auto drawingEngine = dpi->DrawingEngine;
    if (drawingEngine != nullptr)
    {
        IDrawingContext* dc = drawingEngine->GetDrawingContext(dpi);
        dc->DrawSprite(image, coords.x, coords.y, tertiary_colour);
    }
}

void FASTCALL gfx_draw_glyph(rct_drawpixelinfo* dpi, int32_t image, const ScreenCoordsXY& coords, const PaletteMap& paletteMap)
{
    auto drawingEngine = dpi->DrawingEngine;
    if (drawingEngine != nullptr)
    {
        IDrawingContext* dc = drawingEngine->GetDrawingContext(dpi);
        dc->DrawGlyph(image, coords.x, coords.y, paletteMap);
    }
}

void FASTCALL
    gfx_draw_sprite_raw_masked(rct_drawpixelinfo* dpi, const ScreenCoordsXY& coords, int32_t maskImage, int32_t colourImage)
{
    auto drawingEngine = dpi->DrawingEngine;
    if (drawingEngine != nullptr)
    {
        IDrawingContext* dc = drawingEngine->GetDrawingContext(dpi);
        dc->DrawSpriteRawMasked(coords.x, coords.y, maskImage, colourImage);
    }
}

void FASTCALL gfx_draw_sprite_solid(rct_drawpixelinfo* dpi, int32_t image, const ScreenCoordsXY& coords, uint8_t colour)
{
    auto drawingEngine = dpi->DrawingEngine;
    if (drawingEngine != nullptr)
    {
        IDrawingContext* dc = drawingEngine->GetDrawingContext(dpi);
        dc->DrawSpriteSolid(image, coords.x, coords.y, colour);
    }
}

std::string screenshot_dump()
{
    auto drawingEngine = GetDrawingEngine();
    if (drawingEngine != nullptr)
    {
        return drawingEngine->Screenshot();
    }
    return "";
}
