/*
 * (c) 2010 Anastasia Gornostaeva <ermine@ermine.pp.ru>
 */

#include <caml/alloc.h>
#include <caml/mlvalues.h>
#include <caml/fail.h>
#include <caml/memory.h>
#include <caml/custom.h>
#include <caml/signals.h>
#include <caml/callback.h>

#include <db48/db.h>
#include <string.h>

#include "db_flags.h"

#define DB_ENV_val(v)         (*((DB_ENV **) Data_custom_val(v)))
#define DB_val(v)             (*((DB **) Data_custom_val(v)))
#define DB_TXN_val(v)         (*((DB_TXN **) Data_custom_val(v)))
#define DBC_val(v)            (*((DBC **) Data_custom_val(v)))
#define DB_SEQ_val(v)         (*((DB_SEQUENCE **) Data_custom_val(v)))
#define DB_MPOOLFILE_val(v)   (*((DB_MPOOLFILE **) Data_custom_val(v)))
#define DB_LOGC_val(v)        (*((DB_LOGC **) Data_custom_val(v)))

#ifdef CHECK_HANDLE
#define TEST_HANDLE(v) \
  if(v == NULL) caml_invalid_argument(#v ": null pointer")
#else
#define TEST_HANDLE(v)
#endif

#define Some_val(v)     (Field(v,0))

//+ add call TEST_HANDLE
#define DB_TXN_opt_val(v)  (Is_block(v)) ? DB_TXN_val(Some_val(v)) : NULL

static void finalize_dbenv(value block) {
  DB_ENV *dbenv = DB_ENV_val(block);
  printf("finalizing dbenv\n");
  if(dbenv != NULL) {
    (void) dbenv->close(dbenv, 0);
    DB_ENV_val(block) = NULL;
  }
  else
    printf("dbenv is NULL\n");
}

static struct custom_operations dbenv_ops = {
  "caml_dbenv",
  finalize_dbenv,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

static void finalize_db(value block) {
  DB *db = DB_val(block);
  printf("finalizing db\n");
  if(db != NULL) {
    //? how to check retval
    (void) db->close(db, 0);
    DB_val(block) = NULL;
  }
  else
    printf("db is NULL\n");
}

static struct custom_operations db_ops = {
  "caml_db",
  finalize_db,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

static void finalize_dbc(value block) {
  DBC *dbc = DBC_val(block);
  printf("finalizing dbc\n");
  if(dbc != NULL) {
    (void) dbc->close(dbc);
    DBC_val(block) = NULL;
  }
  else
    printf("dbc is NULL\n");
}

static struct custom_operations dbc_ops = {
  "caml_dbc",
  finalize_dbc,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

static struct custom_operations dbtxn_ops = {
  "caml_dbtxn",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

static void finalize_dbmpf(value block) {
  DB_MPOOLFILE *mpf = DB_MPOOLFILE_val(block);
  printf("finalizing dbmpf\n");
  if(mpf != NULL) {
    (void) mpf->close(mpf, 0);
    DB_MPOOLFILE_val(block) = NULL;
  }
  else
    printf("mpf is NULL\n");
}

static struct custom_operations dbmpf_ops = {
  "caml_dbmpf",
  finalize_dbmpf,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

enum db_cb_values {
  V_ERRCALL = 0, // must be first item
  V_DB_ASSOCIATE,
  V_DB_ASSOCIATE_FOREIGN,
  V_DB_DUP_COMPARE,
  TOTAL_DB_VALUES
};

enum dbenv_cb_values {
  V__ERRCALL = 0,  // must be first item
  V_DBENV_EVENT_NOTIFY,
  TOTAL_DBENV_VALUES
};

static value create_app_data(int size) {
  CAMLparam0();
  CAMLlocal1(app_data);
  int i;

  app_data = caml_alloc(size, 0);
  caml_register_generational_global_root(&app_data);
  for(i = 0; i < size; i++)
    Store_field(app_data, i, Val_unit);
  CAMLreturn(app_data);
}

static void check_retval(int ret) {
  static value *mlbdb_error_exn = NULL;
  static value *mlbdb_keyempty_exn = NULL;
  static value *mlbdb_keyexists_exn = NULL;
  static value *mlbdb_deadlock_exn = NULL;
  static value *mlbdb_locknotgranted_exn = NULL;
  static value *mlbdb_runrecovery_exn = NULL;
  static value *mlbdb_secondarybad_exn = NULL;

  char* msg;

  if(ret != 0)
    switch(ret) {
    case DB_NOTFOUND: 
      caml_raise_not_found();
    case DB_KEYEMPTY:
      if (mlbdb_keyempty_exn == NULL) {
        mlbdb_keyempty_exn = caml_named_value("dbkeyempty");
        if (mlbdb_keyempty_exn == NULL)
          caml_invalid_argument("init: Exception Error is not initialized");
      }
      caml_raise_constant(*mlbdb_keyempty_exn);
    case DB_KEYEXIST:
      mlbdb_keyexists_exn = caml_named_value("dbkeyexists");
      if (mlbdb_keyexists_exn == NULL)
        caml_invalid_argument("init: Exception Error is not initialized");
      caml_raise_constant(*mlbdb_keyexists_exn);
    case DB_LOCK_DEADLOCK:
      if (mlbdb_deadlock_exn == NULL) {
        mlbdb_deadlock_exn = caml_named_value("dbdeadlock");
        if (mlbdb_deadlock_exn == NULL)
          caml_invalid_argument("init: Exception Error is not initialized");
      }
      caml_raise_constant(*mlbdb_deadlock_exn);
    case DB_LOCK_NOTGRANTED:
      if (mlbdb_locknotgranted_exn == NULL) {
        mlbdb_locknotgranted_exn = caml_named_value( "dblocknotgranted");
        if (mlbdb_locknotgranted_exn == NULL)
          caml_invalid_argument("init: Exception Error is not initialized");
      }
      caml_raise_constant(*mlbdb_locknotgranted_exn);
    case DB_RUNRECOVERY: 
      if (mlbdb_runrecovery_exn == NULL) {
        mlbdb_runrecovery_exn = caml_named_value( "dbrunrecovery");
        if (mlbdb_runrecovery_exn == NULL)
          caml_invalid_argument("init: Exception RunRecovery is not initialized");
      }
      caml_raise_constant(*mlbdb_runrecovery_exn);
    case DB_SECONDARY_BAD: 
      if (mlbdb_secondarybad_exn == NULL) {
        mlbdb_secondarybad_exn = caml_named_value("dbsecondarybad");
        if (mlbdb_secondarybad_exn == NULL)
          caml_invalid_argument("init: Exception SecondaryBad is not initialized");
      }
      caml_raise_constant(*mlbdb_secondarybad_exn);
    default:
      if (mlbdb_error_exn == NULL) {
        mlbdb_error_exn = caml_named_value("dberror");
        if (mlbdb_error_exn == NULL)
          caml_invalid_argument("init: Exception Error is not initialized");
      }
      msg = db_strerror(ret);
      caml_raise_with_string(*mlbdb_error_exn, msg);
    }
  return;
}

/*
static value Val_flags(int flags, int *tbl) {
  CAMLparam0();
  CAMLlocal2(li, cons);
  int i;
  size_t len = sizeof(tbl)/sizeof(tbl[0]);

  li = Val_emptylist;
  for(i = 0; i < len; i++) {
  if(flags & tbl[i]) {
  cons = caml_alloc(2, 0);
  Store_field(cons, 0, i);
  Store_field(cons, 1, li);
  li = cons;
  }
  }
  CAMLreturn(li);
}
*/

#define Size(v)   sizeof(v)/sizeof(*v)

static inline u_int32_t Variant_val(value vflag, u_int32_t tbl[][2], 
                                    size_t len) {
  int i;
  for(i=0; i<len; i++)
    if(vflag == tbl[i][1])
      return tbl[i][0];
  caml_invalid_argument("Bad flag");
}

static int Variants_val(value vflags, u_int32_t tbl[][2], size_t len) {
  value head;
  u_int32_t flags = 0;

  while(vflags != Val_emptylist) {
    head = Field(vflags, 0);
    flags |= Variant_val(head, tbl, len);
    vflags = Field(vflags, 1);
  }
  return flags;
}

#define DBFlags_val(vflags, tbl) \
  (Is_block(vflags)) ? Variants_val(Some_val(vflags),tbl, Size(tbl)) : 0

static value Val_variants(value vflags, u_int32_t tbl[][2], size_t len) {
  CAMLparam1(vflags);
  CAMLlocal2(li, cons);
  int i;

  li = Val_emptylist;
  for(i = 0; i < len; i++) {
    if(vflags & tbl[i][0]) {
      cons = caml_alloc(2, 0);
      Store_field(cons, 0, tbl[i][1]);
      Store_field(cons, 1, li);
      li = cons;
    }
  }
  CAMLreturn(li);
}

static int ml2enum(value v, int *tbl, size_t len) {
  int i = Int_val(v);
  if(i < len)
    return tbl[i];
  else
    caml_invalid_argument("ml2enum");
}

static value enum2ml(int v, int *tbl, size_t len) {
  int i;

  for(i = 0; i < len && tbl[i] != v; i++);
  if(i < len)
    return Val_int(i);
  else
    caml_invalid_argument("enum2ml");
}

static int associate_stub(DB *db, 
                          const DBT *key, const DBT *data, DBT *result) {
  CAMLparam0();
  CAMLlocal4(vdb, vkey, vdata, vret);

  vdb = caml_alloc_custom(&db_ops, sizeof(DB*), 0, 1);
  DB_val(vdb) = db;
  vkey = caml_alloc_string(key->size);
  memcpy(String_val(vkey), key->data, key->size);
  vdata = caml_alloc_string(data->size);
  memcpy(String_val(vdata), data->data, data->size);

  vret = caml_callback3(Field(db->app_private, V_DB_ASSOCIATE), 
                        vdb, vkey, vdata);

  memset(result, 0, sizeof(DBT));
  result->data = String_val(vret);
  result->size = caml_string_length(vret);
  
  CAMLreturnT(int, 0);
}

static int dbpriority_enum[] = {
  DB_PRIORITY_UNCHANGED,
  DB_PRIORITY_VERY_LOW,
  DB_PRIORITY_LOW,
  DB_PRIORITY_DEFAULT,
  DB_PRIORITY_HIGH,
  DB_PRIORITY_VERY_HIGH
};

#define DBPriority_val(v)   ml2enum(v, dbpriority_enum, Size(dbpriority_enum))
#define Val_dbpriority(v)   enum2ml(v, dbpriority_enum, Size(dbpriority_enum))

CAMLprim value ml_db_associate(value vdb, value vtxn, value vsecondary, 
                               value vcallback, value vflags, value unit) {
  DB *db = DB_val(vdb);
  DB *secondary = DB_val(vsecondary);
  DB_TXN *txn = DB_TXN_opt_val(vtxn);
  u_int32_t flags = DBFlags_val(vflags, flags_db_associate);
  int ret;

  TEST_HANDLE(db);
  TEST_HANDLE(secondary);
  if(secondary->app_private == NULL)                                  \
    secondary->app_private = (void*) create_app_data(TOTAL_DB_VALUES);
  Store_field(secondary->app_private, V_DB_ASSOCIATE, vcallback);
  ret =  db->associate(db, txn, secondary, associate_stub, flags);
  check_retval(ret);
  return Val_unit;
}

CAMLprim value ml_db_associate_byte(value *argv, int argn) {
  return ml_db_associate(argv[0], argv[1], argv[2], argv[3], argv[4], argv[5]);
}

static int associate_foreign_stub(DB *db, 
                                  const DBT *key, DBT *data, const DBT *fkey,
                                  int *changed) {
  CAMLparam0();
  CAMLlocal1(vres);
  CAMLlocalN(vargs, 5);

  vres = caml_alloc_custom(&db_ops, sizeof(DB*), 0, 1);
  DB_val(vres) = db;
  vargs[0] = vres;

  vres = caml_alloc_string(key->size);
  memcpy(String_val(vres), key->data, key->size);
  vargs[1] = vres;

  vres = caml_alloc_string(data->size);
  memcpy(String_val(vres), data->data, data->size);
  vargs[2] = vres;

  vres = caml_alloc_string(fkey->size);
  memcpy(String_val(vres), fkey->data, fkey->size);
  vargs[3] = vres;

  caml_callbackN(Field(db->app_private, V_DB_ASSOCIATE_FOREIGN), 5, vargs);
  /*
  memset(result, 0, sizeof(DBT));
  result->data = String_val(vret);
  result->size = caml_string_length(vret);
  */

  CAMLreturnT(int, 0);
}

CAMLprim value ml_db_associate_foreign(value vdb, value vsecondary,
                                       value vcallback, value vflags, 
                                       value unit) {
  DB *db = DB_val(vdb);
  DB *secondary = DB_val(vsecondary);
  u_int32_t flags = DBFlags_val(vflags, flags_db_associate_foreign);
  int (*callback)(DB *, const DBT *, DBT *, const DBT *, int *) = NULL;
  int ret;

  TEST_HANDLE(db);
  if(Is_block(vcallback)) {
    if(secondary->app_private == NULL)                                  \
      secondary->app_private = (void*) create_app_data(TOTAL_DB_VALUES);
    callback = &associate_foreign_stub;
    Store_field(secondary->app_private, V_DB_ASSOCIATE_FOREIGN, 
                Some_val(vcallback));
  }
  ret =  db->associate_foreign(db, secondary, callback, flags);
  check_retval(ret);
  return Val_unit;
}

static int dup_compare_stub(DB* db, const DBT *dbt1, const DBT *dbt2) {
  CAMLparam0();
  CAMLlocal4(vret, vdb, vdata1, vdata2);

  vdb = caml_alloc_custom(&db_ops, sizeof(DB*), 0, 1);
  DB_val(vdb) = db;
  vdata1 = caml_alloc_string(dbt1->size);
  memcpy(String_val(vdata1), dbt1->data, dbt1->size);
  vdata2 = caml_alloc_string(dbt2->size);
  memcpy(String_val(vdata2), dbt2->data, dbt2->size);

  vret = caml_callback3(Field(db->app_private, V_DB_DUP_COMPARE), 
                        vdb, vdata1, vdata2);

  CAMLreturnT(int, Int_val(vret));
}

CAMLprim value ml_db_set_dup_compare(value vdb, value vcallback) {
  DB *db = DB_val(vdb);
  int ret;
  TEST_HANDLE(db);
  if(db->app_private == NULL)                                  \
    db->app_private = (void*) create_app_data(TOTAL_DB_VALUES);
  Store_field(db->app_private, V_DB_DUP_COMPARE, vcallback);
  ret = db->set_dup_compare(db, dup_compare_stub);
  check_retval(ret);
  return Val_unit;
}

static void errcall_stub(const DB_ENV *dbenv, 
                         const char *errpfx, const char *msg) {
  CAMLparam0();
  CAMLlocal3(vdbenv, vstr1, vstr2);

  vdbenv = caml_alloc_custom(&dbenv_ops, sizeof(DB_ENV*), 0, 1);
  DB_ENV_val(vdbenv) = (DB_ENV*)dbenv;
  vstr1 = caml_copy_string(errpfx);
  vstr2 = caml_copy_string(msg);

  caml_callback3(Field(dbenv->app_private, V_ERRCALL), 
                 vdbenv, vstr1, vstr2);

  CAMLreturn0;
}

//? Hm, where i should save the callback?
//  The callback stub does not see DB->app_private
CAMLprim value ml_db_set_errcall(value vdb, value vcallback) {
  CAMLparam2(vdb, vcallback);
  DB *db = DB_val(vdb);
  TEST_HANDLE(db);

  if(db->app_private == NULL)                                  \
    db->app_private = (void*) create_app_data(TOTAL_DB_VALUES);
  Store_field(db->app_private, V_ERRCALL, vcallback);
  db->set_errcall(db, &errcall_stub);
  CAMLreturn(Val_unit);
}

CAMLprim value ml_dbenv_set_errcall(value vdbenv, value vcallback) {
  CAMLparam2(vdbenv, vcallback);
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  void (*callback) (const DB_ENV*, const char*, const char*) = NULL;
  TEST_HANDLE(dbenv);
  if(Is_block(vcallback)) {
    if(dbenv->app_private == NULL)                                      \
      dbenv->app_private = (void*) create_app_data(TOTAL_DBENV_VALUES);
    Store_field(dbenv->app_private, V_ERRCALL, vcallback);
    callback = errcall_stub;
  } else {
    if(dbenv->app_private != NULL)
      Store_field(dbenv->app_private, V_ERRCALL, Val_unit);
  }
  dbenv->set_errcall(dbenv, callback);
  CAMLreturn(Val_unit);
}

CAMLprim value ml_db_close(value vdb, value vflags, value unit) {
  DB *db = DB_val(vdb);
  u_int32_t flags = DBFlags_val(vflags, flags_db_close);
  int ret;

  TEST_HANDLE(db);
  if(db->app_private != NULL)
    caml_remove_generational_global_root(db->app_private);
  ret = db->close(db, flags);
  DB_val(vdb) = NULL;
  check_retval(ret);
  return Val_unit;
}

CAMLprim value ml_db_create(value vdbenv_opt, value unit) {
  CAMLparam2(vdbenv_opt, unit);
  CAMLlocal1(vres);
  DB *db = NULL;
  DB_ENV *dbenv = NULL;
  int ret;

  if(Is_block(vdbenv_opt))
    dbenv = DB_ENV_val(Some_val(vdbenv_opt));

  ret = db_create(&db, dbenv, 0);
  if(ret != 0 && db != NULL) {
    db->close(db, 0);
    db = NULL;
  }
  check_retval(ret);

  vres = caml_alloc_custom(&db_ops, sizeof(DB*), 0, 1);
  DB_val(vres) = db;
  CAMLreturn(vres);
}

static int dbtype_enum[] = {
  DB_BTREE,
  DB_HASH,
  DB_RECNO,
  DB_QUEUE,
  DB_UNKNOWN
};

CAMLprim value ml_db_open(value vdb, value vtxn, value vfname, 
                          value vdatabase, value vdbtype, 
                          value vflags, value vmode, value unit) {
  DB *db = DB_val(vdb);
  DB_TXN *txn = DB_TXN_opt_val(vtxn);
  DBTYPE dbtype = ml2enum(vdbtype, dbtype_enum, Size(dbtype_enum));
  char *database = NULL;
  u_int32_t flags = DBFlags_val(vflags, flags_db_open);
  int mode = 0;
  int ret;

  TEST_HANDLE(db);

  if(Is_block(vdatabase))
    database = String_val(Some_val(vdatabase));

  if(Is_block(vmode))
    mode = Int_val(Some_val(vmode));

  ret = db->open(db, txn, String_val(vfname), database, dbtype, flags, mode);
  check_retval(ret);

  return Val_unit;
}

CAMLprim value ml_db_open_byte(value *argv, int argn) {
  return ml_db_open(argv[0], argv[1], argv[2], argv[3], argv[4], 
                    argv[5], argv[6], argv[7]);
}
 
CAMLprim value ml_db_del(value vdb, value vtxn, value vkey, value vflags,
                         value unit) {
  DB *db = DB_val(vdb);
  DB_TXN *txn = DB_TXN_opt_val(vtxn);
  DBT key;
  u_int32_t flags = DBFlags_val(vflags, flags_db_del);
  int ret;

  TEST_HANDLE(db);

  memset(&key, 0, sizeof(DBT));
  key.data = String_val(vkey);
  key.size = caml_string_length(vkey);

  ret = db->del(db, txn, &key, flags);
  check_retval(ret);
  return Val_unit;
}

CAMLprim value ml_db_exists(value vdb, value vtxn, value vkey, value vflags,
                            value unit) {
  DB *db = DB_val(vdb);
  DB_TXN *txn = DB_TXN_opt_val(vtxn);
  DBT key;
  u_int32_t flags = DBFlags_val(vflags, flags_db_exists);
  int ret;

  TEST_HANDLE(db);

  memset(&key, 0, sizeof(DBT));
  key.data = String_val(vkey);
  key.size = caml_string_length(vkey);

  ret = db->exists(db, txn, &key, flags);
  if(ret == DB_NOTFOUND)
    return Val_false;
  else {
    check_retval(ret);
    return Val_true;
  }
}

CAMLprim value ml_db_get(value vdb, value vtxn, value vkey, value vflags, 
                         value unit) {
  CAMLparam5(vdb, vtxn, vkey, vflags, unit);
  CAMLlocal1(vres);
  DB *db = DB_val(vdb);
  DB_TXN *txn = DB_TXN_opt_val(vtxn);
  u_int32_t flags = DBFlags_val(vflags, flags_db_get);
  DBT key, data;
  int ret;

  TEST_HANDLE(db);

  memset(&key, 0, sizeof(DBT));
  key.data = String_val(vkey);
  key.size = caml_string_length(vkey);

  memset(&data, 0, sizeof(DBT));

  ret = db->get(db, txn, &key, &data, flags);
  check_retval(ret);

  vres = caml_alloc_string(data.size);
  memcpy(String_val(vres), data.data, data.size);
  CAMLreturn(vres);
}

CAMLprim value ml_db_pget(value vdb, value vtxn, value vkey, value vflags, 
                         value unit) {
  CAMLparam5(vdb, vtxn, vkey, vflags, unit);
  CAMLlocal3(vres, vstr1, vstr2);
  DB *db = DB_val(vdb);
  DB_TXN *txn = DB_TXN_opt_val(vtxn);
  u_int32_t flags = DBFlags_val(vflags, flags_db_get);
  DBT key, pkey, pdata;
  int ret;

  TEST_HANDLE(db);

  memset(&key, 0, sizeof(DBT));
  key.data = String_val(vkey);
  key.size = caml_string_length(vkey);

  memset(&pkey, 0, sizeof(DBT));
  memset(&pdata, 0, sizeof(DBT));

  ret = db->pget(db, txn, &key, &pkey, &pdata, flags);
  check_retval(ret);

  vstr1 = caml_alloc_string(pkey.size);
  memcpy(String_val(vstr1), pkey.data, pkey.size);
  vstr2 = caml_alloc_string(pdata.size);
  memcpy(String_val(vstr2), pdata.data, pdata.size);
  vres = caml_alloc_tuple(2);
  Store_field(vres, 0, vstr1);
  Store_field(vres, 1, vstr2);
  CAMLreturn(vres);
}

CAMLprim value ml_db_join(value vdb, value vcursors, value vflags, value unit) {
  CAMLparam4(vdb, vcursors, vflags, unit);
  CAMLlocal2(head, vres);
  DB *db = DB_val(vdb);
  DBC **cursorlist;
  DBC *newcursor;
  u_int32_t flags = DBFlags_val(vflags, flags_db_join);
  int i, ret, clen;

  TEST_HANDLE(db);

  head = vcursors;
  for(clen = 0; head != Val_emptylist; clen++)
    head = Field(head, 1);
  cursorlist = (DBC**)caml_stat_alloc(sizeof(DBC) * clen);

  for(i = 0; vcursors != Val_emptylist; i++) {
    head = Field(vcursors, 0);
    cursorlist[i] = DBC_val(head);
    TEST_HANDLE(cursorlist[i]);
    vcursors = Field(vcursors, 1);
  }
  cursorlist[i] = NULL;

  ret = db->join(db, cursorlist, &newcursor, flags);
  caml_stat_free(cursorlist);
  check_retval(ret);

  vres = caml_alloc_custom(&dbc_ops, sizeof(DBC*), 0, 1);
  DBC_val(vres) = newcursor;
  CAMLreturn(vres);
} 

CAMLprim value ml_db_get_multiple(value vdb) {
  DB *db = DB_val(vdb);
  int ret;
  TEST_HANDLE(db);
  ret = db->get_multiple(db);
  check_retval(ret);
  return Val_bool(ret);
}

CAMLprim value ml_db_set_encrypt(value vdb, value vpasswd, value vflags, 
                                 value unit) {
  DB *db = DB_val(vdb);
  u_int32_t flags = DBFlags_val(vflags, flags_db_encrypt);
  int ret;

  TEST_HANDLE(db);
  ret = db->set_encrypt(db, String_val(vpasswd), flags);
  check_retval(ret);

  return Val_unit;
}

// make optional return
CAMLprim value ml_db_get_env(value vdb) {
  CAMLparam1(vdb);
  CAMLlocal1(vres);
  DB *db = DB_val(vdb);
  DB_ENV *dbenv = db->get_env(db);
  vres = caml_alloc_custom(&dbenv_ops, sizeof(DB_ENV*), 0, 1);
  DB_ENV_val(vres) = dbenv;
  CAMLreturn(vres);
}  

CAMLprim value ml_db_get_errpfx(value vdb) {
  CAMLparam1(vdb);
  DB *db = DB_val(vdb);
  const char *str;
  TEST_HANDLE(db);
  db->get_errpfx(db, &str);
  CAMLreturn(caml_copy_string(str));
}

CAMLprim value ml_db_set_errpfx(value vdb, value vstr) {
  DB *db = DB_val(vdb);
  TEST_HANDLE(db);
  db->set_errpfx(db, String_val(vstr));
  return Val_unit;
}

CAMLprim value ml_db_get_transactional(value vdb) {
  DB *db = DB_val(vdb);
  int ret;
  TEST_HANDLE(db);
  ret = db->get_transactional(db);
  return Val_bool(ret);
}

CAMLprim value ml_db_get_type(value vdb) {
  DB *db = DB_val(vdb);
  DBTYPE dbtype;
  int ret;
  TEST_HANDLE(db);
  ret = db->get_type(db, &dbtype);
  check_retval(ret);
  return enum2ml(dbtype, dbtype_enum, Size(dbtype_enum));
}

CAMLprim value ml_db_put(value vdb, value vtxn, value vkey, value vdata, 
                         value vflags, value unit) {
  DB *db = DB_val(vdb);
  DB_TXN *txn = DB_TXN_opt_val(vtxn);
  DBT key, data;
  u_int32_t flags = DBFlags_val(vflags, flags_db_put);
  int ret;

  TEST_HANDLE(db);

  memset(&key, 0, sizeof(DBT));
  memset(&data, 0, sizeof(DBT));
  key.data = String_val(vkey);
  key.size = caml_string_length(vkey);
  data.data = String_val(vdata);
  data.size = caml_string_length(vdata);

  ret = db->put(db, txn, &key, &data, flags);
  check_retval(ret);
  return Val_unit;
}

CAMLprim value ml_db_put_byte(value *argv, int argn) {
  return ml_db_put(argv[0], argv[1], argv[2], argv[3], argv[4], argv[5]);
}


CAMLprim value ml_db_remove(value vdb, value vdatabase, value vfile) {
  DB *db = DB_val(vdb);
  char* database = NULL;
  int ret;

  TEST_HANDLE(db);

  if(Is_block(vdatabase))
    database = String_val(Some_val(vdatabase));

  ret = db->remove(db, String_val(vfile), database, 0);
  check_retval(ret);
  return Val_unit;
}

CAMLprim value ml_db_rename(value vdb, value vfile, value vdatabase,
                            value vnewfile) {
  DB *db = DB_val(vdb);
  char* database = NULL;
  int ret;

  TEST_HANDLE(db);

  if(Is_block(vdatabase))
    database = String_val(Some_val(vdatabase));

  ret = db->rename(db, String_val(vfile), database, String_val(vnewfile), 0);
  check_retval(ret);
  return Val_unit;
}

CAMLprim value ml_db_get_cachesize(value vdb) {
  CAMLparam1(vdb);
  CAMLlocal3(vres, vr1, vr2);
  DB *db = DB_val(vdb);
  u_int32_t gbytes, bytes;
  int ncache;
  int ret;

  TEST_HANDLE(db);

  ret = db->get_cachesize(db, &gbytes, &bytes, &ncache);
  check_retval(ret);

  vr1 = caml_copy_int32(gbytes);
  vr2 = caml_copy_int32(bytes);
  vres = caml_alloc_tuple(3);
  Store_field(vres, 0, vr1);
  Store_field(vres, 1, vr2);
  Store_field(vres, 2, Val_int(ncache));
  CAMLreturn(vres);
}

CAMLprim value ml_db_stat(value vdb, value vtxn, value vflags, value unit) {
  DB *db = DB_val(vdb);
  DB_TXN *txn = DB_TXN_opt_val(vtxn);
  void *sp = NULL;
  u_int32_t flags = DBFlags_val(vflags, flags_db_stat);
  int ret;
  TEST_HANDLE(db);
  ret = db->stat(db, txn, sp, flags);
  check_retval(ret);

  return Val_unit;
}

CAMLprim value ml_db_truncate(value vdb, value vtxn, value unit) {
  CAMLparam3(vdb, vtxn, unit);
  DB *db = DB_val(vdb);
  DB_TXN *txn = DB_TXN_opt_val(vtxn);
  u_int32_t count;
  int ret;

  ret = db->truncate(db, txn, &count, 0);
  check_retval(ret);

  CAMLreturn(caml_copy_int32(count));
}

CAMLprim value ml_db_upgrade(value vdb, value vfname, value vflags, value unit) {
  DB *db = DB_val(vdb);
  u_int32_t flags = DBFlags_val(vflags, flags_db_upgrade);
  int ret;

  ret = db->upgrade(db, String_val(vfname), flags);
  check_retval(ret);

  return Val_unit;
}

CAMLprim value ml_db_verify(value vdb, value vfile, value vdatabase, 
                            value vout, value vflags, value unit) {
  DB *db = DB_val(vdb);
  char *database = NULL;
  FILE *outfile = NULL;
  u_int32_t flags = DBFlags_val(vflags, flags_db_verify);
  int ret;

  if(Is_block(vdatabase))
    database = String_val(Some_val(vdatabase));

  ret = db->verify(db, String_val(vfile), database, outfile, flags);
  check_retval(ret);

  return Val_unit;
}

CAMLprim value ml_db_verify_byte(value *argv, int argn) {
  return ml_db_verify(argv[0], argv[1], argv[2], argv[3], argv[4], argv[5]);
}

CAMLprim value ml_db_cursor(value vdb, value vtxn, value vflags, value unit) {
  CAMLparam4(vdb, vtxn, vflags, unit);
  CAMLlocal1(vres);
  DB *db = DB_val(vdb);
  DB_TXN *txn = DB_TXN_opt_val(vtxn);
  DBC *cursor;
  u_int32_t flags = DBFlags_val(vflags, flags_db_cursor);
  int ret;

  TEST_HANDLE(db);
  ret = db->cursor(db, txn, &cursor, flags);
  check_retval(ret);

  vres = caml_alloc_custom(&dbc_ops, sizeof(DBC*), 0, 1);
  DBC_val(vres) = cursor;
  CAMLreturn(vres);
}

CAMLprim value ml_db_get_mpf(value vdb) {
  CAMLparam1(vdb);
  CAMLlocal1(vres);
  DB *db = DB_val(vdb);
  DB_MPOOLFILE *mpf = NULL;
  TEST_HANDLE(db);
  mpf = db->get_mpf(db);
  vres = caml_alloc_custom(&dbmpf_ops, sizeof(DB_MPOOLFILE*), 0, 1);
  DB_MPOOLFILE_val(vres) = mpf;
  CAMLreturn(vres);
}

CAMLprim value ml_db_sync(value vdb) {
  DB *db = DB_val(vdb);
  int ret;

  TEST_HANDLE(db);
  ret = db->sync(db, 0);
  check_retval(ret);

  return Val_unit;
}

CAMLprim value ml_dbenv_get_cachesize(value vdbenv) {
  CAMLparam1(vdbenv);
  CAMLlocal3(vres, vr1, vr2);
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  u_int32_t gbytes, bytes;
  int ncache;
  int ret;

  TEST_HANDLE(dbenv);
  ret = dbenv->get_cachesize(dbenv, &gbytes, &bytes, &ncache);
  check_retval(ret);

  vr1 = caml_copy_int32(gbytes);
  vr2 = caml_copy_int32(bytes);
  vres = caml_alloc_tuple(3);
  Store_field(vres, 0, caml_copy_int32(gbytes));
  Store_field(vres, 1, caml_copy_int32(bytes));
  Store_field(vres, 2, Val_int(ncache));
  CAMLreturn(vres);
}

CAMLprim value ml_dbenv_get_timeout(value vdbenv, value vflag) {
  CAMLparam2(vdbenv, vflag);
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  db_timeout_t timeout;
  u_int32_t flag = 
    Variant_val(vflag, flags_dbenv_get_timeout, Size(flags_dbenv_get_timeout));
  int ret;
  TEST_HANDLE(dbenv);
  ret = dbenv->get_timeout(dbenv, &timeout, flag);
  check_retval(ret);
  CAMLreturn(caml_copy_int32(timeout));
}

CAMLprim value ml_dbenv_set_timeout(value vdbenv, value timeout, value vflag) {
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  u_int32_t flag =
    Variant_val(vflag, flags_dbenv_timeout, Size(flags_dbenv_timeout));
  int ret;
  TEST_HANDLE(dbenv);
  ret = dbenv->set_timeout(dbenv, Int32_val(timeout), flag);
  check_retval(ret);
  return Val_unit;
}

CAMLprim value ml_dbenv_close(value vdbenv, value unit) {
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  int ret;
  TEST_HANDLE(dbenv);
  if(dbenv->app_private != NULL)
    caml_remove_generational_global_root(dbenv->app_private);
  ret = dbenv->close(dbenv, 0);
  DB_ENV_val(vdbenv) = NULL;
  check_retval(ret);
  return Val_unit;
}
  
CAMLprim value ml_dbenv_create(value unit) {
  CAMLparam1(unit);
  CAMLlocal1(vres);
  DB_ENV *dbenv = NULL;
  int ret;

  ret = db_env_create(&dbenv, 0);
  if(ret != 0 && dbenv != NULL) {
    dbenv->close(dbenv, 0);
    dbenv = NULL;
  }
  check_retval(ret);

  vres = caml_alloc_custom(&dbenv_ops, sizeof(DB_ENV*), 0, 1);
  DB_ENV_val(vres) = dbenv;
  CAMLreturn(vres);
}

CAMLprim value ml_dbenv_dbremove(value vdbenv, value vtxn, value vfile,
                                 value vdatabase, value vflags, value unit) {
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  DB_TXN *txn = DB_TXN_opt_val(vtxn);
  u_int32_t flags = DBFlags_val(vflags, flags_dbenv_dbremove);
  int ret;

  TEST_HANDLE(dbenv);
  ret = dbenv->dbremove(dbenv, txn, String_val(vfile), String_val(vdatabase),
                        flags);
  check_retval(ret);

  return Val_unit;
}

CAMLprim value ml_dbenv_dbremove_byte(value *argv, int argn) {
  return ml_dbenv_dbremove(argv[0], argv[1], argv[2], argv[3], argv[4], 
                           argv[5]);
}

CAMLprim value ml_dbenv_dbrename(value vdbenv, value vtxn, value vfile,
                                 value vdatabase, value vnewname, value vflags,
                                 value unit) {
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  DB_TXN *txn = DB_TXN_opt_val(vtxn);
  u_int32_t flags = DBFlags_val(vflags, flags_dbenv_dbrename);
  int ret;
  
  TEST_HANDLE(dbenv);
  ret = dbenv->dbrename(dbenv, txn, String_val(vfile), String_val(vdatabase),
                        String_val(vnewname), flags);
  check_retval(ret);
  
  return Val_unit;
}

CAMLprim value ml_dbenv_dbrename_byte(value *argv, int argn) {
  return ml_dbenv_dbrename(argv[0], argv[1], argv[2], argv[3], argv[4], 
                           argv[5], argv[6]);
}

CAMLprim value ml_dbenv_open(value vdbenv, value vhome, value vflags,
                             value vmode, value unit) {
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  char* db_home = NULL;
  u_int32_t flags = DBFlags_val(vflags, flags_dbenv_open);
  int mode = 0;
  int ret;

  TEST_HANDLE(dbenv);

  if(Is_block(vhome))
    db_home = String_val(Some_val(vhome));

  if(Is_block(vmode))
    mode = Int_val(Some_val(vmode));

  ret = dbenv->open(dbenv, db_home, flags, mode);
  check_retval(ret);

  return Val_unit;
}

CAMLprim value ml_dbenv_remove(value vdbenv, value vhome, value vflags, 
                               value unit) {
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  u_int32_t flags = DBFlags_val(vflags, flags_dbenv_remove);
  int ret;

  TEST_HANDLE(dbenv);
  ret = dbenv->remove(dbenv, String_val(vhome), flags);
  check_retval(ret);

  return Val_unit;
}

CAMLprim value ml_dbenv_set_encrypt(value vdbenv, value vpasswd, value vflags,
                                    value unit) {
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  u_int32_t flags = DBFlags_val(vflags, flags_dbenv_encrypt);
  int ret;
  TEST_HANDLE(dbenv);
  ret = dbenv->set_encrypt(dbenv, String_val(vpasswd), flags);
  check_retval(ret);

  return Val_unit;
}

// make optional return
CAMLprim value ml_db_strerror(value vret) {
  CAMLparam1(vret);
  char* msg = db_strerror(Int_val(vret));
  if(msg != NULL)
    CAMLreturn(caml_copy_string(msg));
  else
    CAMLreturn(caml_copy_string(""));
}

CAMLprim value ml_db_version(value unit) {
  CAMLparam1(unit);
  CAMLlocal1(vres);
  int major = 0, minor = 0, patch = 0;
  char* version = db_version(&major, &minor, &patch);

  vres = caml_alloc_tuple(4);
  Store_field(vres, 0, caml_copy_string(version));
  Store_field(vres, 1, Val_int(major));
  Store_field(vres, 2, Val_int(minor));
  Store_field(vres, 3, Val_int(patch));
  CAMLreturn(vres);
}

/*
CAMLprim value ml_dbenv_lock_stat(value vdbenv, value vstat, value vflags, 
                                  value unit) {
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  u_int32_t flags = DBFlags_val(vflags, flags_dbenv_lock_stat);
  int ret;

  TEST_HANDLE(dbenv);
  ret = dbenv->lock_stat(dbenv, stat, flags);
  check_retval(ret);

  return Val_unit;
}
*/

static int dbverbose_enum[] = {
  DB_VERB_DEADLOCK,
  DB_VERB_FILEOPS,
  DB_VERB_FILEOPS_ALL,
  DB_VERB_RECOVERY,
  DB_VERB_REGISTER,
  DB_VERB_REPLICATION,
  DB_VERB_REP_ELECT,
  DB_VERB_REP_LEASE,
  DB_VERB_REP_MISC,
  DB_VERB_REP_MSGS,
  DB_VERB_REP_SYNC,
  DB_VERB_REPMGR_CONNFAIL,
  DB_VERB_REPMGR_MISC,
  DB_VERB_WAITSFOR
};

CAMLprim value ml_dbenv_set_verbose(value vdbenv, value vwhich, value f) {
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  int which = ml2enum(vwhich, dbverbose_enum, Size(dbverbose_enum));
  int ret;

  TEST_HANDLE(dbenv);
  ret = dbenv->set_verbose(dbenv, which, Bool_val(f));
  check_retval(ret);

  return Val_unit;
}

CAMLprim value ml_dbenv_get_verbose(value vdbenv, value vwhich) {
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  int which = ml2enum(vwhich, dbverbose_enum, Size(dbverbose_enum));
  int onoff;
  int ret;

  TEST_HANDLE(dbenv);
  ret = dbenv->get_verbose(dbenv, which, &onoff);
  check_retval(ret);

  return Val_bool(onoff);
}

static int lk_detect_enum[] = {
  DB_LOCK_DEFAULT,
  DB_LOCK_EXPIRE,
  DB_LOCK_MAXLOCKS,
  DB_LOCK_MAXWRITE,
  DB_LOCK_MINLOCKS,
  DB_LOCK_MINWRITE,
  DB_LOCK_OLDEST,
  DB_LOCK_RANDOM,
  DB_LOCK_YOUNGEST
};

CAMLprim value ml_dbenv_set_lk_detect(value vdbenv, value vmode) {
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  u_int32_t mode = ml2enum(vmode, lk_detect_enum, Size(lk_detect_enum));
  int ret;
  TEST_HANDLE(dbenv);
  ret = dbenv->set_lk_detect(dbenv, mode);
  check_retval(ret);
  return Val_unit;
}

CAMLprim value ml_dbenv_get_lk_detect(value vdbenv) {
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  u_int32_t mode;
  int ret;
  TEST_HANDLE(dbenv);
  ret = dbenv->get_lk_detect(dbenv, &mode);
  check_retval(ret);
  return enum2ml(mode, lk_detect_enum, Size(lk_detect_enum));
}

CAMLprim value ml_dbenv_txn_begin(value vdbenv, value vtxn, value vflags, 
                                  value unit) {
  CAMLparam4(vdbenv, vtxn, vflags, unit);
  CAMLlocal1(vres);
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  DB_TXN *txn = DB_TXN_opt_val(vtxn);
  DB_TXN *txn_begin = NULL;
  u_int32_t flags = DBFlags_val(vflags, flags_dbenv_txn_begin);
  int ret;

  TEST_HANDLE(dbenv);
  ret = dbenv->txn_begin(dbenv, txn, &txn_begin, flags);
  check_retval(ret);

  vres = caml_alloc_custom(&dbtxn_ops, sizeof(DB_TXN*), 0, 1);
  DB_TXN_val(vres) = txn_begin;
  CAMLreturn(vres);
}

CAMLprim value ml_dbcursor_close(value vcursor, value unit) {
  DBC *cursor = DBC_val(vcursor);
  int ret;
  TEST_HANDLE(cursor);
  ret = cursor->close(cursor);
  DBC_val(vcursor) = NULL;
  check_retval(ret);

  return Val_unit;
}  

CAMLprim value ml_dbcursor_cmp(value vcursor, value vother_cursor) {
  DBC *cursor = DBC_val(vcursor);
  DBC *other_cursor = DBC_val(vother_cursor);
  int result;
  int ret;

  TEST_HANDLE(cursor);
  ret = cursor->cmp(cursor, other_cursor, &result, 0);
  check_retval(ret);

  return Val_int(result);
}

CAMLprim value ml_dbcursor_count(value vcursor) {
  CAMLparam1(vcursor);
  DBC *cursor = DBC_val(vcursor);
  db_recno_t count;
  int ret;

  TEST_HANDLE(cursor);
  ret = cursor->count(cursor, &count, 0);
  check_retval(ret);

  CAMLreturn(caml_copy_int32(count));
}

CAMLprim value ml_dbcursor_del(value vcursor, value vflags, value unit) {
  DBC *cursor = DBC_val(vcursor);
  u_int32_t flags = DBFlags_val(vflags, flags_dbcursor_del);
  int ret;

  TEST_HANDLE(cursor);
  ret = cursor->del(cursor, flags);
  check_retval(ret);

  return Val_unit;
}

CAMLprim value ml_dbcursor_dup(value vcursor, value vflags, value unit) {
  CAMLparam3(vcursor, vflags, unit);
  CAMLlocal1(vres);
  DBC *cursor = DBC_val(vcursor);
  DBC *cursor_dup;
  u_int32_t flags = DBFlags_val(vflags, flags_dbcursor_dup);
  int ret;

  TEST_HANDLE(cursor);
  ret = cursor->dup(cursor, &cursor_dup, flags);
  check_retval(ret);

  vres = caml_alloc_custom(&dbc_ops, sizeof(DBC*), 0, 1);
  DBC_val(vres) = cursor_dup;
  CAMLreturn(vres);
}

CAMLprim value ml_dbcursor_get(value vcursor, value vkey, value vdata,
                               value vflags, value unit) {
  CAMLparam5(vcursor, vkey, vdata, vflags, unit);
  CAMLlocal3(vres, vstr1, vstr2);
  DBC *cursor = DBC_val(vcursor);
  DBT key, data;
  u_int32_t flags = DBFlags_val(vflags, flags_dbcursor_get);
  int ret;

  TEST_HANDLE(cursor);
  memset(&key, 0, sizeof(DBT));
  if(Is_block(vkey)) {
    key.data = String_val(Some_val(vkey));
    key.size = caml_string_length(Some_val(vkey));
  }

  memset(&data, 0, sizeof(DBT));
  if(Is_block(vdata)) {
    data.data = String_val(Some_val(vdata));
    data.size = caml_string_length(Some_val(vdata));
  }

  ret = cursor->get(cursor, &key, &data, flags);
  check_retval(ret);

  vstr1 = caml_alloc_string(key.size);
  memcpy(String_val(vstr1), key.data, key.size);
  vstr2 = caml_alloc_string(data.size);
  memcpy(String_val(vstr2), data.data, data.size);
  vres = caml_alloc_tuple(2);
  Store_field(vres, 0, vstr1);
  Store_field(vres, 1, vstr2);
  CAMLreturn(vres);
}

CAMLprim value ml_dbcursor_pget(value vcursor, value vkey, value vdata,
                                value vflags, value unit) {
  CAMLparam5(vcursor, vkey, vdata, vflags, unit);
  CAMLlocal3(vres, vstr1, vstr2);
  DBC *cursor = DBC_val(vcursor);
  DBT key, pkey, data;
  u_int32_t flags = DBFlags_val(vflags, flags_dbcursor_get);
  int ret;

  TEST_HANDLE(cursor);
  memset(&key, 0, sizeof(DBT));
  if(Is_block(vkey)) {
    key.data = String_val(Some_val(vkey));
    key.size = caml_string_length(Some_val(vkey));
  }

  memset(&data, 0, sizeof(DBT));
  if(Is_block(vdata)) {
    data.data = String_val(Some_val(vdata));
    data.size = caml_string_length(Some_val(vdata));
  }

  ret = cursor->pget(cursor, &key, &pkey, &data, flags);
  check_retval(ret);

  vstr1 = caml_alloc_string(pkey.size);
  memcpy(String_val(vstr1), pkey.data, pkey.size);
  vstr2 = caml_alloc_string(data.size);
  memcpy(String_val(vstr2), data.data, data.size);
  vres = caml_alloc_tuple(2);
  Store_field(vres, 0, vstr1);
  Store_field(vres, 1, vstr2);
  CAMLreturn(vres);
}

CAMLprim value ml_dbcursor_put(value vcursor, value vkey, value vdata,
                               value vflags, value unit) {
  DBC *cursor = DBC_val(vcursor);
  DBT key, data;
  u_int32_t flags = DBFlags_val(vflags, flags_dbcursor_put);
  int ret;

  TEST_HANDLE(cursor);
  memset(&key, 0, sizeof(DBT));
  key.data = String_val(vkey);
  key.size = caml_string_length(vkey);

  memset(&data, 0, sizeof(DBT));
  data.data = String_val(vdata);
  data.size = caml_string_length(vdata);

  ret = cursor->put(cursor, &key, &data, flags);
  check_retval(ret);

  return Val_unit;
}

#define Set_flags(module, name, V, tbl)                     \
  CAMLprim value ml_## module ## _ ## name(value v, value vflags) { \
    u_int32_t flags = Variants_val(vflags, tbl, Size(tbl)); \
    TEST_HANDLE(V(v));                                      \
    check_retval(V(v)->name(V(v), flags));                  \
    return Val_unit;                                        \
  }
#define Set_flagsOnOff(module, name, V, tbl)                              \
  CAMLprim value ml_##module ## _ ##name(value v, value vflags, value vonoff) { \
    u_int32_t flags = Variants_val(vflags, tbl, Size(tbl)); \
    TEST_HANDLE(V(v));                                        \
    check_retval(V(v)->name(V(v), flags, Bool_val(vonoff)));  \
    return Val_unit;                                        \
  }

#define ML1(module, name, V)                             \
  CAMLprim value ml_ ## module ## _ ## name (value v) {  \
    TEST_HANDLE(V(v));                                   \
    check_retval(V(v)->name(V(v)));                      \
    return Val_unit;                                     \
  }

#define ML2(module, name, V1, V2)                                   \
  CAMLprim value ml_ ## module ## _ ## name (value v1, value v2) {  \
    TEST_HANDLE(V1(v1));                                              \
    check_retval(V1(v1)->name(V1(v1), V2(v2)));                     \
    return Val_unit;                                                \
  }

#define ML3(module, name, V1, V2, V3)                                   \
  CAMLprim value ml_ ## module ## _ ## name(value v1, value v2, value v3) { \
    TEST_HANDLE(V1(v1));                                                  \
    check_retval(V1(v1)->name(V1(v1), V2(v2), V3(v3)));                 \
    return Val_unit;                                                    \
  }

#define ML4(module, name, V1, V2, V3, V4)                               \
  CAMLprim value ml_ ## module ## _ ## name(value v1, value v2, value v3, value v4) { \
    TEST_HANDLE(V1(v1));                                                  \
    check_retval(V1(v1)->name(V1(v1), V2(v2), V3(v3), V4(v4)));         \
    return Val_unit;                                                    \
  }

