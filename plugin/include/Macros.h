#pragma once

#ifdef DEBUG

#define debugLog(fmt, ...)  console.log(fmt, __VA_ARGS__)

#else

#define debugLog(fmt, ...)

#endif


