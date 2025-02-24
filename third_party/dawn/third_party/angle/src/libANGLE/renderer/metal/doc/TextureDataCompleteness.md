# Texture data completeness's handling in the Metal back-end

The OpenGL spec allows a texture's images to be defined without consistent size and format through
glTexImage*, glCopyImage* calls. The texture doesn't need to be complete when created.

The OpenGL context checks the texture's images during draw calls. It considers the texture complete
if the images are consistent in size and format. Then it uses the texture for rendering.

Metal textures (i.e. MTLTexture) on the other hand require consistent defined images at all times.
MTLTextures are always created complete.

This is an overview of how the Metal back-end implements images' management for a texture to make
sure it is GL spec conformant (TextureMtl):
1. Initially:
    * no actual MTLTexture is created yet.
    * glTexImage/glCopyImage(slice,level):
      * a single image (`images[slice][level]`: 2D/3D MTLTexture no mipmap + single slice) is
        created to store data for the texture at this level/slice.
    * glTexSubImage/glCopyTexSubImage(slice,level):
      * modifies the data of `images[slice][level]`;
2. If the texture is complete at Draw/generateMip/FBO attachment call:
    * an actual MTLTexture object is created. We can call it "native" texture, i.e. the real texture
      that will be consumed by Metal draw calls.
      - `images[0][0]` --> copy to actual texture's slice 0, level 0.
      - `images[0][1]` --> copy to actual texture's slice 0, level 1.
      - `images[0][2]` --> copy to actual texture's slice 0, level 2.
      - ...
    * The images will be destroyed, then re-created to become texture views of the actual texture at
      the specified level/slice.
      - `images[0][0]` -> view of actual texture's slice 0, level 0.
      - `images[0][1]` -> view of actual texture's slice 0, level 1.
      - `images[0][2]` -> view of actual texture's slice 0, level 2.
      - ...
3. After texture is complete:
    * glTexSubImage/glCopyTexSubImage(slice,level):
      * `images[slice][level]`'s content is modified, which means the actual texture's content at
        respective slice & level is modified also. Since the former is a view of the latter at given
        slice & level.
    * glTexImage/glCopyImage(slice,level):
      * If size != `images[slice][level]`.size():
        - Destroy the actual texture (the other views are kept intact), recreate
          `images[slice][level]` as single image same as initial stage. The other views are kept
          intact so that texture data at those slice & level can be reused later.
      * else:
        - behaves as glTexSubImage/glCopyTexSubImage(slice,level).
