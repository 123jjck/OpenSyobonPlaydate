#include "pd_api.h"

typedef int PDEventHandler(
    PlaydateAPI* playdate,
    PDSystemEvent event,
    uint32_t arg
);

extern PDEventHandler eventHandler;

static void* (*pdrealloc)(void* pointer, size_t size);

extern "C" int eventHandlerShim(
    PlaydateAPI* playdate,
    PDSystemEvent event,
    uint32_t arg
)
{
    if (event == kEventInit) {
        pdrealloc = playdate->system->realloc;
    }

    return eventHandler(playdate, event, arg);
}

#if TARGET_PLAYDATE

extern "C" void* _malloc_r(
    struct _reent* reentrancy,
    size_t numberOfBytes
)
{
    (void)reentrancy;
    return pdrealloc(nullptr, numberOfBytes);
}

extern "C" void* _realloc_r(
    struct _reent* reentrancy,
    void* pointer,
    size_t numberOfBytes
)
{
    (void)reentrancy;
    return pdrealloc(pointer, numberOfBytes);
}

extern "C" void _free_r(struct _reent* reentrancy, void* pointer)
{
    (void)reentrancy;
    if (pointer != nullptr) {
        pdrealloc(pointer, 0);
    }
}

#endif
