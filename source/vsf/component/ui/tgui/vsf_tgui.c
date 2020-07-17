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
#include "./vsf_tgui_cfg.h"

#if VSF_USE_TINY_GUI == ENABLED

#define __VSF_TGUI_CONTROLS_CONTROL_CLASS_INHERIT
#define __VSF_TGUI_CLASS_IMPLEMENT
#include "./vsf_tgui.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

static void __vk_tgui_depose_top_container( vsf_tgui_t* ptGUI, 
                                            vsf_tgui_top_container_t *ptTop, 
                                            vsf_tgui_evt_t *ptEVT);

static void __vk_tgui_change_actived_control(
                                        vsf_tgui_t* ptGUI, 
                                        const vsf_tgui_control_t *ptControl);

/*============================ IMPLEMENTATION ================================*/

implement_vsf_rng_buf(__vsf_tgui_evt_queue_t, vsf_tgui_evt_t, __safe_atom_code)


vsf_err_t vk_tgui_init(vsf_tgui_t* ptGUI, const vsf_tgui_cfg_t* ptCFG)
{
    vsf_err_t tErr = VSF_ERR_NONE;
    class_internal(ptGUI, ptThis, vsf_tgui_t);

    VSF_TGUI_ASSERT(NULL != ptGUI);
    VSF_TGUI_ASSERT(NULL != ptCFG);

    do {
        if ((NULL == ptCFG->tEVTQueue.PTR.pObj)
            || (ptCFG->tEVTQueue.nSize <= sizeof(vsf_tgui_evt_t))
#if     VSF_TGUI_CFG_REFRESH_SCHEME == VSF_TGUI_REFRESH_SCHEME_LAYER_BY_LAYER
            || (NULL == ptCFG->tBFSQueue.PTR.pObj)
            || (ptCFG->tBFSQueue.nSize <= 0)
#endif
            ) {

            tErr = VSF_ERR_INVALID_PARAMETER;
            break;
        }

        //! initialise the queue
        vsf_rng_buf_prepare(__vsf_tgui_evt_queue_t,
            &this.tMSGQueue,
            (vsf_tgui_evt_t*)(ptCFG->tEVTQueue.PTR.pObj),
            ptCFG->tEVTQueue.nSize);

        do {
            const vsf_msgt_cfg_t cfg = {
                c_tControlInterfaces,
                UBOUND(c_tControlInterfaces),
            };
            vsf_msgt_init(&(this.use_as__vsf_msgt_t), &cfg);
        } while (0);

#if     VSF_TGUI_CFG_REFRESH_SCHEME == VSF_TGUI_REFRESH_SCHEME_LAYER_BY_LAYER
        vsf_msgt_forward_propagate_msg_bfs_init(
            &(this.use_as__vsf_msgt_t),
            (uint16_t*)(ptCFG->tBFSQueue.PTR.pObj),
            ptCFG->tBFSQueue.nSize, false);
#elif   VSF_TGUI_CFG_REFRESH_SCHEME == VSF_TGUI_REFRESH_SCHEME_Z_ORDER
        vsf_msgt_forward_propagate_msg_pre_order_traversal_init(
            &(this.use_as__vsf_msgt_t), 
            false);
#endif

        vsf_msgt_forward_propagate_msg_dfs_init(&(this.use_as__vsf_msgt_t));

        //! initialise consumer pt task
        do {
            this.tConsumer.param.ptQueue = &this.tMSGQueue;
            this.tConsumer.param.ptMSGTree = &this.use_as__vsf_msgt_t;

            if (NULL != ptCFG->ptRootNode) {
                vk_tgui_set_top_container(ptGUI, (vsf_tgui_top_container_t *)ptCFG->ptRootNode);
            }

            //! start pt task
            init_vsf_pt(__vsf_tgui_evt_shooter_t, &(this.tConsumer), ptCFG->tPriority);
        } while (0);

    } while (0);
    return tErr;
}


