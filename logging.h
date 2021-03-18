#pragma once
#define LOG(...) fprintf(stdout,  __VA_ARGS__)
#define LOG_ERR(...) fprintf(stderr,  __VA_ARGS__)

#ifdef ASSERTS_ON
#define ASSERT(condition, message) if(!(condition)) {LOG_ERR("%s", message); *((u32*)0) = 0;}
#else 
#define ASSERT(condition, message)
#endif