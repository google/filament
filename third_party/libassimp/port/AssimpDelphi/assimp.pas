unit assimp;

interface

uses aiTypes, aiMatrix4x4, aiMatrix3x3, aiMesh, aiScene, aiMaterial, aiColor4d, aiVector3D;

const ASSIMP_DLL = 'assimp32.dll';

function  aiImportFile(filename: pchar; pFlags: integer): PaiScene; cdecl; external ASSIMP_DLL;
procedure aiReleaseImport( pScene: pointer); cdecl; external ASSIMP_DLL;
function  aiGetErrorString(): PChar; cdecl; external ASSIMP_DLL;

//procedure aiDecomposeMatrix( var mat: TaiMatrix4x4; var scaling: TaiVector3D; var rotation: TaiQuaternion; var position: TaiVector3D); cdecl; external ASSIMP_DLL;
procedure aiTransposeMatrix4( var mat: TaiMatrix4x4); cdecl; external ASSIMP_DLL;
procedure aiTransposeMatrix3( var mat: TaiMatrix3x3); cdecl; external ASSIMP_DLL;
procedure aiTransformVecByMatrix3( var vec: TaiVector3D; var mat: TaiMatrix3x3); cdecl; external ASSIMP_DLL;
procedure aiTransformVecByMatrix4( var vec: TaiVector3D; var mat: TaiMatrix4x4); cdecl; external ASSIMP_DLL;

procedure aiMultiplyMatrix4(var dst: TaiMatrix4x4; var src: TaiMatrix4x4); cdecl; external ASSIMP_DLL;
procedure aiMultiplyMatrix3(var dst: TaiMatrix3x3; var src: TaiMatrix3x3); cdecl; external ASSIMP_DLL;


procedure aiIdentityMatrix3(var mat: TaiMatrix3x3); cdecl; external ASSIMP_DLL;
procedure aiIdentityMatrix4(var mat: TaiMatrix4x4); cdecl; external ASSIMP_DLL;


//----- from aiMaterial.h
function aiGetMaterialProperty( pMat: PaiMaterial; pKey: PChar; nType: Cardinal; nIndex: Cardinal; pPropOut: pointer): aiReturn; cdecl; external ASSIMP_DLL;
function aiGetMaterialFloatArray( var pMat: TaiMaterial; pKey: PChar; nType: Cardinal; nIndex: Cardinal; var pOut: Single; var pMax: Cardinal): aiReturn; cdecl; external ASSIMP_DLL;
function aiGetMaterialFloat( var pMat: TaiMaterial; pKey: PChar; nType: Cardinal; nIndex: Cardinal; var pOut: Single): aiReturn;
function aiGetMaterialIntegerArray(var pMat: TaiMaterial; pKey: PChar; nType: Cardinal; nIndex: Cardinal; var pOut: Integer; var pMax: Cardinal): aiReturn; cdecl; external ASSIMP_DLL;
function aiGetMaterialInteger(var pMat: TaiMaterial; pKey: PChar; nType: Cardinal; nIndex: Cardinal; var pOut: Integer): aiReturn;
function aiGetMaterialColor(var pMat: TaiMaterial; pKey: PChar; nType: Cardinal; nIndex: Cardinal; var pOut: TaiColor4d): aiReturn; cdecl; external ASSIMP_DLL;
function aiGetMaterialString(var pMat: TaiMaterial; pKey: PChar; nType: Cardinal; nIndex: Cardinal; var pOut: aiString): aiReturn; cdecl; external ASSIMP_DLL;
function aiGetMaterialTextureCount(var pMat: TaiMaterial; nType: TaiTextureType): Cardinal; cdecl; external ASSIMP_DLL;
function aiGetMaterialTexture(var mat: TaiMaterial; nType: TaiTextureType; nIndex: Cardinal; var path: aiString; var mapping: TaiTextureMapping; var uvindex: Cardinal; var blend: single; var op: TaiTextureOp; var mapmode: TaiTextureMapMode; var flags: Cardinal): aiReturn; cdecl; external ASSIMP_DLL;



implementation

function aiGetMaterialFloat( var pMat: TaiMaterial; pKey: PChar; nType: Cardinal; nIndex: Cardinal; var pOut: Single): aiReturn;
var
   n: cardinal;
begin
   n := 0;
   result := aiGetMaterialFloatArray( pMat, pKey, nType, nIndex, pOut, n);
end;

function aiGetMaterialInteger(var pMat: TaiMaterial; pKey: PChar; nType: Cardinal; nIndex: Cardinal; var pOut: integer): aiReturn;
var
   n: cardinal;
begin
   n := 0;
   result := aiGetMaterialIntegerArray( pMat, pKey, nType, nIndex, pOut, n);
end;

end.