#define Get_flags(module, name, V, tbl)       \
  CAMLprim value ml_##module## _ ##name(value v) { \
  u_int32_t flags;                            \
  TEST_HANDLE(V(v));                          \
  check_retval(V(v)->name(V(v), &flags));     \
  return Val_variants(flags, tbl, Size(tbl)); \
  }

Set_flags(db, set_flags, DB_val, flags_db)
Get_flags(db, get_flags, DB_val, flags_db)
Get_flags(db, get_encrypt_flags, DB_val, flags_db_encrypt)
Set_flagsOnOff(dbenv, set_flags, DB_ENV_val, flags_dbenv)
Get_flags(dbenv, get_flags, DB_ENV_val, flags_dbenv)
Get_flags(dbenv, get_encrypt_flags, DB_ENV_val, flags_dbenv_encrypt)
Get_flags(dbenv, get_open_flags, DB_ENV_val, flags_dbenv_open)
Get_flags(db, get_open_flags, DB_val, flags_db_open)

#define GetVal(module, name, t, V, R)                   \
  CAMLprim value ml_ ## module ## _ ## name (value v) { \
    CAMLparam1(v);                                      \
    t val;                                              \
    TEST_HANDLE(V(v));                                  \
    check_retval(V(v)->name(V(v), &val));               \
    CAMLreturn(R(val));                                 \
  }

