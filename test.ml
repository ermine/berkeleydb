let _ = Printexc.record_backtrace true

open Mlbdb
open Db_flags

let _ =
  Gc.set {(Gc.get ()) with Gc.verbose = 0x01 }

let rec print_flags = function
  | [] -> ()
  | x :: xs ->
      let str =
        match (x:db_open_flag) with
          | `DB_AUTO_COMMIT -> "DB_AUTO_COMMIT"
          | `DB_CREATE -> "DB_CREATE"
          | `DB_EXCL -> "DB_EXCL"
          | `DB_MULTIVERSION -> "DB_MULTIVERSION"
          | `DB_NOMMAP -> "DB_NOMMAP"
          | `DB_RDONLY -> "DB_RDONLY"
          | `DB_READ_UNCOMMITTED -> "DB_READ_UNCOMMITTED"
          | `DB_THREAD -> "DB_THREAD"
          | `DB_TRUNCATE -> "DB_TRUNCATE"
      in
        print_endline str;
        print_flags xs


let pull_data db =
  for i = 0 to 1000 do
    Db.put db ("abc" ^ string_of_int i) ("data" ^ string_of_int i)
      ~flags:[`DB_NOOVERWRITE] ()
  done

let get_data db =
  let data = Db.get db "abcd" () in print_endline data;
    let data = Db.get db "qwe" () in print_endline data

let cursor_data db =
  let c = Db.cursor db () in
  let key, data = DbCursor.get c ~key:"abc2" ~data:"data20"
    ~flags:[`DB_GET_BOTH] () in
    print_endline (key ^ "   " ^ data);
    try while true do
      let key, data = DbCursor.get c ~flags:[`DB_NEXT] () in
        print_endline (key ^ "   " ^ data)
    done with Not_found -> DbCursor.close c ()

let cursor_put db =
  let c = Db.cursor db () in
    for i=0 to 10 do
      DbCursor.put c ("qwe" ^ string_of_int i) ("atat" ^ string_of_int i)
        ~flags:[`DB_KEYFIRST] ()
    done;
    DbCursor.close c ()

let cursor_del db =
  let c = Db.cursor db () in
    try while true do
      let _ = DbCursor.get c ~key:"abc2" ~flags:[`DB_SET] () in
        DbCursor.del c ()
    done with Not_found -> DbCursor.close c ()

let cursor_replace db =
  let c = Db.cursor db () in
  let key, _ = DbCursor.get c ~key:"abc999" ~flags:[`DB_SET] () in
  let i = DbCursor.count c in
    Printf.printf "Count %ld\n" i;
    DbCursor.put c key "hello" ~flags:[`DB_CURRENT] ();
    print_endline "done";
    DbCursor.close c ()

let test () =
  init ();
  let env = DbEnv.create () in
  let () =
    DbEnv.set_errpfx env "my dbenv prefix";
    DbEnv.set_errcall env
      (fun _dbenv prefix msg ->
         print_endline ("errcall: " ^ prefix ^ " " ^ msg));
    DbEnv.set_verbose env DB_VERB_FILEOPS_ALL true;    
    DbEnv.env_open env ~home:"/tmp/abc" ~flags:[`DB_CREATE; `DB_INIT_MPOOL] () in
  let db = Db.create ~env () in
  let () =
    Db.set_errpfx db "my prefix";
    Db.set_errcall db (fun _dbenv prefix msg ->
                         print_endline ("errcall: " ^ prefix ^ " " ^ msg))
  in
(*    
  let () = Db.verify db "test.db" ~flags:[`DB_SALVAGE] () in
    print_endline "after verify..";
*)
  let () = Db.db_open db "test.db" DB_BTREE ~flags:[`DB_CREATE] () in
  let flags = Db.get_open_flags db in
    print_flags flags;
    pull_data db;
(*
    cursor_replace db;
*)
(*    
    Db.close db ();
    DbEnv.close env ()
*)
    ()
        
let _ =
  test ();
  Gc.print_stat stdout
