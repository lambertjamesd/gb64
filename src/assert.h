
#ifndef _ASSERT_H
#define _ASSERT_H

#if NDEBUG
    #define teqassert(assertion)
#else
    void teqassert(int assertion);
#endif

#endif