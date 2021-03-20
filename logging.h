#pragma once
#define LOG(...) fprintf(stdout,  __VA_ARGS__)
#define LOG_ERR(...) fprintf(stderr,  __VA_ARGS__)

#ifdef ASSERTS_ON
#define ASSERT(condition, ...) if(!(condition)) {LOG_ERR(__VA_ARGS__); *((u32*)0) = 0;}
#else 
#define ASSERT(condition, ...)
#endif