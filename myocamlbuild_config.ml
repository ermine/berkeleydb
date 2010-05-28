open Ocamlbuild_plugin
open Ocamlbuild_pack.Ocamlbuild_where
open Command

(*
 * myocamlbuild.config example:
 * # comment
 * install_dir=../../site-lib
 * bytecode=true
 * nativecode=false
 * xml=../../site-lib/xml
 *)
let config =
  if not (Pathname.exists "myocamlbuild.config") then
    []
  else
    List.fold_left (fun acc line ->
                      if line.[0] = '#' then
                        acc
                      else
                        let sp = String.index line '=' in
                        let pair = ((String.sub line 0 sp),
                                    (String.sub line (sp+1)
                                       (String.length line - sp - 1))) in
                          pair :: acc
                   ) [] (string_list_of_file "myocamlbuild.config")

let get_install_dir () =
  try Sys.getenv "INSTALL_DIR" with Not_found ->
    try List.assoc "install_dir" config
    with Not_found ->
      Printf.printf
        "Please specify install_dir parameter in myocamlbuild.config";
      exit 1

let ocamlfind_query pkg =
  let cmd = Printf.sprintf
    "%s/ocamlfind query %s" !bindir (Filename.quote pkg) in
    Ocamlbuild_pack.My_unix.run_and_open cmd
      (fun ic ->
         (* Log.dprintf 5 "Getting Ocaml directory from command %s" cmd; *)
         input_line ic)

let tags =
  ["use_bigarray"; "use_dbm"; "use_dynlink"; "use_graphics";
   "use_nums"; "use_str"; "use_toplevel"; "use_unix"; "use_threads"]

let pkg_tags = ref []

let rec ocamlfind_get_deps predicates pkg =
  let rec aux_parse acc ic =
    let line = try Some (input_line ic) with _ -> None in
      match line with
        | None -> S (List.rev acc)
        | Some v ->
            let r = Ocamlbuild_pack.Lexers.space_sep_strings &
              Lexing.from_string v in
            let pkg, tail = List.hd r, List.tl r in
              if List.mem ("use_" ^ pkg) tags then
                aux_parse acc ic
              else if List.mem ("use_" ^ pkg, predicates) !pkg_tags then
                aux_parse acc ic
              else (
                pkg_tags := ("use_" ^ pkg, predicates) :: !pkg_tags;
                let deps = ocamlfind_get_deps predicates pkg in
                let rest = S[deps; A"-I"; A(List.hd tail);
                             S(List.map (fun a -> A a) (List.tl tail))]
                in
                  aux_parse (rest :: acc) ic
              )
  in
  let cmd = Printf.sprintf
    "%s/ocamlfind query -r -predicates %s -format \"%%p %%d %%A\" %s"
    !bindir predicates pkg in
    Ocamlbuild_pack.My_unix.run_and_open cmd &
      aux_parse []

let add_package pkg =
  let use_pkg = "use_" ^ pkg in
    flag ["ocaml"; "compile"; use_pkg] &
      S[A"-I"; A(ocamlfind_query pkg)];
    flag ["ocaml"; "link"; "native"; use_pkg] &
      ocamlfind_get_deps "native,mt" pkg;
    flag ["ocaml"; "link"; "byte"; use_pkg] &
      ocamlfind_get_deps "byte,mt" pkg

let extern ?cma ?tag_name name =
  try
    let path = List.assoc name config in
      ocaml_lib ~extern:true ?dir:(if path = "" then None else Some path)
        ?tag_name (match cma with | None -> name | Some name -> name)
  with Not_found ->
    add_package name
  
let make_binding ?debug ?include_dir ?lib_dir ~lib ?headers name =
  flag ["c"; "compile"; "debug"] &
    S [A"-ccopt"; A"-O0"; A"-ccopt"; A"-ggdb"];

  (match include_dir with
     | None -> ()
     | Some dir ->
         flag ["c"; "compile"; ("include_" ^ name ^ "_clib")] &
           S[A"-ccopt"; A dir]
  );

  flag ["link"; "library"; "ocaml"; "use_lib" ^ name] &
    S[A"-cclib"; A"-L."; A"-cclib"; A("-l" ^ name)];

  flag ["link"; "ocaml"; "library"; ("use_" ^ name ^ "_clib")] &
    (match lib_dir with
       | None -> S[A"-cclib"; A lib]
       | Some dir -> S[A"-cclib"; A("-L" ^ dir);
                       A"-cclib"; A lib]
    );

  flag ["ocamlmklib"; "c"; ("use_" ^ name ^ "_clib")] &
    (match lib_dir with
       | None -> S[A"-cclib"; A lib]
       | Some dir -> S[A("-L" ^ dir); A lib]
    );

  (* If `static' is true then every ocaml link in bytecode will add -custom *)
  if false then
    flag ["link"; "ocaml"; "byte"; "use_lib" ^ name] & A"-custom";

  ocaml_lib name;
  
  dep  ["link"; "ocaml"; ("use_lib" ^ name)] ["lib" ^ name -.- "a"];
  
  (* This will import headers in the build directory. *)
  match headers with
    | None -> ()
    | Some h ->
        dep  ["compile"; "c"] h
  
let bytecode =
  try bool_of_string (Sys.getenv "BYTECODE") with Not_found ->
    try bool_of_string (List.assoc "bytecode" config) with Not_found -> true
let nativecode =
  try bool_of_string (Sys.getenv "NATIVECODE") with Not_found ->
    try bool_of_string (List.assoc "nativecode" config) with Not_found -> true

let make_deps name =
  if bytecode then
    name -.- "cma" :: (if nativecode then [name -.- "cmxa";
                                           name -.- "cmxs"] else [])
  else
    (if nativecode then [name -.- "cmxa"; name -.- "cmxs"] else [])

let install_lib name ?cma modules =
  let cma =
    match cma with
      | None -> name
      | Some v -> v
  in
  let deps = make_deps cma in
    rule "cmxs"
      ~prod:"%.cmxs"
      ~dep:"%.cmxa"
      (fun env _build ->
         let cmxa = env "%.cmxa"
         and cmxs = env "%.cmxs" in
           Cmd (S[Px"ocamlopt"; A"-shared"; A cmxa; A"-o"; A cmxs])
      );

    rule "Install Library"
      ~prod:"install"
      ~deps
      (fun env _build ->
         let install_dir = get_install_dir () in
         let deps = List.map (fun file -> A file) deps in
         let mllib =
           let mllib = cma -.- "mllib" in
             if Pathname.exists mllib then
               let l = string_list_of_file mllib in
               let add_cmi file acc =
                 if Pathname.exists (file -.- "mli") then
                   A (file -.- "mli") :: A (file -.- "cmi") :: acc
                 else
                   A (file -.- "cmi") :: acc
               in
                 List.fold_left (fun acc f ->
                                   if Pathname.exists (f -.- "cmi") then
                                     add_cmi f acc
                                   else
                                     let f = String.uncapitalize f in
                                       if Pathname.exists (f -.- "cmi") then
                                         add_cmi f acc
                                       else
                                         acc
                                ) [A (cma -.- "a")] l
             else if Pathname.exists (cma -.- "mli") then
               [A (cma -.- "mli") ; A (cma -.- "cmi")]
             else
               [A (cma -.- "cmi")]
         in
         let files = List.map (fun file -> A file) modules in
           Seq [Cmd (S[A"mkdir"; A"-p"; P (install_dir / name)]);
                Cmd (S[Px"install"; S deps; P (install_dir / name)]);
                Cmd (S[Px"install"; S mllib; S files; P (install_dir / name)]);
               ]
      )
