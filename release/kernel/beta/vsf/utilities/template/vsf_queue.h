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

#ifndef __VSF_RING_BUFFER_H__
#define __VSF_RING_BUFFER_H__

/* example:

    // 0. Include vsf header file
    #include "vsf.h"

    // 1. Declare the ring buffer
    declare_vsf_rng_buf(<ring buffer name>);

    // 2. Defining your ring buffer variable with specific type
    def_vsf_rng_buf(<ring buffer name>, <item type>)

    // 3. Implement your ring buffer with specific atom access protection macro:
    implement_vsf_rng_buf(<ring buffer name>, <item type>, <atom access macro>)
    
    NOTE: You can use __SAFE_ATOM_CODE for interrupt, use __vsf_sched_safe for
          scheduler, use NO_RNG_BUF_PROTECT for no protection

    // 4. Defining your ring buffer variable
    NO_INIT static <ring buffer name> <ring buffer var>;

    // 5. Initialise a ring buffer with specific ring buffer buffer 
    vsf_rng_buf_init( <ring buffer name>, <item type>, <address of ring buffer var>, <item count>);

    // 6. If you want to initialise a ring buffer with a given buffer, use vsf_rng_buf_prepare
          rather than vsf_rng_buf_init above:
    vsf_rng_buf_prepare(  <ring buffer name>, 
                        <address of ring buffer var>, 
                        <address of buffer>, 
                        <size of ring buffer buffer> );

    // 7. Use following macro for enqueue, dequeue and peek one item:
    vsf_rng_buf_send(  <ring buffer name>, 
                        <address of ring buffer var>, 
                        <Item>);
    vsf_rng_buf_get(  <ring buffer name>, 
                        <address of ring buffer var>, 
                        <address of item buffer>);
    vsf_rng_buf_peek(     <ring buffer name>, 
                        <address of ring buffer var>, 
                        <address of item pointer>);

    NOTE: Peek returns a refrence to existing data in the ring buffer, there is no copy access

    // 8. Use following macro for enqueue, dequeue and peek multile items:
    vsf_rng_buf_send(  <ring buffer name>, 
                        <address of ring buffer var>, 
                        <address of item buffer>, 
                        <number of Items>);
    vsf_rng_buf_get(  <ring buffer name>, 
                        <address of ring buffer var>, 
                        <address of item buffer>, 
                        <number of Items>);
    vsf_rng_buf_peek(     <ring buffer name>, 
                        <address of ring buffer var>, 
                        <address of item pointer>, 
                        <number of Items>);

    NOTE: Peek returns a refrence to existing data in the ring buffer, there is no copy access

    // 9. You can get the number of items within a ring buffer:
    vsf_rng_buf_count( <ring buffer name>, <address of ring buffer var> )

    // 10. You can get the number of peekable items withi a ring buffer
    vsf_rng_buf_peekable_count( <ring buffer name>, <address of ring buffer var> )

    Example:


    declare_vsf_rng_buf(my_hword_queue_t)
    def_vsf_rng_buf(my_hword_queue_t, uint16_t)

    implement_vsf_rng_buf(my_hword_queue_t, uint16_t, NO_RNG_BUF_PROTECT)


    void vsf_ring_buffer_example(void)
    {
        NO_INIT static my_hword_queue_t s_tQueue;

        static uint16_t s_hwTemp[] = { 0x1234, 0x5678 };
        static uint16_t s_hwItem;

        //NO_INIT static uint8_t s_chQueueBuffer[1024];
        //vsf_rng_buf_prepare(my_hword_queue_t, &s_tQueue, &s_chQueueBuffer, sizeof(s_chQueueBuffer));

        vsf_rng_buf_init(my_hword_queue_t, uint16_t, &s_tQueue, 32);

        vsf_rng_buf_send(my_hword_queue_t, &s_tQueue, 0x1234);


        vsf_rng_buf_send(my_hword_queue_t, &s_tQueue, s_hwTemp, UBOUND(s_hwTemp));

        vsf_rng_buf_get(my_hword_queue_t, &s_tQueue, s_hwTemp, UBOUND(s_hwTemp));
        vsf_rng_buf_get(my_hword_queue_t, &s_tQueue, &s_hwItem);

        const uint16_t* phwItemRef = NULL;
        vsf_rng_buf_peek(my_hword_queue_t, &s_tQueue, &phwItemRef, 2);    
        vsf_rng_buf_peek(my_hword_queue_t, &s_tQueue, &phwItemRef);
    }

 */

