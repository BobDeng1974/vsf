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
#include "vsf.h"

#if VSF_USE_TINY_GUI == ENABLED
#include <stdio.h>
#include "./images/demo_images.h"
#include "./stopwatch/stopwatch.h"
/*============================ MACROS ========================================*/
#define DEMO_OFFSET            0

#define DEMO_BACKGROUND_ALIGN   (VSF_TGUI_ALIGN_TOP | VSF_TGUI_ALIGN_LEFT)
//#define DEMO_BACKGROUND_ALIGN   (VSF_TGUI_ALIGN_TOP | VSF_TGUI_ALIGN_RIGHT)
//#define DEMO_BACKGROUND_ALIGN   (VSF_TGUI_ALIGN_BOTTOM | VSF_TGUI_ALIGN_LEFT)
//#define DEMO_BACKGROUND_ALIGN   (VSF_TGUI_ALIGN_BOTTOM | VSF_TGUI_ALIGN_RIGHT)
//#define DEMO_BACKGROUND_ALIGN   (VSF_TGUI_ALIGN_TOP)
//#define DEMO_BACKGROUND_ALIGN   (VSF_TGUI_ALIGN_BOTTOM)
//#define DEMO_BACKGROUND_ALIGN   (VSF_TGUI_ALIGN_LEFT)
//#define DEMO_BACKGROUND_ALIGN   (VSF_TGUI_ALIGN_RIGHT)
//#define DEMO_BACKGROUND_ALIGN   (VSF_TGUI_ALIGN_CENTER)

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ LOCAL VARIABLES ===============================*/
static NO_INIT vsf_tgui_t s_tTGUIDemo;
static NO_INIT stopwatch_t s_tMyStopwatch;

/*============================ PROTOTYPES ====================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ IMPLEMENTATION ================================*/

void vsf_tgui_low_level_on_ready_to_refresh(void)
{
    vsf_tgui_low_level_refresh_ready(&s_tTGUIDemo);
}

vsf_err_t tgui_demo_init(void)
{
    NO_INIT static vsf_tgui_evt_t s_tEvtQueueBuffer[32];

#if VSF_TGUI_CFG_REFRESH_SCHEME == VSF_TGUI_REFRESH_SCHEME_BREADTH_FIRST_TRAVERSAL
    NO_INIT static uint16_t s_tBFSBuffer[32];
#endif

    const vsf_tgui_cfg_t cfg = {
        .evt_queue = {
            .obj_ptr = s_tEvtQueueBuffer, 
            .s32_size = sizeof(s_tEvtQueueBuffer)
        },
#if VSF_TGUI_CFG_REFRESH_SCHEME == VSF_TGUI_REFRESH_SCHEME_BREADTH_FIRST_TRAVERSAL
        .bfs_queue = {
            .obj_ptr = s_tBFSBuffer,
            .s32_size = sizeof(s_tBFSBuffer),
        },
#endif
    };

    vsf_err_t err = vk_tgui_init(&s_tTGUIDemo, &cfg);

    my_stopwatch_init(&s_tMyStopwatch, &s_tTGUIDemo);

    vk_tgui_set_top_container(&s_tTGUIDemo, (vsf_tgui_top_container_t *)&s_tMyStopwatch);

    return err;
}

void vsf_tgui_on_keyboard_evt(vk_keyboard_evt_t* evt)
{
//! this block of code is used for test purpose only
    vsf_tgui_evt_t event = {
        .tKeyEvt = {
            .msg = vsf_input_keyboard_is_down(evt)
                                ? VSF_TGUI_EVT_KEY_DOWN
                                : VSF_TGUI_EVT_KEY_UP,
            .hwKeyValue = vsf_input_keyboard_get_keycode(evt),
        },
    };

    vk_tgui_send_message(&s_tTGUIDemo, event);

    if (!vsf_input_keyboard_is_down(evt)) {
        event.tKeyEvt.msg = VSF_TGUI_EVT_KEY_PRESSED;
        vk_tgui_send_message(&s_tTGUIDemo, event);
    }
}


