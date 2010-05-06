open Mlbdb

let test () =
  init ();
  let env = DbEnv.create () in
  let () =
    DbEnv.set_errpfx env "my dbenv prefix";
    DbEnv.set_errcall env
      (Some (fun _dbenv prefix msg ->
               print_endline ("errcall: " ^ prefix ^ " " ^ msg)));
    DbEnv.env_open env ~home:"/tmp/trn"
      ~flags:[`DB_CREATE; `DB_INIT_TXN; `DB_INIT_LOCK;
              `DB_INIT_LOG; `DB_INIT_MPOOL] () in
  let db = Db.create ~env () in
  let () = Db.db_open db "test_trn.db" DB_BTREE
    ~flags:[`DB_CREATE; `DB_AUTO_COMMIT] () in
  let lf = DbEnv.log_archive env ~flags:[`DB_ARCH_ABS] () in
  let () = List.iter print_endline lf in
  let txn = DbEnv.txn_begin env () in
  let () =
    try Db.put db ~txn "key" "data" ~flags:[`DB_NOOVERWRITE] ()
    with exn -> DbTxn.abort txn; raise exn in
    DbTxn.commit txn ();
    Db.close db ();
    DbEnv.close env ()

let _ =
  test ()
