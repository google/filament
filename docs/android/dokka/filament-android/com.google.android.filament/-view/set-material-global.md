//[filament-android](../../../index.md)/[com.google.android.filament](../index.md)/[View](index.md)/[setMaterialGlobal](set-material-global.md)

# setMaterialGlobal

[main]\
open fun [setMaterialGlobal](set-material-global.md)(index: Int, valuex: Float, valuey: Float, valuez: Float, valuew: Float)

Set the value of material global variables. 

There are up-to four such variable each of type float4. These variables can be read in a user Material with `getMaterialGlobal{0|1|2|3}()`. All variable start with a default value of { 0, 0, 0, 1 }

#### Parameters

main

| | |
|---|---|
| index | index of the variable to set between 0 and 3. |
| valuex | (x component) new value for the variable. |
| valuey | (y component) new value for the variable. |
| valuez | (z component) new value for the variable. |
| valuew | (w component) new value for the variable. |

#### See also

| |
|---|
| [getMaterialGlobal](get-material-global.md) |

[main]\
open fun [setMaterialGlobal](set-material-global.md)(index: Int, value: Array&lt;Float&gt;)

Set the value of material global variables. 

There are up-to four such variable each of type float4. These variables can be read in a user Material with `getMaterialGlobal{0|1|2|3}()`. All variable start with a default value of { 0, 0, 0, 1 }

#### Parameters

main

| | |
|---|---|
| index | index of the variable to set between 0 and 3. |
| value | new value for the variable. |

#### See also

| |
|---|
| [getMaterialGlobal](get-material-global.md) |
