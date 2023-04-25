#pragma once

#define FCM_CHECK \
{                               \
if (FCM_FAILURE_CODE(res)) {    \
    return res;                 \
}                               \
}

#ifdef DEBUG

#define debugLog(fmt, ...)  console.log(fmt, __VA_ARGS__)

#else

#define debugLog(fmt, ...)

#endif


