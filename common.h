#if defined(__cplusplus) || defined(c_plusplus)
# define EXTERN_C extern "C"
# define START_EXTERN_C extern "C" {
# define END_EXTERN_C }
#else
# define EXTERN_C extern
# define START_EXTERN_C
# define END_EXTERN_C
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif