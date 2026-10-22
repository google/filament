unit aiVector3D;

interface

type TaiVector3D = packed record
   x, y, z: single;
end;
type PaiVector3D = ^TaiVector3D;
type PaiVector3DArray = array [0..0] of PaiVector3D;

type TaiVector3DArray = array[0..0] of TaiVector3D;
type PTaiVector3DArray = ^TaiVector3DArray;

implementation

end.
