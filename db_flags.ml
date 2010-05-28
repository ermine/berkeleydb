type db_associate_flag = [
  | `DB_CREATE
  | `DB_IMMUTABLE_KEY
]

type db_associate_foreign_flag = [
  | `DB_FOREIGN_ABORT
  | `DB_FOREIGN_CASCADE
  | `DB_FOREIGN_NULLIFY
]

type db_close_flag = [
  | `DB_NOSYNC
]

type db_compact_flag = [
  | `DB_FREELIST_ONLY
  | `DB_FREE_SPACE
]

type db_del_flag = [
  | `DB_CONSUME
  | `DB_MULTIPLE
  | `DB_MULTIPLE_KEY
]

type db_exists_flag = [
  | `DB_READ_COMMITTED
  | `DB_READ_UNCOMMITTED
  | `DB_RMW
]

type db_get_flag = [
  | `DB_CONSUME
  | `DB_CONSUME_WAIT
  | `DB_GET_BOTH
  | `DB_SET_RECNO
  | `DB_IGNORE_LEASE
  | `DB_MULTIPLE
  | `DB_READ_COMMITTED
  | `DB_READ_UNCOMMITTED
  | `DB_RMW
]

type db_join_flag = [
  | `DB_JOIN_NOSORT
]

type db_open_flag = [
  | `DB_AUTO_COMMIT
  | `DB_CREATE
  | `DB_EXCL
  | `DB_MULTIVERSION
  | `DB_NOMMAP
  | `DB_RDONLY
  | `DB_READ_UNCOMMITTED
  | `DB_THREAD
  | `DB_TRUNCATE
]

type db_put_flag = [
  | `DB_APPEND
  | `DB_NODUPDATA
  | `DB_NOOVERWRITE
  | `DB_MULTIPLE
  | `DB_MULTIPLE_KEY
  | `DB_OVERWRITE_DUP
]

type db_encrypt_flag = [
  | `DB_ENCRYPT_AES
]

type db_flag = [
  | `DB_CHKSUM
  | `DB_ENCRYPT
  | `DB_TXN_NOT_DURABLE
  | `DB_DUP
  | `DB_DUPSORT
  | `DB_RECNUM
  | `DB_REVSPLITOFF
  | `DB_INORDER
  | `DB_RENUMBER
  | `DB_SNAPSHOT
]

type db_stat_flag = [
  | `DB_FAST_STAT
  | `DB_READ_COMMITTED
  | `DB_READ_UNCOMMITTED
]

type db_stat_print_flag = [
  | `DB_FAST_STAT
  | `DB_STAT_ALL
]

type db_upgrade_flag = [
  | `DB_DUPSORT
]

type db_verify_flag = [
  | `DB_SALVAGE
  | `DB_AGGRESSIVE
  | `DB_PRINTABLE
  | `DB_NOORDERCHK
  | `DB_ORDERCHKONLY
]

type db_cursor_flag = [
  | `DB_READ_COMMITTED
  | `DB_READ_UNCOMMITTED
  | `DB_WRITECURSOR
  | `DB_TXN_SNAPSHOT
]

type dbcursor_del_flag = [
  | `DB_CONSUME
]

type dbcursor_dup_flag = [
  | `DB_POSITION
]

type dbcursor_get_flag = [
  | `DB_CURRENT
  | `DB_FIRST
  | `DB_GET_BOTH
  | `DB_GET_BOTH_RANGE
  | `DB_GET_RECNO
  | `DB_JOIN_ITEM
  | `DB_LAST
  | `DB_NEXT
  | `DB_NEXT_DUP
  | `DB_NEXT_NODUP
  | `DB_PREV
  | `DB_PREV_DUP
  | `DB_PREV_NODUP
  | `DB_SET
  | `DB_SET_RANGE
  | `DB_SET_RECNO
  | `DB_IGNORE_LEASE
  | `DB_READ_COMMITTED
  | `DB_READ_UNCOMMITTED
  | `DB_MULTIPLE
  | `DB_MULTIPLE_KEY
  | `DB_RMW
]

type dbcursor_put_flag = [
  | `DB_AFTER
  | `DB_BEFORE
  | `DB_CURRENT
  | `DB_KEYFIRST
  | `DB_KEYLAST
  | `DB_NODUPDATA
]

type dbenv_dbremove_flag = [
  | `DB_AUTO_COMMIT
]

