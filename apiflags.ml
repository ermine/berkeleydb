external my_hash_variant : string -> int32
  = "my_hash_variant"

let _ =
  let f = open_in Sys.argv.(1) in
  let line () =
    try Some (input_line f)
    with End_of_file -> close_in f; None in
  let rec read_file defines sp =
    match line () with
      | Some l ->
          if String.length l > 0 then
            if l.[0] <> ' ' then (
              read_file defines ((l, []) :: sp)
            ) else if l.[0] = ' ' then
              let flag = String.sub l 1 (String.length l - 1) in
              let newsp =
                let (fname, l) = List.hd sp in
                  (fname, flag :: l) :: List.tl sp in
                if List.mem flag defines then
                  read_file defines newsp
                else
                  read_file (flag :: defines) newsp
            else
              read_file defines sp
          else
            read_file defines sp
      | None -> (List.rev defines, List.rev sp)
  in
  let defines, sp = read_file [] [] in
  let out_h = open_out Sys.argv.(2) in
  let out_ml = open_out Sys.argv.(3) in
    List.iter (fun flag ->
                 Printf.fprintf out_h "#define MLV_%s\t\t0x%lX\n"
                   flag (my_hash_variant flag)
              ) defines;
    Printf.fprintf out_h "\n\n";
    List.iter (fun (fname, l) ->
                 if l <> [] then (
                   Printf.fprintf out_h "static u_int32_t flags_%s[][2] = {\n"
                     fname;
                   Printf.fprintf out_ml "type %s_flag = [\n" fname;
                   List.iter (fun flag ->
                                Printf.fprintf out_h "  {%s, MLV_%s},\n"
                                  flag flag;
                                Printf.fprintf out_ml "  | `%s\n" flag;
                             ) (List.rev l);
                   Printf.fprintf out_h "};\n\n";
                   Printf.fprintf out_ml "]\n\n"
                 ) else
                   ()
              ) sp;
    close_out out_h;
    close_out out_ml
    

