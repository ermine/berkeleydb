open Db_flags

exception Error of string
exception KeyEmpty
exception KeyExists
exception Deadlock
exception LockNotGranted
exception RunRecovery
exception SecondaryBad
  
type env
type 'a dbt
type 'a db
type 'a cursor
type txn
type logcursor
type lsn
type mpoolfile
type sequence

type data = string

type keytype =
  | Recno
  | String

type dbtype =
  | DB_BTREE
  | DB_HASH
  | DB_RECNO
  | DB_QUEUE
  | DB_UNKNOWN

type db_cache_priority =
  | DB_PRIORITY_UNCHANGED
  | DB_PRIORITY_VERY_LOW
  | DB_PRIORITY_LOW
  | DB_PRIORITY_DEFAULT
  | DB_PRIORITY_HIGH
  | DB_PRIORITY_VERY_HIGH
      
type verbose_which =
  | DB_VERB_DEADLOCK
  | DB_VERB_FILEOPS
  | DB_VERB_FILEOPS_ALL
  | DB_VERB_RECOVERY
  | DB_VERB_REGISTER
  | DB_VERB_REPLICATION
  | DB_VERB_REP_ELECT
  | DB_VERB_REP_LEASE
  | DB_VERB_REP_MISC
  | DB_VERB_REP_MSGS
  | DB_VERB_REP_SYNC
  | DB_VERB_REPMGR_CONNFAIL
  | DB_VERB_REPMGR_MISC
  | DB_VERB_WAITSFOR
  
type lk_detect_t =
  | DB_LOCK_DEFAULT
  | DB_LOCK_EXPIRE
  | DB_LOCK_MAXLOCKS
  | DB_LOCK_MAXWRITE
  | DB_LOCK_MINLOCKS
  | DB_LOCK_MINWRITE
  | DB_LOCK_OLDEST
  | DB_LOCK_RANDOM
  | DB_LOCK_YOUNGEST

type repmgr_ack_policy =
  | DB_REPMGR_ACKS_ALL
  | DB_REPMGR_ACKS_ALL_PEERS
  | DB_REPMGR_ACKS_NONE
  | DB_REPMGR_ACKS_ONE
  | DB_REPMGR_ACKS_ONE_PEER
  | DB_REPMGR_ACKS_QUORUM
  
type rep_timeout_which =
  | DB_REP_ACK_TIMEOUT
  | DB_REP_CHECKPOINT_DELAY
  | DB_REP_CONNECTION_RETRY
  | DB_REP_ELECTION_TIMEOUT
  | DB_REP_ELECTION_RETRY
  | DB_REP_FULL_ELECTION_TIMEOUT
  | DB_REP_HEARTBEAT_MONITOR
  | DB_REP_HEARTBEAT_SEND
  | DB_REP_LEASE_TIMEOUT

type event_notify =
  | DB_EVENT_PANIC
  | DB_EVENT_REG_ALIVE
  | DB_EVENT_REG_PANIC
  | DB_EVENT_REP_CLIENT
  | DB_EVENT_REP_ELECTED
  | DB_EVENT_REP_MASTER
  | DB_EVENT_REP_NEWMASTER
  | DB_EVENT_REP_PERM_FAILED
  | DB_EVENT_REP_STARTUPDONE
  | DB_EVENT_WRITE_FAILED

type rep_config_which =
  | DB_REP_CONF_BULK
  | DB_REP_CONF_DELAYCLIENT
  | DB_REP_CONF_INMEM
  | DB_REP_CONF_LEASE
  | DB_REP_CONF_NOAUTOINIT
  | DB_REP_CONF_NOWAIT
  | DB_REPMGR_CONF_2SITE_STRICT
      
module DbEnv =
struct
  type t = env
      
  external add_data_dir : t -> string -> unit
    = "ml_dbenv_add_data_dir"
(*      
  external cdsgroup_begin : t -> txn -> unit
*)  
  external close : t -> unit -> unit
    = "ml_dbenv_close"
  external dbremove : t -> ?txn:txn -> string -> string ->
    ?flags:dbenv_dbremove_flag list -> unit -> unit
    = "ml_dbenv_dbremove_byte" "ml_dbenv_dbremove"
  external dbrename : t -> ?txn:txn -> string -> string -> string ->
    ?flags:dbenv_dbrename_flag list -> unit -> unit
    = "ml_dbenv_rename_byte" "ml_dbenv_dbrename"
  external failchk : t -> unit
    = "ml_dbenv_failchk"
  external fileid_reset : t -> string ->
    ?flags:dbenv_fileid_reset_flag list -> unit -> unit
    = "ml_dbenv_fileid_reset"
  external get_cache_max : t -> int32 * int32
    = "ml_dbenv_get_cache_max"
  external get_cachesize : t -> int32 * int32 * int
    = "ml_dbenv_get_cachesize"
  external get_create_dir : t -> string
    = "ml_dbenv_get_create_dir"
  external get_encrypt_flags : t -> dbenv_encrypt_flag list
    = "ml_dbenv_get_encrypt_flags"
