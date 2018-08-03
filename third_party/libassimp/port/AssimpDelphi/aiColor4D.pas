unit aiColor4D;

interface

const AI_MAX_NUMBER_OF_COLOR_SETS = $04;

type TaiColor4D = packed record
   r, g, b, a: single;
end;
type PaiColor4D = ^TaiColor4D;

type TaiColor4DArray = array[0..0] of TaiColor4D;
type PTaiColor4DArray = ^TaiColor4DArray;

implementation

end.