#define GetVal2(module, name, t1, t2, V, R1, R2)        \
  CAMLprim value ml_ ## module ## _ ## name (value v) { \
    CAMLparam1(v);                                      \
    CAMLlocal3(vres, vr1, vr2);                         \
    t1 val1;                                            \
    t2 val2;                                            \
    TEST_HANDLE(V(v));                                  \
    check_retval(V(v)->name(V(v), &val1, &val2));       \
    vr1 = R1(val1);                                     \
    vr2 = R2(val2);                                     \
    vres = caml_alloc_tuple(2);                         \
    Store_field(vres, 0, vr1);                          \
    Store_field(vres, 1, vr2);                          \
    CAMLreturn(vres);                                   \
  }

GetVal(db, get_byteswapped, int, DB_val, Val_bool)
ML2(db, set_pagesize, DB_val, Int32_val)
GetVal(db, get_pagesize, u_int32_t, DB_val, caml_copy_int32)
GetVal(db, get_h_nelem, u_int32_t, DB_val, caml_copy_int32)
GetVal(db, get_lorder, int, DB_val, Val_int)
GetVal(db, get_h_ffactor, u_int32_t, DB_val, caml_copy_int32)
ML2(db, set_h_ffactor, DB_val, Int32_val)
GetVal(db, get_q_extentsize, u_int32_t, DB_val, caml_copy_int32)
ML2(db, set_q_extentsize, DB_val, Int32_val)
GetVal(db, get_re_delim, int, DB_val, Val_int)
ML2(db, set_re_delim, DB_val, Int_val)
GetVal(db, get_re_len, u_int32_t, DB_val, caml_copy_int32)
ML2(db, set_re_len, DB_val, Int32_val)
GetVal(db, get_re_pad, int, DB_val, Val_int)
ML2(db, set_re_pad, DB_val, Int_val)
GetVal(db, get_re_source, const char *, DB_val, caml_copy_string)
ML2(db, set_re_source, DB_val, String_val)
GetVal(db, get_priority, DB_CACHE_PRIORITY, DB_val, Val_dbpriority)
ML2(db, set_priority, DB_val, DBPriority_val)
GetVal(dbcursor, get_priority, DB_CACHE_PRIORITY, DB_val, Val_dbpriority)
ML2(dbcursor, set_priority, DB_val, DBPriority_val)
GetVal2(dbenv, get_cache_max, u_int32_t, u_int32_t, DB_ENV_val, caml_copy_int32, caml_copy_int32)
ML3(dbenv, set_cache_max, DB_ENV_val, Int32_val, Int32_val)
ML2(db, set_create_dir, DB_val, String_val)
GetVal(db, get_create_dir, const char*, DB_val, caml_copy_string)
ML2(dbenv, set_create_dir, DB_ENV_val, String_val)
GetVal2(db, get_dbname, const char *, const char *, DB_val, caml_copy_string, caml_copy_string)
GetVal(dbenv, get_create_dir, const char *, DB_ENV_val, caml_copy_string)
ML2(dbenv, add_data_dir, DB_ENV_val, String_val)
ML2(dbenv, set_tmp_dir, DB_ENV_val, String_val)
GetVal(dbenv, get_tmp_dir, const char *, DB_ENV_val, caml_copy_string)
ML2(dbenv, set_intermediate_dir_mode, DB_ENV_val, String_val)
GetVal(dbenv, get_intermediate_dir_mode, const char *, DB_ENV_val, caml_copy_string)
GetVal(dbenv, get_home, const char *, DB_ENV_val, caml_copy_string)
GetVal(dbtxn, get_name, const char *, DB_TXN_val, caml_copy_string)
ML2(dbtxn, set_name, DB_TXN_val, String_val)
ML4(db, set_cachesize, DB_val, Int32_val, Int32_val, Int_val)
ML4(dbenv, set_cachesize, DB_ENV_val, Int32_val, Int32_val, Int_val)
GetVal(db, fd, int, DB_val, Int_val)
ML2(db, set_h_nelem, DB_val, Int32_val)
ML2(db, set_lorder, DB_val, Int_val)
GetVal(dbenv, get_lg_bsize, u_int32_t, DB_ENV_val, caml_copy_int32)
ML2(dbenv, set_lg_bsize, DB_ENV_val, Int32_val)
GetVal(dbenv, get_lg_dir, const char *, DB_ENV_val, caml_copy_string)
ML2(dbenv, set_lg_dir, DB_ENV_val, String_val)
GetVal(dbenv, get_lg_filemode, int, DB_ENV_val, Val_int)
ML2(dbenv, set_lg_filemode, DB_ENV_val, Int_val)
GetVal(dbenv, get_lg_max, u_int32_t, DB_ENV_val, caml_copy_int32)
ML2(dbenv, set_lg_max, DB_ENV_val, Int32_val)
GetVal(dbenv, get_lg_regionmax, u_int32_t, DB_ENV_val, caml_copy_int32)
ML2(dbenv, set_lg_regionmax, DB_ENV_val, Int32_val)
GetVal(dbenv, get_lk_max_lockers, u_int32_t, DB_ENV_val, caml_copy_int32)
ML2(dbenv, set_lk_max_lockers, DB_ENV_val, Int32_val)
GetVal(dbenv, get_lk_max_locks, u_int32_t, DB_ENV_val, caml_copy_int32)
ML2(dbenv, set_lk_max_locks, DB_ENV_val, Int32_val)
GetVal(dbenv, get_lk_max_objects, u_int32_t, DB_ENV_val, caml_copy_int32)
ML2(dbenv, set_lk_max_objects, DB_ENV_val, Int32_val)
GetVal(dbenv, get_lk_partitions, u_int32_t, DB_ENV_val, caml_copy_int32)
ML2(dbenv, set_lk_partitions, DB_ENV_val, Int32_val)
GetVal(dbenv, get_mp_max_openfd, int, DB_ENV_val, Val_int)
ML2(dbenv, set_mp_max_openfd, DB_ENV_val, Int_val)
GetVal2(dbenv, get_mp_max_write, int, db_timeout_t, DB_ENV_val, Val_int, caml_copy_int32)
ML3(dbenv, set_mp_max_write, DB_ENV_val, Int_val, Int32_val)
GetVal(dbenv, get_mp_mmapsize, size_t, DB_ENV_val, caml_copy_int32)
ML2(dbenv, set_mp_mmapsize, DB_ENV_val, Int32_val)
GetVal(db, get_bt_minkey, u_int32_t, DB_val, caml_copy_int32)
ML2(db, set_bt_minkey, DB_val, Int32_val)
GetVal(dbenv, get_shm_key, long, DB_ENV_val, Val_long)
ML2(dbenv, set_shm_key, DB_ENV_val, Long_val)
GetVal(dbenv, get_thread_count, u_int32_t, DB_ENV_val, caml_copy_int32)
ML2(dbenv, set_thread_count, DB_ENV_val, Int32_val)
GetVal(dbenv, get_tx_max, u_int32_t, DB_ENV_val, caml_copy_int32)
ML2(dbenv, set_tx_max, DB_ENV_val, Int32_val)
GetVal(dbenv, lock_id, u_int32_t, DB_ENV_val, caml_copy_int32)
ML2(dbenv, lock_id_free, DB_ENV_val, Int32_val)
GetVal2(dbenv, rep_get_clockskew, u_int32_t, u_int32_t, DB_ENV_val, caml_copy_int32, caml_copy_int32)
ML3(dbenv, rep_set_clockskew, DB_ENV_val, Int32_val, Int32_val)
GetVal2(dbenv, rep_get_limit, u_int32_t, u_int32_t, DB_ENV_val, caml_copy_int32, caml_copy_int32)
ML3(dbenv, rep_set_limit, DB_ENV_val, Int32_val, Int32_val)
GetVal(dbenv, rep_get_nsites, u_int32_t, DB_ENV_val, caml_copy_int32)
ML2(dbenv, rep_set_nsites, DB_ENV_val, Int32_val)
GetVal(dbenv, rep_get_priority, u_int32_t, DB_ENV_val, caml_copy_int32)
ML2(dbenv, rep_set_priority, DB_ENV_val, Int32_val)
GetVal2(dbenv, rep_get_request, u_int32_t, u_int32_t, DB_ENV_val, caml_copy_int32, caml_copy_int32)
ML3(dbenv, rep_set_request, DB_ENV_val, Int32_val, Int32_val)

