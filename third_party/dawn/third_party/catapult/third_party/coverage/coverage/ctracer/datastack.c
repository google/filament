/* Licensed under the Apache License: http://www.apache.org/licenses/LICENSE-2.0 */
/* For details: https://bitbucket.org/ned/coveragepy/src/default/NOTICE.txt */

#include "util.h"
#include "datastack.h"

#define STACK_DELTA    100

int
DataStack_init(Stats *pstats, DataStack *pdata_stack)
{
    pdata_stack->depth = -1;
    pdata_stack->stack = NULL;
    pdata_stack->alloc = 0;
    return RET_OK;
}

void
DataStack_dealloc(Stats *pstats, DataStack *pdata_stack)
{
    PyMem_Free(pdata_stack->stack);
}

int
DataStack_grow(Stats *pstats, DataStack *pdata_stack)
{
    pdata_stack->depth++;
    if (pdata_stack->depth >= pdata_stack->alloc) {
        /* We've outgrown our data_stack array: make it bigger. */
        int bigger = pdata_stack->alloc + STACK_DELTA;
        DataStackEntry * bigger_data_stack = PyMem_Realloc(pdata_stack->stack, bigger * sizeof(DataStackEntry));
        STATS( pstats->stack_reallocs++; )
        if (bigger_data_stack == NULL) {
            PyErr_NoMemory();
            pdata_stack->depth--;
            return RET_ERROR;
        }
        pdata_stack->stack = bigger_data_stack;
        pdata_stack->alloc = bigger;
    }
    return RET_OK;
}