(*
  external get_errfile : t -> FILE ** -> unit
  = "ml_dbenv_get_errfile"
*)
  external get_errpfx : t -> string
    = "ml_dbenv_get_errpfx"
  external get_flags : t -> dbenv_flag list
    = "ml_dbenv_get_flags"
  external get_home : t -> string
    = "ml_dbenv_get_home"
  external get_intermediate_dir_mode : t -> string
    = "ml_dbenv_get_intermediate_dir_mode"
  external get_lg_bsize : t -> int32
    = "ml_dbenv_get_lg_bsize"
  external get_lg_dir : t -> string
    = "ml_dbenv_get_lg_dir"
  external get_lg_filemode : t -> int
    = "ml_dbenv_get_lg_filemode"
  external get_lg_max : t -> int32
    = "ml_dbenv_get_lg_max"
  external get_lg_regionmax : t -> int32
    = "ml_dbenv_get_lg_regionmax"
(*  
  external get_lk_conflicts : t -> int ** -> int
    = "ml_dbenv_get_lk_conflicts"
*)  
  external get_lk_detect : t -> lk_detect_t
    = "ml_dbenv_get_lk_detect"
  external get_lk_max_lockers : t -> int32
    = "ml_dbenv_get_lk_max_lockers"
  external get_lk_max_locks : t -> int32
    = "ml_dbenv_get_lk_max_locks"
  external get_lk_max_objects : t -> int32
    = "ml_dbenv_get_lk_max_objects"
  external get_lk_partitions : t -> int32
      = "ml_dbenv_get_lk_partitions"
  external get_mp_max_openfd : t -> int
    = "ml_dbenv_get_mp_max_openfd"
  external get_mp_max_write : t -> int * int32
    = "ml_dbenv_get_mp_max_write"
  external get_mp_mmapsize : t -> int32
    = "ml_dbenv_get_mp_mmapsize"
(*      
  external get_msgfile : t -> FILE ** -> unit
*)  
  external get_open_flags : t -> dbenv_open_flag list
    = "ml_dbenv_get_open_flags"
  external get_shm_key : t -> int
    = "ml_dbenv_get_shm_key"
  external get_thread_count : t -> int32
    = "ml_dbenv_get_thread_count"
  external get_timeout : t -> dbenv_get_timeout_flag -> int32
    = "ml_dbenv_get_timeout"
  external get_tmp_dir : t -> string
    = "ml_dbenv_get_tmp_dir"
  external get_tx_max : t -> int32
    = "ml_dbenv_get_tx_max"
(*    
      external get_tx_timestamp : t -> time_t
*)
  external get_verbose : t -> verbose_which -> bool
    = "ml_dbenv_get_verbose"
  external lock_detect : t -> int32 -> int
    = "ml_dbenv_lock_detect"
(*
  external lock_get : t -> int32 -> int32 -> string -> db_lockmode_t -> DB_LOCK * -> unit
*)  
  external lock_id : t -> int32
    = "ml_dbenv_lock_id"
  external lock_id_free : t -> int32 -> unit
    = "ml_dbenv_lock_id_free"
(*      
  external lock_put : t -> DB_LOCK * -> unit
  external lock_stat : t -> DB_LOCK_STAT ** -> ?flags:dbenv_lock_stat_flag list
  -> unit -> unit
  = "ml_dbenv_lock_stat"
  external lock_stat_print : t -> ?flags:dbenv_lock_stat_print_flag list  ->
  unit -> unit
  external lock_vec : t -> int32 -> int32 -> DB_LOCKREQ * -> int -> DB_LOCKREQ ** -> unit
*)        
  external log_archive : t -> ?flags:log_archive_flag list -> unit -> string list
    = "ml_dbenv_log_archive"
(*    
  external log_cursor : t -> DB_LOGC ** -> unit
  external log_file : t -> DB_LSN * -> string -> size_t -> unit
  external log_flush : t -> DB_LSN * -> unit
*)      
  external log_get_config : t -> log_config_flag -> bool
    = "ml_dbenv_log_get_config"
(*    
  external log_put : t -> DB_LSN * -> string -> ?flags:log_put_flag list ->
  unit
*)      
  external log_set_config : t -> log_config_flag list -> bool -> unit
    = "ml_dbenv_log_set_config"
