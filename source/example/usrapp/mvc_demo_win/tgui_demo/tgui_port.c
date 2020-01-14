/*****************************************************************************
 *   Copyright(C)2009-2019 by VSF Team                                       *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *     http://www.apache.org/licenses/LICENSE-2.0                            *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 ****************************************************************************/

/*============================ INCLUDES ======================================*/

#define __VSF_DISP_CLASS_INHERIT
#include "vsf.h"

#if VSF_USE_UI == ENABLED && VSF_USE_TINY_GUI == ENABLED
#include "./images/demo_images.h"
#include "./images/demo_images_data.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <math.h>

/*============================ MACROS ========================================*/
#define FREETYPE_FONT_PATH              "../usrapp/mvc_demo_win/tgui_demo/wqy-microhei.ttc"
#define FREETYPE_FONT_SIZE              (24)
#define FREETYPE_LOAD_FLAGS             (FT_LOAD_RENDER)
#define FREETYPE_DEFAULT_DPI            72

#define TGUI_PORT_DEBAULT_BACKGROUND_COLOR  0xFF

#ifndef VSF_TGUI_SV_CFG_PORT_LOG
#define VSF_TGUI_SV_CFG_PORT_LOG           DISABLED 
#endif
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
declare_vsf_pt(tgui_demo_t)
def_vsf_pt(tgui_demo_t)

/*============================ GLOBAL VARIABLES ==============================*/
extern vsf_tgui_font_t g_tUserFonts[];
/*============================ LOCAL VARIABLES ===============================*/
static vk_disp_t* s_tDisp;

volatile static bool s_bIsReadyToRefresh = true;

/*============================ PROTOTYPES ====================================*/
bool vsf_tgui_port_is_ready_to_refresh(void);

#ifdef WEAK_VSF_TGUI_LOW_LEVEL_ON_READY_TO_REFRESH_EXTERN
WEAK_VSF_TGUI_LOW_LEVEL_ON_READY_TO_REFRESH_EXTERN
#endif

/*============================ IMPLEMENTATION ================================*/
static vsf_tgui_color_t vk_disp_sdl_get_pixel(vsf_tgui_location_t* ptLocation)
{
    vsf_tgui_color_t* ptPixMap = s_tDisp->ui_data;
    return ptPixMap[ptLocation->nY * VSF_TGUI_HOR_MAX + ptLocation->nX];
}

static void vk_disp_sdl_set_pixel(vsf_tgui_location_t* ptLocation, vsf_tgui_color_t tColor)
{
    vsf_tgui_color_t* ptPixMap = s_tDisp->ui_data;
    ptPixMap[ptLocation->nY * VSF_TGUI_HOR_MAX + ptLocation->nX] = tColor;
}

/********************************************************************************/

vsf_tgui_size_t vsf_tgui_sdl_idx_root_tile_get_size(const vsf_tgui_tile_t* ptTile)
{
    VSF_TGUI_ASSERT(ptTile != NULL);

    uint8_t chIndex = ptTile->tIndexRoot.chIndex;
    if (ptTile->_.tCore.tAttribute.u2ColorType == VSF_TGUI_COLORTYPE_RGB) {           // RGB color
        VSF_TGUI_ASSERT(chIndex < dimof(gIdxRootRGBTileSizes));
        return gIdxRootRGBTileSizes[chIndex];
    } else if (ptTile->_.tCore.tAttribute.u2ColorType == VSF_TGUI_COLORTYPE_RGBA) {    // RGBA color
        VSF_TGUI_ASSERT(chIndex < dimof(gIdxRootRGBATileSizes));
        return gIdxRootRGBATileSizes[chIndex];
    } else {
        VSF_TGUI_ASSERT(0);
    }
}

