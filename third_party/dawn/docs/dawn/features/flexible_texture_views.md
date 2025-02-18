# Flexible Texture Views
In Compat Mode, a texture by default:
- Must be created with texture view dimension specified.
- Cannot use different views in the same draw call.
- Cannot create a 2D view of a 2DArray texture.

The `FlexibleTextureViews` feature removes the above restrictions.
