#ifndef TLSF_CONFIG_H
#define TLSF_CONFIG_H

/* 
 * TLSF Configuration Options
 * Modify these based on your platform and requirements
 */

/* Alignment requirement (must be power of 2) */
#ifndef TLSF_ALIGN_SIZE
#define TLSF_ALIGN_SIZE 8
#endif

/* Minimum block size (must be at least 4 * alignment) */
#ifndef TLSF_MIN_BLOCK_SIZE
#define TLSF_MIN_BLOCK_SIZE (4 * TLSF_ALIGN_SIZE)
#endif

/* Maximum allocation size (adjust based on your needs) */
#ifndef TLSF_MAX_SIZE
#define TLSF_MAX_SIZE (1UL << 31)  /* 2GB */
#endif

/* First level index bits (controls number of first level lists) */
#ifndef TLSF_FL_INDEX_COUNT
#define TLSF_FL_INDEX_COUNT 14     /* First level: 2^14 = 16384 lists */
#endif

/* Second level index bits per first level (controls granularity) */
#ifndef TLSF_SL_INDEX_COUNT
#define TLSF_SL_INDEX_COUNT 8      /* Second level: 256 lists per first level */
#endif

/* Enable/disable thread safety */
#ifndef TLSF_USE_LOCKS
#define TLSF_USE_LOCKS 1
#endif

/* Enable/disable debugging features */
#ifndef TLSF_DEBUG
#define TLSF_DEBUG 0
#endif

/* Enable/disable statistics gathering */
#ifndef TLSF_STATISTICS
#define TLSF_STATISTICS 0
#endif

/* Enable/disable tlsf_check() function */
#ifndef TLSF_ASSERTIONS
#define TLSF_ASSERTIONS 1
#endif

/* Memory filling pattern for debugging */
#ifndef TLSF_FILL_PATTERN
#define TLSF_FILL_PATTERN 0xAA
#endif

#endif /* TLSF_CONFIG_H */
