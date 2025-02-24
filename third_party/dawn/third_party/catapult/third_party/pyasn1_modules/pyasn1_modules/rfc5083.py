# This file is being contributed to of pyasn1-modules software.
#
# Created by Russ Housley without assistance from the asn1ate tool.
# Copyright (c) 2018, Vigil Security, LLC
# License: http://snmplabs.com/pyasn1/license.html
#
#  Authenticated-Enveloped-Data for the Cryptographic Message Syntax (CMS)
#
# ASN.1 source from:
# https://www.rfc-editor.org/rfc/rfc5083.txt

from pyasn1.type import namedtype, tag, univ
from pyasn1_modules import rfc5652


MAX = float('inf')


def _buildOid(*components):
    output = []
    for x in tuple(components):
        if isinstance(x, univ.ObjectIdentifier):
            output.extend(list(x))
        else:
            output.append(int(x))
    return univ.ObjectIdentifier(output)


id_ct_authEnvelopedData = _buildOid(1, 2, 840, 113549, 1, 9, 16, 1, 23)


class AuthEnvelopedData(univ.Sequence):
    pass

AuthEnvelopedData.componentType = namedtype.NamedTypes(
    namedtype.NamedType('version', rfc5652.CMSVersion()),
    namedtype.OptionalNamedType('originatorInfo', rfc5652.OriginatorInfo().subtype(
        implicitTag=tag.Tag(tag.tagClassContext, tag.tagFormatConstructed, 0))),
    namedtype.NamedType('recipientInfos', rfc5652.RecipientInfos()),
    namedtype.NamedType('authEncryptedContentInfo', rfc5652.EncryptedContentInfo()),
    namedtype.OptionalNamedType('authAttrs', rfc5652.AuthAttributes().subtype(
        implicitTag=tag.Tag(tag.tagClassContext, tag.tagFormatSimple, 1))),
    namedtype.NamedType('mac', rfc5652.MessageAuthenticationCode()),
    namedtype.OptionalNamedType('unauthAttrs', rfc5652.UnauthAttributes().subtype(
        implicitTag=tag.Tag(tag.tagClassContext, tag.tagFormatSimple, 2)))
)
