Generating new certificates
---------------------------

Here's how you can regenerate the certificates::

    import trustme

    ca = trustme.CA()
    server_cert = ca.issue_cert(u"localhost")

    ca.cert_pem.write_to_path("cacert.pem")
    ca.private_key_pem.write_to_path("cacert.key")
    server_cert.cert_chain_pems[0].write_to_path("server.crt")
    server_cert.private_key_pem.write_to_path("server.key")

This will break a number of tests: you will need to update the
relevant fingerprints and hashes.