CAMLprim value ml_dbenv_rep_elect(value vdbenv, value v1, value v2) {
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  int ret;
  TEST_HANDLE(dbenv);
  ret = dbenv->rep_elect(dbenv, Int32_val(v1), Int32_val(v2), 0);
  check_retval(ret);
  return Val_unit;
}

static int rep_config_enum[] = {
  DB_REP_CONF_BULK,
  DB_REP_CONF_DELAYCLIENT,
  DB_REP_CONF_INMEM,
  DB_REP_CONF_LEASE,
  DB_REP_CONF_NOAUTOINIT,
  DB_REP_CONF_NOWAIT,
  DB_REPMGR_CONF_2SITE_STRICT

};

CAMLprim value ml_dbenv_rep_get_config(value vdbenv, value vwhich) {
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  u_int32_t which = ml2enum(vwhich, rep_config_enum, Size(rep_config_enum));
  int val;
  int ret;
  TEST_HANDLE(dbenv);
  ret = dbenv->rep_get_config(dbenv, which, &val);
  check_retval(ret);
  return Val_bool(val);
}

CAMLprim value ml_dbenv_rep_set_config(value vdbenv, value vwhich, value val) {
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  u_int32_t which = ml2enum(vwhich, rep_config_enum, Size(rep_config_enum));
  int ret;
  TEST_HANDLE(dbenv);
  ret = dbenv->rep_set_config(dbenv, which, Bool_val(val));
  check_retval(ret);
  return Val_unit;
}