vsf_err_t vk_tgui_set_top_container(vsf_tgui_t* ptGUI, 
                                    vsf_tgui_top_container_t *ptRootNode)
{
    VSF_TGUI_ASSERT(NULL != ptGUI);
    VSF_TGUI_ASSERT(NULL != ptRootNode);

    if (ptRootNode == ptGUI->tConsumer.param.ptRootNode) {
        return VSF_ERR_ALREADY_EXISTS;
    }

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
    vsf_tgui_evt_t tEvent = { { {VSF_TGUI_EVT_ON_SET_TOP_CONTAINER} } };
    tEvent.use_as__vsf_tgui_msg_t.ptTarget = (vsf_tgui_control_t*)ptRootNode;
    return vk_tgui_send_message(ptGUI, tEvent);

#else
    return vk_tgui_send_message(ptGUI,
        (vsf_tgui_evt_t) {
        .tMSG = VSF_TGUI_EVT_ON_SET_TOP_CONTAINER,
            .ptTarget = (vsf_tgui_control_t*)ptRootNode,
    });
#endif
}

/*! \brief tgui msg queue producer */

bool vk_tgui_send_message(vsf_tgui_t* ptGUI, vsf_tgui_evt_t tEvent)
{
    class_internal(ptGUI, ptThis, vsf_tgui_t);
    if (NULL == ptGUI) {
        return false;
    }

    if (!vsf_rng_buf_send_one(__vsf_tgui_evt_queue_t,
        this.tConsumer.param.ptQueue,
        tEvent)) {
        return false;
    }

    __SAFE_ATOM_CODE(
        if (this.tConsumer.param.Attribute.bIsQueueDrain) {
            this.tConsumer.param.Attribute.bIsQueueDrain = false;
            //! wake pt task up
            vsf_eda_post_evt(&(this.tConsumer.use_as__vsf_pt_t.use_as__vsf_eda_t),
                VSF_TGUI_MSG_AVAILABLE);
        }
    )

    return true;
}

bool vk_tgui_update(vsf_tgui_t* ptGUI,
                    const vsf_tgui_control_t* ptTarget)
{

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
    vsf_tgui_evt_t tEvent = { { {VSF_TGUI_EVT_UPDATE} } };
    tEvent.use_as__vsf_tgui_msg_t.ptTarget = (vsf_tgui_control_t*)ptTarget;
    return vk_tgui_send_message(ptGUI, tEvent);

#else
    return vk_tgui_send_message(ptGUI,
        (vsf_tgui_evt_t) {
        .tMSG = VSF_TGUI_EVT_UPDATE,
            .ptTarget = (vsf_tgui_control_t*)ptTarget,
    });
#endif
}

bool vk_tgui_send_timer_event(  vsf_tgui_t* ptGUI,
                                const vsf_tgui_control_t* ptTarget)
{

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
    vsf_tgui_evt_t tEvent = { { {VSF_TGUI_EVT_ON_TIME} } };
    tEvent.use_as__vsf_tgui_msg_t.ptTarget = (vsf_tgui_control_t*)ptTarget;
    return vk_tgui_send_message(ptGUI, tEvent);

#else
    return vk_tgui_send_message(ptGUI,
        (vsf_tgui_evt_t) {
        .tMSG = VSF_TGUI_EVT_ON_TIME,
            .ptTarget = (vsf_tgui_control_t*)ptTarget,
    });
#endif
}

bool vk_tgui_update_tree(vsf_tgui_t* ptGUI,
                        const vsf_tgui_control_t* ptTarget)
{

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
    vsf_tgui_evt_t tEvent = { { {VSF_TGUI_EVT_UPDATE_TREE} } };
    tEvent.use_as__vsf_tgui_msg_t.ptTarget = (vsf_tgui_control_t*)ptTarget;
    return vk_tgui_send_message(ptGUI, tEvent);

#else
    return vk_tgui_send_message(ptGUI,
        (vsf_tgui_evt_t) {
        .tMSG = VSF_TGUI_EVT_UPDATE_TREE,
            .ptTarget = (vsf_tgui_control_t*)ptTarget,
    });
#endif
}

#if VSF_TGUI_CFG_REFRESH_SCHEME != VSF_TGUI_REFRESH_SCHEME_NONE

bool vk_tgui_refresh_ex(vsf_tgui_t* ptGUI,
                        const vsf_tgui_control_t* ptTarget,
                        const vsf_tgui_region_t* ptRegion)
{

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
    vsf_tgui_evt_t tEvent = { { {VSF_TGUI_EVT_REFRESH} } };
    tEvent.use_as__vsf_tgui_msg_t.ptTarget = (vsf_tgui_control_t*)ptTarget;
    tEvent.tRefreshEvt.ptRegion = ptRegion;
    return vk_tgui_send_message(ptGUI, tEvent);

#else
    return vk_tgui_send_message(ptGUI,
        (vsf_tgui_evt_t) {
        .tRefreshEvt = {
            .tMSG = VSF_TGUI_EVT_REFRESH,
            .ptTarget = (vsf_tgui_control_t*)ptTarget,
            .ptRegion = (vsf_tgui_region_t*)ptRegion,
        },
    });
#endif
}

bool vk_tgui_refresh(vsf_tgui_t* ptGUI)
{
    return vk_tgui_refresh_ex(ptGUI, NULL, NULL);
}
#endif



static fsm_rt_t __vsf_tgui_send_msg(vsf_tgui_t* ptGUI,
                                    const vsf_tgui_control_t* ptControl,
                                    vsf_tgui_evt_t* ptEvent)
{
    fsm_rt_t fsm_rt;
    vsf_msgt_t* ptMSGTree = &(ptGUI->use_as__vsf_msgt_t);
    do {
        fsm_rt = vsf_msgt_backward_propagate_msg(
            ptMSGTree,
            (const vsf_msgt_node_t*)ptControl,
            &ptEvent->use_as__vsf_tgui_msg_t.use_as__vsf_msgt_msg_t);
        if (fsm_rt_cpl == fsm_rt) {
            break;
        } else if (fsm_rt < 0) {
            //! msg is not handled
            break;
        } else if (fsm_rt >= fsm_rt_user) {
            break;
        }
    } while (1);

#if VSF_TGUI_CFG_REFRESH_SCHEME != VSF_TGUI_REFRESH_SCHEME_NONE
    if (fsm_rt >= 0) {
        //! update reference to the control which handles the message
        ptControl = (const vsf_tgui_control_t* )
        vsf_msgt_backward_propagate_msg_get_last_node(ptMSGTree);

        if (VSF_TGUI_MSG_RT_REFRESH == fsm_rt) {
        #if VSF_TGUI_CFG_SHOW_REFRESH_EVT_LOG == ENABLED
            VSF_TGUI_LOG(VSF_TRACE_WARNING, " \tRequest Refresh\r\n");
        #endif
            vk_tgui_refresh_ex(ptGUI, ptControl, NULL);
        } else if (VSF_TGUI_MSG_RT_REFRESH_PARENT == fsm_rt) {
        #if VSF_TGUI_CFG_SHOW_REFRESH_EVT_LOG == ENABLED
            VSF_TGUI_LOG(VSF_TRACE_WARNING, " \tRequest Refresh Parent\r\n");
        #endif
            if (NULL == ptControl->use_as__vsf_msgt_node_t.parent_ptr) {
                vk_tgui_refresh_ex(ptGUI, ptControl, NULL);
            } else {
                vk_tgui_refresh_ex( 
                    ptGUI, 
                    (const vsf_tgui_control_t *)ptControl->
                        use_as__vsf_msgt_node_t.parent_ptr, 
                    NULL);
            }
        }
    }
#endif
    
    return fsm_rt;
}


static void __vk_tgui_send_dfs_msg( vsf_msgt_t* ptMSGTree,
                                    const vsf_msgt_node_t* node_ptr,
                                    vsf_tgui_evt_t* ptEvent)
{
    do {
        fsm_rt_t fsm_rt = vsf_msgt_forward_propagate_msg_dfs(
            ptMSGTree,
            node_ptr,
            &(ptEvent->use_as__vsf_tgui_msg_t.use_as__vsf_msgt_msg_t));
        if (fsm_rt_cpl == fsm_rt) {
            break;
        } else if (fsm_rt < 0) {
            //! msg is not handled
            break;
        }
    } while (1);
}

static void __vk_tgui_send_bfs_msg( vsf_msgt_t* ptMSGTree,
                                    const vsf_msgt_node_t* node_ptr,
                                    vsf_tgui_evt_t* ptEvent,
                                    uint_fast8_t chStatusMask)
{
    do {
        fsm_rt_t fsm_rt = vsf_msgt_forward_propagate_msg_bfs(
            ptMSGTree,
            node_ptr,
            &(ptEvent->use_as__vsf_tgui_msg_t.use_as__vsf_msgt_msg_t),
            chStatusMask);
        if (fsm_rt_cpl == fsm_rt) {
            break;
        } else if (fsm_rt < 0) {
            //! msg is not handled
            break;
        }
    } while (1);
}

