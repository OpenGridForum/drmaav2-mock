//#define DEBUG
// used for database debug
#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...)    fprintf(stderr, fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args...)    /* Don't do anything in release builds */
#endif



//#define DRMAA2_DEBUG
// used for api debug
#ifdef DRMAA2_DEBUG
#define DRMAA2_DEBUG_PRINT(fmt, args...)    fprintf(stderr, fmt, ## args)
#else
#define DRMAA2_DEBUG_PRINT(fmt, args...)    /* Don't do anything in release builds */
#endif