CAMLprim value ml_dbtxn_abort(value vtxn) {
  DB_TXN *txn = DB_TXN_val(vtxn);
  int ret;

  TEST_HANDLE(txn);
  ret = txn->abort(txn);
  DB_TXN_val(vtxn) = NULL;
  check_retval(ret);
  return Val_unit;
}

CAMLprim value ml_dbenv_txn_checkpoint(value vdbenv, value vkbyte, value vmin,
                                       value vflags, value unit) {
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  u_int32_t flags = DBFlags_val(vflags, flags_dbenv_txn_checkpoint);
  int ret;
  TEST_HANDLE(dbenv);
  ret = dbenv->txn_checkpoint(dbenv, Int32_val(vkbyte), Int32_val(vmin), flags);
  check_retval(ret);
  return Val_unit;
}

CAMLprim value ml_dbtxn_commit(value vtxn, value vflags, value unit) {
  DB_TXN *txn = DB_TXN_val(vtxn);
  u_int32_t flags = DBFlags_val(vflags, flags_dbtxn_commit);
  int ret;

  TEST_HANDLE(txn);
  ret = txn->commit(txn, flags);
  DB_TXN_val(vtxn) = NULL;
  check_retval(ret);
  return Val_unit;
}

CAMLprim value ml_dbtxn_discard(value vtxn) {
  DB_TXN *txn = DB_TXN_val(vtxn);
  int ret;

  TEST_HANDLE(txn);
  ret = txn->discard(txn, 0);
  check_retval(ret);
  return Val_unit;
}