static void __vk_tgui_send_pot_msg( vsf_msgt_t* ptMSGTree,
                                    const vsf_msgt_node_t* node_ptr,
                                    vsf_tgui_evt_t* ptEvent,
                                    uint_fast8_t chStatusMask)
{
    do {
        fsm_rt_t fsm_rt = vsf_msgt_forward_propagate_msg_pre_order_traversal(
            ptMSGTree,
            node_ptr,
            &(ptEvent->use_as__vsf_tgui_msg_t.use_as__vsf_msgt_msg_t),
            chStatusMask);
        if (fsm_rt_cpl == fsm_rt) {
            break;
        } else if (fsm_rt < 0) {
            //! msg is not handled
            break;
        }
    } while (1);
}

static void __vk_tgui_set_focus(vsf_tgui_t* ptGUI,
                                const vsf_tgui_control_t* ptPrevious,
                                const vsf_tgui_control_t* ptCurrent,
                                vsf_evt_t tMSG)
{
    vsf_tgui_evt_t tTempEvent = {0};// = { { {VSF_TGUI_EVT_LOST_ACTIVE}} };
    tTempEvent.use_as__vsf_tgui_msg_t.use_as__vsf_msgt_msg_t.tMSG = tMSG+1;

    //! send deactivated message first to previous node
    if (NULL != ptPrevious) {
        __vsf_tgui_send_msg(ptGUI, ptPrevious, &tTempEvent);
    }

    if (NULL != ptCurrent) {
        //! send activated message to current node
        tTempEvent.use_as__vsf_tgui_msg_t.use_as__vsf_msgt_msg_t.tMSG = tMSG;

        __vsf_tgui_send_msg(ptGUI, ptCurrent, &tTempEvent);
    }
}

static void __vk_tgui_change_focus_control( vsf_tgui_t* ptGUI, 
                                            const vsf_tgui_control_t *ptControl,
                                            __vk_tgui_focus_t *ptFocus,
                                            vsf_evt_t tMSG)
{
    //class_internal(&(ptGUI->tConsumer.param), ptThis, vsf_pt(__vsf_tgui_evt_shooter_t));

    if (ptFocus->ptCurrent != ptControl) {
        ptFocus->ptPrevious = ptFocus->ptCurrent;
        ptFocus->ptCurrent = ptControl;

        __vk_tgui_set_focus(ptGUI,
            ptFocus->ptPrevious,
            ptFocus->ptCurrent,
            tMSG);
    }
}


const vsf_tgui_control_t *vsf_tgui_actived_control_get(vsf_tgui_t *ptGUI)
{
    VSF_TGUI_ASSERT(NULL != ptGUI);
    return ptGUI->tConsumer.param.tActivated.ptCurrent;
}

#if VSF_TGUI_CFG_SUPPORT_MOUSE == ENABLED
const vsf_tgui_control_t *vsf_tgui_pointed_control_get(vsf_tgui_t *ptGUI)
{
    VSF_TGUI_ASSERT(NULL != ptGUI);
    return ptGUI->tConsumer.param.tPointerAbove.ptCurrent;
}
#endif



void vsf_tgui_low_level_refresh_ready(vsf_tgui_t *ptGUI)
{
#if VSF_TGUI_CFG_REFRESH_SCHEME != VSF_TGUI_REFRESH_SCHEME_NONE
    VSF_TGUI_ASSERT(NULL != ptGUI);

    vsf_eda_post_evt(   &(ptGUI->tConsumer.use_as__vsf_pt_t.use_as__vsf_eda_t),
                        VSF_TGUI_MSG_LOW_LEVEL_READY_TO_REFRESH);
#endif
}

static bool __vk_tgui_decide_refresh_region(vsf_pt(__vsf_tgui_evt_shooter_t) *ptThis,
                                            const vsf_tgui_control_t *ptControl)
{
    bool result = true;
    this.ptRegion = NULL;

    if (!vsf_tgui_control_get_visible_region(ptControl, &this.tTempRegion)) {
        return false;
    }

    if (NULL != this.tEvent.tRefreshEvt.ptRegion) {
        result = vsf_tgui_region_intersect(&this.tTempRegion, &this.tTempRegion, this.tEvent.tRefreshEvt.ptRegion);
    }

    this.ptRegion = &this.tTempRegion;
    return result;
}