(*    
  external log_stat : t -> DB_LOG_STAT ** -> ?flags:log_stat_flag list ->
  unit -> unit
  external log_stat_print : t -> ?flags:log_stat_flag list ->
  unit -> unit
  external lsn_reset : t -> string -> ?flags:dbenv_lsn_reset_flag list ->
  unit -> unit
  external memp_fcreate : t -> DB_MPOOLFILE ** -> unit
  external memp_register : t -> int -> (t -> db_pgno_t ->
                void * -> string -> int) -> (t -> db_pgno_t -> void * -> string -> int) -> unit
  external memp_stat : t -> DB_MPOOL_STAT ** -> DB_MPOOL_FSTAT *** ->
  ?flags:dbenv_memp_stat_flag list -> unit -> unit
  external memp_stat_print : t -> ?flags:dbenv_memp_stat_print_flag list ->
  unit -> unit
  external memp_sync : t -> DB_LSN * -> unit
  external memp_trickle : t -> int -> int
  external mutex_alloc : t -> int32 -> db_mutex_t
  external mutex_free : t -> db_mutex_t -> unit
  external mutex_get_align : t -> int32
  external mutex_get_increment : t -> int32
  external mutex_get_max : t -> int32
  external mutex_get_tas_spins : t -> int32
  external mutex_lock : t -> db_mutex_t -> unit
  external mutex_set_align : t -> int32 -> unit
  external mutex_set_increment : t -> int32 -> unit
  external mutex_set_max : t -> int32 -> unit
  external mutex_set_tas_spins : t -> int32 -> unit
  external mutex_stat : t -> DB_MUTEX_STAT ** ->
  ?flags:dbenv_mutex_stat_flag list -> unit -> unit
  external mutex_stat_print : t -> ?flags:dbenv_mutex_stat_print_flag list ->
  unit -> unit
  external mutex_unlock : t -> db_mutex_t -> unit
*)  
  external env_open : t -> ?home:string -> ?flags:dbenv_open_flag list ->
    ?mode:int -> unit -> unit
    = "ml_dbenv_open"
  external remove : t -> string -> ?flags:dbenv_remove_flag list -> unit -> unit
    = "ml_dbenv_remove"
  external rep_elect : t -> int32 -> int32 -> unit
    = "ml_dbenv_rep_elect"
  external rep_get_clockskew : t -> int32 * int32
    = "ml_dbenv_rep_get_clockskew"
  external rep_get_config : t -> rep_config_which -> bool
    = "ml_dbenv_rep_get_config"
  external rep_get_limit : t -> int32 * int32
    = "ml_dbenv_rep_get_limit"
  external rep_get_nsites : t -> int32
    = "ml_dbenv_rep_get_nsites"
  external rep_get_priority : t -> int32
    = "ml_dbenv_rep_get_priority"
  external rep_get_request : t -> int32 * int32
    = "ml_dbenv_rep_get_request"
  external rep_get_timeout : t -> rep_timeout_which -> int32
    = "ml_dbenv_rep_get_timeout"
(*      
  external rep_process_message : t ->
                 string -> string -> int -> DB_LSN * -> unit
    = "ml_dbenv_rep_process_message"
*)
  external rep_set_clockskew : t -> int32 -> int32 -> unit
    = "ml_dbenv_rep_set_clockskew"
  external rep_set_config : t -> rep_config_which -> bool -> unit
    = "ml_dbenv_rep_set_config"
  external rep_set_limit : t -> int32 -> int32 -> unit
    = "ml_dbenv_rep_set_limit"
  external rep_set_nsites : t -> int32 -> unit
    = "ml_dbenv_rep_set_nsites"
  external rep_set_priority : t -> int32 -> unit
    = "ml_dbenv_rep_set_priority"
  external rep_set_request : t -> int32 -> int32 -> unit
    = "ml_dbenv_rep_set_request"
  external rep_set_timeout : t -> rep_timeout_which -> int32 -> unit
    = "ml_dbenv_rep_set_timeout"