CAMLprim value ml_dbtxn_id(value vtxn) {
  CAMLparam1(vtxn);
  DB_TXN *txn = DB_TXN_val(vtxn);
  TEST_HANDLE(txn);
  u_int32_t ret = txn->id(txn);
  CAMLreturn(caml_copy_int32(ret));
}

/*
CAMLprim value ml_dbtxn_prepare(value vtxn) {
  CAMLparam0();
  CAMLlocal1(head);
  DB_TXN *txn = DB_TXN_val(vtxn);
  u_int32_t gid[DB_XIDDATASIZE];
  int i, ret;

  TEST_HANDLE(txn);
  ret = txn->prepare(txn, gid);
  check_retval(ret);

  CAMLreturn(Val_unit);
}
*/

CAMLprim value ml_dbtxn_set_timeout(value vtxn, value vtimeout, value vflags) {
  DB_TXN *txn = DB_TXN_val(vtxn);
  u_int32_t flags = 
    Variant_val(vflags, flags_dbtxn_timeout, Size(flags_dbtxn_timeout));
  int ret;

  TEST_HANDLE(txn);
  ret = txn->set_timeout(txn, Int32_val(vtimeout), flags);
  check_retval(ret);
  
  return Val_unit;
}

CAMLprim value ml_dblogc_close(value vlogc) {
  DB_LOGC *logc = DB_LOGC_val(vlogc);
  int ret;
  TEST_HANDLE(logc);
  TEST_HANDLE(logc);
  ret = logc->close(logc, 0);
  DB_LOGC_val(vlogc) = NULL;
  check_retval(ret);
  return Val_unit;
}