/*! \brief tgui msg queue consumer */
implement_vsf_pt(__vsf_tgui_evt_shooter_t)
{
    vsf_tgui_t* ptGUI = container_of(ptThis, vsf_tgui_t, tConsumer.param);
    bool result = false;
    vsf_evt_t   tMSG;
    vsf_pt_begin();

    while (1) {

loop_start:
        this.node_ptr = NULL;
        result = false;

        do {
            __SAFE_ATOM_CODE(
                result = vsf_rng_buf_get_one(__vsf_tgui_evt_queue_t, this.ptQueue, &this.tEvent);
                if (!result) {
                    this.Attribute.bIsQueueDrain = true;
                }
            )
            if (result) {
                break;
            }

            //! wait for new event arrival
            vsf_pt_wait_for_evt(VSF_TGUI_MSG_AVAILABLE);
        } while (1);

        tMSG = this.tEvent.use_as__vsf_tgui_msg_t.use_as__vsf_msgt_msg_t.tMSG;

        /*! \note Use user specific ptTarget, otherwise use RootNode */
        if (NULL != this.tEvent.use_as__vsf_tgui_msg_t.ptTarget) {
            /*! use user specified target */
            this.node_ptr = (const vsf_msgt_node_t*)this.tEvent.use_as__vsf_tgui_msg_t.ptTarget;
        } else if (NULL == this.ptRootNode) {
            continue;
        } else {
            this.node_ptr = (const vsf_msgt_node_t*)this.ptRootNode;
        }

        switch (tMSG & VSF_TGUI_MSG_MSK) {
            case VSF_TGUI_MSG_CONTROL_EVT& VSF_TGUI_MSG_MSK:

                switch (tMSG & VSF_TGUI_EVT_MSK) {

                    case VSF_TGUI_EVT_ON_SET_TOP_CONTAINER & VSF_TGUI_EVT_MSK:
                #if VSF_TGUI_CFG_SUPPORT_DESTRUCTOR_SCHEME == ENABLED
                        if (NULL != this.ptRootNode) {
                             __vk_tgui_depose_top_container(ptGUI, 
                                                            (vsf_tgui_top_container_t *)this.ptRootNode, 
                                                            NULL);
                        }
                #endif
                        this.ptRootNode = NULL;
                        this.tActivated.ptCurrent = NULL;
                        this.tActivated.ptPrevious = NULL;

                        do {
                            if (NULL == this.tEvent.use_as__vsf_tgui_msg_t.ptTarget) {
                                break;
                            } else if (!this.node_ptr->Attribute._.is_top) {
                                break;
                            }

                            //! set new top container
                            this.tActivated.ptCurrent = (const vsf_tgui_control_t *)this.node_ptr;
                            this.ptRootNode = (const vsf_tgui_top_container_t *)this.node_ptr;
                            ((vsf_tgui_top_container_t *)this.ptRootNode)->ptGUI = ptGUI;

                    #if VSF_TGUI_CFG_SUPPORT_CONSTRUCTOR_SCHEME == ENABLED
                            //! send on load message 
                            this.tEvent.use_as__vsf_tgui_msg_t.use_as__vsf_msgt_msg_t.tMSG = VSF_TGUI_EVT_ON_LOAD;

                    #   if VSF_TGUI_CFG_REFRESH_SCHEME == VSF_TGUI_REFRESH_SCHEME_BREADTH_FIRST_TRAVERSAL
                            vsf_msgt_forward_propagate_msg_bfs_setting(this.ptMSGTree, false);
                            /*! constructor message */
                            __vk_tgui_send_bfs_msg( this.ptMSGTree, 
                                                    this.node_ptr, 
                                                    &this.tEvent,
                                                    0);
                    #   elif VSF_TGUI_CFG_REFRESH_SCHEME == VSF_TGUI_REFRESH_SCHEME_PRE_ORDER_TRAVERSAL
                            vsf_msgt_forward_propagate_msg_pre_order_traversal_setting(this.ptMSGTree, false);
                            /*! constructor message */
                            __vk_tgui_send_pot_msg( this.ptMSGTree, 
                                                    this.node_ptr, 
                                                    &this.tEvent,
                                                    0);
                    #   endif
                    #endif

                            this.tEvent.use_as__vsf_tgui_msg_t.use_as__vsf_msgt_msg_t.tMSG = VSF_TGUI_EVT_UPDATE_TREE;
                            __vk_tgui_send_dfs_msg(this.ptMSGTree, this.node_ptr, &this.tEvent);

                        } while(0);
                        goto loop_start;

                #if VSF_TGUI_CFG_SUPPORT_CONSTRUCTOR_SCHEME == ENABLED
                    case VSF_TGUI_EVT_ON_LOAD & VSF_TGUI_EVT_MSK:
                #   if VSF_TGUI_CFG_REFRESH_SCHEME == VSF_TGUI_REFRESH_SCHEME_BREADTH_FIRST_TRAVERSAL
                        vsf_msgt_forward_propagate_msg_bfs_setting(this.ptMSGTree, false);
                        /*! constructor message */
                        __vk_tgui_send_bfs_msg( this.ptMSGTree, 
                                                this.node_ptr, 
                                                &this.tEvent,
                                                0);
                #   elif VSF_TGUI_CFG_REFRESH_SCHEME == VSF_TGUI_REFRESH_SCHEME_PRE_ORDER_TRAVERSAL
                        vsf_msgt_forward_propagate_msg_pre_order_traversal_setting(this.ptMSGTree, false);
                        /*! constructor message */
                        __vk_tgui_send_pot_msg( this.ptMSGTree, 
                                                this.node_ptr, 
                                                &this.tEvent,
                                                0);
                #   endif
                        goto loop_start;
                #endif 


                #if VSF_TGUI_CFG_SUPPORT_DESTRUCTOR_SCHEME == ENABLED
                    case VSF_TGUI_EVT_ON_DEPOSE & VSF_TGUI_EVT_MSK: {
                        __vk_tgui_depose_top_container(ptGUI, (vsf_tgui_top_container_t *)this.node_ptr, &this.tEvent);

                        goto loop_start;
                    }
                #endif
                    case VSF_TGUI_EVT_ON_TIME & VSF_TGUI_EVT_MSK:
                        __vsf_tgui_send_msg(ptGUI, (const vsf_tgui_control_t*)this.node_ptr, &this.tEvent);
                        break;

                    case VSF_TGUI_EVT_UPDATE & VSF_TGUI_EVT_MSK:

                        if (!vsf_tgui_control_status_get((vsf_tgui_control_t*)this.node_ptr).tValues.__bContainBuiltInStructure) {
                            //! send message
                            __vsf_tgui_send_msg(ptGUI, (const vsf_tgui_control_t*)this.node_ptr, &this.tEvent);
                            //goto loop_start;
                            break;
                        }
                        //this.tEvent.use_as__vsf_tgui_msg_t.use_as__vsf_msgt_msg_t.tMSG = VSF_TGUI_EVT_UPDATE_TREE;
                        // fall through

                    case VSF_TGUI_EVT_UPDATE_TREE & VSF_TGUI_EVT_MSK:
                        //! send message
                        __vk_tgui_send_dfs_msg(this.ptMSGTree, this.node_ptr, &this.tEvent);
                        goto loop_start;

        #if VSF_TGUI_CFG_REFRESH_SCHEME != VSF_TGUI_REFRESH_SCHEME_NONE
                    case VSF_TGUI_EVT_REFRESH & VSF_TGUI_EVT_MSK:
                        if (NULL != this.tEvent.use_as__vsf_tgui_msg_t.ptTarget) {
                            /*! use user specified target */
                            /*! do not refresh it if the control doesn't belong to current top container*/
                            if (this.ptRootNode != vk_tgui_control_get_top((const vsf_tgui_control_t*)this.node_ptr)) {
                                goto loop_start;
                            }
                        }

                        if (!__vk_tgui_decide_refresh_region(ptThis, (const vsf_tgui_control_t *)this.node_ptr)) {
                            goto loop_start;
                        }

        #   if   VSF_TGUI_CFG_SUPPORT_TRANSPARENT_CONTROL == ENABLED
                        bool bIsRequestRefreshParent = false;

                        //! if the target control is transparent, refresh its parent
                        do {
                            __vsf_tgui_control_core_t* ptCore = vsf_tgui_control_get_core((vsf_tgui_control_t*)this.node_ptr);
                            if (ptCore->tStatus.tValues.bIsControlTransparent) {
                                //! try to fetch its parent
                                if (NULL != this.node_ptr->parent_ptr) {
                                    this.node_ptr = (const vsf_msgt_node_t*)this.node_ptr->parent_ptr;
                                    bIsRequestRefreshParent = true;
                                    continue;
                                } 
                            }
                            break;
                        } while (true);

                        if (bIsRequestRefreshParent) {
                            const vsf_tgui_container_t *container_ptr = (const vsf_tgui_container_t *)this.node_ptr;
                            if (container_ptr->tContainerAttribute.bIsForceRefreshWholeBackground) {
                                if (!__vk_tgui_decide_refresh_region(ptThis, (const vsf_tgui_control_t *)container_ptr)) {
                                    goto loop_start;
                                }
                            }
                        }
        #   endif
                        
                        goto refresh_loop;
        #endif

                    case VSF_TGUI_EVT_GET_ACTIVE& VSF_TGUI_EVT_MSK: 
                        __vk_tgui_change_focus_control( ptGUI, 
                                                        (const vsf_tgui_control_t *)this.node_ptr, 
                                                        &this.tActivated, 
                                                        VSF_TGUI_EVT_GET_ACTIVE);
                        goto loop_start;

                    default:
                    case VSF_TGUI_EVT_LOST_ACTIVE& VSF_TGUI_EVT_MSK:
                        //case VSF_TGUI_EVT_ON_LOAD & VSF_TGUI_EVT_MSK:
                            //! message we don't need to handle
                        goto loop_start;
                }
                break;

            case VSF_TGUI_MSG_POINTER_EVT & VSF_TGUI_MSG_MSK:

                /*! \note specified target will not change current activated control*/
                if (NULL != this.tEvent.use_as__vsf_tgui_msg_t.ptTarget) {
                    break;
                }

                //! send message to currently activated control
                this.node_ptr = &(this.tActivated.ptCurrent->use_as__vsf_msgt_node_t);
                VSF_TGUI_ASSERT( NULL != this.node_ptr );
                

                //! pointer up event could only be sent to active node
                if (VSF_TGUI_EVT_POINTER_DOWN == tMSG) {

                    //! check new target of activation
                    this.node_ptr = vsf_msgt_shoot_node(
                        this.ptMSGTree,
                        (const vsf_msgt_node_t*)this.ptRootNode,
                        (uintptr_t) & (this.tEvent.tPointerEvt.use_as__vsf_tgui_location_t));
                

                    if (NULL == this.node_ptr) {
                        //! missed all node (control)
                        goto loop_start;
                    }

                #if VSF_TGUI_CFG_SUPPORT_MOUSE == ENABLED
                    __vk_tgui_change_focus_control( ptGUI, 
                                    (const vsf_tgui_control_t *)this.node_ptr, 
                                    &this.tPointerAbove, 
                                    VSF_TGUI_EVT_POINTER_ENTER);
                #endif

                    __vk_tgui_change_focus_control( ptGUI, 
                                    (const vsf_tgui_control_t *)this.node_ptr, 
                                    &this.tActivated, 
                                    VSF_TGUI_EVT_GET_ACTIVE);
                }
            #if VSF_TGUI_CFG_SUPPORT_MOUSE == ENABLED
                else if (VSF_TGUI_EVT_POINTER_MOVE == tMSG) {
                    //! check new target of activation
                    this.node_ptr = vsf_msgt_shoot_node(
                        this.ptMSGTree,
                        (const vsf_msgt_node_t*)this.ptRootNode,
                        (uintptr_t) & (this.tEvent.tPointerEvt.use_as__vsf_tgui_location_t));

                    __vk_tgui_change_focus_control( ptGUI, 
                                    (const vsf_tgui_control_t *)this.node_ptr, 
                                    &this.tPointerAbove, 
                                    VSF_TGUI_EVT_POINTER_ENTER);

                #if VSF_TGUI_CFG_SUPPORT_MOUSE_MOVE_HANDLING == ENABLED
                    if (NULL == this.node_ptr) {
                        //! missed all node (control)
                        goto loop_start;
                    }
                #else
                    goto loop_start;
                #endif
                }
            #endif
                break;

            case VSF_TGUI_MSG_KEY_EVT & VSF_TGUI_MSG_MSK:
            case VSF_TGUI_MSG_GESTURE_EVT& VSF_TGUI_MSG_MSK:
            case VSF_TGUI_MSG_CONTROL_SPECIFIC_EVT & VSF_TGUI_MSG_MSK:
                if (NULL == this.tEvent.use_as__vsf_tgui_msg_t.ptTarget) {
                    //! send message to currently activated control
                    this.node_ptr = &(this.tActivated.ptCurrent->use_as__vsf_msgt_node_t);
                    VSF_TGUI_ASSERT( NULL != this.node_ptr );
                }
                break;

            default:
                //! message we don't need to handle
                goto loop_start;
        }

        //! default message handling
        __vsf_tgui_send_msg(ptGUI, (const vsf_tgui_control_t*)this.node_ptr, &this.tEvent);

        continue;

    #if VSF_TGUI_CFG_REFRESH_SCHEME != VSF_TGUI_REFRESH_SCHEME_NONE
refresh_loop :
        do {
            vsf_tgui_region_t *ptRegion = NULL;

            do {
                ptRegion = vsf_tgui_v_refresh_loop_begin(ptGUI, this.ptRegion);
                if (NULL == ptRegion) {
                    /*! wait until it is ready to refresh */
                    vsf_pt_wait_for_evt(VSF_TGUI_MSG_LOW_LEVEL_READY_TO_REFRESH);
                    ptRegion = NULL;
                }
            } while(NULL == ptRegion);
            this.ptRegion = ptRegion;

            /* this statement is very important, do not remove */
            this.tEvent.tRefreshEvt.ptRegion = (vsf_tgui_region_t*)this.ptRegion;

            //! set first refresh node bit
            do {
                __vsf_tgui_control_core_t* ptCore =
                    vsf_tgui_control_get_core((const vsf_tgui_control_t*)this.node_ptr);
                ptCore->tStatus.tValues.__bIsTheFirstRefreshNode = true;
            } while (0);

        #if VSF_TGUI_CFG_REFRESH_SCHEME == VSF_TGUI_REFRESH_SCHEME_BREADTH_FIRST_TRAVERSAL
            vsf_msgt_forward_propagate_msg_bfs_setting(this.ptMSGTree, true);
            __vk_tgui_send_bfs_msg( this.ptMSGTree, 
                                    this.node_ptr, 
                                    &this.tEvent,
                                    (   VSF_TGUI_CTRL_STATUS_INITIALISED
                                    |   VSF_TGUI_CTRL_STATUS_ENABLED
                                    |   VSF_TGUI_CTRL_STATUS_VISIBLE
                                    ));
        #elif VSF_TGUI_CFG_REFRESH_SCHEME == VSF_TGUI_REFRESH_SCHEME_PRE_ORDER_TRAVERSAL
            vsf_msgt_forward_propagate_msg_pre_order_traversal_setting(this.ptMSGTree, true);
            __vk_tgui_send_pot_msg( this.ptMSGTree, 
                                    this.node_ptr, 
                                    &this.tEvent,
                                    (   VSF_TGUI_CTRL_STATUS_INITIALISED
                                    |   VSF_TGUI_CTRL_STATUS_ENABLED
                                    |   VSF_TGUI_CTRL_STATUS_VISIBLE
                                    ));
        #endif
        } while (vsf_tgui_v_refresh_loop_end(ptGUI));
    #endif
    }

    vsf_pt_end();
}