char* vsf_tgui_sdl_tile_get_pixelmap(const vsf_tgui_tile_t* ptTile)
{
    VSF_TGUI_ASSERT(ptTile != NULL);

    if (ptTile->_.tCore.tAttribute.u2RootTileType == 0) {     // buf tile
        return ptTile->tBufRoot.ptBitmap;
    } else {                                                // index tile
        uint8_t chIndex = ptTile->tIndexRoot.chIndex;
        if (ptTile->_.tCore.tAttribute.u2ColorType == VSF_TGUI_COLORTYPE_RGB) {           // RGB color
            VSF_TGUI_ASSERT(chIndex < dimof(rgb_pixmap_array));
            return (char *)rgb_pixmap_array[chIndex];
        } else if (ptTile->_.tCore.tAttribute.u2ColorType == VSF_TGUI_COLORTYPE_RGBA) {    // RGBA color
            VSF_TGUI_ASSERT(chIndex < dimof(rgba_pixmap_array));
            return (char *)rgba_pixmap_array[chIndex];
        } else {
            VSF_TGUI_ASSERT(0);
        }

        return NULL;
    }
}

/*********************************************************************************/
static uint8_t freetype_get_char_height(FT_Face ptFace)
{
    /*
     * msyh test
        | fontsize | fontsize | fontsize | ��(U+2589) height |
        | -------- | -------- | -------- | :--------------- |
        | 12       | 16       | 17       | 18               |
        | 13       | 17       | 18       | 19               |
        | 14       | 18       | 19       | 20               |
        | 15       | 20       | 20       | 21               |
        | 16       | 21       | 22       | 22               |
        | 17       | 22       | 23       | 25               |
        | 18       | 24       | 25       | 26               |
        | 19       | 25       | 26       | 27               |
        | 20       | 26       | 28       | 28               |
        | 21       | 28       | 29       | 30               |
        | 22       | 29       | 30       | 32               |
        | 23       | 30       | 32       | 33               |
        | 24       | 32       | 33       | 34               |
        | 25       | 33       | 34       | 35               |
        | 26       | 34       | 35       | 38               |
        | 27       | 36       | 37       | 39               |
        | 28       | 37       | 38       | 40               |
        | 29       | 38       | 39       | 41               |
        | 30       | 40       | 40       | 43               |
        | 31       | 41       | 42       | 45               |
        | 32       | 42       | 43       | 46               |
        | 33       | 44       | 44       | 47               |
        | 34       | 45       | 45       | 48               |
        | 35       | 46       | 48       | 50               |
        | 36       | 48       | 49       | 52               |
        | 37       | 49       | 50       | 53               |
        | 38       | 50       | 51       | 54               |
        | 39       | 51       | 53       | 56               |
        | 40       | 53       | 54       | 57               |
        | 41       | 54       | 55       | 59               |
        | 42       | 55       | 56       | 60               |
        | 43       | 57       | 58       | 61               |
        | 44       | 58       | 59       | 63               |
        | 45       | 59       | 60       | 65               |
        | 46       | 61       | 62       | 66               |
        | 47       | 62       | 63       | 67               |
        | 48       | 63       | 64       | 68               |
     */

     //return ptFace->size->metrics.height >> 6;

     // https://stackoverflow.com/questions/26486642/whats-the-proper-way-of-getting-text-bounding-box-in-freetype-2
     // https://github.com/cocos2d/cocos2d-x/blob/b26e1bb08648c11a9482d0736a236aebc345008a/cocos/2d/CCFontFreeType.cpp#L180
     //return (ptFace->size->metrics.ascender - ptFace->size->metrics.descender) >> 6;

     //double pixel_size = FREETYPE_FONT_SIZE * FREETYPE_DEFAULT_DPI / 72;
     //return round((ptFace->bbox.yMax - ptFace->bbox.yMin) * pixel_size / ptFace->units_per_EM);

    //return (ptFace->size->metrics.height >> 6) * 12 / 10;
    return (ptFace->size->metrics.ascender >> 6) * 12 / 10;
}

uint8_t vsf_tgui_font_get_char_width(const uint8_t chFontIndex, uint32_t wChar)
{
    const vsf_tgui_font_t* ptFont = vsf_tgui_font_get(chFontIndex);
    VSF_TGUI_ASSERT(ptFont != NULL);

    FT_Face ptFace = (FT_Face)ptFont->ptData;
    VSF_TGUI_ASSERT(ptFace != NULL);
    if (FT_Err_Ok == FT_Load_Char(ptFace, wChar, FREETYPE_LOAD_FLAGS)) {
        return ptFace->glyph->advance.x >> 6;
    }

    return 0;
}