void vsf_tgui_on_touchscreen_evt(vk_touchscreen_evt_t* ts_evt)
{
    //! this block of code is used for test purpose only

    vsf_tgui_evt_t event = {
        .PointerEvt = {
            .msg = vsf_input_touchscreen_is_down(ts_evt) 
                                ?   VSF_TGUI_EVT_POINTER_DOWN 
                                :   VSF_TGUI_EVT_POINTER_UP,
            
            .iX = vsf_input_touchscreen_get_x(ts_evt),
            .iY = vsf_input_touchscreen_get_y(ts_evt),
        },
    };

    vk_tgui_send_message(&s_tTGUIDemo, event);

    
    if (!vsf_input_touchscreen_is_down(ts_evt)) {
        event.PointerEvt.msg = VSF_TGUI_EVT_POINTER_CLICK;
        vk_tgui_send_message(&s_tTGUIDemo, event);
    }
}

#if VSF_TGUI_CFG_SUPPORT_MOUSE == ENABLED
void vsf_tgui_on_mouse_evt(vk_mouse_evt_t *mouse_evt)
{
/*
    switch (vk_input_mouse_evt_get(mouse_evt)) {
        case VSF_INPUT_MOUSE_EVT_BUTTON:
            vsf_trace_debug("mouse button: %d %s @(%d, %d)" VSF_TRACE_CFG_LINEEND,
                vk_input_mouse_evt_button_get(mouse_evt),
                vk_input_mouse_evt_button_is_down(mouse_evt) ? "down" : "up",
                vk_input_mouse_evt_get_x(mouse_evt),
                vk_input_mouse_evt_get_y(mouse_evt));
            break;
        case VSF_INPUT_MOUSE_EVT_MOVE:
            vsf_trace_debug("mouse move: @(%d, %d)" VSF_TRACE_CFG_LINEEND,
                vk_input_mouse_evt_get_x(mouse_evt),
                vk_input_mouse_evt_get_y(mouse_evt));
            break;
        case VSF_INPUT_MOUSE_EVT_WHEEL:
            vsf_trace_debug("mouse wheel: (%d, %d)" VSF_TRACE_CFG_LINEEND,
                vk_input_mouse_evt_get_x(mouse_evt),
                vk_input_mouse_evt_get_y(mouse_evt));
            break;
    }

*/
    //! this block of code is used for test purpose only

    

    switch (vk_input_mouse_evt_get(mouse_evt)) {
        case VSF_INPUT_MOUSE_EVT_BUTTON: {
            
            vsf_tgui_evt_t event = {
                .PointerEvt = {
                    .msg = vk_input_mouse_evt_button_is_down(mouse_evt) 
                                        ?   VSF_TGUI_EVT_POINTER_DOWN 
                                        :   VSF_TGUI_EVT_POINTER_UP,
            
                    .iX = vk_input_mouse_evt_get_x(mouse_evt),
                    .iY = vk_input_mouse_evt_get_y(mouse_evt),
                },
            };

            vk_tgui_send_message(&s_tTGUIDemo, event);

    
            if (!vk_input_mouse_evt_button_is_down(mouse_evt)) {
                event.PointerEvt.msg = VSF_TGUI_EVT_POINTER_CLICK;
                vk_tgui_send_message(&s_tTGUIDemo, event);
            }
            break;
        }

        case VSF_INPUT_MOUSE_EVT_MOVE: {
            vsf_tgui_evt_t event = {
                .PointerEvt = {
                    .msg = VSF_TGUI_EVT_POINTER_MOVE,
                    .iX = vk_input_mouse_evt_get_x(mouse_evt),
                    .iY = vk_input_mouse_evt_get_y(mouse_evt),
                },
            };

            vk_tgui_send_message(&s_tTGUIDemo, event);

            break;
        }

        case VSF_INPUT_MOUSE_EVT_WHEEL:  {
                vsf_tgui_evt_t event = {
                    .tGestureEvt = {
                        .msg = VSF_TGUI_EVT_GESTURE_SLIDE,
                        .tDelta = {
                            .iX = 0,
                            .iY = vk_input_mouse_evt_get_y(mouse_evt),
                            .hwMillisecond = 20,            //! 50Hz 
                        },
                    },
                };

                vsf_tgui_control_set_active(vsf_tgui_pointed_control_get(&s_tTGUIDemo));
                vk_tgui_send_message(&s_tTGUIDemo, event);
                break;
            }

        default:
            break;
    } 

}
#endif


#endif


/* EOF */