static void __vk_tgui_depose_top_container( vsf_tgui_t* ptGUI, 
                                            vsf_tgui_top_container_t *ptTop, 
                                            vsf_tgui_evt_t *ptEVT)
{
    vsf_tgui_evt_t tEvent;
    class_internal(&(ptGUI->tConsumer.param), ptThis, vsf_pt(__vsf_tgui_evt_shooter_t));

    if (!ptTop->use_as__vsf_tgui_container_t.
            use_as__vsf_msgt_container_t.
                use_as__vsf_msgt_node_t.Attribute._.is_top) {
        //! we only deal with top node
        return ;
    }

    if (NULL == ptEVT) {
        ptEVT = &tEvent;
        tEvent.use_as__vsf_tgui_msg_t.use_as__vsf_msgt_msg_t.tMSG = VSF_TGUI_EVT_ON_DEPOSE;
        tEvent.use_as__vsf_tgui_msg_t.ptTarget = (vsf_tgui_control_t *)ptTop;  
    }

    //! send message
    __vk_tgui_send_dfs_msg(this.ptMSGTree, (const vsf_msgt_node_t *)ptTop, ptEVT);

    //! remove reference to tGUI obj
    do {
        ptTop->ptGUI = NULL;
        if (ptTop == this.ptRootNode) {
            this.ptRootNode = NULL;
        }
    } while(0);
}



#endif


/* EOF */
