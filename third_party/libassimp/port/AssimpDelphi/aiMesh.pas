unit aiMesh;

interface

uses aiTypes, aiMatrix4x4, aiVector3D, aiColor4D;

const
   AI_MAX_NUMBER_OF_COLOR_SETS = $4;
   AI_MAX_NUMBER_OF_TEXTURECOORDS = $4;

type TaiFace = packed record
   mNumIndicies: cardinal;
   mIndices: PCardinalArray;
end;
type PaiFace = ^TaiFace;
type PaiFaceArray = array [0..0] of PaiFace;

type TaiFaceArray = array [0..0] of TaiFace;
type PTaiFaceArray = ^TaiFaceArray;

type TaiVertexWeight = packed record
   mVertexId: cardinal;
   mWeight: single;
end;

type TaiBone = packed record
   mName: aiString;
   mNumWeights: cardinal;
   mWeights: Pointer;
   mOffsetMatrix: TaiMatrix4x4;
end;
type PaiBone = ^TaiBone;

type TaiPrimitiveType =
   (
   	aiPrimitiveType_POINT       = $1,
   	aiPrimitiveType_LINE        = $2,
	   aiPrimitiveType_TRIANGLE    = $4,
   	aiPrimitiveType_POLYGON     = $8
	   //,_aiPrimitiveType_Force32Bit = $9fffffff
   );

type TaiMesh = packed record
   mPrimitiveTypes: cardinal;
   mNumVertices: cardinal;
   mNumFaces: cardinal;
   mVertices: PTaiVector3DArray;
   mNormals: PTaiVector3DArray;
   mTangents: PaiVector3DArray;
   mBitangents: PaiVector3DArray;
   mColors: array[0..3] of PTaiColor4Darray; //array [0..3] of PaiColor4DArray; //array of 4
   mTextureCoords: array [0..3] of PTaiVector3DArray; //array of 4
   mNumUVComponents: array[0..AI_MAX_NUMBER_OF_TEXTURECOORDS -1] of cardinal;
   mFaces: PTaiFaceArray;
   mNumBones: cardinal;
   mBones: PaiBone;
   mMaterialIndex: cardinal;
   mName: aiString;
   mNumAniMeshes: cardinal;
   mAniMeshes: pointer;
end;
type PaiMesh = ^TaiMesh;
type PPaiMesh = ^PaiMesh;
type PaiMeshArray = array [0..0] of PaiMesh;
type PPaiMeshArray = ^PaiMeshArray;



implementation

end.
