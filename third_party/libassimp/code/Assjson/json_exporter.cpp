/*
Assimp2Json
Copyright (c) 2011, Alexander C. Gessler

Licensed under a 3-clause BSD license. See the LICENSE file for more information.

*/

#ifndef ASSIMP_BUILD_NO_EXPORT
#ifndef ASSIMP_BUILD_NO_ASSJSON_EXPORTER

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>
#include <assimp/scene.h>

#include <sstream>
#include <limits>
#include <cassert>
#include <memory>

#define CURRENT_FORMAT_VERSION 100

// grab scoped_ptr from assimp to avoid a dependency on boost. 
//#include <assimp/../../code/BoostWorkaround/boost/scoped_ptr.hpp>

#include "mesh_splitter.h"

extern "C" {
    #include "cencode.h"
}
namespace Assimp {

void ExportAssimp2Json(const char*, Assimp::IOSystem*, const aiScene*, const Assimp::ExportProperties*);

// small utility class to simplify serializing the aiScene to Json
class JSONWriter {
public:
    enum {
        Flag_DoNotIndent = 0x1,
        Flag_WriteSpecialFloats = 0x2,
    };

    JSONWriter(Assimp::IOStream& out, unsigned int flags = 0u)
    : out(out)
    , first()
    , flags(flags) {
        // make sure that all formatting happens using the standard, C locale and not the user's current locale
        buff.imbue(std::locale("C"));
    }

    ~JSONWriter() {
        Flush();
    }

    void Flush() {
        const std::string s = buff.str();
        out.Write(s.c_str(), s.length(), 1);
        buff.clear();
    }

    void PushIndent() {
        indent += '\t';
    }

    void PopIndent() {
        indent.erase(indent.end() - 1);
    }

    void Key(const std::string& name) {
        AddIndentation();
        Delimit();
        buff << '\"' + name + "\": ";
    }

    template<typename Literal>
    void Element(const Literal& name) {
        AddIndentation();
        Delimit();

        LiteralToString(buff, name) << '\n';
    }

    template<typename Literal>
    void SimpleValue(const Literal& s) {
        LiteralToString(buff, s) << '\n';
    }

    void SimpleValue(const void* buffer, size_t len) {
        base64_encodestate s;
        base64_init_encodestate(&s);

        char* const out = new char[std::max(len * 2, static_cast<size_t>(16u))];
        const int n = base64_encode_block(reinterpret_cast<const char*>(buffer), static_cast<int>(len), out, &s);
        out[n + base64_encode_blockend(out + n, &s)] = '\0';

        // base64 encoding may add newlines, but JSON strings may not contain 'real' newlines
        // (only escaped ones). Remove any newlines in out.
        for (char* cur = out; *cur; ++cur) {
            if (*cur == '\n') {
                *cur = ' ';
            }
        }

        buff << '\"' << out << "\"\n";
        delete[] out;
    }

    void StartObj(bool is_element = false) {
        // if this appears as a plain array element, we need to insert a delimiter and we should also indent it
        if (is_element) {
            AddIndentation();
            if (!first) {
                buff << ',';
            }
        }
        first = true;
        buff << "{\n";
        PushIndent();
    }

    void EndObj() {
        PopIndent();
        AddIndentation();
        first = false;
        buff << "}\n";
    }

    void StartArray(bool is_element = false) {
        // if this appears as a plain array element, we need to insert a delimiter and we should also indent it
        if (is_element) {
            AddIndentation();
            if (!first) {
                buff << ',';
            }
        }
        first = true;
        buff << "[\n";
        PushIndent();
    }

    void EndArray() {
        PopIndent();
        AddIndentation();
        buff << "]\n";
        first = false;
    }

    void AddIndentation() {
        if (!(flags & Flag_DoNotIndent)) {
            buff << indent;
        }
    }