uint8_t vsf_tgui_font_get_char_height(const uint8_t chFontIndex)
{
    const vsf_tgui_font_t* ptFont = vsf_tgui_font_get(chFontIndex);
    VSF_TGUI_ASSERT(ptFont != NULL);

    return ptFont->chHeight;
}

/*********************************************************************************/

void vsf_tgui_draw_rect(vsf_tgui_location_t* ptLocation, vsf_tgui_size_t* ptSize, vsf_tgui_color_t tRectColor)
{
    //vsf_tgui_color_t* ptPixMap = s_tDisp->ui_data;
    int16_t iHeight = ptSize->nHeight;
    int16_t iWidth = ptSize->nWidth;
    VSF_TGUI_ASSERT(ptLocation != NULL);
    VSF_TGUI_ASSERT((0 <= ptLocation->nX) && (ptLocation->nX < VSF_TGUI_HOR_MAX));  // x_start point in screen
    VSF_TGUI_ASSERT((0 <= ptLocation->nY) && (ptLocation->nY < VSF_TGUI_VER_MAX));  // y_start point in screen
    VSF_TGUI_ASSERT(0 <= (ptLocation->nX + ptSize->nWidth));                        // x_end   point in screen
    
    VSF_TGUI_ASSERT(0 <= (ptLocation->nY + ptSize->nHeight));                       // y_end   point in screen

    /* only draw visible part */
    if ((ptLocation->nY + ptSize->nHeight) > VSF_TGUI_VER_MAX) {
        iHeight = VSF_TGUI_VER_MAX - ptLocation->nY;
    }

    if ((ptLocation->nX + ptSize->nWidth) > VSF_TGUI_HOR_MAX) {
        iWidth = VSF_TGUI_HOR_MAX - ptLocation->nX;
    }


    for (uint16_t i = 0; i < iHeight; i++) {
        for (uint16_t j = 0; j < iWidth; j++) {
            vsf_tgui_location_t tPixelLocation = { .nX = ptLocation->nX + j, .nY = ptLocation->nY + i };
            vsf_tgui_color_t tPixelColor = vk_disp_sdl_get_pixel(&tPixelLocation);
            tPixelColor = vsf_tgui_color_mix(tRectColor, tPixelColor, tRectColor.tChannel.chA);
            vk_disp_sdl_set_pixel(&tPixelLocation, tPixelColor);
        }
    }
}