CAMLprim value ml_dblogc_get(value vlogc, value vlsn, value vdata, 
                             value vflags, value unit) {
  DB_LOGC *logc = DB_LOGC_val(vlogc);
  DB_LSN *lsn = NULL;
  DBT data;
  u_int32_t flags = DBFlags_val(vflags, flags_dblogc_get);

  TEST_HANDLE(logc);

  memset(&data, 0, sizeof(DBT));
  
  check_retval(logc->get(logc, lsn, &data, flags));
  return Val_unit;
}

CAMLprim value ml_dbenv_lock_detect(value vdbenv, value vatype) {
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  int rejected, ret;
  TEST_HANDLE(dbenv);
  ret = dbenv->lock_detect(dbenv, 0, Int32_val(vatype), &rejected);
  check_retval(ret);
  return Val_int(rejected);
}

CAMLprim value ml_dbenv_get_errpfx(value vdbenv) {
  CAMLparam1(vdbenv);
  const char *str;
  DB_ENV_val(vdbenv)->get_errpfx(DB_ENV_val(vdbenv), &str);
  CAMLreturn(caml_copy_string(str));
}

CAMLprim value ml_dbenv_set_errpfx(value vdbenv, value vstr) {
  DB_ENV_val(vdbenv)->set_errpfx(DB_ENV_val(vdbenv), String_val(vstr));
  return Val_unit;
}

CAMLprim value ml_dbenv_failchk(value vdbenv) {
  check_retval(DB_ENV_val(vdbenv)->failchk(DB_ENV_val(vdbenv), 0));
  return Val_unit;
}

CAMLprim value ml_dbenv_fileid_reset(value vdbenv, value vfile, value vflags,
                                     value unit) {
  u_int32_t flags = DBFlags_val(vflags, flags_dbenv_fileid_reset);
  check_retval(DB_ENV_val(vdbenv)->fileid_reset(DB_ENV_val(vdbenv), 
                                                String_val(vfile), flags));
  return Val_unit;
}

CAMLprim value ml_dbenv_log_archive(value vdbenv, value vflags, value unit) {
  CAMLparam3(vdbenv, vflags, unit);
  CAMLlocal3(vres, cons, vstr);
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  u_int32_t flags = DBFlags_val(vflags, flags_log_archive);
  char **list;
  int ret;
  TEST_HANDLE(dbenv);
  ret = dbenv->log_archive(dbenv, &list, flags);
  check_retval(ret);
  vres = Val_emptylist;
  if(list != NULL)
    for(; *list != NULL; list++) {
      vstr = caml_copy_string(*list);
      cons = caml_alloc(2, 0);
      Store_field(cons, 0, vstr);
      Store_field(cons, 1, vres);
      vres = cons;
    }
  CAMLreturn(vres);
}

CAMLprim value ml_dbenv_log_get_config(value vdbenv, value vwhich) {
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  u_int32_t which = Variant_val(vwhich, flags_log_config, 
                                Size(flags_log_config));
  int onoff;
  int ret;
  TEST_HANDLE(dbenv);
  ret = dbenv->log_get_config(dbenv, which, &onoff);
  check_retval(ret);
  return Val_bool(onoff);
}

CAMLprim value ml_dbenv_log_set_config(value vdbenv, value vwhich,
                                       value vonoff) {
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  u_int32_t which = Variant_val(vwhich, flags_log_config, 
                                Size(flags_log_config));
  int onoff;
  int ret;
  TEST_HANDLE(dbenv);
  ret = dbenv->log_get_config(dbenv, which, &onoff);
  check_retval(ret);
  return Val_bool(onoff);
}

CAMLprim value ml_dbenv_repmgr_add_remote_site(value vdbenv, 
                                               value vhost, value vport,
                                               value vflags, value unit) {
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  int eid;
  u_int32_t flags = DBFlags_val(vflags, flags_repmgr_add_remote_site);
  int ret;
  TEST_HANDLE(dbenv);
  ret = dbenv->repmgr_add_remote_site(dbenv, String_val(vhost), Int_val(vport),
                                      &eid, flags);
  check_retval(ret);
  return Val_int(eid);
}

static int repmgr_ack_policy_enum[] = {
  DB_REPMGR_ACKS_ALL,
  DB_REPMGR_ACKS_ALL_PEERS,
  DB_REPMGR_ACKS_NONE,
  DB_REPMGR_ACKS_ONE,
  DB_REPMGR_ACKS_ONE_PEER,
  DB_REPMGR_ACKS_QUORUM
};

CAMLprim value ml_dbenv_repmgr_get_ack_policy(value vdbenv) {
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  int policy;
  int ret;
  TEST_HANDLE(dbenv);
  ret = dbenv->repmgr_get_ack_policy(dbenv, &policy);
  check_retval(ret);
  return enum2ml(policy, repmgr_ack_policy_enum, Size(repmgr_ack_policy_enum));
}

CAMLprim value ml_dbenv_repmgr_set_ack_policy(value vdbenv, value vpolicy) {
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  int policy = ml2enum(vpolicy, repmgr_ack_policy_enum, 
                       Size(repmgr_ack_policy_enum));
  int ret;
  TEST_HANDLE(dbenv);
  ret = dbenv->repmgr_set_ack_policy(dbenv, policy);
  check_retval(ret);
  return Val_unit;
}

CAMLprim value ml_dbenv_repmgr_set_local_site(value vdbenv, 
                                              value vhost, value vport) {
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  int ret;
  TEST_HANDLE(dbenv);
  ret = dbenv->repmgr_set_local_site(dbenv, 
                                      String_val(vhost), Int_val(vport), 0);
  check_retval(ret);
  return Val_unit;
}

CAMLprim value ml_dbenv_repmgr_start(value vdbenv, value vthreads, value vflag) {
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  u_int32_t flag = Variant_val(vflag, flags_repmgr_start,
                               Size(flags_repmgr_start));
  int ret;
  TEST_HANDLE(dbenv);
  ret = dbenv->repmgr_start(dbenv, Int_val(vthreads), flag);
  check_retval(ret);
  return Val_unit;
}

