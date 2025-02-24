"""
Unit tests for CLI entry points.
"""

from __future__ import print_function

import unittest
import sys
import functools
from contextlib import contextmanager

import os
from io import StringIO, BytesIO

import rsa
import rsa.cli
import rsa.util
from rsa._compat import PY2


def make_buffer():
    if PY2:
        return BytesIO()
    buf = StringIO()
    buf.buffer = BytesIO()
    return buf


def get_bytes_out(out):
    if PY2:
        # Python 2.x writes 'str' to stdout
        return out.getvalue()
    # Python 3.x writes 'bytes' to stdout.buffer
    return out.buffer.getvalue()


@contextmanager
def captured_output():
    """Captures output to stdout and stderr"""

    new_out, new_err = make_buffer(), make_buffer()
    old_out, old_err = sys.stdout, sys.stderr
    try:
        sys.stdout, sys.stderr = new_out, new_err
        yield new_out, new_err
    finally:
        sys.stdout, sys.stderr = old_out, old_err


@contextmanager
def cli_args(*new_argv):
    """Updates sys.argv[1:] for a single test."""

    old_args = sys.argv[:]
    sys.argv[1:] = [str(arg) for arg in new_argv]

    try:
        yield
    finally:
        sys.argv[1:] = old_args


def remove_if_exists(fname):
    """Removes a file if it exists."""

    if os.path.exists(fname):
        os.unlink(fname)


def cleanup_files(*filenames):
    """Makes sure the files don't exist when the test runs, and deletes them afterward."""

    def remove():
        for fname in filenames:
            remove_if_exists(fname)

    def decorator(func):
        @functools.wraps(func)
        def wrapper(*args, **kwargs):
            remove()
            try:
                return func(*args, **kwargs)
            finally:
                remove()

        return wrapper

    return decorator


class AbstractCliTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # Ensure there is a key to use
        cls.pub_key, cls.priv_key = rsa.newkeys(512)
        cls.pub_fname = '%s.pub' % cls.__name__
        cls.priv_fname = '%s.key' % cls.__name__

        with open(cls.pub_fname, 'wb') as outfile:
            outfile.write(cls.pub_key.save_pkcs1())

        with open(cls.priv_fname, 'wb') as outfile:
            outfile.write(cls.priv_key.save_pkcs1())

    @classmethod
    def tearDownClass(cls):
        if hasattr(cls, 'pub_fname'):
            remove_if_exists(cls.pub_fname)
        if hasattr(cls, 'priv_fname'):
            remove_if_exists(cls.priv_fname)

    def assertExits(self, status_code, func, *args, **kwargs):
        try:
            func(*args, **kwargs)
        except SystemExit as ex:
            if status_code == ex.code:
                return
            self.fail('SystemExit() raised by %r, but exited with code %r, expected %r' % (
                func, ex.code, status_code))
        else:
            self.fail('SystemExit() not raised by %r' % func)


class KeygenTest(AbstractCliTest):
    def test_keygen_no_args(self):
        with cli_args():
            self.assertExits(1, rsa.cli.keygen)

    def test_keygen_priv_stdout(self):
        with captured_output() as (out, err):
            with cli_args(128):
                rsa.cli.keygen()

        lines = get_bytes_out(out).splitlines()
        self.assertEqual(b'-----BEGIN RSA PRIVATE KEY-----', lines[0])
        self.assertEqual(b'-----END RSA PRIVATE KEY-----', lines[-1])

        # The key size should be shown on stderr
        self.assertTrue('128-bit key' in err.getvalue())

    @cleanup_files('test_cli_privkey_out.pem')
    def test_keygen_priv_out_pem(self):
        with captured_output() as (out, err):
            with cli_args('--out=test_cli_privkey_out.pem', '--form=PEM', 128):
                rsa.cli.keygen()

        # The key size should be shown on stderr
        self.assertTrue('128-bit key' in err.getvalue())

        # The output file should be shown on stderr
        self.assertTrue('test_cli_privkey_out.pem' in err.getvalue())

        # If we can load the file as PEM, it's good enough.
        with open('test_cli_privkey_out.pem', 'rb') as pemfile:
            rsa.PrivateKey.load_pkcs1(pemfile.read())

    @cleanup_files('test_cli_privkey_out.der')
    def test_keygen_priv_out_der(self):
        with captured_output() as (out, err):
            with cli_args('--out=test_cli_privkey_out.der', '--form=DER', 128):
                rsa.cli.keygen()

        # The key size should be shown on stderr
        self.assertTrue('128-bit key' in err.getvalue())

        # The output file should be shown on stderr
        self.assertTrue('test_cli_privkey_out.der' in err.getvalue())

        # If we can load the file as der, it's good enough.
        with open('test_cli_privkey_out.der', 'rb') as derfile:
            rsa.PrivateKey.load_pkcs1(derfile.read(), format='DER')

    @cleanup_files('test_cli_privkey_out.pem', 'test_cli_pubkey_out.pem')
    def test_keygen_pub_out_pem(self):
        with captured_output() as (out, err):
            with cli_args('--out=test_cli_privkey_out.pem',
                          '--pubout=test_cli_pubkey_out.pem',
                          '--form=PEM', 256):
                rsa.cli.keygen()

        # The key size should be shown on stderr
        self.assertTrue('256-bit key' in err.getvalue())

        # The output files should be shown on stderr
        self.assertTrue('test_cli_privkey_out.pem' in err.getvalue())
        self.assertTrue('test_cli_pubkey_out.pem' in err.getvalue())

        # If we can load the file as PEM, it's good enough.
        with open('test_cli_pubkey_out.pem', 'rb') as pemfile:
            rsa.PublicKey.load_pkcs1(pemfile.read())


