// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package webpagereplay

import (
	"bytes"
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
)

var (
	// A temporary directory created to install a test root CA. This is used as a
	// substititue for android system directory for root CA certs in bind mount.
	androidTempCertDir = "/data/cacerts"

	// Android system directory for root CA certs.
	androidSystemCertDir = "/system/etc/security/cacerts"
)

type Installer struct {
	AndroidDeviceId    string
	AdbBinaryPath      string
	CertUtilBinaryPath string
}

// Runs the adb command.
func (i *Installer) adb(args ...string) error {
	newArgs := append([]string{"-s", i.AndroidDeviceId}, args...)
	cmd := exec.Command(i.AdbBinaryPath, newArgs...)
	fmt.Println(cmd.Args)
	var out bytes.Buffer
	cmd.Stdout = &out
	if err := cmd.Run(); err != nil {
		return err
	}
	fmt.Print(out.String())
	return nil
}

// Runs the adb shell command.
func (i *Installer) adbShell(args ...string) error {
	shellArgs := append([]string{"shell"}, args...)
	return i.adb(shellArgs...)
}

// The issuer hash is used as filename for the installed cert.
func getIssuerHashFileName(certPath string) (string, error) {
	cmd := exec.Command("openssl", "x509", "-in", certPath, "-issuer_hash_old", "-noout")
	var out bytes.Buffer
	var stderr bytes.Buffer
	cmd.Stdout = &out
	cmd.Stderr = &stderr
	err := cmd.Run()
	if err != nil {
		return "", fmt.Errorf("%v : %s", err, stderr.String())
	}
	fmt.Print(out.String())
	return strings.Trim(out.String(), "\r\n") + ".0", nil
}

// Formats the cert and returns the formatted cert.
func formatCert(certPath string) (string, error) {
	cmd := exec.Command("openssl", "x509", "-inform", "PEM", "-text", "-in", certPath)
	var out bytes.Buffer
	cmd.Stdout = &out
	err := cmd.Run()
	if err != nil {
		return "", err
	}
	output := out.String()
	index := strings.Index(output, "-----BEGIN CERTIFICATE")
	return strings.Join([]string{output[index:], output[:index]}, ""), nil
}

func (i *Installer) AdbInstallRoot(certPath string) error {
	var err error
	issuerHashFileName, err := getIssuerHashFileName(certPath)
	if err != nil {
		return fmt.Errorf("cannot create issuer hash: %v", err)
	}
	newCert, err := formatCert(certPath)
	if err != nil {
		return fmt.Errorf("cannot format cert: %v", err)
	}
	tmpdir, err := ioutil.TempDir("", "adb_install_root")
	if err != nil {
		return fmt.Errorf("cannot make tempdir: %v", err)
	}
	defer os.RemoveAll(tmpdir)
	newCertFilePath := filepath.Join(tmpdir, issuerHashFileName)
	if err = ioutil.WriteFile(newCertFilePath, []byte(newCert), 0666); err != nil {
		return fmt.Errorf("failed to write to temp file %v", err)
	}
	if err = i.adbShell("mkdir", androidTempCertDir); err != nil {
		return err
	}
	if err = i.adbShell("cp", androidSystemCertDir+"/*", androidTempCertDir); err != nil {
		return err
	}
	if err = i.adbShell("mount", "-o", "bind", androidTempCertDir, androidSystemCertDir); err != nil {
		return err
	}
	if err = i.adb("push", newCertFilePath, androidSystemCertDir); err != nil {
		return err
	}
	return nil
}

func (i *Installer) AdbUninstallRoot() error {
	var err error
	if err = i.adbShell("umount", androidSystemCertDir); err != nil {
		return err
	}
	if err = i.adbShell("rm", "-r", androidTempCertDir); err != nil {
		return err
	}
	return nil
}