    void Delimit() {
        if (!first) {
            buff << ',';
        }
        else {
            buff << ' ';
            first = false;
        }
    }

private:
    template<typename Literal>
    std::stringstream& LiteralToString(std::stringstream& stream, const Literal& s) {
        stream << s;
        return stream;
    }

    std::stringstream& LiteralToString(std::stringstream& stream, const aiString& s) {
        std::string t;

        // escape backslashes and single quotes, both would render the JSON invalid if left as is
        t.reserve(s.length);
        for (size_t i = 0; i < s.length; ++i) {

            if (s.data[i] == '\\' || s.data[i] == '\'' || s.data[i] == '\"') {
                t.push_back('\\');
            }

            t.push_back(s.data[i]);
        }
        stream << "\"";
        stream << t;
        stream << "\"";
        return stream;
    }

    std::stringstream& LiteralToString(std::stringstream& stream, float f) {
        if (!std::numeric_limits<float>::is_iec559) {
            // on a non IEEE-754 platform, we make no assumptions about the representation or existence
            // of special floating-point numbers. 
            stream << f;
            return stream;
        }

        // JSON does not support writing Inf/Nan
        // [RFC 4672: "Numeric values that cannot be represented as sequences of digits
        // (such as Infinity and NaN) are not permitted."]
        // Nevertheless, many parsers will accept the special keywords Infinity, -Infinity and NaN
        if (std::numeric_limits<float>::infinity() == fabs(f)) {
            if (flags & Flag_WriteSpecialFloats) {
                stream << (f < 0 ? "\"-" : "\"") + std::string("Infinity\"");
                return stream;
            }
            //  we should print this warning, but we can't - this is called from within a generic assimp exporter, we cannot use cerr
            //	std::cerr << "warning: cannot represent infinite number literal, substituting 0 instead (use -i flag to enforce Infinity/NaN)" << std::endl;
            stream << "0.0";
            return stream;
        }
        // f!=f is the most reliable test for NaNs that I know of
        else if (f != f) {
            if (flags & Flag_WriteSpecialFloats) {
                stream << "\"NaN\"";
                return stream;
            }
            //  we should print this warning, but we can't - this is called from within a generic assimp exporter, we cannot use cerr
            //	std::cerr << "warning: cannot represent infinite number literal, substituting 0 instead (use -i flag to enforce Infinity/NaN)" << std::endl;
            stream << "0.0";
            return stream;
        }

        stream << f;
        return stream;
    }

private:
    Assimp::IOStream& out;
    std::string indent, newline;
    std::stringstream buff;
    bool first;

