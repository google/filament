unit aiScene;

interface

uses aiTypes, aiMatrix4x4, aiMesh, aiMaterial, aiTexture;


type
  PaiNode = ^TaiNode;
  PPaiNode = ^PaiNode;
  PaiNodeArray = array[0..0] of PaiNode;
  PPaiNodeArray = ^PaiNodeArray;

  TaiNode = packed record
   mName: aiString;
   mTransformation: TaiMatrix4x4;
   mParent: PPaiNode;
   mNumChildren: cardinal;
   mChildren: PPaiNodeArray;
   mNumMeshes: cardinal;
   mMeshes: PCardinalArray;
  end;



type TaiScene = packed record
   mFlags: cardinal;
   mRootNode: PaiNode;
   mNumMeshes: Cardinal;
   mMeshes: PPaiMeshArray; //?
   mNumMaterials: Cardinal;
   mMaterials: PPaiMaterialArray;
   mNumAnimations: Cardinal;
   mAnimations: Pointer;
   mNumTextures: Cardinal;
   mTextures: PPaiTextureArray;
   mNumLights: Cardinal;
   mLights: Pointer;
   mNumCameras: Cardinal;
   mCameras: Pointer;
end;
type PaiScene = ^TaiScene;

implementation

end.
