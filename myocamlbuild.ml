open Ocamlbuild_plugin
open Myocamlbuild_config

let _ =  dispatch begin function
  | After_rules ->
      make_binding ~include_dir:"-I/usr/local/include"
        ~lib_dir:"-L/usr/local/lib"
        ~lib:"-ldb-4.8" 
        ~headers:["db_flags.h"]
        "mlbdb";

      install_lib "mlbdb" ["libmlbdb.a"; "dllmlbdb.so"]
  | _ ->
      ()
end