/*============================ INCLUDES ======================================*/

/*! \NOTE: Make sure #include "utilities/ooc_class.h" is close to the class
 *!        definition and there is NO ANY OTHER module-interface-header file
 *!        included in this file
 */
#define __PLOOC_CLASS_USE_STRICT_TEMPLATE__

#if     defined(__VSF_QUEUE_CLASS_IMPLEMENT)
#   define __PLOOC_CLASS_IMPLEMENT
#   undef __VSF_QUEUE_CLASS_IMPLEMENT
#elif   defined(__VSF_QUEUE_CLASS_INHERIT)
#   define __PLOOC_CLASS_INHERIT
#   undef __VSF_QUEUE_CLASS_INHERIT
#endif   

#include "utilities/ooc_class.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/



#define __declare_vsf_rng_buf(__NAME)                                           \
    typedef struct __NAME __NAME;                                               \
    typedef struct __NAME##_cfg_t __NAME##_cfg_t;
#define declare_vsf_rng_buf(__NAME)       __declare_vsf_rng_buf(__NAME)


#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L

#   define NO_RNG_BUF_PROTECT(__CODE)               __CODE
#   define no_rng_buf_protect(__code)               __code

#   define __def_vsf_rng_buf(__NAME, __TYPE)                                    \
    struct __NAME {                                                             \
        implement(vsf_rng_buf_t)                                                \
        __TYPE *ptBuffer;                                                       \
    };                                                                          \
                                                                                \
    struct __NAME##_cfg_t {                                                     \
        __TYPE *ptBuffer;                                                       \
        uint16_t hwSize;                                                        \
        bool     bInitAsFull;                                                   \
    };                                                                          \
                                                                                \
extern                                                                          \
void __NAME##_init(__NAME* ptQ, __NAME##_cfg_t* ptCFG);                         \
                                                                                \
extern                                                                          \
bool __NAME##_send_one(__NAME* ptQ, __TYPE tItem);                              \
                                                                                \
extern                                                                          \
bool __NAME##_get_one(__NAME* ptQ, __TYPE* ptItem);                             \
                                                                                \
