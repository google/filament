Compile CivetWeb to the repository root and run it from there

./civetweb examples/multidomain/base_domain.conf

Check what cerfificate is used

echo | openssl s_client -showcerts -servername default-domain -connect localhost:443 2>/dev/null | openssl x509 -inform pem -noout -text | grep Serial

echo | openssl s_client -showcerts -servername localhost -connect localhost:443 2>/dev/null | openssl x509 -inform pem -noout -text | grep Serial

