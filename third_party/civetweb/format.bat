#!/bin/sh
clang-format -i src/civetweb.c
clang-format -i src/main.c
clang-format -i src/CivetServer.cpp
clang-format -i src/civetweb_private_lua.h
clang-format -i src/md5.inl
clang-format -i src/sha1.inl
clang-format -i src/mod_lua.inl
clang-format -i src/mod_lua_shared.inl
clang-format -i src/mod_duktape.inl
clang-format -i src/mod_zlib.inl
clang-format -i src/timer.inl
clang-format -i src/handle_form.inl

clang-format -i src/third_party/civetweb_lua.h

clang-format -i include/civetweb.h
clang-format -i include/CivetServer.h

clang-format -i unittest/public_func.h
clang-format -i unittest/public_func.c
clang-format -i unittest/public_server.h
clang-format -i unittest/public_server.c
clang-format -i unittest/private.h
clang-format -i unittest/private.c
clang-format -i unittest/private_exe.h
clang-format -i unittest/private_exe.c
clang-format -i unittest/shared.h
clang-format -i unittest/shared.c
clang-format -i unittest/timertest.h
clang-format -i unittest/timertest.c
clang-format -i unittest/civetweb_check.h
clang-format -i unittest/main.c

clang-format -i examples/embedded_c/embedded_c.c
clang-format -i examples/rest/rest.c

