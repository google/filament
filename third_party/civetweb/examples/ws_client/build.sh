#!/bin/sh
rm ws_client_example
cc ws_client.c ../../src/civetweb.c -DUSE_WEBSOCKET -I../../include -lpthread -ldl -g -O0 -DNO_SSL -o ws_client_example -Wall
ls -la ws_client_example

