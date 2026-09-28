//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[Engine](index.md)/[getConfig](get-config.md)

# getConfig

[main]\
open fun [getConfig](get-config.md)(): [Engine.Config](-config/index.md)

Retrieves the configuration settings of this Engine. 

This method returns the configuration object that was supplied to the Engine's Builder::config method during the creation of this Engine. If the Builder::config method was not explicitly called (or called with nullptr), this method returns the default configuration settings.

#### Return

a Config object with this Engine's configuration

#### See also

| |
|---|
| [Engine.Builder](-builder/config.md) |