class EncryptDecryptTest(AbstractCliTest):
    def test_empty_decrypt(self):
        with cli_args():
            self.assertExits(1, rsa.cli.decrypt)

    def test_empty_encrypt(self):
        with cli_args():
            self.assertExits(1, rsa.cli.encrypt)

    @cleanup_files('encrypted.txt', 'cleartext.txt')
    def test_encrypt_decrypt(self):
        with open('cleartext.txt', 'wb') as outfile:
            outfile.write(b'Hello cleartext RSA users!')

        with cli_args('-i', 'cleartext.txt', '--out=encrypted.txt', self.pub_fname):
            with captured_output():
                rsa.cli.encrypt()

        with cli_args('-i', 'encrypted.txt', self.priv_fname):
            with captured_output() as (out, err):
                rsa.cli.decrypt()

        # We should have the original cleartext on stdout now.
        output = get_bytes_out(out)
        self.assertEqual(b'Hello cleartext RSA users!', output)

    @cleanup_files('encrypted.txt', 'cleartext.txt')
    def test_encrypt_decrypt_unhappy(self):
        with open('cleartext.txt', 'wb') as outfile:
            outfile.write(b'Hello cleartext RSA users!')

        with cli_args('-i', 'cleartext.txt', '--out=encrypted.txt', self.pub_fname):
            with captured_output():
                rsa.cli.encrypt()

        # Change a few bytes in the encrypted stream.
        with open('encrypted.txt', 'r+b') as encfile:
            encfile.seek(40)
            encfile.write(b'hahaha')

        with cli_args('-i', 'encrypted.txt', self.priv_fname):
            with captured_output() as (out, err):
                self.assertRaises(rsa.DecryptionError, rsa.cli.decrypt)


class SignVerifyTest(AbstractCliTest):
    def test_empty_verify(self):
        with cli_args():
            self.assertExits(1, rsa.cli.verify)

    def test_empty_sign(self):
        with cli_args():
            self.assertExits(1, rsa.cli.sign)

    @cleanup_files('signature.txt', 'cleartext.txt')
    def test_sign_verify(self):
        with open('cleartext.txt', 'wb') as outfile:
            outfile.write(b'Hello RSA users!')

        with cli_args('-i', 'cleartext.txt', '--out=signature.txt', self.priv_fname, 'SHA-256'):
            with captured_output():
                rsa.cli.sign()

        with cli_args('-i', 'cleartext.txt', self.pub_fname, 'signature.txt'):
            with captured_output() as (out, err):
                rsa.cli.verify()

        self.assertFalse(b'Verification OK' in get_bytes_out(out))

    @cleanup_files('signature.txt', 'cleartext.txt')
    def test_sign_verify_unhappy(self):
        with open('cleartext.txt', 'wb') as outfile:
            outfile.write(b'Hello RSA users!')

        with cli_args('-i', 'cleartext.txt', '--out=signature.txt', self.priv_fname, 'SHA-256'):
            with captured_output():
                rsa.cli.sign()

        # Change a few bytes in the cleartext file.
        with open('cleartext.txt', 'r+b') as encfile:
            encfile.seek(6)
            encfile.write(b'DSA')

        with cli_args('-i', 'cleartext.txt', self.pub_fname, 'signature.txt'):
            with captured_output() as (out, err):
                self.assertExits('Verification failed.', rsa.cli.verify)


class PrivatePublicTest(AbstractCliTest):
    """Test CLI command to convert a private to a public key."""

    @cleanup_files('test_private_to_public.pem')
    def test_private_to_public(self):

        with cli_args('-i', self.priv_fname, '-o', 'test_private_to_public.pem'):
            with captured_output():
                rsa.util.private_to_public()

        # Check that the key is indeed valid.
        with open('test_private_to_public.pem', 'rb') as pemfile:
            key = rsa.PublicKey.load_pkcs1(pemfile.read())

        self.assertEqual(self.priv_key.n, key.n)
        self.assertEqual(self.priv_key.e, key.e)
