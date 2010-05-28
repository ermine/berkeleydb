#define   MUTEX_INVALID   0

/*
 * DB_IS_THREADED --
 *      The database handle is free-threaded (was opened with DB_THREAD).
 */

#define DB_IS_THREADED(dbp)                     \
  ((dbp)->mutex != MUTEX_INVALID)
