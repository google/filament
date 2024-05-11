unit aiTypes;

interface

//added for Delphi interface
type
   TCardinalArray = array [0..0] of Cardinal;
   PCardinalArray = ^TCardinalArray;

   TSingleArray = array[0..0] of Single;
   PSingleArray = ^TSingleArray; 

type aiString = packed record
   length: Cardinal;
   data: array [0..1023] of char;
end;
type PaiString = ^aiString;

type aiReturn = (
	aiReturn_SUCCESS = $0,
	aiReturn_FAILURE = -$1,
	aiReturn_OUTOFMEMORY = -$3,
	_AI_ENFORCE_ENUM_SIZE = $7fffffff
);

const AI_SUCCESS = aiReturn_SUCCESS;
const AI_FAILURE = aiReturn_FAILURE;
const AI_OUTOFMEMORY = aiReturn_OUTOFMEMORY;




function aiStringToDelphiString( a: aiString): AnsiString;


implementation

function aiStringToDelphiString( a: aiString): AnsiString;
var
   i: integer;
begin
   result := '';
   if a.length > 0 then
   begin
      SetLength( result, a.length);
      for i := 1 to a.length do
      begin
         result[i] := a.data[i-1];
      end;
   end;
end;

end.