(*      
  external rep_set_transport : t -> int -> int ( * )(dbenv ->
                string -> string -> DB_LSN * -> int -> int32) -> unit
        ?flags:rep_transport_flag list -> unit -> unit
    = "ml_dbenv_rep_set_transport"
  external rep_start : t -> string ->
    ?flags:rep_start_flag_list -> unit -> unit
    "ml_dbenv_rep_start"
  external rep_stat : t -> DB_REP_STAT ** ->
  ?flags:rep_stat_flag list -> unit
    = "ml_dbenv_rep_stat"
  external rep_stat_print : t -> ?flags:rep_stat_print_flag list ->
  unit -> unit
    = "ml_dbenv_rep_stat_print"
  external rep_sync : t -> unit
    = "ml_dbenv_rep_sync"
*)
  external repmgr_add_remote_site : t -> string -> int ->
    ?flags:repmgr_add_remote_site_flag list -> unit -> int
    = "ml_dbenv_repmgr_add_remote_site"
  external repmgr_get_ack_policy : t -> repmgr_ack_policy
    = "ml_dbenv_repmgr_get_ack_policy"
  external repmgr_set_ack_policy : t -> repmgr_ack_policy -> unit
    = "ml_dbenv_repmgr_set_ack_policy"
  external repmgr_set_local_site : t -> string -> int -> unit
    = "ml_dbenv_repmgr_set_local_site"
(*    
  external repmgr_site_list : t -> int * -> DB_REPMGR_SITE ** -> unit
      = "ml_dbenv_repmgr_site_list"
*)
  external repmgr_start : t -> int -> repmgr_start_flag -> unit
    = "ml_dbenv_repmgr_start"
(*    
  external repmgr_stat : t -> DB_REPMGR_STAT ** -> ?flags:repmgr_stat_flag list
      -> unit -> unit
    = "ml_dbenv_repmgr_stat"
  external repmgr_stat_print : t -> flags -> unit
    = "ml_dbenv_repmgr_stat_print"
  external set_app_dispatch : t -> int ( * )(dbenv -> string -> DB_LSN * -> db_recops) -> unit
    = "ml_dbenv_set_app_dispatch"
*)
  external set_cache_max : t -> int32 -> int32 -> unit
    = "ml_dbenv_set_cache_max"
  external set_cachesize : t -> int32 -> int32 -> int -> unit
    = "ml_dbenv_set_cachesize"
  external set_create_dir : t -> string -> unit
    = "ml_dbenv_set_create_dir"
  external set_encrypt : t -> string -> ?flags:dbenv_encrypt_flag list ->
    unit -> unit
    = "ml_dbenv_set_encrypt"
  external set_errcall : t -> (t -> string -> string -> unit) option -> unit
    = "ml_dbenv_set_errcall"
(*
  external set_errfile : t -> FILE * -> unit
*)
  external set_errpfx : t -> string -> unit
    = "ml_dbenv_set_errpfx"
(*      
  external set_event_notify : t -> (t -> event_notify -> void * -> unit) -> unit
    = "ml_dbenv_set_event_notify"
*)
(*      
  external set_feedback : t -> (t -> int -> int) -> unit
    = "ml_dbenv_set_feedback"
*)
  external set_flags : t -> dbenv_flag list -> bool -> unit
    = "ml_dbenv_set_flags"
  external set_intermediate_dir_mode : t -> string
    = "ml_dbenv_set_intermediate_dir_mode"
(*
  external set_isalive : t ->
  int ( * )(dbenv -> pid_t -> db_threadid_t -> int32) -> unit
*)
  external set_lg_bsize : t -> int32 -> unit
    = "ml_dbenv_set_lg_bsize"
  external set_lg_dir : t -> string -> unit
    = "ml_dbenv_set_lg_dir"
  external set_lg_filemode : t -> int -> unit
    = "ml_dbenv_set_lg_filemode"
  external set_lg_max : t -> int32 -> unit
    = "ml_dbenv_set_lg_max"
  external set_lg_regionmax : t -> int32 -> unit
    = "ml_dbenv_set_lg_regionmax"
(*      
  external set_lk_conflicts : t -> int -> int -> unit
    = "ml_dbenv_set_lg_conflicts"
*)
  external set_lk_detect : t -> lk_detect_t -> unit
    = "ml_dbenv_set_lk_detect"
  external set_lk_max_lockers : t -> int32 -> unit
    = "ml_dbenv_set_lk_max_lockers"
  external set_lk_max_locks : t -> int32 -> unit
    = "ml_dbenv_set_lk_max_locks"
  external set_lk_max_objects : t -> int32 -> unit
    = "ml_dbenv_set_lk_max_objects"
  external set_lk_partitions : t -> int32 -> unit
    = "ml_dbenv_set_lk_partitions"
  external set_mp_max_openfd : t -> int -> unit
    = "ml_dbenv_set_mp_max_openfd"
  external set_mp_max_write : t -> int -> int32 -> unit
    = "ml_dbenv_set_mp_max_write"
  external set_mp_mmapsize : t -> int32 -> unit
    = "ml_dbenv_set_mp_mmapsize"
