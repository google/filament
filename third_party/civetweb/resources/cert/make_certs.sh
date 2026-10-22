#!/bin/sh
#using "pass" for every password

echo "Generating client certificate ..."

openssl genrsa -des3 -out client.key 2048
openssl req -new -key client.key -out client.csr

cp client.key client.key.orig

openssl rsa -in client.key.orig -out client.key

openssl x509 -req -days 3650 -in client.csr -signkey client.key -out client.crt

cp client.crt client.pem
cat client.key >> client.pem

openssl pkcs12 -export -inkey client.key -in client.pem -name ClientName -out client.pfx


echo "Generating first server certificate ..."

openssl genrsa -des3 -out server.key 2048
openssl req -new -key server.key -out server.csr

cp server.key server.key.orig

openssl rsa -in server.key.orig -out server.key

openssl x509 -req -days 3650 -in server.csr -signkey server.key -out server.crt

cp server.crt server.pem
cat server.key >> server.pem

openssl pkcs12 -export -inkey server.key -in server.pem -name ServerName -out server.pfx

echo "First server certificate hash for Public-Key-Pins header:"

openssl x509 -pubkey < server.crt | openssl pkey -pubin -outform der | openssl dgst -sha256 -binary | base64 > server.pin

cat server.pin

echo "Generating backup server certificate ..."

openssl genrsa -des3 -out server_bkup.key 2048
openssl req -new -key server_bkup.key -out server_bkup.csr

cp server_bkup.key server_bkup.key.orig

openssl rsa -in server_bkup.key.orig -out server_bkup.key

openssl x509 -req -days 3650 -in server_bkup.csr -signkey server_bkup.key -out server_bkup.crt

cp server_bkup.crt server_bkup.pem
cat server_bkup.key >> server_bkup.pem

openssl pkcs12 -export -inkey server_bkup.key -in server_bkup.pem -name ServerName -out server_bkup.pfx

echo "Backup server certificate hash for Public-Key-Pins header:"

openssl x509 -pubkey < server_bkup.crt | openssl pkey -pubin -outform der | openssl dgst -sha256 -binary | base64 > server_bkup.pin

cat server_bkup.pin