SECTION(".text." #__NAME "_item_count")                                         \
extern                                                                          \
uint_fast16_t __NAME##_item_count(__NAME* ptQ);                                 \
                                                                                \
SECTION(".text." #__NAME "_send_multiple")                                      \
extern                                                                          \
int32_t __NAME##_send_multiple(__NAME* ptQ,                                     \
                                        const __TYPE* ptItem,                   \
                                        uint16_t hwCount);                      \
                                                                                \
SECTION(".text." #__NAME "_get_multiple")                                       \
extern                                                                          \
int32_t __NAME##_get_multiple(  __NAME* ptQ,                                    \
                                    __TYPE* ptItem,                             \
                                    uint16_t hwCount);                          \
                                                                                \
SECTION(".text." #__NAME "_peek_one")                                           \
extern                                                                          \
bool __NAME##_peek_one(__NAME* ptQ, const __TYPE** pptItem);                    \
                                                                                \
SECTION(".text." #__NAME "_item_count_peekable")                                \
extern                                                                          \
uint_fast16_t __NAME##_item_count_peekable(__NAME* ptQ);                        \
                                                                                \
SECTION(".text." #__NAME "_peek_multiple")                                      \
extern                                                                          \
int32_t __NAME##_peek_multiple( __NAME* ptQ,                                    \
                                const __TYPE** pptItem,                         \
                                uint16_t hwCount);

#   define def_vsf_rng_buf(__NAME, __TYPE)                                      \
            __def_vsf_rng_buf(__NAME, __TYPE) 

#else
#   define NO_RNG_BUF_PROTECT(...)               __VA_ARGS__
#   define no_rng_buf_protect(...)               __VA_ARGS__

#   define __def_vsf_rng_buf(__NAME, __TYPE, ...)                               \
    struct __NAME {                                                             \
        implement(vsf_rng_buf_t)                                                \
        __TYPE *ptBuffer;                                                       \
        __VA_ARGS__                                                             \
    };                                                                          \
                                                                                \
    struct __NAME##_cfg_t {                                                     \
        __TYPE *ptBuffer;                                                       \
        uint16_t hwSize;                                                        \
        bool     bInitAsFull;                                                   \
    };                                                                          \
                                                                                \
extern                                                                          \
void __NAME##_init(__NAME* ptQ, __NAME##_cfg_t* ptCFG);                         \
                                                                                \
extern                                                                          \
bool __NAME##_send_one(__NAME* ptQ, __TYPE tItem);                              \
                                                                                \
extern                                                                          \
bool __NAME##_get_one(__NAME* ptQ, __TYPE* ptItem);                             \
                                                                                \
SECTION(".text." #__NAME "_item_count")                                         \
extern                                                                          \
uint_fast16_t __NAME##_item_count(__NAME* ptQ);                                 \
                                                                                \
SECTION(".text." #__NAME "_send_multiple")                                      \
extern                                                                          \
int32_t __NAME##_send_multiple(__NAME* ptQ,                                     \
                                        const __TYPE* ptItem,                   \
                                        uint16_t hwCount);                      \
                                                                                \
SECTION(".text." #__NAME "_get_multiple")                                       \
extern                                                                          \
int32_t __NAME##_get_multiple(  __NAME* ptQ,                                    \
                                    __TYPE* ptItem,                             \
                                    uint16_t hwCount);                          \
                                                                                \
SECTION(".text." #__NAME "_peek_one")                                           \
extern                                                                          \
bool __NAME##_peek_one(__NAME* ptQ, const __TYPE** pptItem);                    \
                                                                                \
SECTION(".text." #__NAME "_item_count_peekable")                                \
extern                                                                          \
uint_fast16_t __NAME##_item_count_peekable(__NAME* ptQ);                        \
                                                                                \
SECTION(".text." #__NAME "_peek_multiple")                                      \
extern                                                                          \
int32_t __NAME##_peek_multiple( __NAME* ptQ,                                    \
                                const __TYPE** pptItem,                         \
                                uint16_t hwCount);

#   define def_vsf_rng_buf(__NAME, __TYPE, ...)                                 \
            __def_vsf_rng_buf(__NAME, __TYPE, __VA_ARGS__) 

#endif

#define __implement_vsf_rng_buf(__NAME, __TYPE, __QUEUE_PROTECT)                \
void __NAME##_init(__NAME* ptQ, __NAME##_cfg_t* ptCFG)                          \
{                                                                               \
    ASSERT(NULL != ptQ && NULL != ptCFG);                                       \
    ASSERT(NULL != ptCFG->ptBuffer);                                            \
    ASSERT(ptCFG->hwSize >= sizeof(__TYPE));                                    \
    ptQ->ptBuffer = ptCFG->ptBuffer;                                            \
    __vsf_rng_buf_init(&(ptQ->use_as__vsf_rng_buf_t),                           \
        ptCFG->hwSize / sizeof(__TYPE),                                         \
        ptCFG->bInitAsFull);                                                    \
}                                                                               \
                                                                                \
bool __NAME##_send_one(__NAME *ptQ, __TYPE tItem)                               \
{                                                                               \
    bool bResult = false;                                                       \
    ASSERT(NULL != ptQ);                                                        \
    __QUEUE_PROTECT(                                                            \
        do {                                                                    \
            int32_t nIndex =                                                    \
                __vsf_rng_buf_send_one(&(ptQ->use_as__vsf_rng_buf_t));          \
            if (nIndex < 0) {                                                   \
                break;                                                          \
            }                                                                   \
            ptQ->ptBuffer[nIndex] = tItem;                                      \
            bResult = true;                                                     \
        } while(0);                                                             \
    )                                                                           \
                                                                                \
    return bResult;                                                             \
}                                                                               \
                                                                                \
bool __NAME##_get_one(__NAME * ptQ, __TYPE *ptItem)                             \
{                                                                               \
    bool bResult = false;                                                       \
    ASSERT(NULL != ptQ);                                                        \
    __QUEUE_PROTECT(                                                            \
        do {                                                                    \
            int32_t nIndex =                                                    \
                __vsf_rng_buf_get_one(&(ptQ->use_as__vsf_rng_buf_t));           \
            if (nIndex < 0) {                                                   \
                break;                                                          \
            }                                                                   \
            if (NULL != ptItem) {                                               \
                *ptItem = ptQ->ptBuffer[nIndex];                                \
            }                                                                   \
            bResult = true;                                                     \
        } while (0);                                                            \
    )                                                                           \
                                                                                \
    return bResult;                                                             \
}                                                                               \
                                                                                \
SECTION(".text." #__NAME "_item_count")                                         \
uint_fast16_t __NAME##_item_count(__NAME * ptQ)                                 \
{                                                                               \
    ASSERT(NULL != ptQ);                                                        \
    return __vsf_rng_buf_item_count(&(ptQ->use_as__vsf_rng_buf_t));             \
}                                                                               \
                                                                                \
SECTION(".text." #__NAME "_send_multiple")                                      \
int32_t __NAME##_send_multiple(  __NAME * ptQ,                                  \
                                    const __TYPE*ptItem,                        \
                                    uint16_t hwCount)                           \
{                                                                               \
    int32_t nResult = -1, nIndex = 0;                                           \
    ASSERT(NULL != ptQ);                                                        \
    __QUEUE_PROTECT(                                                            \
        do {                                                                    \
            if (NULL == ptItem || 0 == hwCount) {                               \
                break;                                                          \
            }                                                                   \
            nIndex =                                                            \
                __vsf_rng_buf_send_multiple(   &(ptQ->use_as__vsf_rng_buf_t),   \
                                                &hwCount);                      \
            if (nIndex < 0) {                                                   \
                break;                                                          \
            }                                                                   \
                                                                                \
            memcpy(&(ptQ->ptBuffer[nIndex]), ptItem, hwCount * sizeof(__TYPE)); \
            nResult = hwCount;                                                  \
        } while(0);                                                             \
    )                                                                           \
    return nResult;                                                             \
}                                                                               \
                                                                                \
SECTION(".text." #__NAME "_get_multiple")                                       \
int32_t __NAME##_get_multiple(  __NAME * ptQ,                                   \
                                    __TYPE* ptItem,                             \
                                    uint16_t hwCount)                           \
{                                                                               \
    int32_t nResult = -1, nIndex = 0;                                           \
    ASSERT(NULL != ptQ);                                                        \
    __QUEUE_PROTECT(                                                            \
        do {                                                                    \
            if (NULL == ptItem || 0 == hwCount) {                               \
                break;                                                          \
            }                                                                   \
            nIndex =                                                            \
                __vsf_rng_buf_get_multiple(   &(ptQ->use_as__vsf_rng_buf_t),    \
                                                &hwCount);                      \
            if (nIndex < 0) {                                                   \
                break;                                                          \
            }                                                                   \
                                                                                \
            memcpy(ptItem, &(ptQ->ptBuffer[nIndex]), hwCount * sizeof(__TYPE)); \
            nResult = hwCount;                                                  \
        } while(0);                                                             \
    )                                                                           \
    return nResult;                                                             \
}                                                                               \
                                                                                \
SECTION(".text." #__NAME "_peek_one")                                           \
bool __NAME##_peek_one(__NAME *ptQ, const __TYPE** pptItem)                     \
{                                                                               \
    bool bResult = false;                                                       \
    ASSERT(NULL != ptQ);                                                        \
    __QUEUE_PROTECT(                                                            \
        do {                                                                    \
            int32_t nIndex = __vsf_rng_buf_peek_one(&(ptQ->use_as__vsf_rng_buf_t)); \
            if (nIndex < 0) {                                                   \
                break;                                                          \
            }                                                                   \
            if (NULL != pptItem) {                                              \
                (*pptItem) = (const __TYPE*)&ptQ->ptBuffer[nIndex];             \
            }                                                                   \
            bResult = true;                                                     \
        } while (0);                                                            \
    )                                                                           \
                                                                                \
    return bResult;                                                             \
}                                                                               \
                                                                                \
SECTION(".text." #__NAME "_item_count_peekable")                                \
uint_fast16_t __NAME##_item_count_peekable(__NAME *ptQ)                         \
{                                                                               \
    ASSERT(NULL != ptQ);                                                        \
    return __vsf_rng_buf_item_count_peekable(&ptQ->use_as__vsf_rng_buf_t);      \
}                                                                               \
                                                                                \
SECTION(".text." #__NAME "_peek_multiple")                                      \
int32_t __NAME##_peek_multiple( __NAME * ptQ,                                   \
                                const __TYPE** pptItem,                         \
                                uint16_t hwCount)                               \
{                                                                               \
    int32_t nResult = -1, nIndex = 0;                                           \
    ASSERT(NULL != ptQ);                                                        \
    __QUEUE_PROTECT(                                                            \
        do {                                                                    \
            if (NULL == pptItem || 0 == hwCount) {                              \
                break;                                                          \
            }                                                                   \
            nIndex =                                                            \
                __vsf_rng_buf_peek_multiple(  &(ptQ->use_as__vsf_rng_buf_t),    \
                                            &hwCount);                          \
            if (nIndex < 0) {                                                   \
                break;                                                          \
            }                                                                   \
                                                                                \
/*memcpy(ptItem, &(ptQ->ptBuffer[nIndex]), hwCount * sizeof(__TYPE));*/         \
            (*pptItem) = (const __TYPE*)&(ptQ->ptBuffer[nIndex]);               \
            nResult = hwCount;                                                  \
        } while(0);                                                             \
    )                                                                           \
    return nResult;                                                             \
}


#define implement_vsf_rng_buf(__NAME, __TYPE, __QUEUE_PROTECT)                  \
            __implement_vsf_rng_buf(__NAME, __TYPE, __QUEUE_PROTECT) 


#define __vsf_rng_buf_init(__NAME, __TYPE, __QADDR, __ITEM_COUNT)               \
    do {                                                                        \
        NO_INIT static uint16_t s_hwBuffer[(__ITEM_COUNT)];                     \
        __NAME##_cfg_t tCFG = {                                                 \
            s_hwBuffer,                                                         \
            sizeof(s_hwBuffer),                                                 \
        };                                                                      \
        __NAME##_init((__QADDR), & tCFG);                                       \
    } while(0)

#define vsf_rng_buf_init(__NAME, __TYPE, __QADDR, __ITEM_COUNT)                 \
        __vsf_rng_buf_init(__NAME, __TYPE, (__QADDR), (__ITEM_COUNT))



#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L

#define __vsf_rng_buf_prepare(__NAME, __QADDR, __BUFFER, __SIZE)                \
    do {                                                                        \
        __NAME##_cfg_t tCFG = {0};                                              \
        tCFG.ptBuffer = (__BUFFER);                                             \
        tCFG.hwSize = (__SIZE);                                                 \
        __NAME##_init((__QADDR), & tCFG);                                       \
    } while(0)

#   define vsf_rng_buf_prepare(__NAME, __QADDR, __BUFFER, __SIZE)               \
                __vsf_rng_buf_prepare(__NAME, (__QADDR), (__BUFFER), (__SIZE))
#else

#   define __vsf_rng_buf_prepare(__NAME, __QADDR, __BUFFER, __SIZE, ...)        \
    do {                                                                        \
        __NAME##_cfg_t tCFG = {                                                 \
            (__BUFFER),                                                         \
            (__SIZE),                                                           \
            __VA_ARGS__                                                         \
        };                                                                      \
        __NAME##_init((__QADDR), & tCFG);                                       \
    } while(0)

#   define vsf_rng_buf_prepare(__NAME, __QADDR, __BUFFER, __SIZE, ...)          \
        __vsf_rng_buf_prepare(__NAME, (__QADDR), (__BUFFER), (__SIZE), __VA_ARGS__)
#endif

#define __vsf_rng_buf_send_1(__NAME, __QADDR, __ITEM)                           \
            __NAME##_send_one((__QADDR), __ITEM)

#define __vsf_rng_buf_send_2(__NAME, __QADDR, __BUFFER, __SIZE)                 \
            __NAME##_send_multiple((__QADDR), (__BUFFER), (__SIZE))

#define __vsf_rng_buf_get_1(__NAME, __QADDR, __ITEM)                            \
            __NAME##_get_one((__QADDR), __ITEM)

#define __vsf_rng_buf_get_2(__NAME, __QADDR, __BUFFER, __SIZE)                  \
            __NAME##_get_multiple((__QADDR), (__BUFFER), (__SIZE))

#define __vsf_rng_buf_peek_1(__NAME, __QADDR, __ITEM)                           \
            __NAME##_peek_one((__QADDR), __ITEM)

#define __vsf_rng_buf_peek_2(__NAME, __QADDR, __BUFFER, __SIZE)                 \
            __NAME##_peek_multiple((__QADDR), (__BUFFER), (__SIZE))

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#   define vsf_rng_buf_send(__NAME, __QADDR, ...)                               \
            __PLOOC_EVAL(__VSF_RNG_BUF_SEND_, __VA_ARGS__)                      \
                (__NAME, __QADDR, __VA_ARGS__)

#   define vsf_rng_buf_get(__NAME, __QADDR, ...)                                \
            __PLOOC_EVAL(__vsf_rng_buf_get_, __VA_ARGS__)                       \
                (__NAME, __QADDR, __VA_ARGS__)

#   define vsf_rng_buf_peek(__NAME, __QADDR, ...)                               \
            __PLOOC_EVAL(__VSF_QUEUE_PEEK_, __VA_ARGS__)                        \
                (__NAME, __QADDR, __VA_ARGS__)
#endif

#define vsf_rng_buf_send_one(__NAME, __QADDR, __ITEM)                           \
            __vsf_rng_buf_send_1(__NAME, (__QADDR), (__ITEM))
            
#define vsf_rng_buf_send_buf(__NAME, __QADDR, __BUFFER, __SIZE)                 \
            __vsf_rng_buf_send_2(__NAME, (__QADDR), (__BUFFER), (__SIZE))
            
#define vsf_rng_buf_get_one(__NAME, __QADDR, __ITEM)                            \
            __vsf_rng_buf_get_1(__NAME, (__QADDR), (__ITEM))
            
#define vsf_rng_buf_get_buf(__NAME, __QADDR, __BUFFER, __SIZE)                  \
            __vsf_rng_buf_get_2(__NAME, (__QADDR), (__BUFFER), (__SIZE))

#define __vsf_rng_buf_count(__NAME, __QADDR)                                    \
            __NAME##_item_count((__QADDR))
#define vsf_rng_buf_count(__NAME, __QADDR)                                      \
            __vsf_rng_buf_count(__NAME, __QADDR)

#define __vsf_rng_buf_peekable_count(__NAME, __QADDR)                           \
            __NAME##_item_count_peekable((__QADDR))
#define vsf_rng_buf_peekable_count(__NAME, __QADDR)                             \
            __vsf_rng_buf_peekable_count(__NAME, __QADDR)

/*============================ TYPES =========================================*/

declare_class(vsf_rng_buf_t)

def_class(vsf_rng_buf_t,
    
    protected_member(
        uint16_t hwBufferItemCount;
        uint16_t hwTail;
        uint16_t hwLength;
        uint16_t hwHead;
        uint16_t hwPeekCount;
        uint16_t hwPeek;
    )
)
end_def_class(vsf_rng_buf_t)

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/

extern 
void __vsf_rng_buf_init(  vsf_rng_buf_t* ptObj, 
                        uint_fast16_t hwBufferItemCount, 
                        bool bInitAsFull);

extern 
int32_t __vsf_rng_buf_send_one(vsf_rng_buf_t* ptObj);

extern 
int32_t __vsf_rng_buf_get_one(vsf_rng_buf_t* ptObj);

SECTION(".text.vsf.utilities.__vsf_rng_buf_send_multiple")
extern 
int32_t __vsf_rng_buf_send_multiple(vsf_rng_buf_t* ptObj, uint16_t* phwItemCount);

SECTION(".text.vsf.utilities.__vsf_rng_buf_get_multiple")
extern 
int32_t __vsf_rng_buf_get_multiple(vsf_rng_buf_t* ptObj, uint16_t* phwItemCount);

SECTION(".text.vsf.utilities.__vsf_rng_buf_item_count")
extern 
uint_fast16_t __vsf_rng_buf_item_count(vsf_rng_buf_t* ptObj);

SECTION(".text.vsf.utilities.__vsf_rng_buf_peek_one")
extern 
int32_t __vsf_rng_buf_peek_one(vsf_rng_buf_t* ptObj);

SECTION(".text.vsf.utilities.__vsf_rng_buf_item_count_peekable")
extern
uint_fast16_t __vsf_rng_buf_item_count_peekable(vsf_rng_buf_t* ptObj);

SECTION(".text.vsf.utilities.__vsf_rng_buf_peek_multiple")
extern
int32_t __vsf_rng_buf_peek_multiple(vsf_rng_buf_t* ptObj, uint16_t* phwItemCount);
#endif