(*    
      external set_msgcall : t -> (t -> string -> unit) -> unit
      external set_msgfile : t -> FILE * -> unit
*)
  external set_shm_key : t -> int -> unit
    = "ml_dbenv_set_shm_key"
  external set_thread_count : t -> int32 -> unit
    = "ml_dbenv_set_thread_count"
(*      
      external set_thread_id : t -> (t -> pid_t * -> db_threadid_t * -> unit) ->
        unit
  external set_thread_id_string : t ->
                (t -> pid_t -> db_threadid_t -> string -> string) -> unit
*)
  external set_timeout : t -> int32 -> dbenv_timeout_flag -> unit
    = "ml_dbenv_set_timeout"
  external set_tmp_dir : t -> string -> unit
    = "ml_dbenv_set_tmp_dir"
  external set_tx_max : t -> int32 -> unit
    = "ml_dbenv_set_tx_max"
(*
  external set_tx_timestamp : t time_t * -> unit
*)
  external set_verbose : t -> verbose_which -> bool -> unit
    = "ml_dbenv_set_verbose"
(*
  external stat_print : t -> flags -> unit
  = "ml_dbenv_stat_print"
*)
  external txn_begin : t -> ?txn:txn -> ?flags:dbenv_txn_begin_flag list ->
    unit -> txn
    = "ml_dbenv_txn_begin"
  external txn_checkpoint : t -> int32 -> int32 ->
    ?flags:dbenv_txn_checkpoint_flag list -> unit -> unit
    = "ml_dbenv_txn_checkpoint"
(*    
  external txn_recover : t -> DB_PREPLIST * -> int32 -> int32 * -> flags -> unit
      = "ml_dbenv_txn_recover"
  external txn_stat : t -> DB_TXN_STAT ** -> flags -> unit
      = "ml_dbenv_txn_stat"
  external txn_stat_print : t -> flags -> unit
      = "ml_dbenv_txn_stat_print"
*)
  
  external create : unit -> t
    = "ml_dbenv_create"
end
  
(*
module DBT =
struct
  type 'a t = 'a dbt
  external create_recno : unit -> int t
    = "ml_dbt_create_int"
  external create_string : unit -> string t
    = "ml_dbt_create_string"
  external set_data : 'a t -> 'a -> unit
    = "ml_dbt_set_data"
  external get_data : 'a t -> 'a
    = "ml_dbt_get_data"
  external set_flags : 'a t -> dbt_flag list -> unit
    = "ml_dbt_set_flags"
  external close : 'a t -> unit
    = "ml_dbt_close"
end
*)

module DbCursor =
struct
  type 'a t = 'a cursor
  external close : 'a t -> unit -> unit
    = "ml_dbcursor_close"
  external cmp : 'a t -> 'a t -> int
    = "ml_dbcursor_cmp"
  external count : 'a t -> int32
    = "ml_dbcursor_count"
  external del : 'a t -> ?flags:dbcursor_del_flag list -> unit -> unit
    = "ml_dbcursor_del"
  external dup : 'a t -> ?flags:dbcursor_dup_flag list -> unit -> 'a t
    = "ml_dbcursor_dup"
  external get : 'a t -> ?key:'a -> ?data:data -> dbcursor_get_flag list ->
    'a * data
    = "ml_dbcursor_get"
  external get_priority : 'a t -> db_cache_priority
    = "ml_dbcursor_get_priority"
  external pget : 'a t -> ?key:'a -> ?data:data ->
    ?flags:dbcursor_get_flag list -> unit -> string * data
    = "ml_dbcursor_pget"
  external put : 'a t -> ?key:'a -> data -> ?flags:dbcursor_put_flag list ->
    unit -> 'a
    = "ml_dbcursor_put"
  external set_priority : 'a t -> db_cache_priority -> unit
    = "ml_dbcursor_set_priority"
end
  
