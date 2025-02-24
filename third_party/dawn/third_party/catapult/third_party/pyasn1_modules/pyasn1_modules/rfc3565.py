# This file is being contributed to pyasn1-modules software.
#
# Created by Russ Housley.
#
# Copyright (c) 2019, Vigil Security, LLC
# License: http://snmplabs.com/pyasn1/license.html
#
# Use of the Elliptic Curve Diffie-Hellman Key Agreement Algorithm
#   with X25519 and X448 in the Cryptographic Message Syntax (CMS)
#
# ASN.1 source from:
# https://www.rfc-editor.org/rfc/rfc3565.txt


from pyasn1.type import univ, constraint

from pyasn1_modules import rfc5280


class AlgorithmIdentifier(rfc5280.AlgorithmIdentifier):
    pass


class AES_IV(univ.OctetString):
    pass

AES_IV.subtypeSpec = constraint.ValueSizeConstraint(16, 16)


id_aes128_CBC = univ.ObjectIdentifier('2.16.840.1.101.3.4.1.2')

id_aes192_CBC = univ.ObjectIdentifier('2.16.840.1.101.3.4.1.22')

id_aes256_CBC = univ.ObjectIdentifier('2.16.840.1.101.3.4.1.42')


id_aes128_wrap = univ.ObjectIdentifier('2.16.840.1.101.3.4.1.5')

id_aes192_wrap = univ.ObjectIdentifier('2.16.840.1.101.3.4.1.25')

id_aes256_wrap = univ.ObjectIdentifier('2.16.840.1.101.3.4.1.45')