    unsigned int flags;
};

void Write(JSONWriter& out, const aiVector3D& ai, bool is_elem = true) {
    out.StartArray(is_elem);
    out.Element(ai.x);
    out.Element(ai.y);
    out.Element(ai.z);
    out.EndArray();
}

void Write(JSONWriter& out, const aiQuaternion& ai, bool is_elem = true) {
    out.StartArray(is_elem);
    out.Element(ai.w);
    out.Element(ai.x);
    out.Element(ai.y);
    out.Element(ai.z);
    out.EndArray();
}

void Write(JSONWriter& out, const aiColor3D& ai, bool is_elem = true) {
    out.StartArray(is_elem);
    out.Element(ai.r);
    out.Element(ai.g);
    out.Element(ai.b);
    out.EndArray();
}

void Write(JSONWriter& out, const aiMatrix4x4& ai, bool is_elem = true) {
    out.StartArray(is_elem);
    for (unsigned int x = 0; x < 4; ++x) {
        for (unsigned int y = 0; y < 4; ++y) {
            out.Element(ai[x][y]);
        }
    }
    out.EndArray();
}

void Write(JSONWriter& out, const aiBone& ai, bool is_elem = true) {
    out.StartObj(is_elem);

    out.Key("name");
    out.SimpleValue(ai.mName);

    out.Key("offsetmatrix");
    Write(out, ai.mOffsetMatrix, false);

    out.Key("weights");
    out.StartArray();
    for (unsigned int i = 0; i < ai.mNumWeights; ++i) {
        out.StartArray(true);
        out.Element(ai.mWeights[i].mVertexId);
        out.Element(ai.mWeights[i].mWeight);
        out.EndArray();
    }
    out.EndArray();
    out.EndObj();
}

void Write(JSONWriter& out, const aiFace& ai, bool is_elem = true) {
    out.StartArray(is_elem);
    for (unsigned int i = 0; i < ai.mNumIndices; ++i) {
        out.Element(ai.mIndices[i]);
    }
    out.EndArray();
}

void Write(JSONWriter& out, const aiMesh& ai, bool is_elem = true) {
    out.StartObj(is_elem);

    out.Key("name");
    out.SimpleValue(ai.mName);

    out.Key("materialindex");
    out.SimpleValue(ai.mMaterialIndex);

    out.Key("primitivetypes");
    out.SimpleValue(ai.mPrimitiveTypes);

    out.Key("vertices");
    out.StartArray();
    for (unsigned int i = 0; i < ai.mNumVertices; ++i) {
        out.Element(ai.mVertices[i].x);
        out.Element(ai.mVertices[i].y);
        out.Element(ai.mVertices[i].z);
    }
    out.EndArray();

    if (ai.HasNormals()) {
        out.Key("normals");
        out.StartArray();
        for (unsigned int i = 0; i < ai.mNumVertices; ++i) {
            out.Element(ai.mNormals[i].x);
            out.Element(ai.mNormals[i].y);
            out.Element(ai.mNormals[i].z);
        }
        out.EndArray();
    }

    if (ai.HasTangentsAndBitangents()) {
        out.Key("tangents");
        out.StartArray();
        for (unsigned int i = 0; i < ai.mNumVertices; ++i) {
            out.Element(ai.mTangents[i].x);
            out.Element(ai.mTangents[i].y);
            out.Element(ai.mTangents[i].z);
        }
        out.EndArray();

        out.Key("bitangents");
        out.StartArray();
        for (unsigned int i = 0; i < ai.mNumVertices; ++i) {
            out.Element(ai.mBitangents[i].x);
            out.Element(ai.mBitangents[i].y);
            out.Element(ai.mBitangents[i].z);
        }
        out.EndArray();
    }

    if (ai.GetNumUVChannels()) {
        out.Key("numuvcomponents");
        out.StartArray();
        for (unsigned int n = 0; n < ai.GetNumUVChannels(); ++n) {
            out.Element(ai.mNumUVComponents[n]);
        }
        out.EndArray();

        out.Key("texturecoords");
        out.StartArray();
        for (unsigned int n = 0; n < ai.GetNumUVChannels(); ++n) {
            const unsigned int numc = ai.mNumUVComponents[n] ? ai.mNumUVComponents[n] : 2;

            out.StartArray(true);
            for (unsigned int i = 0; i < ai.mNumVertices; ++i) {
                for (unsigned int c = 0; c < numc; ++c) {
                    out.Element(ai.mTextureCoords[n][i][c]);
                }
            }
            out.EndArray();
        }
        out.EndArray();
    }

    if (ai.GetNumColorChannels()) {
        out.Key("colors");
        out.StartArray();
        for (unsigned int n = 0; n < ai.GetNumColorChannels(); ++n) {
            out.StartArray(true);
            for (unsigned int i = 0; i < ai.mNumVertices; ++i) {
                out.Element(ai.mColors[n][i].r);
                out.Element(ai.mColors[n][i].g);
                out.Element(ai.mColors[n][i].b);
                out.Element(ai.mColors[n][i].a);
            }
            out.EndArray();
        }
        out.EndArray();
    }

    if (ai.mNumBones) {
        out.Key("bones");
        out.StartArray();
        for (unsigned int n = 0; n < ai.mNumBones; ++n) {
            Write(out, *ai.mBones[n]);
        }
        out.EndArray();
    }

    out.Key("faces");
    out.StartArray();
    for (unsigned int n = 0; n < ai.mNumFaces; ++n) {
        Write(out, ai.mFaces[n]);
    }
    out.EndArray();

    out.EndObj();
}

void Write(JSONWriter& out, const aiNode& ai, bool is_elem = true) {
    out.StartObj(is_elem);

    out.Key("name");
    out.SimpleValue(ai.mName);

    out.Key("transformation");
    Write(out, ai.mTransformation, false);

    if (ai.mNumMeshes) {
        out.Key("meshes");
        out.StartArray();
        for (unsigned int n = 0; n < ai.mNumMeshes; ++n) {
            out.Element(ai.mMeshes[n]);
        }
        out.EndArray();
    }

    if (ai.mNumChildren) {
        out.Key("children");
        out.StartArray();
        for (unsigned int n = 0; n < ai.mNumChildren; ++n) {
            Write(out, *ai.mChildren[n]);
        }
        out.EndArray();
    }

    out.EndObj();
}

void Write(JSONWriter& out, const aiMaterial& ai, bool is_elem = true) {
    out.StartObj(is_elem);

    out.Key("properties");
    out.StartArray();
    for (unsigned int i = 0; i < ai.mNumProperties; ++i) {
        const aiMaterialProperty* const prop = ai.mProperties[i];
        out.StartObj(true);
        out.Key("key");
        out.SimpleValue(prop->mKey);
        out.Key("semantic");
        out.SimpleValue(prop->mSemantic);
        out.Key("index");
        out.SimpleValue(prop->mIndex);

        out.Key("type");
        out.SimpleValue(prop->mType);

        out.Key("value");
        switch (prop->mType) {
            case aiPTI_Float:
                if (prop->mDataLength / sizeof(float) > 1) {
                    out.StartArray();
                    for (unsigned int i = 0; i < prop->mDataLength / sizeof(float); ++i) {
                        out.Element(reinterpret_cast<float*>(prop->mData)[i]);
                    }
                    out.EndArray();
                }
                else {
                    out.SimpleValue(*reinterpret_cast<float*>(prop->mData));
                }
                break;

            case aiPTI_Integer:
                if (prop->mDataLength / sizeof(int) > 1) {
                    out.StartArray();
                    for (unsigned int i = 0; i < prop->mDataLength / sizeof(int); ++i) {
                        out.Element(reinterpret_cast<int*>(prop->mData)[i]);
                    }
                    out.EndArray();
                } else {
                    out.SimpleValue(*reinterpret_cast<int*>(prop->mData));
                }
                break;

            case aiPTI_String:
                {
                    aiString s;
                    aiGetMaterialString(&ai, prop->mKey.data, prop->mSemantic, prop->mIndex, &s);
                    out.SimpleValue(s);
                }
                break;
            case aiPTI_Buffer:
                {
                    // binary data is written as series of hex-encoded octets
                    out.SimpleValue(prop->mData, prop->mDataLength);
                }
                break;
            default:
                assert(false);
        }

        out.EndObj();
    }

    out.EndArray();
    out.EndObj();
}

void Write(JSONWriter& out, const aiTexture& ai, bool is_elem = true) {
    out.StartObj(is_elem);

    out.Key("width");
    out.SimpleValue(ai.mWidth);

    out.Key("height");
    out.SimpleValue(ai.mHeight);

    out.Key("formathint");
    out.SimpleValue(aiString(ai.achFormatHint));

    out.Key("data");
    if (!ai.mHeight) {
        out.SimpleValue(ai.pcData, ai.mWidth);
    }
    else {
        out.StartArray();
        for (unsigned int y = 0; y < ai.mHeight; ++y) {
            out.StartArray(true);
            for (unsigned int x = 0; x < ai.mWidth; ++x) {
                const aiTexel& tx = ai.pcData[y*ai.mWidth + x];
                out.StartArray(true);
                out.Element(static_cast<unsigned int>(tx.r));
                out.Element(static_cast<unsigned int>(tx.g));
                out.Element(static_cast<unsigned int>(tx.b));
                out.Element(static_cast<unsigned int>(tx.a));
                out.EndArray();
            }
            out.EndArray();
        }
        out.EndArray();
    }

    out.EndObj();
}

void Write(JSONWriter& out, const aiLight& ai, bool is_elem = true) {
    out.StartObj(is_elem);

    out.Key("name");
    out.SimpleValue(ai.mName);

    out.Key("type");
    out.SimpleValue(ai.mType);

    if (ai.mType == aiLightSource_SPOT || ai.mType == aiLightSource_UNDEFINED) {
        out.Key("angleinnercone");
        out.SimpleValue(ai.mAngleInnerCone);

        out.Key("angleoutercone");
        out.SimpleValue(ai.mAngleOuterCone);
    }

    out.Key("attenuationconstant");
    out.SimpleValue(ai.mAttenuationConstant);

    out.Key("attenuationlinear");
    out.SimpleValue(ai.mAttenuationLinear);

    out.Key("attenuationquadratic");
    out.SimpleValue(ai.mAttenuationQuadratic);

    out.Key("diffusecolor");
    Write(out, ai.mColorDiffuse, false);

    out.Key("specularcolor");
    Write(out, ai.mColorSpecular, false);

    out.Key("ambientcolor");
    Write(out, ai.mColorAmbient, false);

    if (ai.mType != aiLightSource_POINT) {
        out.Key("direction");
        Write(out, ai.mDirection, false);

    }

    if (ai.mType != aiLightSource_DIRECTIONAL) {
        out.Key("position");
        Write(out, ai.mPosition, false);
    }

    out.EndObj();
}

void Write(JSONWriter& out, const aiNodeAnim& ai, bool is_elem = true) {
    out.StartObj(is_elem);

    out.Key("name");
    out.SimpleValue(ai.mNodeName);

    out.Key("prestate");
    out.SimpleValue(ai.mPreState);

    out.Key("poststate");
    out.SimpleValue(ai.mPostState);

    if (ai.mNumPositionKeys) {
        out.Key("positionkeys");
        out.StartArray();
        for (unsigned int n = 0; n < ai.mNumPositionKeys; ++n) {
            const aiVectorKey& pos = ai.mPositionKeys[n];
            out.StartArray(true);
            out.Element(pos.mTime);
            Write(out, pos.mValue);
            out.EndArray();
        }
        out.EndArray();
    }

    if (ai.mNumRotationKeys) {
        out.Key("rotationkeys");
        out.StartArray();
        for (unsigned int n = 0; n < ai.mNumRotationKeys; ++n) {
            const aiQuatKey& rot = ai.mRotationKeys[n];
            out.StartArray(true);
            out.Element(rot.mTime);
            Write(out, rot.mValue);
            out.EndArray();
        }
        out.EndArray();
    }

    if (ai.mNumScalingKeys) {
        out.Key("scalingkeys");
        out.StartArray();
        for (unsigned int n = 0; n < ai.mNumScalingKeys; ++n) {
            const aiVectorKey& scl = ai.mScalingKeys[n];
            out.StartArray(true);
            out.Element(scl.mTime);
            Write(out, scl.mValue);
            out.EndArray();
        }
        out.EndArray();
    }
    out.EndObj();
}

void Write(JSONWriter& out, const aiAnimation& ai, bool is_elem = true) {
    out.StartObj(is_elem);

    out.Key("name");
    out.SimpleValue(ai.mName);

    out.Key("tickspersecond");
    out.SimpleValue(ai.mTicksPerSecond);

    out.Key("duration");
    out.SimpleValue(ai.mDuration);

    out.Key("channels");
    out.StartArray();
    for (unsigned int n = 0; n < ai.mNumChannels; ++n) {
        Write(out, *ai.mChannels[n]);
    }
    out.EndArray();
    out.EndObj();
}

void Write(JSONWriter& out, const aiCamera& ai, bool is_elem = true) {
    out.StartObj(is_elem);

    out.Key("name");
    out.SimpleValue(ai.mName);

    out.Key("aspect");
    out.SimpleValue(ai.mAspect);

    out.Key("clipplanefar");
    out.SimpleValue(ai.mClipPlaneFar);

    out.Key("clipplanenear");
    out.SimpleValue(ai.mClipPlaneNear);

    out.Key("horizontalfov");
    out.SimpleValue(ai.mHorizontalFOV);

    out.Key("up");
    Write(out, ai.mUp, false);

    out.Key("lookat");
    Write(out, ai.mLookAt, false);

    out.EndObj();
}

void WriteFormatInfo(JSONWriter& out) {
    out.StartObj();
    out.Key("format");
    out.SimpleValue("\"assimp2json\"");
    out.Key("version");
    out.SimpleValue(CURRENT_FORMAT_VERSION);
    out.EndObj();
}

void Write(JSONWriter& out, const aiScene& ai) {
    out.StartObj();

    out.Key("__metadata__");
    WriteFormatInfo(out);

    out.Key("rootnode");
    Write(out, *ai.mRootNode, false);

    out.Key("flags");
    out.SimpleValue(ai.mFlags);

    if (ai.HasMeshes()) {
        out.Key("meshes");
        out.StartArray();
        for (unsigned int n = 0; n < ai.mNumMeshes; ++n) {
            Write(out, *ai.mMeshes[n]);
        }
        out.EndArray();
    }

    if (ai.HasMaterials()) {
        out.Key("materials");
        out.StartArray();
        for (unsigned int n = 0; n < ai.mNumMaterials; ++n) {
            Write(out, *ai.mMaterials[n]);
        }
        out.EndArray();
    }

    if (ai.HasAnimations()) {
        out.Key("animations");
        out.StartArray();
        for (unsigned int n = 0; n < ai.mNumAnimations; ++n) {
            Write(out, *ai.mAnimations[n]);
        }
        out.EndArray();
    }

    if (ai.HasLights()) {
        out.Key("lights");
        out.StartArray();
        for (unsigned int n = 0; n < ai.mNumLights; ++n) {
            Write(out, *ai.mLights[n]);
        }
        out.EndArray();
    }

    if (ai.HasCameras()) {
        out.Key("cameras");
        out.StartArray();
        for (unsigned int n = 0; n < ai.mNumCameras; ++n) {
            Write(out, *ai.mCameras[n]);
        }
        out.EndArray();
    }

    if (ai.HasTextures()) {
        out.Key("textures");
        out.StartArray();
        for (unsigned int n = 0; n < ai.mNumTextures; ++n) {
            Write(out, *ai.mTextures[n]);
        }
        out.EndArray();
    }
    out.EndObj();
}


void ExportAssimp2Json(const char* file, Assimp::IOSystem* io, const aiScene* scene, const Assimp::ExportProperties*) {
    std::unique_ptr<Assimp::IOStream> str(io->Open(file, "wt"));
    if (!str) {
        //throw Assimp::DeadlyExportError("could not open output file");
    }

    // get a copy of the scene so we can modify it
    aiScene* scenecopy_tmp;
    aiCopyScene(scene, &scenecopy_tmp);

    try {
        // split meshes so they fit into a 16 bit index buffer
        MeshSplitter splitter;
        splitter.SetLimit(1 << 16);
        splitter.Execute(scenecopy_tmp);

        // XXX Flag_WriteSpecialFloats is turned on by default, right now we don't have a configuration interface for exporters
        JSONWriter s(*str, JSONWriter::Flag_WriteSpecialFloats);
        Write(s, *scenecopy_tmp);

    }
    catch (...) {
        aiFreeScene(scenecopy_tmp);
        throw;
    }
    aiFreeScene(scenecopy_tmp);
}

}

#endif // ASSIMP_BUILD_NO_ASSJSON_EXPORTER
#endif // ASSIMP_BUILD_NO_EXPORT
