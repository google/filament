Civetweb Install Guide
====

This guide covers the distributions for CivetWeb.  The latest source code is available at [https://github.com/civetweb/civetweb](https://github.com/civetweb/civetweb).

Windows
---

This pre-built version comes pre-built wit Lua support. Libraries for SSL support are not included due to licensing restrictions;
however, users may add an SSL library themselves.
Instructions for adding SSL support can be found in [https://github.com/civetweb/civetweb/tree/master/docs](https://github.com/civetweb/civetweb/tree/master/docs)

1. In case the Visual C++ Redistributable are not already installed:
  32 Bit Version: Install the [Redistributable for Visual Studio 2010](http://www.microsoft.com/en-us/download/details.aspx?id=8328)
  64 Bit Version: Install the [Redistributable for Visual Studio 2015](http://www.microsoft.com/en-us/download/details.aspx?id=48145)
  Note: The required version of the Redistributables may vary, depending on the CivetWeb version.
2. Download latest *civetweb-win.zip* from [SourceForge](https://sourceforge.net/projects/civetweb/files/)
3. When started, Civetweb puts itself into the tray.

OS X
---

This pre-built version comes with Lua, IPV6 and SSL support.

1. Download the latest *Civetweb.dmg* from [SourceForge](https://sourceforge.net/projects/civetweb/files/)
2. Click on the it and look for the attachment in the finder.
4. Drag Civetweb to the Applications folder.
5. When started, Civetweb puts itself into top menu.

Linux
---

1. Download the latest *civetweb.tar.gz* from [SourceForge](https://sourceforge.net/projects/civetweb/files/) or [GitHub](https://github.com/civetweb/civetweb/releases)
2. Open archive and change to the new directory.
3. make help
4. make
5. make install
6. Run the program ```/usr/local/bin/civetweb```, it will use the configuration file */usr/local/etc/civetweb.conf*.

Most Linux systems support auto completion of command line arguments.  To enable bash auto completion for the civetweb stand-alone executable, set *resources/complete.lua* as complete command.  See comments in that file for further instructions.