void vsf_tgui_draw_root_tile(vsf_tgui_location_t* ptLocation,
                        vsf_tgui_location_t* ptTileLocation,
                        vsf_tgui_size_t* ptSize,
                        const vsf_tgui_tile_t* ptTile)
{
    VSF_TGUI_ASSERT(ptLocation != NULL);
    VSF_TGUI_ASSERT(ptTileLocation != NULL);
    VSF_TGUI_ASSERT(ptSize != NULL);
    VSF_TGUI_ASSERT(ptTile != NULL);
    VSF_TGUI_ASSERT((0 <= ptLocation->nX) && (ptLocation->nX < VSF_TGUI_HOR_MAX));  // x_start point in screen
    VSF_TGUI_ASSERT((0 <= ptLocation->nY) && (ptLocation->nY < VSF_TGUI_VER_MAX));  // y_start point in screen
    VSF_TGUI_ASSERT(0 <= (ptLocation->nX + ptSize->nWidth));                        // x_end   point in screen
    VSF_TGUI_ASSERT((ptLocation->nX + ptSize->nWidth) <= VSF_TGUI_HOR_MAX);
    VSF_TGUI_ASSERT(0 <= (ptLocation->nY + ptSize->nHeight));                       // y_end   point in screen
    VSF_TGUI_ASSERT((ptLocation->nY + ptSize->nHeight) <= VSF_TGUI_VER_MAX);
    VSF_TGUI_ASSERT(vsf_tgui_tile_is_root(ptTile));

    vsf_tgui_size_t tTileSize = vsf_tgui_root_tile_get_size(ptTile);
    VSF_TGUI_ASSERT(ptTileLocation->nX < tTileSize.nWidth);
    VSF_TGUI_ASSERT(ptTileLocation->nY < tTileSize.nHeight);

    vsf_tgui_region_t tDisplay;
    tDisplay.tLocation = *ptLocation;
    tDisplay.tSize.nWidth = min(ptSize->nWidth, tTileSize.nWidth - ptTileLocation->nX);
    tDisplay.tSize.nHeight = min(ptSize->nHeight, tTileSize.nHeight - ptTileLocation->nY);
    if (tDisplay.tSize.nHeight <= 0 || tDisplay.tSize.nWidth <= 0) {
        return ;
    }

    uint32_t wSize;
    if (ptTile->_.tCore.tAttribute.u3ColorSize == VSF_TGUI_COLORSIZE_32IT) {
        wSize = 4;
    } else if (ptTile->_.tCore.tAttribute.u3ColorSize == VSF_TGUI_COLORSIZE_24IT) {
        wSize = 3;
    } else {
        VSF_TGUI_ASSERT(0);
    }
    const char* pchPixelmap = vsf_tgui_sdl_tile_get_pixelmap(ptTile);

    for (uint16_t i = 0; i < tDisplay.tSize.nHeight; i++) {
        uint32_t wOffset = wSize * ((ptTileLocation->nY + i) * tTileSize.nWidth + ptTileLocation->nX);
        const char* pchData = pchPixelmap + wOffset;

        for (uint16_t j = 0; j < tDisplay.tSize.nWidth; j++) {
            vsf_tgui_location_t tPixelLocation = { .nX = tDisplay.tLocation.nX + j, .nY = tDisplay.tLocation.nY + i };
            vsf_tgui_color_t tTileColor;
            tTileColor.tChannel.chR = *pchData++;
            tTileColor.tChannel.chG = *pchData++;
            tTileColor.tChannel.chB = *pchData++;

            if (ptTile->_.tCore.tAttribute.u2ColorType == VSF_TGUI_COLORTYPE_RGBA) {
                tTileColor.tChannel.chA = *pchData++;
                vsf_tgui_color_t tPixelColor = vk_disp_sdl_get_pixel(&tPixelLocation);
                tPixelColor = vsf_tgui_color_mix(tTileColor, tPixelColor, tTileColor.tChannel.chA);
                vk_disp_sdl_set_pixel(&tPixelLocation, tPixelColor);
            } else if (ptTile->_.tCore.tAttribute.u2ColorType == VSF_TGUI_COLORTYPE_RGB) {
                tTileColor.tChannel.chA = 0xFF;
                vk_disp_sdl_set_pixel(&tPixelLocation, tTileColor);
            } else {
                VSF_TGUI_ASSERT(0);
            }
        }
    }
}