module Db =
struct
  type 'a t = 'a db
      
  external associate : 'a t -> ?txn:txn -> 'b t ->
    ('b t -> 'a -> string -> string) option ->
    ?flags:db_associate_flag list -> unit -> unit
    = "ml_db_associate_byte" "ml_db_associate"
  external associate_foreign : 'a t -> 'b t ->
    ('b t -> 'c -> string -> 'a -> int) option ->
    ?flags:db_associate_foreign_flag list -> unit
    = "ml_db_associate_foreign"
  external close : 'a t -> ?flags:db_close_flag list -> unit -> unit
    = "ml_db_close"
(*
  external compact : t ->
  txn -> string -> string -> DB_COMPACT * -> int32 -> string -> unit
*)    
  external create : ?env:DbEnv.t -> keytype -> 'a t
    = "ml_db_create"
  let create_recno ?env () : int t =
    create ?env Recno
  let create_string ?env () : string t =
    create ?env String

  external cursor : 'a t -> ?txn:txn -> ?flags:db_cursor_flag list -> unit ->
    'a DbCursor.t
    = "ml_db_cursor"
  external del : 'a t -> ?txn:txn -> string -> ?flags:db_del_flag list ->
    unit -> unit
    = "ml_db_del"
(*      
  external err : t -> int -> string -> unit
    = "ml_db_err"
  external errx : t -> string -> unit
    = "ml_db_errx"
*)        
  external exists : 'a t -> ?txn:txn -> 'a -> ?flags:db_exists_flag ->
    unit -> bool
    = "ml_db_exists"
  external fd : 'a t -> Unix.file_descr
    = "ml_db_fd"
  external get : 'a t -> ?txn:txn -> 'a ->
    ?flags:db_get_flag list -> unit -> data
    = "ml_db_get"
  external get_bt_minkey : 'a t -> int32
    = "ml_db_get_bt_minkey"
  external get_byteswapped : 'a t -> bool
    = "ml_db_get_byteswapped"
  external get_cachesize : 'a t -> int32 * int32 * int
    = "ml_db_get_cachesize"
  external get_create_dir : 'a t -> string
    = "ml_db_get_create_dir"
  external get_dbname : 'a t -> string * string
    = "ml_db_get_dbname"
  external get_encrypt_flags : 'a t -> ?flags:db_encrypt_flag list ->
    unit -> unit
    = "ml_db_get_encrypt_flags"
  external get_env : 'a t -> DbEnv.t
    = "ml_db_get_env"
(*
  external get_errfile : t -> FILE ** -> unit
*)
  external get_errpfx : 'a t -> string
    = "ml_db_get_errpfx"
  external get_flags : 'a t -> db_flag list
    = "ml_db_get_flags"
  external get_h_ffactor : 'a t -> int32
    = "ml_db_get_h_ffactor"
  external get_h_nelem : 'a t -> int32
    = "ml_db_get_h_nelem"
  external get_lorder : 'a t -> int
    = "ml_db_get_lorder"
  external get_mpf : 'a t -> mpoolfile
    = "ml_db_get_mpf"
(*    
  external get_msgfile) db -> FILE ** -> unit
*)
  external get_multiple : 'a t -> bool
    = "ml_db_get_multiple"
  external get_open_flags : 'a t -> db_open_flag list
    = "ml_db_get_open_flags"
  external get_pagesize : 'a t -> int32
    = "ml_db_get_pagesize"
(*      
  external get_partition_dirs : 'a t -> string list
        = "ml_db_get_partition_dirs"
*)
  external get_priority : 'a t -> db_cache_priority
    = "ml_db_get_priority"
  external get_q_extentsize : 'a t -> int32
    = "ml_db_get_q_extentsize"
  external get_re_delim : 'a t -> int
    = "ml_db_get_re_delim"
  external get_re_len : 'a t -> int32
    = "ml_db_get_re_len"
  external get_re_pad : 'a t -> int
    = "ml_db_get_re_pad"
  external get_re_source : 'a t -> string
    = "ml_db_get_re_source"
  external get_transactional : 'a t -> bool
    = "ml_db_get_transactional"
  external get_type : 'a t -> dbtype
    = "ml_db_get_type"
  external join : 'a t -> ('b DbCursor.t) list -> ?flags:db_join_flag list ->
    unit -> unit
    = "ml_db_join"
(*      
  external key_range : 'a t -> txn -> string -> DB_KEY_RANGE * -> unit
*)
  external db_open : 'a t -> ?txn:txn -> string -> ?database:string ->
    dbtype -> ?flags:db_open_flag list -> ?mode:int -> unit -> unit
    = "ml_db_open_byte" "ml_db_open"
  external pget : 'a t -> ?txn:txn -> 'a -> ?data:data ->
    ?flags:db_get_flag list -> unit -> string * data
    = "ml_db_pget_byte" "ml_db_pget"
  external put : 'a t -> ?txn:txn -> 'a -> data ->
    ?flags:db_put_flag list -> unit -> 'a
    = "ml_db_put_byte" "ml_db_put"
  external remove : 'a t -> ?database:string -> string -> unit
    = "ml_db_remove"
  external rename : 'a t -> string -> ?database:string -> string -> unit
    = "ml_db_rename"
(*      
  external set_append_recno : 'a t -> (t -> string -> int32) -> unit
  external set_bt_compare : 'a t -> (t -> string -> string -> int) -> unit
  external set_bt_compress : 'a t ->
        (t -> string -> string -> string -> string -> string -> int) ->
        (t -> string -> string -> string -> string -> string -> int) -> unit
*)        
  external set_bt_minkey : 'a t -> int32 -> unit
    = "ml_db_set_bt_minkey"
