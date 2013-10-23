set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I..")

set(libs
  Math.c
)

foreach (LIB ${libs})
  add_library(${LIB} SHARED ${LIB})
  # output Foo.c as libfoo.a/so
  string(REGEX REPLACE ".c" "" EXTLESS_LIB ${LIB})
  set_target_properties(${LIB} PROPERTIES PREFIX "" OUTPUT_NAME ${EXTLESS_LIB})
endforeach(LIB)
