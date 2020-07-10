#include <QtGlobal>
#include "vsassert.h"

void VSAssert(bool test)
{
    if(false == test)
    {
        Q_ASSERT(false);
    }
}
