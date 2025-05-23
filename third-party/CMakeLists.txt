include(NuggetDoomSettings)

add_library(md5 STATIC md5/md5.c)
target_nuggetdoom_settings(md5)
target_include_directories(md5 INTERFACE md5)

add_library(miniz STATIC miniz/miniz.c)
target_nuggetdoom_settings(miniz)
target_compile_definitions(miniz PRIVATE MINIZ_NO_TIME)
target_include_directories(miniz INTERFACE miniz)

add_library(pffft STATIC pffft/pffft.c)
target_nuggetdoom_settings(pffft)
target_include_directories(pffft INTERFACE pffft)

add_library(spng STATIC spng/spng.c)
target_nuggetdoom_settings(spng)
target_compile_definitions(spng PRIVATE SPNG_USE_MINIZ INTERFACE SPNG_STATIC)
target_include_directories(spng INTERFACE spng)
target_link_libraries(spng miniz)

if(NOT yyjson_FOUND)
    add_library(yyjson STATIC yyjson/yyjson.c)
    target_nuggetdoom_settings(yyjson)
    target_compile_definitions(yyjson PRIVATE YYJSON_DISABLE_WRITER=1 YYJSON_DISABLE_UTILS=1)
    target_include_directories(yyjson INTERFACE yyjson)
    add_library(yyjson::yyjson ALIAS yyjson)
endif()

add_library(sha1 STATIC sha1/sha1.c)
target_nuggetdoom_settings(sha1)
target_include_directories(sha1 INTERFACE sha1)
