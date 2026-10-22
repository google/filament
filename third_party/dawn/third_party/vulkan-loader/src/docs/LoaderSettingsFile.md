<!-- markdownlint-disable MD041 -->
[![Khronos Vulkan][1]][2]

[1]: https://vulkan.lunarg.com/img/Vulkan_100px_Dec16.png "https://www.khronos.org/vulkan/"
[2]: https://www.khronos.org/vulkan/

# Loader Settings File <!-- omit from toc -->

[![Creative Commons][3]][4]

<!-- Copyright &copy; 2025 LunarG, Inc. -->

[3]: https://i.creativecommons.org/l/by-nd/4.0/88x31.png "Creative Commons License"
[4]: https://creativecommons.org/licenses/by-nd/4.0/


## Table of Contents <!-- omit from toc -->

- [Purpose of the Settings File](#purpose-of-the-settings-file)
- [Settings File Discovery](#settings-file-discovery)
  - [Windows](#windows)
  - [Linux/MacOS/BSD/QNX/Fuchsia/GNU](#linuxmacosbsdqnxfuchsiagnu)
  - [Other Platforms](#other-platforms)
  - [Exception for Elevated Privileges](#exception-for-elevated-privileges)
- [Per-Application Settings File](#per-application-settings-file)
- [File Format](#file-format)
- [Example Settings File](#example-settings-file)
  - [Fields](#fields)
- [Behavior](#behavior)


## Purpose of the Settings File

The purpose of the Loader Settings File is to give developers superb control over the
behavior of the Vulkan-Loader.
It enables enhanced controls over which layers to load, the order layers in the call chain,
logging, and which drivers are available.

The Loader Settings File is intended to be used by "Developer Control Panels" for the Vulkan API, such as the Vulkan Configurator, as a replacement for setting debug envrionment variables.

## Settings File Discovery

The Loader Settings File is located by searching in specific file system paths or through
platform specific mechanisms such as the Windows Registry.

### Windows

The Vulkan Loader first searches the Registry Key  HKEY_CURRENT_USER\SOFTWARE\Khronos\Vulkan\LoaderSettings for a DWORD value whose name is
a valid path to a file named 'vk_loader_settings.json'.
If there are no matching values or the file doesn't exist, the Vulkan Loader performs the
same behavior as described above for the Registry Key  HKEY_LOCAL_MACHINE\SOFTWARE\Khronos\Vulkan\LoaderSettings.

### Linux/MacOS/BSD/QNX/Fuchsia/GNU

The Loader Settings File is located by searching for a file named vk_loader_settings.json in the following locations:

`$HOME/.local/share/vulkan/loader_settings.d/`
`$XDG_DATA_HOME/vulkan/loader_settings.d/`
`/etc/vulkan/loader_settings.d/`

Where $HOME and %XDG_DATA_HOME refer to the values contained in the environment variables of the same name.
If a given environment variables is not present, that path is ignored.

### Other Platforms

Platforms not listed above currently do not support the Loader Settings File due to not having an appropriate search mechanism.

### Exception for Elevated Privileges

Because the Loader Settings File contains paths to Layer and ICD manifests, which contain
the paths to various executable binaries, it is necessary to restrict the use of the Loader
Settings File when the application is running with elevated privileges.

This is accomplished by not using any Loader Settings Files that are found in non-privileged locations.

On Windows, running with Elevated Privileges will ignore HKEY_CURRENT_USER\SOFTWARE\Khronos\Vulkan\LoaderSettings.

On Linux/MacOS/BSD/QNX/Fuchsia/GNU, running with Elevated Privileges will use a secure method of querying $HOME and $XDG_DATA_HOME to prevent
malicious injection of unsecure search directories.

## Per-Application Settings File

## File Format

The Loader Settings File is a JSON file with a


## Example Settings File


```json
{
   "file_format_version" : "1.0.1",
   "settings": {

   }
}
```

### Fields

<table style="width:100%">
  <tr>
    <th>JSON Node</th>
    <th>Description and Notes</th>
    <th>Restrictions</th>
    <th>Parent</th>
    <th>Introspection Query</th>
  </tr>




## Behavior
