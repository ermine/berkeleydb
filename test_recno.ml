open BerkeleyDB

type dbs = {
  env : DbEnv.t;
  posts : int Db.t;
  posts_index : string Db.t
}

let marshal value =
  Marshal.to_string value [Marshal.No_sharing]
let unmarshal str =
  Marshal.from_string str 0

let associate_post_index _db recno content =
  let time, str = unmarshal content in
    print_endline ("associate: " ^ string_of_int recno ^ " " ^ str);
    string_of_int (Hashtbl.hash (recno, content))


let dbs =
  init ();
  let env =
    DbEnv.create () in
  let () =
    DbEnv.set_errpfx env "erm_db";
    DbEnv.set_errcall env (Some (fun _ _ msg -> print_endline msg));
    DbEnv.env_open env ~home:"/tmp/recno"
      ~flags:[`DB_CREATE; `DB_INIT_TXN; `DB_INIT_LOCK;
              `DB_INIT_LOG; `DB_INIT_MPOOL; `DB_THREAD] () in
  let posts = Db.create_recno ~env () in
  let posts_index= Db.create_string ~env () in
  let () =
    Db.db_open posts "posts.db" DB_RECNO ~flags:[`DB_CREATE; `DB_AUTO_COMMIT] ();
    Db.db_open posts_index "posts_index.db" DB_HASH
      ~flags:[`DB_CREATE; `DB_AUTO_COMMIT] ();
    Db.associate posts posts_index (Some associate_post_index) ();
  in
    {
      env = env;
      posts = posts;
      posts_index = posts_index
    }

let add db key value =
  let f () =
    let txn = DbEnv.txn_begin dbs.env () in
    let recno =
      try
        let recno = Db.put db ~txn key (marshal value) ~flags:[`DB_APPEND] () in
          DbTxn.commit txn ();
          recno
      with exn -> DbTxn.abort txn; raise exn
    in
      print_endline ("result " ^ string_of_int recno)
  in
    f ()

let get db key =
  let f () =
    let r = Db.get db key () in
      r
  in
    f ()

let store_post str =
  let time = Unix.gettimeofday () in
    add dbs.posts 0 (time, str)

let print_data key (time, data) =
  print_endline ("result: " ^ string_of_int key ^ ", " ^
                   string_of_float time ^ ", " ^
                   data)

let _ =
  Pervasives.at_exit (fun () ->
                        Db.close dbs.posts_index ();
                        Db.close dbs.posts ();
                        DbEnv.close dbs.env ()
                     )

let _ =
  store_post "qwe";
  let c = Db.cursor dbs.posts () in
  let key, d = DbCursor.get c [`DB_FIRST] in
    print_data key (unmarshal d);
    while true do
      let key, d = DbCursor.get c [`DB_NEXT] in
        print_data key (unmarshal d);
        let d1 = Db.get dbs.posts key () in
        let time, str = unmarshal d1 in
          print_endline str;
          Gc.compact ()
    done
    
