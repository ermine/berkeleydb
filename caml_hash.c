 #include <caml/alloc.h>
 #include <caml/mlvalues.h>
 #include <caml/fail.h>
 #include <caml/memory.h>
 #include <caml/custom.h>
 #include <caml/signals.h>
 #include <caml/callback.h>

CAMLprim value my_hash_variant(value vs) {
  char* s = String_val(vs);
  return caml_copy_int32(caml_hash_variant(s));
}