void vsf_tgui_draw_char(vsf_tgui_location_t* ptLocation,
                        vsf_tgui_location_t* ptFontLocation,
                        vsf_tgui_size_t* ptSize,
                        uint8_t chFontIndex,
                        uint32_t wChar,
                        vsf_tgui_color_t tCharColor)
{
    const vsf_tgui_font_t* ptFont;
    VSF_TGUI_ASSERT(ptLocation != NULL);
    VSF_TGUI_ASSERT(ptFontLocation != NULL);
    VSF_TGUI_ASSERT(ptSize != NULL);

    ptFont = vsf_tgui_font_get(chFontIndex);
    VSF_TGUI_ASSERT(ptFont != NULL);

    FT_Face ptFace = (FT_Face)ptFont->ptData;
    VSF_TGUI_ASSERT(ptFace != NULL);

    if (FT_Err_Ok == FT_Load_Char(ptFace, wChar, FREETYPE_LOAD_FLAGS)) {
        FT_GlyphSlot glyph = ptFace->glyph;
        uint32_t wBaseLine = ptFace->size->metrics.ascender >> 6;
        int32_t wTop = glyph->bitmap_top;
        int32_t wLeft = glyph->bitmap_left;
        VSF_TGUI_ASSERT(wBaseLine >= wTop);

        vsf_tgui_location_t tBitmapStart = {
            .nX = max(0, wLeft),    // todo: support negative advance
            .nY = wBaseLine - wTop,
        };
        vsf_tgui_region_t tUpdateRegion = {
            .tLocation = *ptFontLocation,
            .tSize = *ptSize,
        };
        vsf_tgui_region_t tBitmapRegion = {
            .tLocation = tBitmapStart,
            .tSize = {.nWidth = glyph->bitmap.width, .nHeight = glyph->bitmap.rows},
        };
        vsf_tgui_region_t tRealBitmapRegion;
        if (vsf_tgui_region_intersect(&tRealBitmapRegion, &tUpdateRegion, &tBitmapRegion)) {
            uint32_t wXOffset = tRealBitmapRegion.tLocation.nX - tBitmapStart.nX;
            uint32_t wYOffset = tRealBitmapRegion.tLocation.nY - tBitmapStart.nY;

            for (uint16_t j = 0; j < tRealBitmapRegion.tSize.nHeight; j++) {
                for (uint16_t i = 0; i < tRealBitmapRegion.tSize.nWidth; i++) {
                    vsf_tgui_location_t tPixelLocation = { 
                        .nX = ptLocation->nX + i + tRealBitmapRegion.tLocation.nX - tUpdateRegion.tLocation.nX,
                        .nY = ptLocation->nY + j + tRealBitmapRegion.tLocation.nY - tUpdateRegion.tLocation.nY,
                    };
                    uint8_t mix = glyph->bitmap.buffer[  (j + tRealBitmapRegion.tLocation.nY - tBitmapRegion.tLocation.nY) * glyph->bitmap.width 
                                                       + (i + tRealBitmapRegion.tLocation.nX - tBitmapRegion.tLocation.nX)];

                    vsf_tgui_color_t tPixelColor = vk_disp_sdl_get_pixel(&tPixelLocation);
                    tPixelColor = vsf_tgui_color_mix(tCharColor, tPixelColor, mix);
                    vk_disp_sdl_set_pixel(&tPixelLocation, tPixelColor);
                }
            }

        }
    } else {
        VSF_TGUI_ASSERT(0);
    }
}

/**********************************************************************************/
/*! \brief begin a refresh loop
 *! \param ptGUI the tgui object address
 *! \param ptPlannedRefreshRegion the planned refresh region
 *! \retval NULL    No need to refresh (or rendering service is not ready)
 *! \retval !NULL   The actual refresh region
 *!
 *! \note When NULL is returned, current refresh iteration (i.e. a refresh activites
 *!       between vsf_tgui_v_refresh_loop_begin and vsf_tgui_v_refresh_loop_end )
 *!       will be ignored and vsf_tgui_v_refresh_loop_end is called immediately
 **********************************************************************************/
vsf_tgui_region_t *vsf_tgui_v_refresh_loop_begin(   
                    vsf_tgui_t *ptGUI, 
                    const vsf_tgui_region_t *ptPlannedRefreshRegion)
{
    if (!vsf_tgui_port_is_ready_to_refresh()) {
        return NULL;
    }
#if VSF_TGUI_SV_CFG_PORT_LOG == ENABLED
    VSF_TGUI_LOG(VSF_TRACE_INFO,
                 "[Simple View Port]Begin Refresh Loop" VSF_TRACE_CFG_LINEEND);
#endif


    return (vsf_tgui_region_t *)ptPlannedRefreshRegion;
}

bool vsf_tgui_v_refresh_loop_end(vsf_tgui_t* ptGUI)
{
    vsf_tgui_color_t* pixmap = s_tDisp->ui_data;

    vk_disp_area_t area = {
        .pos = {.x = 0, .y = 0},
        .size = {.x = VSF_TGUI_HOR_MAX, .y = VSF_TGUI_VER_MAX},
    };

     __vsf_sched_safe(
        if (s_bIsReadyToRefresh) {
            s_bIsReadyToRefresh = false;   
        }
    )

#if VSF_TGUI_SV_CFG_PORT_LOG == ENABLED
    VSF_TGUI_LOG(VSF_TRACE_INFO,
                 "[Simple View Port]End Refresh Loop." VSF_TRACE_CFG_LINEEND "\trequest low level refresh start in area, pos(x:0x%d, y:0x%d), size(w:0x%d, h:0x%d)" VSF_TRACE_CFG_LINEEND,
                 area.pos.x, area.pos.y, area.size.x, area.size.y);
#endif
    vk_disp_refresh(s_tDisp, &area, pixmap);

    return false;
}