type dbenv_dbrename_flag = [
  | `DB_AUTO_COMMIT
]

type dbenv_fileid_reset_flag = [
  | `DB_ENCRYPT
]

type dbenv_lsn_reset_flag = [
  | `DB_ENCRYPT
]

type dbenv_open_flag = [
  | `DB_INIT_CDB
  | `DB_INIT_LOCK
  | `DB_INIT_LOG
  | `DB_INIT_MPOOL
  | `DB_INIT_REP
  | `DB_INIT_TXN
  | `DB_RECOVER
  | `DB_RECOVER_FATAL
  | `DB_USE_ENVIRON
  | `DB_USE_ENVIRON_ROOT
  | `DB_CREATE
  | `DB_LOCKDOWN
  | `DB_FAILCHK
  | `DB_PRIVATE
  | `DB_REGISTER
  | `DB_SYSTEM_MEM
  | `DB_THREAD
]

type dbenv_remove_flag = [
  | `DB_FORCE
  | `DB_USE_ENVIRON
  | `DB_USE_ENVIRON_ROOT
]

type dbenv_encrypt_flag = [
  | `DB_ENCRYPT_AES
]

type dbenv_flag = [
  | `DB_AUTO_COMMIT
  | `DB_CDB_ALLDB
  | `DB_DIRECT_DB
  | `DB_DSYNC_DB
  | `DB_MULTIVERSION
  | `DB_NOLOCKING
  | `DB_NOMMAP
  | `DB_NOPANIC
  | `DB_OVERWRITE
  | `DB_PANIC_ENVIRONMENT
  | `DB_REGION_INIT
  | `DB_TIME_NOTGRANTED
  | `DB_TXN_NOSYNC
  | `DB_TXN_NOWAIT
  | `DB_TXN_SNAPSHOT
  | `DB_TXN_WRITE_NOSYNC
  | `DB_YIELDCPU
]

type dbenv_isalive_flag = [
  | `DB_MUTEX_PROCESS_ONLY
]

type dbenv_timeout_flag = [
  | `DB_SET_LOCK_TIMEOUT
  | `DB_SET_REG_TIMEOUT 
  | `DB_SET_TXN_TIMEOUT
]

type dbenv_get_timeout_flag = [
  | `DB_SET_LOCK_TIMEOUT
  | `DB_SET_TXN_TIMEOUT
]

type dbenv_stat_print_flag = [
  | `DB_STAT_ALL
  | `DB_STAT_CLEAR
  | `DB_STAT_SUBSYSTEM
]

type dbenv_lock_get_flag = [
  | `DB_LOCK_NOWAIT
]

type dbenv_lock_stat_flag = [
  | `DB_STAT_CLEAR
]

type dbenv_lock_stat_print_flag = [
  | `DB_STAT_ALL
  | `DB_STAT_CLEAR
  | `DB_STAT_LOCK_CONF
  | `DB_STAT_LOCK_LOCKERS
  | `DB_STAT_LOCK_OBJECTS
  | `DB_STAT_LOCK_PARAMS
]

type dbenv_lock_vec_flag = [
  | `DB_LOCK_NOWAIT
]

type log_archive_flag = [
  | `DB_ARCH_ABS
  | `DB_ARCH_DATA
  | `DB_ARCH_LOG
  | `DB_ARCH_REMOVE
]

type log_put_flag = [
  | `DB_FLUSH
]

type log_config_flag = [
  | `DB_LOG_DIRECT
  | `DB_LOG_DSYNC
  | `DB_LOG_AUTO_REMOVE
  | `DB_LOG_IN_MEMORY
  | `DB_LOG_ZERO
]

type log_stat_flag = [
  | `DB_STAT_CLEAR
]

type log_stat_print_flag = [
  | `DB_STAT_ALL
  | `DB_STAT_CLEAR
]

type db_logs_get_flag = [
  | `DB_CURRENT
  | `DB_FIRST
  | `DB_LAST
  | `DB_NEXT
  | `DB_PREV
  | `DB_SET
]

