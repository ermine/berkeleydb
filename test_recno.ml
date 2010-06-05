open BerkeleyDB

type dbs = {
  env : DbEnv.t;
  posts : recno Db.t;
  posts_index : string Db.t;
  a_posts_index : (string, recno) Db.secondary;
  comments : recno Db.t
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
  let a_posts_index =
    Db.db_open posts "posts.db" DB_RECNO ~flags:[`DB_CREATE; `DB_AUTO_COMMIT] ();
    Db.db_open posts_index "posts_index.db" DB_HASH
      ~flags:[`DB_CREATE; `DB_AUTO_COMMIT] ();
    Db.associate posts posts_index
      (Some associate_post_index) ()
  in
  let comments = Db.create_recno ~env () in
    Db.set_flags comments [`DB_DUP];
    Db.db_open comments "comments.db" DB_BTREE
      ~flags:[`DB_CREATE; `DB_AUTO_COMMIT] ();
    {
      env = env;
      posts = posts;
      posts_index = posts_index;
      a_posts_index = a_posts_index;
      comments = comments
    }

let add db key value =
  let txn = DbEnv.txn_begin dbs.env () in
  let recno =
    try
      let recno = Db.put db ~txn key (marshal value) ~flags:[`DB_APPEND] () in
        DbTxn.commit txn ();
        recno
    with exn -> DbTxn.abort txn; raise exn
  in
    print_endline ("result " ^ string_of_int recno);
    recno

let get db key =
  let r = Db.get db key () in
    r

let store_post str =
  let time = Unix.gettimeofday () in
  let txn = DbEnv.txn_begin dbs.env () in
  let recno =
    try
      let recno = Db.put dbs.posts ~txn 0 (marshal (time, str))
        ~flags:[`DB_APPEND] () in
        for i = 0 to 5 do
          Db.put dbs.comments ~txn recno ("comment " ^ string_of_int i) ()
        done;
        DbTxn.commit txn ();
        recno
    with exn -> DbTxn.abort txn; raise exn
  in
    print_endline ("result " ^ string_of_int recno);
    recno

let print_data key (time, data) =
  print_endline ("result: " ^ string_of_int key ^ ", " ^
                   string_of_float time ^ ", " ^
                   data)

let _ =
  Pervasives.at_exit (fun () ->
                        Db.close dbs.posts_index ();
                        Db.close dbs.posts ();
                        Db.close dbs.comments ();
                        DbEnv.close dbs.env ()
                     )

let print_data1 key data =
  print_int key;
  print_string " ";
  print_endline data

let _ =
  store_post "qwe";
  let c = Db.cursor dbs.posts () in
  let key, d = DbCursor.get c [`DB_FIRST] in
    print_data key (unmarshal d);
    (try
       while true do
         let key, d = DbCursor.get c [`DB_NEXT] in
           print_data key (unmarshal d);
           let d1 = Db.get dbs.posts key () in
           let time, str = unmarshal d1 in
           let key, data = Db.pget dbs.a_posts_index 
             (string_of_int (Hashtbl.hash (key, d1))) () in
             print_data key (unmarshal data);
             Gc.compact ()
       done
     with _ -> ());
    let c = Db.cursor dbs.comments () in
    let key, data = DbCursor.get c ~key:6 [`DB_SET] in
      print_data1 key data;
      while true do
        let key, data = DbCursor.get c [`DB_NEXT_DUP] in
          print_data1 key data
      done