#ifndef WEAK_VSF_TGUI_LOW_LEVEL_ON_READY_TO_REFRESH
WEAK(vsf_tgui_low_level_on_ready_to_refresh)
void vsf_tgui_low_level_on_ready_to_refresh(void)
{}
#endif

static void vsf_tgui_on_ready(vk_disp_t* disp)
{
#if VSF_TGUI_SV_CFG_PORT_LOG == ENABLED
    VSF_TGUI_LOG(VSF_TRACE_INFO,
                 "[Simple View Port] refresh ready" VSF_TRACE_CFG_LINEEND);
#endif

    s_bIsReadyToRefresh = true;

#ifndef WEAK_VSF_TGUI_LOW_LEVEL_ON_READY_TO_REFRESH
    vsf_tgui_low_level_on_ready_to_refresh();
#else
    WEAK_VSF_TGUI_LOW_LEVEL_ON_READY_TO_REFRESH();
#endif
}

bool vsf_tgui_port_is_ready_to_refresh(void)
{
    return s_bIsReadyToRefresh;
}

static bool vsf_tgui_sv_fonts_init(vsf_tgui_font_t* ptFont, size_t tSize)
{
    int i;
    FT_Library tLibrary;
    FT_Face ptFace;
    uint8_t chFontSize;
    uint8_t chFontHeight;

    VSF_TGUI_ASSERT(ptFont != NULL);

    for (i = 0; i < tSize; i++) {
        if (FT_Err_Ok != FT_Init_FreeType(&tLibrary)) {
            VSF_TGUI_ASSERT(0);
            return false;
        }

        if (FT_Err_Ok != FT_New_Face(tLibrary, ptFont->pchFontPath, 0, &ptFace)) {
            VSF_TGUI_ASSERT(0);
            return false;
        }

        if (ptFont->chFontSize != 0) {
            if (FT_Err_Ok != FT_Set_Char_Size(ptFace, 0, ptFont->chFontSize * 64, 0, 0)) {
                VSF_TGUI_ASSERT(0);
                return false;
            }
            ptFont->chHeight = freetype_get_char_height(ptFace);
        } else if (ptFont->chHeight != 0) {
            chFontSize = ptFont->chHeight;
            do {
                if (FT_Err_Ok != FT_Set_Char_Size(ptFace, 0, chFontSize * 64, 0, 0)) {
                    VSF_TGUI_ASSERT(0);
                    return false;
                }

                chFontHeight = freetype_get_char_height(ptFace);
                if (ptFont->chHeight < chFontHeight) {
                    chFontSize--;
                } else {
                    ptFont->chHeight = chFontHeight;
                    break;
                }
            } while (1);
        } else {
            VSF_TGUI_ASSERT(0);
            return false;
        }
        ptFont->ptData = ptFace;

        ptFont++;
    }

    return true;
}

void vsf_tgui_bind(vk_disp_t* ptDisp, void* ptData)
{
#ifdef TGUI_PORT_DEBAULT_BACKGROUND_COLOR
    VSF_TGUI_ASSERT(ptData)
    memset(ptData, TGUI_PORT_DEBAULT_BACKGROUND_COLOR, VSF_TGUI_HOR_MAX * VSF_TGUI_VER_MAX * sizeof(vsf_tgui_color_t));
#endif

    vsf_tgui_sv_fonts_init((vsf_tgui_font_t *)vsf_tgui_font_get(0), 
                            vsf_tgui_font_number());

    ptDisp->ui_data = ptData;
    ptDisp->ui_on_ready = vsf_tgui_on_ready;
    vk_disp_init(ptDisp);

    s_tDisp = ptDisp;
}
#endif

/* EOF */
