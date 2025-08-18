// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

package com.google.protobuf.osgi;

import aQute.bnd.osgi.Analyzer;
import aQute.bnd.osgi.Jar;
import java.io.File;
import java.util.Arrays;
import java.util.concurrent.Callable;
import java.util.jar.Manifest;
import java.util.stream.Collectors;
import picocli.CommandLine;
import picocli.CommandLine.Command;
import picocli.CommandLine.Option;

/** Java binary that runs bndlib to analyze a jar file to generate OSGi bundle manifest. */
@Command(name = "osgi_wrapper")
public final class OsgiWrapper implements Callable<Integer> {
  private static final String REMOVEHEADERS =
      Arrays.stream(
              new String[] {
                "Embed-Dependency",
                "Embed-Transitive",
                "Built-By",
                // "Tool",
                "Created-By",
                // "Build-Jdk",
                "Originally-Created-By",
                "Archiver-Version",
                "Include-Resource",
                "Private-Package",
                "Ignore-Package",
                // "Bnd-LastModified",
                "Target-Label"
              })
          .collect(Collectors.joining(","));

  @Option(
      names = {"--input_jar"},
      description = "The jar file to wrap with OSGi metadata")
  private File inputJar;

  @Option(
      names = {"--output_jar"},
      description = "Output path to the wrapped jar")
  private File outputJar;

  @Option(
      names = {"--classpath"},
      description = "The classpath that contains dependencies of the input jar, separated with :")
  private String classpath;

  @Option(
      names = {"--automatic_module_name"},
      description = "The automatic module name of the bundle")
  private String automaticModuleName;

  @Option(
      names = {"--bundle_copyright"},
      description = "Copyright string for the bundle")
  private String bundleCopyright;

  @Option(
      names = {"--bundle_description"},
      description = "Description of the bundle")
  private String bundleDescription;

  @Option(
      names = {"--bundle_doc_url"},
      description = "Documentation URL for the bundle")
  private String bundleDocUrl;

  @Option(
      names = {"--bundle_license"},
      description = "URL for the license of the bundle")
  private String bundleLicense;

  @Option(
      names = {"--bundle_name"},
      description = "The name of the bundle")
  private String bundleName;

  @Option(
      names = {"--bundle_symbolic_name"},
      description = "The symbolic name of the bundle")
  private String bundleSymbolicName;

  @Option(
      names = {"--bundle_version"},
      description = "The version of the bundle")
  private String bundleVersion;

  @Option(
      names = {"--export_package"},
      description = "The exported packages from this bundle")
  private String exportPackage;

  @Option(
      names = {"--import_package"},
      description = "The imported packages from this bundle")
  private String importPackage;

  @Override
  public Integer call() throws Exception {
    Jar bin = new Jar(inputJar);

    Analyzer analyzer = new Analyzer();
    analyzer.setJar(bin);
    analyzer.setProperty(Analyzer.AUTOMATIC_MODULE_NAME, automaticModuleName);
    analyzer.setProperty(Analyzer.BUNDLE_NAME, bundleName);
    analyzer.setProperty(Analyzer.BUNDLE_SYMBOLICNAME, bundleSymbolicName);
    analyzer.setProperty(Analyzer.BUNDLE_VERSION, bundleVersion);
    analyzer.setProperty(Analyzer.IMPORT_PACKAGE, importPackage);
    analyzer.setProperty(Analyzer.EXPORT_PACKAGE, exportPackage);
    analyzer.setProperty(Analyzer.BUNDLE_DESCRIPTION, bundleDescription);
    analyzer.setProperty(Analyzer.BUNDLE_COPYRIGHT, bundleCopyright);
    analyzer.setProperty(Analyzer.BUNDLE_DOCURL, bundleDocUrl);
    analyzer.setProperty(Analyzer.BUNDLE_LICENSE, bundleLicense);
    analyzer.setProperty(Analyzer.REMOVEHEADERS, REMOVEHEADERS);

    if (classpath != null) {
      for (String dep : Arrays.asList(classpath.split(":"))) {
        analyzer.addClasspath(new File(dep));
      }
    }

    analyzer.analyze();

    Manifest manifest = analyzer.calcManifest();

    if (analyzer.isOk()) {
      analyzer.getJar().setManifest(manifest);
      if (analyzer.save(outputJar, true)) {
        return 0;
      }
    }
    return 1;
  }

  public static void main(String[] args) {
    int exitCode = new CommandLine(new OsgiWrapper()).execute(args);
    System.exit(exitCode);
  }
}
