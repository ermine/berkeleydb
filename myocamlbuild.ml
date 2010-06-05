open Ocamlbuild_plugin
open Myocamlbuild_config

let _ =  dispatch begin function
  | After_rules ->
      make_binding 
        ~include_dir:"-I/usr/local/include"
        ~lib_dir:"/usr/local/lib"
        ~lib:"-ldb-4.8" 
        ~headers:["db_flags.h"; "db_helper.h"]
        "berkeleyDB";

      install_lib "berkeleyDB" ["libberkeleyDB.a"; "dllberkeleyDB.so"];

  | _ ->
      ()
end