(*      
  external set_bt_prefix : 'a t -> (t -> string -> string -> int) -> unit
*)        
  external set_cachesize : 'a t -> int32 -> int32 -> int -> unit
    = "ml_db_set_cachesize"
  external set_create_dir : 'a t -> string -> unit
    = "ml_db_set_create_dir"
  external set_dup_compare : 'a t -> ('a t -> 'a -> 'a -> int) option ->
    unit -> unit
    = "ml_db_set_dup_compare"
  external set_encrypt : 'a t -> string -> ?flags:db_encrypt_flag list -> unit ->
    unit
    = "ml_db_set_encrypt"
(*
  external set_errcall : 'a t -> (DbEnv.t -> string -> string -> unit) -> unit
    = "ml_db_set_errcall"
*)
(*
  external set_errfile : 'a t -> FILE * -> unit
*)
  external set_errpfx : 'a t -> string -> unit
    = "ml_db_set_errpfx"
(*    
  external set_feedback : 'a t -> (t -> int -> int -> unit) -> unit
*)
  external set_flags : 'a t -> db_flag list -> unit
    = "ml_db_set_flags"
(*
  external set_h_compare : 'a t -> (t -> string -> string -> int) -> unit
*)
  external set_h_ffactor : 'a t -> int32 -> unit
    = "ml_db_set_h_ffactor"
(*      
  external set_h_hash : 'a t -> (t -> void * -> int32 -> int32) -> unit
*)
  external set_h_nelem : 'a t -> int32 -> unit
    = "ml_db_set_h_nelem"
  external set_lorder : 'a t -> int -> unit
    = "ml_db_set_lorder"
(*    
  external set_msgcall : 'a t -> (DbEnv.t -> string -> unit) -> unit
  external set_msgfile : 'a t -> FILE * -> unit
*)
  external set_pagesize : 'a t -> int32 -> unit
    = "ml_db_set_pagesize"
(*
  external set_partition : 'a t ->
  int32 -> string -> int32 ( * )(db -> string key) -> unit
  external set_partition_dirs : 'a t -> string list -> unit
*)
  external set_priority : 'a t -> db_cache_priority -> unit
    = "ml_db_set_priority"
  external set_q_extentsize : 'a t -> int32 -> unit
    = "ml_db_set_q_extentsize"
  external set_re_delim : 'a t -> int -> unit
    = "ml_db_set_re_delim"
  external set_re_len : 'a t -> int32 -> unit
    = "ml_db_set_re_len"
  external set_re_pad : 'a t -> int -> unit
    = "ml_db_set_re_pad"
  external set_re_source : 'a t -> string -> unit
    = "ml_db_set_re_source"
  external stat : 'a t -> ?txn:txn -> ?flags:db_stat_flag list -> unit -> unit
    = "ml_db_stat"
(*      
  external stat_print : 'a t -> db -> flags -> unit
*)
  external sync : 'a t -> unit
    = "ml_db_sync"
  external truncate : 'a t -> ?txn:txn -> unit -> int32
    = "ml_db_truncate"
  external upgrade : 'a t -> string -> ?flags:db_upgrade_flag list -> unit -> unit
    = "ml_db_upgrade"
  external verify : 'a t -> string -> ?database:string -> ?out:Unix.file_descr ->
    ?flags:db_verify_flag list -> unit -> unit
    = "ml_db_verify_byte" "ml_db_verify"
end

module DbTxn =
struct
  type t = txn

  external abort : t -> unit
    = "ml_dbtxn_abort"
  external commit : t -> ?flags:dbtxn_commit_flag list -> unit -> unit
    = "ml_dbtxn_commit"
  external discard : t -> unit
    = "ml_dbtxn_discard"
  external get_name : t -> string
    = "ml_dbtxn_get_name"
  external id : t -> int32
    = "ml_dbtxn_id"
(*
  external prepare : t -> int32 list -> unit
    = "ml_dbtxn_prepare"
*)
  external set_name : t -> string -> unit
    = "ml_dbtxn_set_name"
  external set_timeout : t -> int32 -> dbtxn_timeout_flag -> unit
    = "ml_dbtxn_set_timeout"
end

module LogCursor =
struct
  type t = logcursor

  external close : t -> unit -> unit
    = "ml_dblogc_close"
  external get : t -> lsn -> string -> ?flags:dblogc_get_flag list -> unit
    = "ml_dblogc_get"
end
  