type dbenv_memp_stat_flag = [
  | `DB_STAT_CLEAR
]

type dbenv_memp_stat_print_flag = [
  | `DB_STAT_ALL
  | `DB_STAT_CLEAR
  | `DB_STAT_MEMP_HASH
]

type dbmpoolfile_get_flag = [
  | `DB_MPOOL_CREATE
  | `DB_MPOOL_DIRTY
  | `DB_MPOOL_EDIT
  | `DB_MPOOL_LAST
  | `DB_MPOOL_NEW
]

type dbmpoolfile_open_flag = [
  | `DB_CREATE
  | `DB_DIRECT
  | `DB_MULTIVERSION
  | `DB_NOMMAP
  | `DB_ODDFILESIZE
  | `DB_RDONLY
]

type dbmpoolfile_flag = [
  | `DB_MPOOL_NOFILE
  | `DB_MPOOL_UNLINK
]

type mutex_alloc_flag = [
  | `DB_MUTEX_PROCESS_ONLY
  | `DB_MUTEX_SELF_BLOCK
]

type mutex_stat_flag = [
  | `DB_STAT_CLEAR
]

type mutex_stat_print_flag = [
  | `DB_STAT_ALL
  | `DB_STAT_CLEAR
]

type rep_transport_flag = [
  | `DB_REP_ANYWHERE
  | `DB_REP_NOBUFFER
  | `DB_REP_PERMANENT
  | `DB_REP_REREQUEST
]

type rep_start_flag = [
  | `DB_REP_CLIENT
  | `DB_REP_MASTER
]

type rep_stat_flag = [
  | `DB_STAT_CLEAR
]

type rep_stat_print_flag = [
  | `DB_STAT_ALL
  | `DB_STAT_CLEAR
]

type repmgr_add_remote_site_flag = [
  | `DB_REPMGR_PEER
]

type repmgr_start_flag = [
  | `DB_REP_MASTER
  | `DB_REP_CLIENT
  | `DB_REP_ELECTION
]

type repmgr_stat_flag = [
  | `DB_STAT_CLEAR
]

type repmgr_stat_print_flag = [
  | `DB_STAT_ALL
  | `DB_STAT_CLEAR
]

type dbsequence_get_flag = [
  | `DB_TXN_NOSYNC
]

type dbsequence_open_flag = [
  | `DB_CREATE
  | `DB_EXCL
  | `DB_THREAD
]

type dbsequence_remove_flag = [
  | `DB_TXN_NOSYNC
]

type dbsequence_flag = [
  | `DB_SEQ_DEC
  | `DB_SEQ_INC
  | `DB_SEQ_WRAP
]

type dbsequence_stat_flag = [
  | `DB_STAT_CLEAR
]

type dbsequence_stat_print_flag = [
  | `DB_STAT_CLEAR
]

type dbenv_txn_begin_flag = [
  | `DB_READ_COMMITTED
  | `DB_READ_UNCOMMITTED
  | `DB_TXN_NOSYNC
  | `DB_TXN_NOWAIT
  | `DB_TXN_SNAPSHOT
  | `DB_TXN_SYNC
  | `DB_TXN_WAIT
  | `DB_TXN_WRITE_NOSYNC
]

type dbenv_txn_checkpoint_flag = [
  | `DB_FORCE
]

type dbtxn_commit_flag = [
  | `DB_TXN_NOSYNC
  | `DB_TXN_SYNC
]

type dbenv_txn_recover_flag = [
  | `DB_FIRST
  | `DB_NEXT
]

type dbtxn_timeout_flag = [
  | `DB_SET_LOCK_TIMEOUT
  | `DB_SET_TXN_TIMEOUT
]

type dbenv_txn_stat_flag = [
  | `DB_STAT_CLEAR
]

type dbenv_txn_stat_print_flag = [
  | `DB_STAT_ALL
  | `DB_STAT_CLEAR
]

type dblogc_get_flag = [
  | `DB_CURRENT
  | `DB_FIRST
  | `DB_LAST
  | `DB_NEXT
  | `DB_PREV
  | `DB_SET
]