static int rep_timeout_enum[] = {
  DB_REP_ACK_TIMEOUT,
  DB_REP_CHECKPOINT_DELAY,
  DB_REP_CONNECTION_RETRY,
  DB_REP_ELECTION_TIMEOUT,
  DB_REP_ELECTION_RETRY,
  DB_REP_FULL_ELECTION_TIMEOUT,
  DB_REP_HEARTBEAT_MONITOR,
  DB_REP_HEARTBEAT_SEND,
  DB_REP_LEASE_TIMEOUT
};

CAMLprim value ml_dbenv_rep_set_timeout(value vdbenv, value vwhich, 
                                        value vtimeout) {
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  int which = ml2enum(vwhich, rep_timeout_enum, Size(rep_timeout_enum));
  int ret;
  TEST_HANDLE(dbenv);
  ret = dbenv->rep_set_timeout(dbenv, which, Int32_val(vtimeout));
  check_retval(ret);
  return Val_unit;
}

CAMLprim value ml_dbenv_rep_get_timeout(value vdbenv, value vwhich) {
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  int which = ml2enum(vwhich, rep_timeout_enum, Size(rep_timeout_enum));
  u_int32_t timeout;
  int ret;
  TEST_HANDLE(dbenv);
  ret = dbenv->rep_get_timeout(dbenv, which, &timeout);
  check_retval(ret);
  return caml_copy_int32(timeout);
}

static int event_notify_enum[] = {
  DB_EVENT_PANIC,
  DB_EVENT_REG_ALIVE,
  DB_EVENT_REG_PANIC,
  DB_EVENT_REP_CLIENT,
  DB_EVENT_REP_ELECTED,
  DB_EVENT_REP_MASTER,
  DB_EVENT_REP_NEWMASTER,
  DB_EVENT_REP_PERM_FAILED,
  DB_EVENT_REP_STARTUPDONE,
  DB_EVENT_WRITE_FAILED
};

static void event_notify_stub(DB_ENV *dbenv, u_int32_t which, void *info) {
  CAMLparam0();
  CAMLlocal3(vdbenv, vwhich, vdata);
  vdbenv = caml_alloc_custom(&dbenv_ops, sizeof(DB_ENV*), 0, 1);
  DB_ENV_val(vdbenv) = dbenv;
  vwhich = enum2ml(which, event_notify_enum, Size(event_notify_enum));
  vdata = Val_unit;
  caml_callback3(Field(dbenv->app_private, V_DBENV_EVENT_NOTIFY), 
                       vdbenv, vwhich, vdata);
  CAMLreturn0;
}

/*
CAMLprim value ml_dbenv_set_event_notify(value vdbenv, value vcallback) {
  DB_ENV *dbenv = DB_ENV_val(vdbenv);
  int ret;
  TEST_HANDLE(dbenv);
  ret = dbenv->set_event_notify(dbenv, callback);
  check_retval(ret);
  return Val_unit;
}
*/

/*
CAMLprim value ml_dbmpoolfile_close(value vmpf) {
  DB_MPOOLFILE *mpf =  DB_MPOOLFILE_val(vmpf);
  int ret;
  TEST_HANDLE(mpf);
  ret = mpf->close(mpf, 0);
  DB_MPOOLFILE_val(vmpf) = NULL;
  check_retval(ret);
  return Val_unit;
}

CAMLprim value ml_dbmpoolfile_get(value vmpf, value vtxn, value vflags) {
  DB_MPOOLFILE *mpf =  DB_MPOOLFILE_val(vmpf);
  DB_TXN *txn = DB_TXN_opt_val(vtxn);
  u_int32_t flags = DBFlags_val(vflags, flags_dbmpoolfile_get);
  db_pgno_t pgnoaddr;
  void *page;
  int ret;

  TEST_HANDLE(mpf);
  ret = mpf->get(mpf, &pgnoaddr, txn, flags, &page);
  return page
}

CAMLprim value ml_dbmpoolfile_open(value vmpf, value vfile, value vflags,
                                   value vmode, value vsize) {
  DB_MPOOLFILE *mpf =  DB_MPOOLFILE_val(vmpf);
  u_int32_t flags = DBFlags_val(vflags, flags_dbmpoolfile_get);
  int ret;
  
  TEST_HANDLE(mpf);
  ret = mpf->open(mpf, String_val(vfile), flags, 
                  Int_val(vmode), Int32_val(vsize));
  check_retval(ret);
  return Val_unit;
}

CAMLprim value ml_dbmpoolfile_set_pgcookie(value vmpf, value vcookie) {
  DB_MPOOLFILE *mpf =  DB_MPOOLFILE_val(vmpf);
  DBT cookie;
  memset(&cookie, 0, sizeof(DBT));
  cookie.data = String_val(vcookie);
  cookie.size = caml_string_length(vcookie);
  int ret;

  TEST_HANDLE(mpf);
  ret = mpf->set_pgcookie(mpf, &cookie);
  check_retval(ret);
  return Val_unit;
}

CAMLprim value ml_dbmpoolfile_get_pgcookie(value vmpf) {
  CAMLparam1(vmpf);
  CAMLlocal1(vres);
  DB_MPOOLFILE *mpf =  DB_MPOOLFILE_val(vmpf);
  DBT cookie;
  memset(&cookie, 0, sizeof(DBT));
  int ret;

  TEST_HANDLE(mpf);
  ret = mpf->set_pgcookie(mpf, &cookie);
  check_retval(ret);
  vres = caml_alloc_string(cookie.size);
  memcpy(String_val(vres), cookie.data, cookie.size);
  CAMLreturn(vres);
}

Set_flagsOnOff(dbmpoolfile, set_flags, DB_MPOOLFILE_val, flags_dbmpoolfile)
Get_flags(dbmpoolfile, get_flags, DB_MPOOLFILE_val, flags_dbmpoolfile)
GetVal(dbmpoolfile, get_priority, DB_CACHE_PRIORITY, DB_val, Val_dbpriority)
ML2(dbmpoolfile, set_priority, DB_val, DBPriority_val)
GetVal(dbmpoolfile, get_clear_len, u_int32_t, DB_MPOOLFILE_val, caml_copy_int32)
ML2(dbmpoolfile, set_clear_len, DB_MPOOLFILE_val, Int32_val)
GetVal(dbmpoolfile, get_fileid, u_int8_t, DB_MPOOLFILE_val, Val_int)
ML2(dbmpoolfile, set_fileid, DB_MPOOLFILE_val, Int_val)
GetVal(dbmpoolfile, get_ftype, int, DB_MPOOLFILE_val, Val_int)
ML2(dbmpoolfile, set_ftype, DB_MPOOLFILE_val, Int_val)
GetVal(dbmpoolfile, get_last_pgno, u_int32_t, DB_MPOOLFILE_val, caml_copy_int32)
GetVal(dbmpoolfile, get_lsn_offset, int32_t, DB_MPOOLFILE_val, caml_copy_int32)
ML2(dbmpoolfile, set_lsn_offset, DB_MPOOLFILE_val, Int32_val)
GetVal2(dbmpoolfile, get_maxsize, u_int32_t, u_int32_t, DB_MPOOLFILE_val, caml_copy_int32, caml_copy_int32)
ML3(dbmpoolfile, set_maxsize, DB_MPOOLFILE_val, Int32_val, Int32_val)
ML1(dbmpoolfile, sync, DB_MPOOLFILE_val)

*/


CAMLprim value ml_dbsequence_close(value vseq) {
  DB_SEQUENCE *seq = DB_SEQ_val(vseq);
  int ret;
  TEST_HANDLE(seq);
  ret = seq->close(seq, 0);
  DB_SEQ_val(vseq) = NULL;
  check_retval(ret);

  return Val_unit;
}

CAMLprim value ml_dbsequence_get_db(value vseq) {
  CAMLparam1(vseq);
  CAMLlocal1(vres);
  DB_SEQUENCE *seq = DB_SEQ_val(vseq);
  DB *db;
  int ret;

  TEST_HANDLE(seq);
  ret = seq->get_db(seq, &db);
  check_retval(ret);

  vres = caml_alloc_custom(&db_ops, sizeof(DB*), 0, 1);
  DB_val(vres) = db;
  CAMLreturn(vres);
}
  
CAMLprim value ml_dbsequence_get_key(value vseq) {
  CAMLparam1(vseq);
  CAMLlocal1(vres);
  DB_SEQUENCE *seq = DB_SEQ_val(vseq);
  DBT key;
  int ret;

  TEST_HANDLE(seq);
  ret = seq->get_key(seq, &key);
  check_retval(ret);

  vres = caml_alloc_string(key.size);
  memcpy(String_val(vres), key.data, key.size);
  CAMLreturn(vres);
}

CAMLprim value ml_dbsequence_open(value vseq, value vtxn, value vkey,
                                  value vflags, value unit) {
  DB_SEQUENCE *seq = DB_SEQ_val(vseq);
  DB_TXN *txn = DB_TXN_opt_val(vtxn);
  DBT key;
  u_int32_t flags = DBFlags_val(vflags, flags_dbsequence_open);
  int ret;

  TEST_HANDLE(seq);

  memset(&key, 0, sizeof(DBT));
  key.data = String_val(vkey);
  key.size = caml_string_length(vkey);

  ret = seq->open(seq, txn, &key, flags);
  check_retval(ret);

  return Val_unit;
}

CAMLprim value ml_dbsequence_remove(value vseq, value vtxn, value vflags,
                                    value unit) {
  DB_SEQUENCE *seq = DB_SEQ_val(vseq);
  DB_TXN *txn = DB_TXN_opt_val(vtxn);
  u_int32_t flags = DBFlags_val(vflags, flags_dbsequence_remove);
  int ret;
  TEST_HANDLE(seq);
  ret = seq->remove(seq, txn, flags);
  check_retval(ret);

  return Val_unit;
}

Set_flags(dbsequence, set_flags, DB_SEQ_val, flags_dbsequence)
Get_flags(dbsequence, get_flags, DB_SEQ_val, flags_dbsequence)
GetVal(dbsequence, get_cachesize, int32_t, DB_SEQ_val, caml_copy_int32)
ML2(dbsequence, set_cachesize, DB_SEQ_val, Int32_val)