(*
module MPoolFile =
struct
  type t = mpoolfile

  external close  : t -> unit
    = "ml_dbmpoolfile_close"
  external get : t -> ?txn:txn -> ?flags:dbmpoolfile_get_flag list -> page
    = "ml_dbmpoolfile_get"
  external get_clear_len  : t -> int32
    = "ml_dbmpoolfile_get_clear_len"
  external get_fileid : t -> int
    = "ml_dbmpoolfile_get_fileid"
  external get_flags : t -> db_flag list
    = "ml_dbmpoolfile_get_flags"
  external get_ftype : t -> int
    = "ml_dbmpoolfile_get_ftype"
  external get_last_pgno : t -> int32
    = "ml_dbmpoolfile_get_last_pgno"
  external get_lsn_offset : t -> int32
    = "ml_dbmpoolfile_get_lsn_offset"
  external get_maxsize : t -> int32 * int32
    = "ml_dbmpoolfile_get_maxsize"
  external get_pgcookie : t -> string
    = "ml_dbmpoolfile_get_pgcookie"
   external get_priority : t -> db_cache_priority
     = "ml_dbmpoolfile_get_priority"
   external mpf_open : t -> string -> ?flags:dbmpoolfile_open_flag list  ->
     int -> int32 -> unit
     = "ml_dbmpoolfile_open"
  external put : t -> void * -> db_cache_priority -> flags -> unit
    = "ml_dbmpoolfile_put"
   external set_clear_len : t -> int32 -> unit
     = "ml_dbmpoolfile_set_clear_len"
   external set_fileid : t -> int -> unit
     = "ml_dbmpoolfile_set_fileid"
   external set_flags : t -> int32 -> int -> unit
     = "ml_dbmpoolfile_set_flags"
  external set_ftype : t -> int -> unit
    = "ml_dbmpoolfile_set_ftype"
  external set_lsn_offset : t -> int32 -> unit
    = "ml_dbmpoolfile_set_lsn_offset"
  external set_maxsize : t -> int32 -> int32 -> unit
    = "ml_dbmpoolfile_set_maxsize"
  external set_pgcookie : t -> string -> unit
    = "ml_dbmpoolfile_set_pgcookie"
  external set_priority : t -> db_cache_priority-> unit
    = "ml_dbmpoolfile_set_priority"
  external sync : t -> unit
    = "ml_dbmpoolfile_sync"
end
*)
  
module DbSequence =
struct
  type t = sequence
      
  external close : t -> unit
    = "ml_dbsequence_close"
(*      
  external get : t -> txn -> int32 -> db_seq_t -> flags -> unit
    = "ml_dbsequence_get"
*)
  external get_cachesize : t -> int32
    = "ml_dbsequence_get_cachesize"
  external get_db : t -> 'a Db.t
    = "ml_dbsequence_get_db"
  external get_flags : t -> dbsequence_flag list
    = "ml_dbsequence_get_flags"
  external get_key : t -> string
    = "ml_dbsequence_get_key"
(*      
  external get_range : t -> db_seq_t * -> db_seq_t * -> unit
    = "ml_dbsequence_get_range"
  external initial_value : t -> db_seq_t -> unit
    = "ml_dbsequence_initial_value"
*)
  external seq_open : t -> ?txn:txn -> string ->
    ?flags:dbsequence_open_flag list -> unit -> unit
    = "ml_dbsequence_open"
  external remove : t -> ?txn:txn -> ?flags:dbsequence_remove_flag list ->
    unit -> unit
    = "ml_dbsequence_remove"
  external set_cachesize : t -> int32 -> unit
    = "ml_dbsequence_set_cachesize"
  external set_flags : t -> dbsequence_flag list -> unit
    = "ml_dbsequence_set_flags"
(*
  external set_range : t -> db_seq_t -> db_seq_t -> unit
    = "ml_dbsequence_set_range"
  external stat : t -> DB_SEQUENCE_STAT ** -> flags -> unit
    = "ml_dbsequence_stat"
  external stat_print : t -> flags -> unit
    = "ml_dbsequence_stat_print"
*)
end

external db_strerror : int -> unit
  = "ml_db_strerror"
      
external db_version : unit -> string * int * int * int
  = "ml_db_version"

let init () =
  Callback.register_exception "dberror" (Error "");
  Callback.register_exception "dbkeyempty" KeyEmpty;
  Callback.register_exception "dbkeyexists" KeyExists;
  Callback.register_exception "dbdeadlock" Deadlock;
  Callback.register_exception "dblocknotgranted" LockNotGranted;
  Callback.register_exception "dbrunrecovery" RunRecovery;
  Callback.register_exception "dbsecondarybad" SecondaryBad
