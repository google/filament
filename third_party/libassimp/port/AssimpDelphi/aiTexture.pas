unit aiTexture;

interface

type TaiTexel = packed record
   b, g, r, a: byte;
end;
PaiTexel = ^TaiTexel;
TaiTexelArray = array[0..0] of TaiTexel;
PaiTexelArray = ^TaiTexelArray;

type TaiTexture = packed record
   mWidth: Cardinal; //width in pixels, OR total embedded file size if texture is a jpg/png/etc
   mHeight: Cardinal; //0 if texture is an embedded file
   achFormatHint: array[0..3] of byte;
   pcData: PaiTexelArray;
end;
PaiTexture = ^TaiTexture;
PaiTextureArray = array [0..0] of PaiTexture;
PPaiTextureArray = ^PaiTextureArray;



implementation

end.
