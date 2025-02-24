#pragma once
#include "DynamicEgl.h"
#include "pvr_openlib.h"

#ifdef GL_PROTOTYPES
#undef GL_PROTOTYPES
#endif
#define GL_NO_PROTOTYPES
#define EGL_NO_PROTOTYPES
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif
#include <utility>
#include <stdint.h>
#include <string>

// DETERMINE FEATURE LEVEL
#if defined(DYNAMICGLES_GLES31)
#define DYNAMICGLES_ENABLE_GLES31
#define DYNAMICGLES_ENABLE_GLES3
#define DYNAMICGLES_ENABLE_GLES2
#elif defined(DYNAMICGLES_GLES30)
#define DYNAMICGLES_ENABLE_GLES30
#define DYNAMICGLES_ENABLE_GLES2
#elif defined(DYNAMICGLES_GLES20)
#define DYNAMICGLES_ENABLE_GLES2
#else
#if !defined(DYNAMICGLES_GLES32)
#define DYNAMICGLES_GLES32
#endif
#define DYNAMICGLES_ENABLE_GLES32
#define DYNAMICGLES_ENABLE_GLES31
#define DYNAMICGLES_ENABLE_GLES3
#define DYNAMICGLES_ENABLE_GLES2
#endif

// DETERMINE IF FUNCTIONS SHOULD BE PREPENDED WITH gl OR PUT IN gl:: NAMESPACE (DEFAULT IS NAMESPACE)
#ifndef DYNAMICGLES_NO_NAMESPACE
#define DYNAMICGLES_FUNCTION(name) GL_APIENTRY name
#else
#if TARGET_OS_IPHONE
#define DYNAMICGLES_FUNCTION(name) GL_APIENTRY name
#else
#define DYNAMICGLES_FUNCTION(name) GL_APIENTRY gl##name
#endif
#endif

// DYNAMICGLES_FUNCTION THE PLATFORM SPECIFIC LIBRARY NAME
namespace gl {
namespace internals {
#ifdef _WIN32
static const char* libName = "libGLESv2.dll";
#elif defined(__APPLE__)
static const char* libName = "libGLESv2.dylib";
#else
static const char* libName = "libGLESv2.so";
#endif
} // namespace internals
} // namespace gl

// LOAD HEADER FILES DEPENDING ON FEATURE LEVEL
#if !TARGET_OS_IPHONE
#include "GLES3/gl32.h"
#include "GLES2/gl2ext.h"
#else
#include <openGLES/ES3/gl.h>
#include <openGLES/ES3/glext.h>
#include <openGLES/ES2/glext.h>
#endif

namespace gl {
namespace internals {
namespace Gl31FuncName {
// KEEP A LIST USED TO REFERENCE THE FUNCTIONS
enum OpenGLES31FunctionName
{
	//// OPENGL ES 31 ////

	DispatchCompute,
	DispatchComputeIndirect,
	DrawArraysIndirect,
	DrawElementsIndirect,
	FramebufferParameteri,
	GetFramebufferParameteriv,
	GetProgramInterfaceiv,
	GetProgramResourceIndex,
	GetProgramResourceName,
	GetProgramResourceiv,
	GetProgramResourceLocation,
	UseProgramStages,
	ActiveShaderProgram,
	CreateShaderProgramv,
	BindProgramPipeline,
	DeleteProgramPipelines,
	GenProgramPipelines,
	IsProgramPipeline,
	GetProgramPipelineiv,
	ProgramUniform1i,
	ProgramUniform2i,
	ProgramUniform3i,
	ProgramUniform4i,
	ProgramUniform1ui,
	ProgramUniform2ui,
	ProgramUniform3ui,
	ProgramUniform4ui,
	ProgramUniform1f,
	ProgramUniform2f,
	ProgramUniform3f,
	ProgramUniform4f,
	ProgramUniform1iv,
	ProgramUniform2iv,
	ProgramUniform3iv,
	ProgramUniform4iv,
	ProgramUniform1uiv,
	ProgramUniform2uiv,
	ProgramUniform3uiv,
	ProgramUniform4uiv,
	ProgramUniform1fv,
	ProgramUniform2fv,
	ProgramUniform3fv,
	ProgramUniform4fv,
	ProgramUniformMatrix2fv,
	ProgramUniformMatrix3fv,
	ProgramUniformMatrix4fv,
	ProgramUniformMatrix2x3fv,
	ProgramUniformMatrix3x2fv,
	ProgramUniformMatrix2x4fv,
	ProgramUniformMatrix4x2fv,
	ProgramUniformMatrix3x4fv,
	ProgramUniformMatrix4x3fv,
	ValidateProgramPipeline,
	GetProgramPipelineInfoLog,
	BindImageTexture,
	GetBooleani_v,
	MemoryBarrier,
	MemoryBarrierByRegion,
	TexStorage2DMultisample,
	GetMultisamplefv,
	SampleMaski,
	GetTexLevelParameteriv,
	GetTexLevelParameterfv,
	BindVertexBuffer,
	VertexAttribFormat,
	VertexAttribIFormat,
	VertexAttribBinding,
	VertexBindingDivisor,
	NUMBER_OF_OPENGLES3_FUNCTIONS
};
} // namespace Gl31FuncName

// Pre-loads the OpenGL ES 3.1 function pointers the first time any OpenGL ES 3.1 function call is made
inline void* getEs31Function(gl::internals::Gl31FuncName::OpenGLES31FunctionName funcname)
{
	static void* FunctionTable[Gl31FuncName::NUMBER_OF_OPENGLES3_FUNCTIONS];

#if !TARGET_OS_IPHONE
	// Retrieve the OpenGL ES 3.1 functions pointers once
	if (!FunctionTable[0])
	{
		pvr::lib::LIBTYPE lib = pvr::lib::openlib(libName);
		if (!lib) { Log_Error("OpenGL ES Bindings: Failed to open library %s\n", libName); }
		else
		{
			Log_Info("OpenGL ES Bindings: Successfully loaded library %s for OpenGL ES 3.1\n", libName);
		}

		FunctionTable[Gl31FuncName::DispatchCompute] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDispatchCompute");
		FunctionTable[Gl31FuncName::DispatchComputeIndirect] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDispatchComputeIndirect");
		FunctionTable[Gl31FuncName::DrawArraysIndirect] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDrawArraysIndirect");
		FunctionTable[Gl31FuncName::DrawElementsIndirect] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDrawElementsIndirect");
		FunctionTable[Gl31FuncName::FramebufferParameteri] = pvr::lib::getLibFunctionChecked<void*>(lib, "glFramebufferParameteri");
		FunctionTable[Gl31FuncName::GetFramebufferParameteriv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetFramebufferParameteriv");
		FunctionTable[Gl31FuncName::GetProgramInterfaceiv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetProgramInterfaceiv");
		FunctionTable[Gl31FuncName::GetProgramResourceIndex] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetProgramResourceIndex");
		FunctionTable[Gl31FuncName::GetProgramResourceName] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetProgramResourceName");
		FunctionTable[Gl31FuncName::GetProgramResourceiv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetProgramResourceiv");
		FunctionTable[Gl31FuncName::GetProgramResourceLocation] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetProgramResourceLocation");
		FunctionTable[Gl31FuncName::UseProgramStages] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUseProgramStages");
		FunctionTable[Gl31FuncName::ActiveShaderProgram] = pvr::lib::getLibFunctionChecked<void*>(lib, "glActiveShaderProgram");
		FunctionTable[Gl31FuncName::CreateShaderProgramv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glCreateShaderProgramv");
		FunctionTable[Gl31FuncName::BindProgramPipeline] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBindProgramPipeline");
		FunctionTable[Gl31FuncName::DeleteProgramPipelines] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDeleteProgramPipelines");
		FunctionTable[Gl31FuncName::GenProgramPipelines] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGenProgramPipelines");
		FunctionTable[Gl31FuncName::IsProgramPipeline] = pvr::lib::getLibFunctionChecked<void*>(lib, "glIsProgramPipeline");
		FunctionTable[Gl31FuncName::GetProgramPipelineiv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetProgramPipelineiv");
		FunctionTable[Gl31FuncName::ProgramUniform1i] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniform1i");
		FunctionTable[Gl31FuncName::ProgramUniform2i] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniform2i");
		FunctionTable[Gl31FuncName::ProgramUniform3i] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniform3i");
		FunctionTable[Gl31FuncName::ProgramUniform4i] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniform4i");
		FunctionTable[Gl31FuncName::ProgramUniform1ui] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniform1ui");
		FunctionTable[Gl31FuncName::ProgramUniform2ui] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniform2ui");
		FunctionTable[Gl31FuncName::ProgramUniform3ui] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniform3ui");
		FunctionTable[Gl31FuncName::ProgramUniform4ui] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniform4ui");
		FunctionTable[Gl31FuncName::ProgramUniform1f] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniform1f");
		FunctionTable[Gl31FuncName::ProgramUniform2f] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniform2f");
		FunctionTable[Gl31FuncName::ProgramUniform3f] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniform3f");
		FunctionTable[Gl31FuncName::ProgramUniform4f] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniform4f");
		FunctionTable[Gl31FuncName::ProgramUniform1iv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniform1iv");
		FunctionTable[Gl31FuncName::ProgramUniform2iv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniform2iv");
		FunctionTable[Gl31FuncName::ProgramUniform3iv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniform3iv");
		FunctionTable[Gl31FuncName::ProgramUniform4iv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniform4iv");
		FunctionTable[Gl31FuncName::ProgramUniform1uiv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniform1uiv");
		FunctionTable[Gl31FuncName::ProgramUniform2uiv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniform2uiv");
		FunctionTable[Gl31FuncName::ProgramUniform3uiv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniform3uiv");
		FunctionTable[Gl31FuncName::ProgramUniform4uiv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniform4uiv");
		FunctionTable[Gl31FuncName::ProgramUniform1fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniform1fv");
		FunctionTable[Gl31FuncName::ProgramUniform2fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniform2fv");
		FunctionTable[Gl31FuncName::ProgramUniform3fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniform3fv");
		FunctionTable[Gl31FuncName::ProgramUniform4fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniform4fv");
		FunctionTable[Gl31FuncName::ProgramUniformMatrix2fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniformMatrix2fv");
		FunctionTable[Gl31FuncName::ProgramUniformMatrix3fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniformMatrix3fv");
		FunctionTable[Gl31FuncName::ProgramUniformMatrix4fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniformMatrix4fv");
		FunctionTable[Gl31FuncName::ProgramUniformMatrix2x3fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniformMatrix2x3fv");
		FunctionTable[Gl31FuncName::ProgramUniformMatrix3x2fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniformMatrix3x2fv");
		FunctionTable[Gl31FuncName::ProgramUniformMatrix2x4fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniformMatrix2x4fv");
		FunctionTable[Gl31FuncName::ProgramUniformMatrix4x2fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniformMatrix4x2fv");
		FunctionTable[Gl31FuncName::ProgramUniformMatrix3x4fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniformMatrix3x4fv");
		FunctionTable[Gl31FuncName::ProgramUniformMatrix4x3fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramUniformMatrix4x3fv");
		FunctionTable[Gl31FuncName::ValidateProgramPipeline] = pvr::lib::getLibFunctionChecked<void*>(lib, "glValidateProgramPipeline");
		FunctionTable[Gl31FuncName::GetProgramPipelineInfoLog] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetProgramPipelineInfoLog");
		FunctionTable[Gl31FuncName::BindImageTexture] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBindImageTexture");
		FunctionTable[Gl31FuncName::GetBooleani_v] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetBooleani_v");
		FunctionTable[Gl31FuncName::MemoryBarrier] = pvr::lib::getLibFunctionChecked<void*>(lib, "glMemoryBarrier");
		FunctionTable[Gl31FuncName::MemoryBarrierByRegion] = pvr::lib::getLibFunctionChecked<void*>(lib, "glMemoryBarrierByRegion");
		FunctionTable[Gl31FuncName::TexStorage2DMultisample] = pvr::lib::getLibFunctionChecked<void*>(lib, "glTexStorage2DMultisample");
		FunctionTable[Gl31FuncName::GetMultisamplefv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetMultisamplefv");
		FunctionTable[Gl31FuncName::SampleMaski] = pvr::lib::getLibFunctionChecked<void*>(lib, "glSampleMaski");
		FunctionTable[Gl31FuncName::GetTexLevelParameteriv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetTexLevelParameteriv");
		FunctionTable[Gl31FuncName::GetTexLevelParameterfv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetTexLevelParameterfv");
		FunctionTable[Gl31FuncName::BindVertexBuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBindVertexBuffer");
		FunctionTable[Gl31FuncName::VertexAttribFormat] = pvr::lib::getLibFunctionChecked<void*>(lib, "glVertexAttribFormat");
		FunctionTable[Gl31FuncName::VertexAttribIFormat] = pvr::lib::getLibFunctionChecked<void*>(lib, "glVertexAttribIFormat");
		FunctionTable[Gl31FuncName::VertexAttribBinding] = pvr::lib::getLibFunctionChecked<void*>(lib, "glVertexAttribBinding");
		FunctionTable[Gl31FuncName::VertexBindingDivisor] = pvr::lib::getLibFunctionChecked<void*>(lib, "glVertexBindingDivisor");
	}
#endif
	return FunctionTable[funcname];
}
} // namespace internals
} // namespace gl

#if TARGET_OS_IPHONE
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#endif

/************ OPENGL ES API ************/
#ifndef DYNAMICGLES_NO_NAMESPACE
namespace gl {
#elif TARGET_OS_IPHONE
namespace gl {
namespace internals {
#endif
inline void DYNAMICGLES_FUNCTION(DispatchCompute)(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLDISPATCHCOMPUTEPROC _DispatchCompute = (PFNGLDISPATCHCOMPUTEPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::DispatchCompute);
	return _DispatchCompute(num_groups_x, num_groups_y, num_groups_z);
#endif
}
inline void DYNAMICGLES_FUNCTION(DispatchComputeIndirect)(GLintptr indirect)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLDISPATCHCOMPUTEINDIRECTPROC _DispatchComputeIndirect = (PFNGLDISPATCHCOMPUTEINDIRECTPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::DispatchComputeIndirect);
	return _DispatchComputeIndirect(indirect);
#endif
}
inline void DYNAMICGLES_FUNCTION(DrawArraysIndirect)(GLenum mode, const void* indirect)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLDRAWARRAYSINDIRECTPROC _DrawArraysIndirect = (PFNGLDRAWARRAYSINDIRECTPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::DrawArraysIndirect);
	return _DrawArraysIndirect(mode, indirect);
#endif
}
inline void DYNAMICGLES_FUNCTION(DrawElementsIndirect)(GLenum mode, GLenum type, const void* indirect)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLDRAWELEMENTSINDIRECTPROC _DrawElementsIndirect = (PFNGLDRAWELEMENTSINDIRECTPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::DrawElementsIndirect);
	return _DrawElementsIndirect(mode, type, indirect);
#endif
}
inline void DYNAMICGLES_FUNCTION(FramebufferParameteri)(GLenum target, GLenum pname, GLint param)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLFRAMEBUFFERPARAMETERIPROC _FramebufferParameteri = (PFNGLFRAMEBUFFERPARAMETERIPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::FramebufferParameteri);
	return _FramebufferParameteri(target, pname, param);
#endif
}
inline void DYNAMICGLES_FUNCTION(GetFramebufferParameteriv)(GLenum target, GLenum pname, GLint* params)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETFRAMEBUFFERPARAMETERIVPROC _GetFramebufferParameteriv =
		(PFNGLGETFRAMEBUFFERPARAMETERIVPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::GetFramebufferParameteriv);
	return _GetFramebufferParameteriv(target, pname, params);
#endif
}
inline void DYNAMICGLES_FUNCTION(GetProgramInterfaceiv)(GLuint program, GLenum programInterface, GLenum pname, GLint* params)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETPROGRAMINTERFACEIVPROC _GetProgramInterfaceiv = (PFNGLGETPROGRAMINTERFACEIVPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::GetProgramInterfaceiv);
	return _GetProgramInterfaceiv(program, programInterface, pname, params);
#endif
}
inline GLuint DYNAMICGLES_FUNCTION(GetProgramResourceIndex)(GLuint program, GLenum programInterface, const GLchar* name)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETPROGRAMRESOURCEINDEXPROC _GetProgramResourceIndex = (PFNGLGETPROGRAMRESOURCEINDEXPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::GetProgramResourceIndex);
	return _GetProgramResourceIndex(program, programInterface, name);
#endif
}
inline void DYNAMICGLES_FUNCTION(GetProgramResourceName)(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei* length, GLchar* name)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETPROGRAMRESOURCENAMEPROC _GetProgramResourceName = (PFNGLGETPROGRAMRESOURCENAMEPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::GetProgramResourceName);
	return _GetProgramResourceName(program, programInterface, index, bufSize, length, name);
#endif
}
inline void DYNAMICGLES_FUNCTION(GetProgramResourceiv)(
	GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum* props, GLsizei bufSize, GLsizei* length, GLint* params)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETPROGRAMRESOURCEIVPROC _GetProgramResourceiv = (PFNGLGETPROGRAMRESOURCEIVPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::GetProgramResourceiv);
	return _GetProgramResourceiv(program, programInterface, index, propCount, props, bufSize, length, params);
#endif
}
inline GLint DYNAMICGLES_FUNCTION(GetProgramResourceLocation)(GLuint program, GLenum programInterface, const GLchar* name)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETPROGRAMRESOURCELOCATIONPROC _GetProgramResourceLocation =
		(PFNGLGETPROGRAMRESOURCELOCATIONPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::GetProgramResourceLocation);
	return _GetProgramResourceLocation(program, programInterface, name);
#endif
}
inline void DYNAMICGLES_FUNCTION(UseProgramStages)(GLuint pipeline, GLbitfield stages, GLuint program)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLUSEPROGRAMSTAGESPROC _UseProgramStages = (PFNGLUSEPROGRAMSTAGESPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::UseProgramStages);
	return _UseProgramStages(pipeline, stages, program);
#endif
}
inline void DYNAMICGLES_FUNCTION(ActiveShaderProgram)(GLuint pipeline, GLuint program)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLACTIVESHADERPROGRAMPROC _ActiveShaderProgram = (PFNGLACTIVESHADERPROGRAMPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ActiveShaderProgram);
	return _ActiveShaderProgram(pipeline, program);
#endif
}
inline GLuint DYNAMICGLES_FUNCTION(CreateShaderProgramv)(GLenum type, GLsizei count, const GLchar* const* strings)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLCREATESHADERPROGRAMVPROC _CreateShaderProgramv = (PFNGLCREATESHADERPROGRAMVPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::CreateShaderProgramv);
	return _CreateShaderProgramv(type, count, strings);
#endif
}
inline void DYNAMICGLES_FUNCTION(BindProgramPipeline)(GLuint pipeline)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLBINDPROGRAMPIPELINEPROC _BindProgramPipeline = (PFNGLBINDPROGRAMPIPELINEPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::BindProgramPipeline);
	return _BindProgramPipeline(pipeline);
#endif
}
inline void DYNAMICGLES_FUNCTION(DeleteProgramPipelines)(GLsizei n, const GLuint* pipelines)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLDELETEPROGRAMPIPELINESPROC _DeleteProgramPipelines = (PFNGLDELETEPROGRAMPIPELINESPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::DeleteProgramPipelines);
	return _DeleteProgramPipelines(n, pipelines);
#endif
}
inline void DYNAMICGLES_FUNCTION(GenProgramPipelines)(GLsizei n, GLuint* pipelines)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGENPROGRAMPIPELINESPROC _GenProgramPipelines = (PFNGLGENPROGRAMPIPELINESPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::GenProgramPipelines);
	return _GenProgramPipelines(n, pipelines);
#endif
}
inline GLboolean DYNAMICGLES_FUNCTION(IsProgramPipeline)(GLuint pipeline)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLISPROGRAMPIPELINEPROC _IsProgramPipeline = (PFNGLISPROGRAMPIPELINEPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::IsProgramPipeline);
	return _IsProgramPipeline(pipeline);
#endif
}
inline void DYNAMICGLES_FUNCTION(GetProgramPipelineiv)(GLuint pipeline, GLenum pname, GLint* params)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETPROGRAMPIPELINEIVPROC _GetProgramPipelineiv = (PFNGLGETPROGRAMPIPELINEIVPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::GetProgramPipelineiv);
	return _GetProgramPipelineiv(pipeline, pname, params);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform1i)(GLuint program, GLint location, GLint v0)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM1IPROC _ProgramUniform1i = (PFNGLPROGRAMUNIFORM1IPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniform1i);
	return _ProgramUniform1i(program, location, v0);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform2i)(GLuint program, GLint location, GLint v0, GLint v1)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM2IPROC _ProgramUniform2i = (PFNGLPROGRAMUNIFORM2IPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniform2i);
	return _ProgramUniform2i(program, location, v0, v1);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform3i)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM3IPROC _ProgramUniform3i = (PFNGLPROGRAMUNIFORM3IPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniform3i);
	return _ProgramUniform3i(program, location, v0, v1, v2);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform4i)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM4IPROC _ProgramUniform4i = (PFNGLPROGRAMUNIFORM4IPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniform4i);
	return _ProgramUniform4i(program, location, v0, v1, v2, v3);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform1ui)(GLuint program, GLint location, GLuint v0)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM1UIPROC _ProgramUniform1ui = (PFNGLPROGRAMUNIFORM1UIPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniform1ui);
	return _ProgramUniform1ui(program, location, v0);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform2ui)(GLuint program, GLint location, GLuint v0, GLuint v1)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM2UIPROC _ProgramUniform2ui = (PFNGLPROGRAMUNIFORM2UIPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniform2ui);
	return _ProgramUniform2ui(program, location, v0, v1);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform3ui)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM3UIPROC _ProgramUniform3ui = (PFNGLPROGRAMUNIFORM3UIPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniform3ui);
	return _ProgramUniform3ui(program, location, v0, v1, v2);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform4ui)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM4UIPROC _ProgramUniform4ui = (PFNGLPROGRAMUNIFORM4UIPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniform4ui);
	return _ProgramUniform4ui(program, location, v0, v1, v2, v3);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform1f)(GLuint program, GLint location, GLfloat v0)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM1FPROC _ProgramUniform1f = (PFNGLPROGRAMUNIFORM1FPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniform1f);
	return _ProgramUniform1f(program, location, v0);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform2f)(GLuint program, GLint location, GLfloat v0, GLfloat v1)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM2FPROC _ProgramUniform2f = (PFNGLPROGRAMUNIFORM2FPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniform2f);
	return _ProgramUniform2f(program, location, v0, v1);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform3f)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM3FPROC _ProgramUniform3f = (PFNGLPROGRAMUNIFORM3FPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniform3f);
	return _ProgramUniform3f(program, location, v0, v1, v2);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform4f)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM4FPROC _ProgramUniform4f = (PFNGLPROGRAMUNIFORM4FPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniform4f);
	return _ProgramUniform4f(program, location, v0, v1, v2, v3);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform1iv)(GLuint program, GLint location, GLsizei count, const GLint* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM1IVPROC _ProgramUniform1iv = (PFNGLPROGRAMUNIFORM1IVPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniform1iv);
	return _ProgramUniform1iv(program, location, count, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform2iv)(GLuint program, GLint location, GLsizei count, const GLint* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM2IVPROC _ProgramUniform2iv = (PFNGLPROGRAMUNIFORM2IVPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniform2iv);
	return _ProgramUniform2iv(program, location, count, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform3iv)(GLuint program, GLint location, GLsizei count, const GLint* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM3IVPROC _ProgramUniform3iv = (PFNGLPROGRAMUNIFORM3IVPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniform3iv);
	return _ProgramUniform3iv(program, location, count, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform4iv)(GLuint program, GLint location, GLsizei count, const GLint* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM4IVPROC _ProgramUniform4iv = (PFNGLPROGRAMUNIFORM4IVPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniform4iv);
	return _ProgramUniform4iv(program, location, count, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform1uiv)(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM1UIVPROC _ProgramUniform1uiv = (PFNGLPROGRAMUNIFORM1UIVPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniform1uiv);
	return _ProgramUniform1uiv(program, location, count, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform2uiv)(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM2UIVPROC _ProgramUniform2uiv = (PFNGLPROGRAMUNIFORM2UIVPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniform2uiv);
	return _ProgramUniform2uiv(program, location, count, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform3uiv)(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM3UIVPROC _ProgramUniform3uiv = (PFNGLPROGRAMUNIFORM3UIVPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniform3uiv);
	return _ProgramUniform3uiv(program, location, count, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform4uiv)(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM4UIVPROC _ProgramUniform4uiv = (PFNGLPROGRAMUNIFORM4UIVPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniform4uiv);
	return _ProgramUniform4uiv(program, location, count, value);

#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform1fv)(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM1FVPROC _ProgramUniform1fv = (PFNGLPROGRAMUNIFORM1FVPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniform1fv);
	return _ProgramUniform1fv(program, location, count, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform2fv)(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM2FVPROC _ProgramUniform2fv = (PFNGLPROGRAMUNIFORM2FVPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniform2fv);
	return _ProgramUniform2fv(program, location, count, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform3fv)(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM3FVPROC _ProgramUniform3fv = (PFNGLPROGRAMUNIFORM3FVPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniform3fv);
	return _ProgramUniform3fv(program, location, count, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform4fv)(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM4FVPROC _ProgramUniform4fv = (PFNGLPROGRAMUNIFORM4FVPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniform4fv);
	return _ProgramUniform4fv(program, location, count, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniformMatrix2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORMMATRIX2FVPROC _ProgramUniformMatrix2fv = (PFNGLPROGRAMUNIFORMMATRIX2FVPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniformMatrix2fv);
	return _ProgramUniformMatrix2fv(program, location, count, transpose, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniformMatrix3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORMMATRIX3FVPROC _ProgramUniformMatrix3fv = (PFNGLPROGRAMUNIFORMMATRIX3FVPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniformMatrix3fv);
	return _ProgramUniformMatrix3fv(program, location, count, transpose, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniformMatrix4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORMMATRIX4FVPROC _ProgramUniformMatrix4fv = (PFNGLPROGRAMUNIFORMMATRIX4FVPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniformMatrix4fv);
	return _ProgramUniformMatrix4fv(program, location, count, transpose, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniformMatrix2x3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC _ProgramUniformMatrix2x3fv =
		(PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniformMatrix2x3fv);
	return _ProgramUniformMatrix2x3fv(program, location, count, transpose, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniformMatrix3x2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC _ProgramUniformMatrix3x2fv =
		(PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniformMatrix3x2fv);
	return _ProgramUniformMatrix3x2fv(program, location, count, transpose, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniformMatrix2x4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC _ProgramUniformMatrix2x4fv =
		(PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniformMatrix2x4fv);
	return _ProgramUniformMatrix2x4fv(program, location, count, transpose, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniformMatrix4x2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC _ProgramUniformMatrix4x2fv =
		(PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniformMatrix4x2fv);
	return _ProgramUniformMatrix4x2fv(program, location, count, transpose, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniformMatrix3x4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC _ProgramUniformMatrix3x4fv =
		(PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniformMatrix3x4fv);
	return _ProgramUniformMatrix3x4fv(program, location, count, transpose, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniformMatrix4x3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC _ProgramUniformMatrix4x3fv =
		(PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ProgramUniformMatrix4x3fv);
	return _ProgramUniformMatrix4x3fv(program, location, count, transpose, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ValidateProgramPipeline)(GLuint pipeline)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLVALIDATEPROGRAMPIPELINEPROC _ValidateProgramPipeline = (PFNGLVALIDATEPROGRAMPIPELINEPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::ValidateProgramPipeline);
	return _ValidateProgramPipeline(pipeline);
#endif
}
inline void DYNAMICGLES_FUNCTION(GetProgramPipelineInfoLog)(GLuint pipeline, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETPROGRAMPIPELINEINFOLOGPROC _GetProgramPipelineInfoLog =
		(PFNGLGETPROGRAMPIPELINEINFOLOGPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::GetProgramPipelineInfoLog);
	return _GetProgramPipelineInfoLog(pipeline, bufSize, length, infoLog);
#endif
}
inline void DYNAMICGLES_FUNCTION(BindImageTexture)(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLBINDIMAGETEXTUREPROC _BindImageTexture = (PFNGLBINDIMAGETEXTUREPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::BindImageTexture);
	return _BindImageTexture(unit, texture, level, layered, layer, access, format);
#endif
}
inline void DYNAMICGLES_FUNCTION(GetBooleani_v)(GLenum target, GLuint index, GLboolean* data)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETBOOLEANI_VPROC _GetBooleani_v = (PFNGLGETBOOLEANI_VPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::GetBooleani_v);
	return _GetBooleani_v(target, index, data);
#endif
}
inline void DYNAMICGLES_FUNCTION(MemoryBarrier)(GLbitfield barriers)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLMEMORYBARRIERPROC _MemoryBarrier = (PFNGLMEMORYBARRIERPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::MemoryBarrier);
	return _MemoryBarrier(barriers);
#endif
}
inline void DYNAMICGLES_FUNCTION(MemoryBarrierByRegion)(GLbitfield barriers)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLMEMORYBARRIERBYREGIONPROC _MemoryBarrierByRegion = (PFNGLMEMORYBARRIERBYREGIONPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::MemoryBarrierByRegion);
	return _MemoryBarrierByRegion(barriers);
#endif
}
inline void DYNAMICGLES_FUNCTION(TexStorage2DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLTEXSTORAGE2DMULTISAMPLEPROC _TexStorage2DMultisample = (PFNGLTEXSTORAGE2DMULTISAMPLEPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::TexStorage2DMultisample);
	return _TexStorage2DMultisample(target, samples, internalformat, width, height, fixedsamplelocations);
#endif
}
inline void DYNAMICGLES_FUNCTION(GetMultisamplefv)(GLenum pname, GLuint index, GLfloat* val)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETMULTISAMPLEFVPROC _GetMultisamplefv = (PFNGLGETMULTISAMPLEFVPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::GetMultisamplefv);
	return _GetMultisamplefv(pname, index, val);
#endif
}
inline void DYNAMICGLES_FUNCTION(SampleMaski)(GLuint maskNumber, GLbitfield mask)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLSAMPLEMASKIPROC _SampleMaski = (PFNGLSAMPLEMASKIPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::SampleMaski);
	return _SampleMaski(maskNumber, mask);
#endif
}
inline void DYNAMICGLES_FUNCTION(GetTexLevelParameteriv)(GLenum target, GLint level, GLenum pname, GLint* params)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETTEXLEVELPARAMETERIVPROC _GetTexLevelParameteriv = (PFNGLGETTEXLEVELPARAMETERIVPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::GetTexLevelParameteriv);
	return _GetTexLevelParameteriv(target, level, pname, params);
#endif
}
inline void DYNAMICGLES_FUNCTION(GetTexLevelParameterfv)(GLenum target, GLint level, GLenum pname, GLfloat* params)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETTEXLEVELPARAMETERFVPROC _GetTexLevelParameterfv = (PFNGLGETTEXLEVELPARAMETERFVPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::GetTexLevelParameterfv);
	return _GetTexLevelParameterfv(target, level, pname, params);
#endif
}
inline void DYNAMICGLES_FUNCTION(BindVertexBuffer)(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLBINDVERTEXBUFFERPROC _BindVertexBuffer = (PFNGLBINDVERTEXBUFFERPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::BindVertexBuffer);
	return _BindVertexBuffer(bindingindex, buffer, offset, stride);
#endif
}
inline void DYNAMICGLES_FUNCTION(VertexAttribFormat)(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLVERTEXATTRIBFORMATPROC _VertexAttribFormat = (PFNGLVERTEXATTRIBFORMATPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::VertexAttribFormat);
	return _VertexAttribFormat(attribindex, size, type, normalized, relativeoffset);
#endif
}
inline void DYNAMICGLES_FUNCTION(VertexAttribIFormat)(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLVERTEXATTRIBIFORMATPROC _VertexAttribIFormat = (PFNGLVERTEXATTRIBIFORMATPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::VertexAttribIFormat);
	return _VertexAttribIFormat(attribindex, size, type, relativeoffset);
#endif
}
inline void DYNAMICGLES_FUNCTION(VertexAttribBinding)(GLuint attribindex, GLuint bindingindex)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLVERTEXATTRIBBINDINGPROC _VertexAttribBinding = (PFNGLVERTEXATTRIBBINDINGPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::VertexAttribBinding);
	return _VertexAttribBinding(attribindex, bindingindex);
#endif
}
inline void DYNAMICGLES_FUNCTION(VertexBindingDivisor)(GLuint bindingindex, GLuint divisor)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLVERTEXBINDINGDIVISORPROC _VertexBindingDivisor = (PFNGLVERTEXBINDINGDIVISORPROC)gl::internals::getEs31Function(gl::internals::Gl31FuncName::VertexBindingDivisor);
	return _VertexBindingDivisor(bindingindex, divisor);
#endif
}
#ifndef DYNAMICGLES_NO_NAMESPACE
}
#elif TARGET_OS_IPHONE
}
}
#endif

namespace gl {
namespace internals {
namespace Gl3FuncName {
enum OpenGLES3FunctionName
{
	//// OPENGL ES 3 ////

	ReadBuffer,
	DrawRangeElements,
	TexImage3D,
	TexSubImage3D,
	CopyTexSubImage3D,
	CompressedTexImage3D,
	CompressedTexSubImage3D,
	GenQueries,
	DeleteQueries,
	IsQuery,
	BeginQuery,
	EndQuery,
	GetQueryiv,
	GetQueryObjectuiv,
	UnmapBuffer,
	GetBufferPointerv,
	DrawBuffers,
	UniformMatrix2x3fv,
	UniformMatrix3x2fv,
	UniformMatrix2x4fv,
	UniformMatrix4x2fv,
	UniformMatrix3x4fv,
	UniformMatrix4x3fv,
	BlitFramebuffer,
	RenderbufferStorageMultisample,
	FramebufferTextureLayer,
	MapBufferRange,
	FlushMappedBufferRange,
	BindVertexArray,
	DeleteVertexArrays,
	GenVertexArrays,
	IsVertexArray,
	GetIntegeri_v,
	BeginTransformFeedback,
	EndTransformFeedback,
	BindBufferRange,
	BindBufferBase,
	TransformFeedbackVaryings,
	GetTransformFeedbackVarying,
	VertexAttribIPointer,
	GetVertexAttribIiv,
	GetVertexAttribIuiv,
	VertexAttribI4i,
	VertexAttribI4ui,
	VertexAttribI4iv,
	VertexAttribI4uiv,
	GetUniformuiv,
	GetFragDataLocation,
	Uniform1ui,
	Uniform2ui,
	Uniform3ui,
	Uniform4ui,
	Uniform1uiv,
	Uniform2uiv,
	Uniform3uiv,
	Uniform4uiv,
	ClearBufferiv,
	ClearBufferuiv,
	ClearBufferfv,
	ClearBufferfi,
	GetStringi,
	CopyBufferSubData,
	GetUniformIndices,
	GetActiveUniformsiv,
	GetUniformBlockIndex,
	GetActiveUniformBlockiv,
	GetActiveUniformBlockName,
	UniformBlockBinding,
	DrawArraysInstanced,
	DrawElementsInstanced,
	FenceSync,
	IsSync,
	DeleteSync,
	ClientWaitSync,
	WaitSync,
	GetInteger64v,
	GetSynciv,
	GetInteger64i_v,
	GetBufferParameteri64v,
	GenSamplers,
	DeleteSamplers,
	IsSampler,
	BindSampler,
	SamplerParameteri,
	SamplerParameteriv,
	SamplerParameterf,
	SamplerParameterfv,
	GetSamplerParameteriv,
	GetSamplerParameterfv,
	VertexAttribDivisor,
	BindTransformFeedback,
	DeleteTransformFeedbacks,
	GenTransformFeedbacks,
	IsTransformFeedback,
	PauseTransformFeedback,
	ResumeTransformFeedback,
	GetProgramBinary,
	ProgramBinary,
	ProgramParameteri,
	InvalidateFramebuffer,
	InvalidateSubFramebuffer,
	TexStorage2D,
	TexStorage3D,
	GetInternalformativ,
	NUMBER_OF_OPENGLES3_FUNCTIONS
};
}

// Pre-loads the OpenGL ES 3.0 function pointers the first time any OpenGL ES 3.0 function call is made
inline void* getEs3Function(gl::internals::Gl3FuncName::OpenGLES3FunctionName funcname)
{
	static void* FunctionTable[Gl3FuncName::NUMBER_OF_OPENGLES3_FUNCTIONS];

	// Retrieve the OpenGL ES 3.0 functions pointers once
	if (!FunctionTable[0])
	{
#if !TARGET_OS_IPHONE
		pvr::lib::LIBTYPE lib = pvr::lib::openlib(gl::internals::libName);
		if (!lib) { Log_Error("OpenGL ES Bindings: Failed to open library %s\n", gl::internals::libName); }
		else
		{
			Log_Info("OpenGL ES Bindings: Successfully loaded library %s for OpenGL ES 3.0\n", gl::internals::libName);
		}
		FunctionTable[Gl3FuncName::ReadBuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "glReadBuffer");
		FunctionTable[Gl3FuncName::DrawRangeElements] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDrawRangeElements");
		FunctionTable[Gl3FuncName::TexImage3D] = pvr::lib::getLibFunctionChecked<void*>(lib, "glTexImage3D");
		FunctionTable[Gl3FuncName::TexSubImage3D] = pvr::lib::getLibFunctionChecked<void*>(lib, "glTexSubImage3D");
		FunctionTable[Gl3FuncName::CopyTexSubImage3D] = pvr::lib::getLibFunctionChecked<void*>(lib, "glCopyTexSubImage3D");
		FunctionTable[Gl3FuncName::CompressedTexImage3D] = pvr::lib::getLibFunctionChecked<void*>(lib, "glCompressedTexImage3D");
		FunctionTable[Gl3FuncName::CompressedTexSubImage3D] = pvr::lib::getLibFunctionChecked<void*>(lib, "glCompressedTexSubImage3D");
		FunctionTable[Gl3FuncName::GenQueries] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGenQueries");
		FunctionTable[Gl3FuncName::DeleteQueries] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDeleteQueries");
		FunctionTable[Gl3FuncName::IsQuery] = pvr::lib::getLibFunctionChecked<void*>(lib, "glIsQuery");
		FunctionTable[Gl3FuncName::BeginQuery] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBeginQuery");
		FunctionTable[Gl3FuncName::EndQuery] = pvr::lib::getLibFunctionChecked<void*>(lib, "glEndQuery");
		FunctionTable[Gl3FuncName::GetQueryiv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetQueryiv");
		FunctionTable[Gl3FuncName::GetQueryObjectuiv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetQueryObjectuiv");
		FunctionTable[Gl3FuncName::UnmapBuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUnmapBuffer");
		FunctionTable[Gl3FuncName::GetBufferPointerv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetBufferPointerv");
		FunctionTable[Gl3FuncName::DrawBuffers] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDrawBuffers");
		FunctionTable[Gl3FuncName::UniformMatrix2x3fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniformMatrix2x3fv");
		FunctionTable[Gl3FuncName::UniformMatrix3x2fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniformMatrix3x2fv");
		FunctionTable[Gl3FuncName::UniformMatrix2x4fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniformMatrix2x4fv");
		FunctionTable[Gl3FuncName::UniformMatrix4x2fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniformMatrix4x2fv");
		FunctionTable[Gl3FuncName::UniformMatrix3x4fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniformMatrix3x4fv");
		FunctionTable[Gl3FuncName::UniformMatrix4x3fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniformMatrix4x3fv");
		FunctionTable[Gl3FuncName::BlitFramebuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBlitFramebuffer");
		FunctionTable[Gl3FuncName::RenderbufferStorageMultisample] = pvr::lib::getLibFunctionChecked<void*>(lib, "glRenderbufferStorageMultisample");
		FunctionTable[Gl3FuncName::FramebufferTextureLayer] = pvr::lib::getLibFunctionChecked<void*>(lib, "glFramebufferTextureLayer");
		FunctionTable[Gl3FuncName::MapBufferRange] = pvr::lib::getLibFunctionChecked<void*>(lib, "glMapBufferRange");
		FunctionTable[Gl3FuncName::FlushMappedBufferRange] = pvr::lib::getLibFunctionChecked<void*>(lib, "glFlushMappedBufferRange");
		FunctionTable[Gl3FuncName::BindVertexArray] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBindVertexArray");
		FunctionTable[Gl3FuncName::DeleteVertexArrays] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDeleteVertexArrays");
		FunctionTable[Gl3FuncName::GenVertexArrays] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGenVertexArrays");
		FunctionTable[Gl3FuncName::IsVertexArray] = pvr::lib::getLibFunctionChecked<void*>(lib, "glIsVertexArray");
		FunctionTable[Gl3FuncName::GetIntegeri_v] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetIntegeri_v");
		FunctionTable[Gl3FuncName::BeginTransformFeedback] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBeginTransformFeedback");
		FunctionTable[Gl3FuncName::EndTransformFeedback] = pvr::lib::getLibFunctionChecked<void*>(lib, "glEndTransformFeedback");
		FunctionTable[Gl3FuncName::BindBufferRange] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBindBufferRange");
		FunctionTable[Gl3FuncName::BindBufferBase] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBindBufferBase");
		FunctionTable[Gl3FuncName::TransformFeedbackVaryings] = pvr::lib::getLibFunctionChecked<void*>(lib, "glTransformFeedbackVaryings");
		FunctionTable[Gl3FuncName::GetTransformFeedbackVarying] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetTransformFeedbackVarying");
		FunctionTable[Gl3FuncName::VertexAttribIPointer] = pvr::lib::getLibFunctionChecked<void*>(lib, "glVertexAttribIPointer");
		FunctionTable[Gl3FuncName::GetVertexAttribIiv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetVertexAttribIiv");
		FunctionTable[Gl3FuncName::GetVertexAttribIuiv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetVertexAttribIuiv");
		FunctionTable[Gl3FuncName::VertexAttribI4i] = pvr::lib::getLibFunctionChecked<void*>(lib, "glVertexAttribI4i");
		FunctionTable[Gl3FuncName::VertexAttribI4ui] = pvr::lib::getLibFunctionChecked<void*>(lib, "glVertexAttribI4ui");
		FunctionTable[Gl3FuncName::VertexAttribI4iv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glVertexAttribI4iv");
		FunctionTable[Gl3FuncName::VertexAttribI4uiv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glVertexAttribI4uiv");
		FunctionTable[Gl3FuncName::GetUniformuiv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetUniformuiv");
		FunctionTable[Gl3FuncName::GetFragDataLocation] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetFragDataLocation");
		FunctionTable[Gl3FuncName::Uniform1ui] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform1ui");
		FunctionTable[Gl3FuncName::Uniform2ui] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform2ui");
		FunctionTable[Gl3FuncName::Uniform3ui] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform3ui");
		FunctionTable[Gl3FuncName::Uniform4ui] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform4ui");
		FunctionTable[Gl3FuncName::Uniform1uiv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform1uiv");
		FunctionTable[Gl3FuncName::Uniform2uiv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform2uiv");
		FunctionTable[Gl3FuncName::Uniform3uiv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform3uiv");
		FunctionTable[Gl3FuncName::Uniform4uiv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform4uiv");
		FunctionTable[Gl3FuncName::ClearBufferiv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glClearBufferiv");
		FunctionTable[Gl3FuncName::ClearBufferuiv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glClearBufferuiv");
		FunctionTable[Gl3FuncName::ClearBufferfv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glClearBufferfv");
		FunctionTable[Gl3FuncName::ClearBufferfi] = pvr::lib::getLibFunctionChecked<void*>(lib, "glClearBufferfi");
		FunctionTable[Gl3FuncName::GetStringi] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetStringi");
		FunctionTable[Gl3FuncName::CopyBufferSubData] = pvr::lib::getLibFunctionChecked<void*>(lib, "glCopyBufferSubData");
		FunctionTable[Gl3FuncName::GetUniformIndices] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetUniformIndices");
		FunctionTable[Gl3FuncName::GetActiveUniformsiv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetActiveUniformsiv");
		FunctionTable[Gl3FuncName::GetUniformBlockIndex] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetUniformBlockIndex");
		FunctionTable[Gl3FuncName::GetActiveUniformBlockiv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetActiveUniformBlockiv");
		FunctionTable[Gl3FuncName::GetActiveUniformBlockName] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetActiveUniformBlockName");
		FunctionTable[Gl3FuncName::UniformBlockBinding] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniformBlockBinding");
		FunctionTable[Gl3FuncName::DrawArraysInstanced] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDrawArraysInstanced");
		FunctionTable[Gl3FuncName::DrawElementsInstanced] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDrawElementsInstanced");
		FunctionTable[Gl3FuncName::FenceSync] = pvr::lib::getLibFunctionChecked<void*>(lib, "glFenceSync");
		FunctionTable[Gl3FuncName::IsSync] = pvr::lib::getLibFunctionChecked<void*>(lib, "glIsSync");
		FunctionTable[Gl3FuncName::DeleteSync] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDeleteSync");
		FunctionTable[Gl3FuncName::ClientWaitSync] = pvr::lib::getLibFunctionChecked<void*>(lib, "glClientWaitSync");
		FunctionTable[Gl3FuncName::WaitSync] = pvr::lib::getLibFunctionChecked<void*>(lib, "glWaitSync");
		FunctionTable[Gl3FuncName::GetInteger64v] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetInteger64v");
		FunctionTable[Gl3FuncName::GetSynciv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetSynciv");
		FunctionTable[Gl3FuncName::GetInteger64i_v] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetInteger64i_v");
		FunctionTable[Gl3FuncName::GetBufferParameteri64v] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetBufferParameteri64v");
		FunctionTable[Gl3FuncName::GenSamplers] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGenSamplers");
		FunctionTable[Gl3FuncName::DeleteSamplers] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDeleteSamplers");
		FunctionTable[Gl3FuncName::IsSampler] = pvr::lib::getLibFunctionChecked<void*>(lib, "glIsSampler");
		FunctionTable[Gl3FuncName::BindSampler] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBindSampler");
		FunctionTable[Gl3FuncName::SamplerParameteri] = pvr::lib::getLibFunctionChecked<void*>(lib, "glSamplerParameteri");
		FunctionTable[Gl3FuncName::SamplerParameteriv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glSamplerParameteriv");
		FunctionTable[Gl3FuncName::SamplerParameterf] = pvr::lib::getLibFunctionChecked<void*>(lib, "glSamplerParameterf");
		FunctionTable[Gl3FuncName::SamplerParameterfv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glSamplerParameterfv");
		FunctionTable[Gl3FuncName::GetSamplerParameteriv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetSamplerParameteriv");
		FunctionTable[Gl3FuncName::GetSamplerParameterfv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetSamplerParameterfv");
		FunctionTable[Gl3FuncName::VertexAttribDivisor] = pvr::lib::getLibFunctionChecked<void*>(lib, "glVertexAttribDivisor");
		FunctionTable[Gl3FuncName::BindTransformFeedback] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBindTransformFeedback");
		FunctionTable[Gl3FuncName::DeleteTransformFeedbacks] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDeleteTransformFeedbacks");
		FunctionTable[Gl3FuncName::GenTransformFeedbacks] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGenTransformFeedbacks");
		FunctionTable[Gl3FuncName::IsTransformFeedback] = pvr::lib::getLibFunctionChecked<void*>(lib, "glIsTransformFeedback");
		FunctionTable[Gl3FuncName::PauseTransformFeedback] = pvr::lib::getLibFunctionChecked<void*>(lib, "glPauseTransformFeedback");
		FunctionTable[Gl3FuncName::ResumeTransformFeedback] = pvr::lib::getLibFunctionChecked<void*>(lib, "glResumeTransformFeedback");
		FunctionTable[Gl3FuncName::GetProgramBinary] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetProgramBinary");
		FunctionTable[Gl3FuncName::ProgramBinary] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramBinary");
		FunctionTable[Gl3FuncName::ProgramParameteri] = pvr::lib::getLibFunctionChecked<void*>(lib, "glProgramParameteri");
		FunctionTable[Gl3FuncName::InvalidateFramebuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "glInvalidateFramebuffer");
		FunctionTable[Gl3FuncName::InvalidateSubFramebuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "glInvalidateSubFramebuffer");
		FunctionTable[Gl3FuncName::TexStorage2D] = pvr::lib::getLibFunctionChecked<void*>(lib, "glTexStorage2D");
		FunctionTable[Gl3FuncName::TexStorage3D] = pvr::lib::getLibFunctionChecked<void*>(lib, "glTexStorage3D");
		FunctionTable[Gl3FuncName::GetInternalformativ] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetInternalformativ");
#else
		FunctionTable[Gl3FuncName::ReadBuffer] = (void*)&glReadBuffer;
		FunctionTable[Gl3FuncName::DrawRangeElements] = (void*)&glDrawRangeElements;
		FunctionTable[Gl3FuncName::TexImage3D] = (void*)&glTexImage3D;
		FunctionTable[Gl3FuncName::TexSubImage3D] = (void*)&glTexSubImage3D;
		FunctionTable[Gl3FuncName::CopyTexSubImage3D] = (void*)&glCopyTexSubImage3D;
		FunctionTable[Gl3FuncName::CompressedTexImage3D] = (void*)&glCompressedTexImage3D;
		FunctionTable[Gl3FuncName::CompressedTexSubImage3D] = (void*)&glCompressedTexSubImage3D;
		FunctionTable[Gl3FuncName::GenQueries] = (void*)&glGenQueries;
		FunctionTable[Gl3FuncName::DeleteQueries] = (void*)&glDeleteQueries;
		FunctionTable[Gl3FuncName::IsQuery] = (void*)&glIsQuery;
		FunctionTable[Gl3FuncName::BeginQuery] = (void*)&glBeginQuery;
		FunctionTable[Gl3FuncName::EndQuery] = (void*)&glEndQuery;
		FunctionTable[Gl3FuncName::GetQueryiv] = (void*)&glGetQueryiv;
		FunctionTable[Gl3FuncName::GetQueryObjectuiv] = (void*)&glGetQueryObjectuiv;
		FunctionTable[Gl3FuncName::UnmapBuffer] = (void*)&glUnmapBuffer;
		FunctionTable[Gl3FuncName::GetBufferPointerv] = (void*)&glGetBufferPointerv;
		FunctionTable[Gl3FuncName::DrawBuffers] = (void*)&glDrawBuffers;
		FunctionTable[Gl3FuncName::UniformMatrix2x3fv] = (void*)&glUniformMatrix2x3fv;
		FunctionTable[Gl3FuncName::UniformMatrix3x2fv] = (void*)&glUniformMatrix3x2fv;
		FunctionTable[Gl3FuncName::UniformMatrix2x4fv] = (void*)&glUniformMatrix2x4fv;
		FunctionTable[Gl3FuncName::UniformMatrix4x2fv] = (void*)&glUniformMatrix4x2fv;
		FunctionTable[Gl3FuncName::UniformMatrix3x4fv] = (void*)&glUniformMatrix3x4fv;
		FunctionTable[Gl3FuncName::UniformMatrix4x3fv] = (void*)&glUniformMatrix4x3fv;
		FunctionTable[Gl3FuncName::BlitFramebuffer] = (void*)&glBlitFramebuffer;
		FunctionTable[Gl3FuncName::RenderbufferStorageMultisample] = (void*)&glRenderbufferStorageMultisample;
		FunctionTable[Gl3FuncName::FramebufferTextureLayer] = (void*)&glFramebufferTextureLayer;
		FunctionTable[Gl3FuncName::MapBufferRange] = (void*)&glMapBufferRange;
		FunctionTable[Gl3FuncName::FlushMappedBufferRange] = (void*)&glFlushMappedBufferRange;
		FunctionTable[Gl3FuncName::BindVertexArray] = (void*)&glBindVertexArray;
		FunctionTable[Gl3FuncName::DeleteVertexArrays] = (void*)&glDeleteVertexArrays;
		FunctionTable[Gl3FuncName::GenVertexArrays] = (void*)&glGenVertexArrays;
		FunctionTable[Gl3FuncName::IsVertexArray] = (void*)&glIsVertexArray;
		FunctionTable[Gl3FuncName::GetIntegeri_v] = (void*)&glGetIntegeri_v;
		FunctionTable[Gl3FuncName::BeginTransformFeedback] = (void*)&glBeginTransformFeedback;
		FunctionTable[Gl3FuncName::EndTransformFeedback] = (void*)&glEndTransformFeedback;
		FunctionTable[Gl3FuncName::BindBufferRange] = (void*)&glBindBufferRange;
		FunctionTable[Gl3FuncName::BindBufferBase] = (void*)&glBindBufferBase;
		FunctionTable[Gl3FuncName::TransformFeedbackVaryings] = (void*)&glTransformFeedbackVaryings;
		FunctionTable[Gl3FuncName::GetTransformFeedbackVarying] = (void*)&glGetTransformFeedbackVarying;
		FunctionTable[Gl3FuncName::VertexAttribIPointer] = (void*)&glVertexAttribIPointer;
		FunctionTable[Gl3FuncName::GetVertexAttribIiv] = (void*)&glGetVertexAttribIiv;
		FunctionTable[Gl3FuncName::GetVertexAttribIuiv] = (void*)&glGetVertexAttribIuiv;
		FunctionTable[Gl3FuncName::VertexAttribI4i] = (void*)&glVertexAttribI4i;
		FunctionTable[Gl3FuncName::VertexAttribI4ui] = (void*)&glVertexAttribI4ui;
		FunctionTable[Gl3FuncName::VertexAttribI4iv] = (void*)&glVertexAttribI4iv;
		FunctionTable[Gl3FuncName::VertexAttribI4uiv] = (void*)&glVertexAttribI4uiv;
		FunctionTable[Gl3FuncName::GetUniformuiv] = (void*)&glGetUniformuiv;
		FunctionTable[Gl3FuncName::GetFragDataLocation] = (void*)&glGetFragDataLocation;
		FunctionTable[Gl3FuncName::Uniform1ui] = (void*)&glUniform1ui;
		FunctionTable[Gl3FuncName::Uniform2ui] = (void*)&glUniform2ui;
		FunctionTable[Gl3FuncName::Uniform3ui] = (void*)&glUniform3ui;
		FunctionTable[Gl3FuncName::Uniform4ui] = (void*)&glUniform4ui;
		FunctionTable[Gl3FuncName::Uniform1uiv] = (void*)&glUniform1uiv;
		FunctionTable[Gl3FuncName::Uniform2uiv] = (void*)&glUniform2uiv;
		FunctionTable[Gl3FuncName::Uniform3uiv] = (void*)&glUniform3uiv;
		FunctionTable[Gl3FuncName::Uniform4uiv] = (void*)&glUniform4uiv;
		FunctionTable[Gl3FuncName::ClearBufferiv] = (void*)&glClearBufferiv;
		FunctionTable[Gl3FuncName::ClearBufferuiv] = (void*)&glClearBufferuiv;
		FunctionTable[Gl3FuncName::ClearBufferfv] = (void*)&glClearBufferfv;
		FunctionTable[Gl3FuncName::ClearBufferfi] = (void*)&glClearBufferfi;
		FunctionTable[Gl3FuncName::GetStringi] = (void*)&glGetStringi;
		FunctionTable[Gl3FuncName::CopyBufferSubData] = (void*)&glCopyBufferSubData;
		FunctionTable[Gl3FuncName::GetUniformIndices] = (void*)&glGetUniformIndices;
		FunctionTable[Gl3FuncName::GetActiveUniformsiv] = (void*)&glGetActiveUniformsiv;
		FunctionTable[Gl3FuncName::GetUniformBlockIndex] = (void*)&glGetUniformBlockIndex;
		FunctionTable[Gl3FuncName::GetActiveUniformBlockiv] = (void*)&glGetActiveUniformBlockiv;
		FunctionTable[Gl3FuncName::GetActiveUniformBlockName] = (void*)&glGetActiveUniformBlockName;
		FunctionTable[Gl3FuncName::UniformBlockBinding] = (void*)&glUniformBlockBinding;
		FunctionTable[Gl3FuncName::DrawArraysInstanced] = (void*)&glDrawArraysInstanced;
		FunctionTable[Gl3FuncName::DrawElementsInstanced] = (void*)&glDrawElementsInstanced;
		FunctionTable[Gl3FuncName::FenceSync] = (void*)&glFenceSync;
		FunctionTable[Gl3FuncName::IsSync] = (void*)&glIsSync;
		FunctionTable[Gl3FuncName::DeleteSync] = (void*)&glDeleteSync;
		FunctionTable[Gl3FuncName::ClientWaitSync] = (void*)&glClientWaitSync;
		FunctionTable[Gl3FuncName::WaitSync] = (void*)&glWaitSync;
		FunctionTable[Gl3FuncName::GetInteger64v] = (void*)&glGetInteger64v;
		FunctionTable[Gl3FuncName::GetSynciv] = (void*)&glGetSynciv;
		FunctionTable[Gl3FuncName::GetInteger64i_v] = (void*)&glGetInteger64i_v;
		FunctionTable[Gl3FuncName::GetBufferParameteri64v] = (void*)&glGetBufferParameteri64v;
		FunctionTable[Gl3FuncName::GenSamplers] = (void*)&glGenSamplers;
		FunctionTable[Gl3FuncName::DeleteSamplers] = (void*)&glDeleteSamplers;
		FunctionTable[Gl3FuncName::IsSampler] = (void*)&glIsSampler;
		FunctionTable[Gl3FuncName::BindSampler] = (void*)&glBindSampler;
		FunctionTable[Gl3FuncName::SamplerParameteri] = (void*)&glSamplerParameteri;
		FunctionTable[Gl3FuncName::SamplerParameteriv] = (void*)&glSamplerParameteriv;
		FunctionTable[Gl3FuncName::SamplerParameterf] = (void*)&glSamplerParameterf;
		FunctionTable[Gl3FuncName::SamplerParameterfv] = (void*)&glSamplerParameterfv;
		FunctionTable[Gl3FuncName::GetSamplerParameteriv] = (void*)&glGetSamplerParameteriv;
		FunctionTable[Gl3FuncName::GetSamplerParameterfv] = (void*)&glGetSamplerParameterfv;
		FunctionTable[Gl3FuncName::VertexAttribDivisor] = (void*)&glVertexAttribDivisor;
		FunctionTable[Gl3FuncName::BindTransformFeedback] = (void*)&glBindTransformFeedback;
		FunctionTable[Gl3FuncName::DeleteTransformFeedbacks] = (void*)&glDeleteTransformFeedbacks;
		FunctionTable[Gl3FuncName::GenTransformFeedbacks] = (void*)&glGenTransformFeedbacks;
		FunctionTable[Gl3FuncName::IsTransformFeedback] = (void*)&glIsTransformFeedback;
		FunctionTable[Gl3FuncName::PauseTransformFeedback] = (void*)&glPauseTransformFeedback;
		FunctionTable[Gl3FuncName::ResumeTransformFeedback] = (void*)&glResumeTransformFeedback;
		FunctionTable[Gl3FuncName::GetProgramBinary] = (void*)&glGetProgramBinary;
		FunctionTable[Gl3FuncName::ProgramBinary] = (void*)&glProgramBinary;
		FunctionTable[Gl3FuncName::ProgramParameteri] = (void*)&glProgramParameteri;
		FunctionTable[Gl3FuncName::InvalidateFramebuffer] = (void*)&glInvalidateFramebuffer;
		FunctionTable[Gl3FuncName::InvalidateSubFramebuffer] = (void*)&glInvalidateSubFramebuffer;
		FunctionTable[Gl3FuncName::TexStorage2D] = (void*)&glTexStorage2D;
		FunctionTable[Gl3FuncName::TexStorage3D] = (void*)&glTexStorage3D;
		FunctionTable[Gl3FuncName::GetInternalformativ] = (void*)&glGetInternalformativ;
#endif
	}
	return FunctionTable[funcname];
}
} // namespace internals
} // namespace gl
#ifndef DYNAMICGLES_NO_NAMESPACE
namespace gl {
#elif TARGET_OS_IPHONE
namespace gl {
namespace internals {
#endif
inline void DYNAMICGLES_FUNCTION(ReadBuffer)(GLenum src)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glReadBuffer) PFNGLREADBUFFERPROC;
#endif
	PFNGLREADBUFFERPROC _ReadBuffer = (PFNGLREADBUFFERPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::ReadBuffer);
	return _ReadBuffer(src);
}
inline void DYNAMICGLES_FUNCTION(DrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void* indices)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glDrawRangeElements) PFNGLDRAWRANGEELEMENTSPROC;
#endif
	PFNGLDRAWRANGEELEMENTSPROC _DrawRangeElements = (PFNGLDRAWRANGEELEMENTSPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::DrawRangeElements);
	return _DrawRangeElements(mode, start, end, count, type, indices);
}
inline void DYNAMICGLES_FUNCTION(TexImage3D)(
	GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glTexImage3D) PFNGLTEXIMAGE3DPROC;
#endif
	PFNGLTEXIMAGE3DPROC _TexImage3D = (PFNGLTEXIMAGE3DPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::TexImage3D);
	return _TexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels);
}
inline void DYNAMICGLES_FUNCTION(TexSubImage3D)(
	GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glTexSubImage3D) PFNGLTEXSUBIMAGE3DPROC;
#endif
	PFNGLTEXSUBIMAGE3DPROC _TexSubImage3D = (PFNGLTEXSUBIMAGE3DPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::TexSubImage3D);
	return _TexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}
inline void DYNAMICGLES_FUNCTION(CopyTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glCopyTexSubImage3D) PFNGLCOPYTEXSUBIMAGE3DPROC;
#endif
	PFNGLCOPYTEXSUBIMAGE3DPROC _CopyTexSubImage3D = (PFNGLCOPYTEXSUBIMAGE3DPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::CopyTexSubImage3D);
	return _CopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
}
inline void DYNAMICGLES_FUNCTION(CompressedTexImage3D)(
	GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* data)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glCompressedTexImage3D) PFNGLCOMPRESSEDTEXIMAGE3DPROC;
#endif
	PFNGLCOMPRESSEDTEXIMAGE3DPROC _CompressedTexImage3D = (PFNGLCOMPRESSEDTEXIMAGE3DPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::CompressedTexImage3D);
	return _CompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data);
}
inline void DYNAMICGLES_FUNCTION(CompressedTexSubImage3D)(
	GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glCompressedTexSubImage3D) PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC;
#endif
	PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC _CompressedTexSubImage3D = (PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::CompressedTexSubImage3D);
	return _CompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
}
inline void DYNAMICGLES_FUNCTION(GenQueries)(GLsizei n, GLuint* ids)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGenQueries) PFNGLGENQUERIESPROC;
#endif
	PFNGLGENQUERIESPROC _GenQueries = (PFNGLGENQUERIESPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::GenQueries);
	return _GenQueries(n, ids);
}
inline void DYNAMICGLES_FUNCTION(DeleteQueries)(GLsizei n, const GLuint* ids)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glDeleteQueries) PFNGLDELETEQUERIESPROC;
#endif
	PFNGLDELETEQUERIESPROC _DeleteQueries = (PFNGLDELETEQUERIESPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::DeleteQueries);
	return _DeleteQueries(n, ids);
}
inline GLboolean DYNAMICGLES_FUNCTION(IsQuery)(GLuint id)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glIsQuery) PFNGLISQUERYPROC;
#endif
	PFNGLISQUERYPROC _IsQuery = (PFNGLISQUERYPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::IsQuery);
	return _IsQuery(id);
}
inline void DYNAMICGLES_FUNCTION(BeginQuery)(GLenum target, GLuint id)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glBeginQuery) PFNGLBEGINQUERYPROC;
#endif
	PFNGLBEGINQUERYPROC _BeginQuery = (PFNGLBEGINQUERYPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::BeginQuery);
	return _BeginQuery(target, id);
}
inline void DYNAMICGLES_FUNCTION(EndQuery)(GLenum target)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glEndQuery) PFNGLENDQUERYPROC;
#endif
	PFNGLENDQUERYPROC _EndQuery = (PFNGLENDQUERYPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::EndQuery);
	return _EndQuery(target);
}
inline void DYNAMICGLES_FUNCTION(GetQueryiv)(GLenum target, GLenum pname, GLint* params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetQueryiv) PFNGLGETQUERYIVPROC;
#endif
	PFNGLGETQUERYIVPROC _GetQueryiv = (PFNGLGETQUERYIVPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::GetQueryiv);
	return _GetQueryiv(target, pname, params);
}
inline void DYNAMICGLES_FUNCTION(GetQueryObjectuiv)(GLuint id, GLenum pname, GLuint* params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetQueryObjectuiv) PFNGLGETQUERYOBJECTUIVPROC;
#endif
	PFNGLGETQUERYOBJECTUIVPROC _GetQueryObjectuiv = (PFNGLGETQUERYOBJECTUIVPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::GetQueryObjectuiv);
	return _GetQueryObjectuiv(id, pname, params);
}
inline GLboolean DYNAMICGLES_FUNCTION(UnmapBuffer)(GLenum target)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUnmapBuffer) PFNGLUNMAPBUFFERPROC;
#endif
	PFNGLUNMAPBUFFERPROC _UnmapBuffer = (PFNGLUNMAPBUFFERPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::UnmapBuffer);
	return _UnmapBuffer(target);
}
inline void DYNAMICGLES_FUNCTION(GetBufferPointerv)(GLenum target, GLenum pname, void** params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetBufferPointerv) PFNGLGETBUFFERPOINTERVPROC;
#endif
	PFNGLGETBUFFERPOINTERVPROC _GetBufferPointerv = (PFNGLGETBUFFERPOINTERVPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::GetBufferPointerv);
	return _GetBufferPointerv(target, pname, params);
}
inline void DYNAMICGLES_FUNCTION(DrawBuffers)(GLsizei n, const GLenum* bufs)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glDrawBuffers) PFNGLDRAWBUFFERSPROC;
#endif
	PFNGLDRAWBUFFERSPROC _DrawBuffers = (PFNGLDRAWBUFFERSPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::DrawBuffers);
	return _DrawBuffers(n, bufs);
}
inline void DYNAMICGLES_FUNCTION(UniformMatrix2x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniformMatrix2x3fv) PFNGLUNIFORMMATRIX2X3FVPROC;
#endif
	PFNGLUNIFORMMATRIX2X3FVPROC _UniformMatrix2x3fv = (PFNGLUNIFORMMATRIX2X3FVPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::UniformMatrix2x3fv);
	return _UniformMatrix2x3fv(location, count, transpose, value);
}
inline void DYNAMICGLES_FUNCTION(UniformMatrix3x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniformMatrix3x2fv) PFNGLUNIFORMMATRIX3X2FVPROC;
#endif
	PFNGLUNIFORMMATRIX3X2FVPROC _UniformMatrix3x2fv = (PFNGLUNIFORMMATRIX3X2FVPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::UniformMatrix3x2fv);
	return _UniformMatrix3x2fv(location, count, transpose, value);
}
inline void DYNAMICGLES_FUNCTION(UniformMatrix2x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniformMatrix2x4fv) PFNGLUNIFORMMATRIX2X4FVPROC;
#endif
	PFNGLUNIFORMMATRIX2X4FVPROC _UniformMatrix2x4fv = (PFNGLUNIFORMMATRIX2X4FVPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::UniformMatrix2x4fv);
	return _UniformMatrix2x4fv(location, count, transpose, value);
}
inline void DYNAMICGLES_FUNCTION(UniformMatrix4x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniformMatrix4x2fv) PFNGLUNIFORMMATRIX4X2FVPROC;
#endif
	PFNGLUNIFORMMATRIX4X2FVPROC _UniformMatrix4x2fv = (PFNGLUNIFORMMATRIX4X2FVPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::UniformMatrix4x2fv);
	return _UniformMatrix4x2fv(location, count, transpose, value);
}
inline void DYNAMICGLES_FUNCTION(UniformMatrix3x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniformMatrix3x4fv) PFNGLUNIFORMMATRIX3X4FVPROC;
#endif
	PFNGLUNIFORMMATRIX3X4FVPROC _UniformMatrix3x4fv = (PFNGLUNIFORMMATRIX3X4FVPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::UniformMatrix3x4fv);
	return _UniformMatrix3x4fv(location, count, transpose, value);
}
inline void DYNAMICGLES_FUNCTION(UniformMatrix4x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniformMatrix4x3fv) PFNGLUNIFORMMATRIX4X3FVPROC;
#endif
	PFNGLUNIFORMMATRIX4X3FVPROC _UniformMatrix4x3fv = (PFNGLUNIFORMMATRIX4X3FVPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::UniformMatrix4x3fv);
	return _UniformMatrix4x3fv(location, count, transpose, value);
}
inline void DYNAMICGLES_FUNCTION(BlitFramebuffer)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glBlitFramebuffer) PFNGLBLITFRAMEBUFFERPROC;
#endif
	PFNGLBLITFRAMEBUFFERPROC _BlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::BlitFramebuffer);
	return _BlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}
inline void DYNAMICGLES_FUNCTION(RenderbufferStorageMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glRenderbufferStorageMultisample) PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC;
#endif
	PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC _RenderbufferStorageMultisample =
		(PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::RenderbufferStorageMultisample);
	return _RenderbufferStorageMultisample(target, samples, internalformat, width, height);
}
inline void DYNAMICGLES_FUNCTION(FramebufferTextureLayer)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glFramebufferTextureLayer) PFNGLFRAMEBUFFERTEXTURELAYERPROC;
#endif
	PFNGLFRAMEBUFFERTEXTURELAYERPROC _FramebufferTextureLayer = (PFNGLFRAMEBUFFERTEXTURELAYERPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::FramebufferTextureLayer);
	return _FramebufferTextureLayer(target, attachment, texture, level, layer);
}
inline void* DYNAMICGLES_FUNCTION(MapBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glMapBufferRange) PFNGLMAPBUFFERRANGEPROC;
#endif
	PFNGLMAPBUFFERRANGEPROC _MapBufferRange = (PFNGLMAPBUFFERRANGEPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::MapBufferRange);
	return _MapBufferRange(target, offset, length, access);
}
inline void DYNAMICGLES_FUNCTION(FlushMappedBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glFlushMappedBufferRange) PFNGLFLUSHMAPPEDBUFFERRANGEPROC;
#endif
	PFNGLFLUSHMAPPEDBUFFERRANGEPROC _FlushMappedBufferRange = (PFNGLFLUSHMAPPEDBUFFERRANGEPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::FlushMappedBufferRange);
	return _FlushMappedBufferRange(target, offset, length);
}
inline void DYNAMICGLES_FUNCTION(BindVertexArray)(GLuint array)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glBindVertexArray) PFNGLBINDVERTEXARRAYPROC;
#endif
	PFNGLBINDVERTEXARRAYPROC _BindVertexArray = (PFNGLBINDVERTEXARRAYPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::BindVertexArray);
	return _BindVertexArray(array);
}
inline void DYNAMICGLES_FUNCTION(DeleteVertexArrays)(GLsizei n, const GLuint* arrays)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glDeleteVertexArrays) PFNGLDELETEVERTEXARRAYSPROC;
#endif
	PFNGLDELETEVERTEXARRAYSPROC _DeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::DeleteVertexArrays);
	return _DeleteVertexArrays(n, arrays);
}
inline void DYNAMICGLES_FUNCTION(GenVertexArrays)(GLsizei n, GLuint* arrays)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGenVertexArrays) PFNGLGENVERTEXARRAYSPROC;
#endif
	PFNGLGENVERTEXARRAYSPROC _GenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::GenVertexArrays);
	return _GenVertexArrays(n, arrays);
}
inline GLboolean DYNAMICGLES_FUNCTION(IsVertexArray)(GLuint array)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glIsVertexArray) PFNGLISVERTEXARRAYPROC;
#endif
	PFNGLISVERTEXARRAYPROC _IsVertexArray = (PFNGLISVERTEXARRAYPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::IsVertexArray);
	return _IsVertexArray(array);
}
inline void DYNAMICGLES_FUNCTION(GetIntegeri_v)(GLenum target, GLuint index, GLint* data)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetIntegeri_v) PFNGLGETINTEGERI_VPROC;
#endif
	PFNGLGETINTEGERI_VPROC _GetIntegeri_v = (PFNGLGETINTEGERI_VPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::GetIntegeri_v);
	return _GetIntegeri_v(target, index, data);
}
inline void DYNAMICGLES_FUNCTION(BeginTransformFeedback)(GLenum primitiveMode)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glBeginTransformFeedback) PFNGLBEGINTRANSFORMFEEDBACKPROC;
#endif
	PFNGLBEGINTRANSFORMFEEDBACKPROC _BeginTransformFeedback = (PFNGLBEGINTRANSFORMFEEDBACKPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::BeginTransformFeedback);
	return _BeginTransformFeedback(primitiveMode);
}
inline void DYNAMICGLES_FUNCTION(EndTransformFeedback)(void)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glEndTransformFeedback) PFNGLENDTRANSFORMFEEDBACKPROC;
#endif
	PFNGLENDTRANSFORMFEEDBACKPROC _EndTransformFeedback = (PFNGLENDTRANSFORMFEEDBACKPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::EndTransformFeedback);
	return _EndTransformFeedback();
}
inline void DYNAMICGLES_FUNCTION(BindBufferRange)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glBindBufferRange) PFNGLBINDBUFFERRANGEPROC;
#endif
	PFNGLBINDBUFFERRANGEPROC _BindBufferRange = (PFNGLBINDBUFFERRANGEPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::BindBufferRange);
	return _BindBufferRange(target, index, buffer, offset, size);
}
inline void DYNAMICGLES_FUNCTION(BindBufferBase)(GLenum target, GLuint index, GLuint buffer)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glBindBufferBase) PFNGLBINDBUFFERBASEPROC;
#endif
	PFNGLBINDBUFFERBASEPROC _BindBufferBase = (PFNGLBINDBUFFERBASEPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::BindBufferBase);
	return _BindBufferBase(target, index, buffer);
}
inline void DYNAMICGLES_FUNCTION(TransformFeedbackVaryings)(GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glTransformFeedbackVaryings) PFNGLTRANSFORMFEEDBACKVARYINGSPROC;
#endif
	PFNGLTRANSFORMFEEDBACKVARYINGSPROC _TransformFeedbackVaryings =
		(PFNGLTRANSFORMFEEDBACKVARYINGSPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::TransformFeedbackVaryings);
	return _TransformFeedbackVaryings(program, count, varyings, bufferMode);
}
inline void DYNAMICGLES_FUNCTION(GetTransformFeedbackVarying)(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetTransformFeedbackVarying) PFNGLGETTRANSFORMFEEDBACKVARYINGPROC;
#endif
	PFNGLGETTRANSFORMFEEDBACKVARYINGPROC _GetTransformFeedbackVarying =
		(PFNGLGETTRANSFORMFEEDBACKVARYINGPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::GetTransformFeedbackVarying);
	return _GetTransformFeedbackVarying(program, index, bufSize, length, size, type, name);
}
inline void DYNAMICGLES_FUNCTION(VertexAttribIPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glVertexAttribIPointer) PFNGLVERTEXATTRIBIPOINTERPROC;
#endif
	PFNGLVERTEXATTRIBIPOINTERPROC _VertexAttribIPointer = (PFNGLVERTEXATTRIBIPOINTERPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::VertexAttribIPointer);
	return _VertexAttribIPointer(index, size, type, stride, pointer);
}
inline void DYNAMICGLES_FUNCTION(GetVertexAttribIiv)(GLuint index, GLenum pname, GLint* params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetVertexAttribIiv) PFNGLGETVERTEXATTRIBIIVPROC;
#endif
	PFNGLGETVERTEXATTRIBIIVPROC _GetVertexAttribIiv = (PFNGLGETVERTEXATTRIBIIVPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::GetVertexAttribIiv);
	return _GetVertexAttribIiv(index, pname, params);
}
inline void DYNAMICGLES_FUNCTION(GetVertexAttribIuiv)(GLuint index, GLenum pname, GLuint* params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetVertexAttribIuiv) PFNGLGETVERTEXATTRIBIUIVPROC;
#endif
	PFNGLGETVERTEXATTRIBIUIVPROC _GetVertexAttribIuiv = (PFNGLGETVERTEXATTRIBIUIVPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::GetVertexAttribIuiv);
	return _GetVertexAttribIuiv(index, pname, params);
}
inline void DYNAMICGLES_FUNCTION(VertexAttribI4i)(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glVertexAttribI4i) PFNGLVERTEXATTRIBI4IPROC;
#endif
	PFNGLVERTEXATTRIBI4IPROC _VertexAttribI4i = (PFNGLVERTEXATTRIBI4IPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::VertexAttribI4i);
	return _VertexAttribI4i(index, x, y, z, w);
}
inline void DYNAMICGLES_FUNCTION(VertexAttribI4ui)(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glVertexAttribI4ui) PFNGLVERTEXATTRIBI4UIPROC;
#endif
	PFNGLVERTEXATTRIBI4UIPROC _VertexAttribI4ui = (PFNGLVERTEXATTRIBI4UIPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::VertexAttribI4ui);
	return _VertexAttribI4ui(index, x, y, z, w);
}
inline void DYNAMICGLES_FUNCTION(VertexAttribI4iv)(GLuint index, const GLint* v)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glVertexAttribI4iv) PFNGLVERTEXATTRIBI4IVPROC;
#endif
	PFNGLVERTEXATTRIBI4IVPROC _VertexAttribI4iv = (PFNGLVERTEXATTRIBI4IVPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::VertexAttribI4iv);
	return _VertexAttribI4iv(index, v);
}
inline void DYNAMICGLES_FUNCTION(VertexAttribI4uiv)(GLuint index, const GLuint* v)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glVertexAttribI4uiv) PFNGLVERTEXATTRIBI4UIVPROC;
#endif
	PFNGLVERTEXATTRIBI4UIVPROC _VertexAttribI4uiv = (PFNGLVERTEXATTRIBI4UIVPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::VertexAttribI4uiv);
	return _VertexAttribI4uiv(index, v);
}
inline void DYNAMICGLES_FUNCTION(GetUniformuiv)(GLuint program, GLint location, GLuint* params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetUniformuiv) PFNGLGETUNIFORMUIVPROC;
#endif
	PFNGLGETUNIFORMUIVPROC _GetUniformuiv = (PFNGLGETUNIFORMUIVPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::GetUniformuiv);
	return _GetUniformuiv(program, location, params);
}
inline GLint DYNAMICGLES_FUNCTION(GetFragDataLocation)(GLuint program, const GLchar* name)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetFragDataLocation) PFNGLGETFRAGDATALOCATIONPROC;
#endif
	PFNGLGETFRAGDATALOCATIONPROC _GetFragDataLocation = (PFNGLGETFRAGDATALOCATIONPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::GetFragDataLocation);
	return _GetFragDataLocation(program, name);
}
inline void DYNAMICGLES_FUNCTION(Uniform1ui)(GLint location, GLuint v0)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniform1ui) PFNGLUNIFORM1UIPROC;
#endif
	PFNGLUNIFORM1UIPROC _Uniform1ui = (PFNGLUNIFORM1UIPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::Uniform1ui);
	return _Uniform1ui(location, v0);
}
inline void DYNAMICGLES_FUNCTION(Uniform2ui)(GLint location, GLuint v0, GLuint v1)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniform2ui) PFNGLUNIFORM2UIPROC;
#endif
	PFNGLUNIFORM2UIPROC _Uniform2ui = (PFNGLUNIFORM2UIPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::Uniform2ui);
	return _Uniform2ui(location, v0, v1);
}
inline void DYNAMICGLES_FUNCTION(Uniform3ui)(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniform3ui) PFNGLUNIFORM3UIPROC;
#endif
	PFNGLUNIFORM3UIPROC _Uniform3ui = (PFNGLUNIFORM3UIPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::Uniform3ui);
	return _Uniform3ui(location, v0, v1, v2);
}
inline void DYNAMICGLES_FUNCTION(Uniform4ui)(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniform4ui) PFNGLUNIFORM4UIPROC;
#endif
	PFNGLUNIFORM4UIPROC _Uniform4ui = (PFNGLUNIFORM4UIPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::Uniform4ui);
	return _Uniform4ui(location, v0, v1, v2, v3);
}
inline void DYNAMICGLES_FUNCTION(Uniform1uiv)(GLint location, GLsizei count, const GLuint* value)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniform1uiv) PFNGLUNIFORM1UIVPROC;
#endif
	PFNGLUNIFORM1UIVPROC _Uniform1uiv = (PFNGLUNIFORM1UIVPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::Uniform1uiv);
	return _Uniform1uiv(location, count, value);
}
inline void DYNAMICGLES_FUNCTION(Uniform2uiv)(GLint location, GLsizei count, const GLuint* value)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniform2uiv) PFNGLUNIFORM2UIVPROC;
#endif
	PFNGLUNIFORM2UIVPROC _Uniform2uiv = (PFNGLUNIFORM2UIVPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::Uniform2uiv);
	return _Uniform2uiv(location, count, value);
}
inline void DYNAMICGLES_FUNCTION(Uniform3uiv)(GLint location, GLsizei count, const GLuint* value)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniform3uiv) PFNGLUNIFORM3UIVPROC;
#endif
	PFNGLUNIFORM3UIVPROC _Uniform3uiv = (PFNGLUNIFORM3UIVPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::Uniform3uiv);
	return _Uniform3uiv(location, count, value);
}
inline void DYNAMICGLES_FUNCTION(Uniform4uiv)(GLint location, GLsizei count, const GLuint* value)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniform4uiv) PFNGLUNIFORM4UIVPROC;
#endif
	PFNGLUNIFORM4UIVPROC _Uniform4uiv = (PFNGLUNIFORM4UIVPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::Uniform4uiv);
	return _Uniform4uiv(location, count, value);
}
inline void DYNAMICGLES_FUNCTION(ClearBufferiv)(GLenum buffer, GLint drawbuffer, const GLint* value)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glClearBufferiv) PFNGLCLEARBUFFERIVPROC;
#endif
	PFNGLCLEARBUFFERIVPROC _ClearBufferiv = (PFNGLCLEARBUFFERIVPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::ClearBufferiv);
	return _ClearBufferiv(buffer, drawbuffer, value);
}
inline void DYNAMICGLES_FUNCTION(ClearBufferuiv)(GLenum buffer, GLint drawbuffer, const GLuint* value)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glClearBufferuiv) PFNGLCLEARBUFFERUIVPROC;
#endif
	PFNGLCLEARBUFFERUIVPROC _ClearBufferuiv = (PFNGLCLEARBUFFERUIVPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::ClearBufferuiv);
	return _ClearBufferuiv(buffer, drawbuffer, value);
}
inline void DYNAMICGLES_FUNCTION(ClearBufferfv)(GLenum buffer, GLint drawbuffer, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glClearBufferfv) PFNGLCLEARBUFFERFVPROC;
#endif
	PFNGLCLEARBUFFERFVPROC _ClearBufferfv = (PFNGLCLEARBUFFERFVPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::ClearBufferfv);
	return _ClearBufferfv(buffer, drawbuffer, value);
}
inline void DYNAMICGLES_FUNCTION(ClearBufferfi)(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glClearBufferfi) PFNGLCLEARBUFFERFIPROC;
#endif
	PFNGLCLEARBUFFERFIPROC _ClearBufferfi = (PFNGLCLEARBUFFERFIPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::ClearBufferfi);
	return _ClearBufferfi(buffer, drawbuffer, depth, stencil);
}
inline const GLubyte* DYNAMICGLES_FUNCTION(GetStringi)(GLenum name, GLuint index)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetStringi) PFNGLGETSTRINGIPROC;
#endif
	PFNGLGETSTRINGIPROC _GetStringi = (PFNGLGETSTRINGIPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::GetStringi);
	return _GetStringi(name, index);
}
inline void DYNAMICGLES_FUNCTION(CopyBufferSubData)(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glCopyBufferSubData) PFNGLCOPYBUFFERSUBDATAPROC;
#endif
	PFNGLCOPYBUFFERSUBDATAPROC _CopyBufferSubData = (PFNGLCOPYBUFFERSUBDATAPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::CopyBufferSubData);
	return _CopyBufferSubData(readTarget, writeTarget, readOffset, writeOffset, size);
}
inline void DYNAMICGLES_FUNCTION(GetUniformIndices)(GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetUniformIndices) PFNGLGETUNIFORMINDICESPROC;
#endif
	PFNGLGETUNIFORMINDICESPROC _GetUniformIndices = (PFNGLGETUNIFORMINDICESPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::GetUniformIndices);
	return _GetUniformIndices(program, uniformCount, uniformNames, uniformIndices);
}
inline void DYNAMICGLES_FUNCTION(GetActiveUniformsiv)(GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetActiveUniformsiv) PFNGLGETACTIVEUNIFORMSIVPROC;
#endif
	PFNGLGETACTIVEUNIFORMSIVPROC _GetActiveUniformsiv = (PFNGLGETACTIVEUNIFORMSIVPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::GetActiveUniformsiv);
	return _GetActiveUniformsiv(program, uniformCount, uniformIndices, pname, params);
}
inline GLuint DYNAMICGLES_FUNCTION(GetUniformBlockIndex)(GLuint program, const GLchar* uniformBlockName)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetUniformBlockIndex) PFNGLGETUNIFORMBLOCKINDEXPROC;
#endif
	PFNGLGETUNIFORMBLOCKINDEXPROC _GetUniformBlockIndex = (PFNGLGETUNIFORMBLOCKINDEXPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::GetUniformBlockIndex);
	return _GetUniformBlockIndex(program, uniformBlockName);
}
inline void DYNAMICGLES_FUNCTION(GetActiveUniformBlockiv)(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetActiveUniformBlockiv) PFNGLGETACTIVEUNIFORMBLOCKIVPROC;
#endif
	PFNGLGETACTIVEUNIFORMBLOCKIVPROC _GetActiveUniformBlockiv = (PFNGLGETACTIVEUNIFORMBLOCKIVPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::GetActiveUniformBlockiv);
	return _GetActiveUniformBlockiv(program, uniformBlockIndex, pname, params);
}
inline void DYNAMICGLES_FUNCTION(GetActiveUniformBlockName)(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetActiveUniformBlockName) PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC;
#endif
	PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC _GetActiveUniformBlockName =
		(PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::GetActiveUniformBlockName);
	return _GetActiveUniformBlockName(program, uniformBlockIndex, bufSize, length, uniformBlockName);
}
inline void DYNAMICGLES_FUNCTION(UniformBlockBinding)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniformBlockBinding) PFNGLUNIFORMBLOCKBINDINGPROC;
#endif
	PFNGLUNIFORMBLOCKBINDINGPROC _UniformBlockBinding = (PFNGLUNIFORMBLOCKBINDINGPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::UniformBlockBinding);
	return _UniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
}
inline void DYNAMICGLES_FUNCTION(DrawArraysInstanced)(GLenum mode, GLint first, GLsizei count, GLsizei instancecount)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glDrawArraysInstanced) PFNGLDRAWARRAYSINSTANCEDPROC;
#endif
	PFNGLDRAWARRAYSINSTANCEDPROC _DrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::DrawArraysInstanced);
	return _DrawArraysInstanced(mode, first, count, instancecount);
}
inline void DYNAMICGLES_FUNCTION(DrawElementsInstanced)(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glDrawElementsInstanced) PFNGLDRAWELEMENTSINSTANCEDPROC;
#endif
	PFNGLDRAWELEMENTSINSTANCEDPROC _DrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::DrawElementsInstanced);
	return _DrawElementsInstanced(mode, count, type, indices, instancecount);
}
inline GLsync DYNAMICGLES_FUNCTION(FenceSync)(GLenum condition, GLbitfield flags)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glFenceSync) PFNGLFENCESYNCPROC;
#endif
	PFNGLFENCESYNCPROC _FenceSync = (PFNGLFENCESYNCPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::FenceSync);
	return _FenceSync(condition, flags);
}
inline GLboolean DYNAMICGLES_FUNCTION(IsSync)(GLsync sync)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glIsSync) PFNGLISSYNCPROC;
#endif
	PFNGLISSYNCPROC _IsSync = (PFNGLISSYNCPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::IsSync);
	return _IsSync(sync);
}
inline void DYNAMICGLES_FUNCTION(DeleteSync)(GLsync sync)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glDeleteSync) PFNGLDELETESYNCPROC;
#endif
	PFNGLDELETESYNCPROC _DeleteSync = (PFNGLDELETESYNCPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::DeleteSync);
	return _DeleteSync(sync);
}
inline GLenum DYNAMICGLES_FUNCTION(ClientWaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glClientWaitSync) PFNGLCLIENTWAITSYNCPROC;
#endif
	PFNGLCLIENTWAITSYNCPROC _ClientWaitSync = (PFNGLCLIENTWAITSYNCPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::ClientWaitSync);
	return _ClientWaitSync(sync, flags, timeout);
}
inline void DYNAMICGLES_FUNCTION(WaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glWaitSync) PFNGLWAITSYNCPROC;
#endif
	PFNGLWAITSYNCPROC _WaitSync = (PFNGLWAITSYNCPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::WaitSync);
	return _WaitSync(sync, flags, timeout);
}
inline void DYNAMICGLES_FUNCTION(GetInteger64v)(GLenum pname, GLint64* data)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetInteger64v) PFNGLGETINTEGER64VPROC;
#endif
	PFNGLGETINTEGER64VPROC _GetInteger64v = (PFNGLGETINTEGER64VPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::GetInteger64v);
	return _GetInteger64v(pname, data);
}
inline void DYNAMICGLES_FUNCTION(GetSynciv)(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetSynciv) PFNGLGETSYNCIVPROC;
#endif
	PFNGLGETSYNCIVPROC _GetSynciv = (PFNGLGETSYNCIVPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::GetSynciv);
	return _GetSynciv(sync, pname, bufSize, length, values);
}
inline void DYNAMICGLES_FUNCTION(GetInteger64i_v)(GLenum target, GLuint index, GLint64* data)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetInteger64i_v) PFNGLGETINTEGER64I_VPROC;
#endif
	PFNGLGETINTEGER64I_VPROC _GetInteger64i_v = (PFNGLGETINTEGER64I_VPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::GetInteger64i_v);
	return _GetInteger64i_v(target, index, data);
}
inline void DYNAMICGLES_FUNCTION(GetBufferParameteri64v)(GLenum target, GLenum pname, GLint64* params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetBufferParameteri64v) PFNGLGETBUFFERPARAMETERI64VPROC;
#endif
	PFNGLGETBUFFERPARAMETERI64VPROC _GetBufferParameteri64v = (PFNGLGETBUFFERPARAMETERI64VPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::GetBufferParameteri64v);
	return _GetBufferParameteri64v(target, pname, params);
}
inline void DYNAMICGLES_FUNCTION(GenSamplers)(GLsizei count, GLuint* samplers)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGenSamplers) PFNGLGENSAMPLERSPROC;
#endif
	PFNGLGENSAMPLERSPROC _GenSamplers = (PFNGLGENSAMPLERSPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::GenSamplers);
	return _GenSamplers(count, samplers);
}
inline void DYNAMICGLES_FUNCTION(DeleteSamplers)(GLsizei count, const GLuint* samplers)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glDeleteSamplers) PFNGLDELETESAMPLERSPROC;
#endif
	PFNGLDELETESAMPLERSPROC _DeleteSamplers = (PFNGLDELETESAMPLERSPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::DeleteSamplers);
	return _DeleteSamplers(count, samplers);
}
inline GLboolean DYNAMICGLES_FUNCTION(IsSampler)(GLuint sampler)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glIsSampler) PFNGLISSAMPLERPROC;
#endif
	PFNGLISSAMPLERPROC _IsSampler = (PFNGLISSAMPLERPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::IsSampler);
	return _IsSampler(sampler);
}
inline void DYNAMICGLES_FUNCTION(BindSampler)(GLuint unit, GLuint sampler)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glBindSampler) PFNGLBINDSAMPLERPROC;
#endif
	PFNGLBINDSAMPLERPROC _BindSampler = (PFNGLBINDSAMPLERPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::BindSampler);
	return _BindSampler(unit, sampler);
}
inline void DYNAMICGLES_FUNCTION(SamplerParameteri)(GLuint sampler, GLenum pname, GLint param)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glSamplerParameteri) PFNGLSAMPLERPARAMETERIPROC;
#endif
	PFNGLSAMPLERPARAMETERIPROC _SamplerParameteri = (PFNGLSAMPLERPARAMETERIPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::SamplerParameteri);
	return _SamplerParameteri(sampler, pname, param);
}
inline void DYNAMICGLES_FUNCTION(SamplerParameteriv)(GLuint sampler, GLenum pname, const GLint* param)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glSamplerParameteriv) PFNGLSAMPLERPARAMETERIVPROC;
#endif
	PFNGLSAMPLERPARAMETERIVPROC _SamplerParameteriv = (PFNGLSAMPLERPARAMETERIVPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::SamplerParameteriv);
	return _SamplerParameteriv(sampler, pname, param);
}
inline void DYNAMICGLES_FUNCTION(SamplerParameterf)(GLuint sampler, GLenum pname, GLfloat param)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glSamplerParameterf) PFNGLSAMPLERPARAMETERFPROC;
#endif
	PFNGLSAMPLERPARAMETERFPROC _SamplerParameterf = (PFNGLSAMPLERPARAMETERFPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::SamplerParameterf);
	return _SamplerParameterf(sampler, pname, param);
}
inline void DYNAMICGLES_FUNCTION(SamplerParameterfv)(GLuint sampler, GLenum pname, const GLfloat* param)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glSamplerParameterfv) PFNGLSAMPLERPARAMETERFVPROC;
#endif
	PFNGLSAMPLERPARAMETERFVPROC _SamplerParameterfv = (PFNGLSAMPLERPARAMETERFVPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::SamplerParameterfv);
	return _SamplerParameterfv(sampler, pname, param);
}
inline void DYNAMICGLES_FUNCTION(GetSamplerParameteriv)(GLuint sampler, GLenum pname, GLint* params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetSamplerParameteriv) PFNGLGETSAMPLERPARAMETERIVPROC;
#endif
	PFNGLGETSAMPLERPARAMETERIVPROC _GetSamplerParameteriv = (PFNGLGETSAMPLERPARAMETERIVPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::GetSamplerParameteriv);
	return _GetSamplerParameteriv(sampler, pname, params);
}
inline void DYNAMICGLES_FUNCTION(GetSamplerParameterfv)(GLuint sampler, GLenum pname, GLfloat* params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetSamplerParameterfv) PFNGLGETSAMPLERPARAMETERFVPROC;
#endif
	PFNGLGETSAMPLERPARAMETERFVPROC _GetSamplerParameterfv = (PFNGLGETSAMPLERPARAMETERFVPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::GetSamplerParameterfv);
	return _GetSamplerParameterfv(sampler, pname, params);
}
inline void DYNAMICGLES_FUNCTION(VertexAttribDivisor)(GLuint index, GLuint divisor)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glVertexAttribDivisor) PFNGLVERTEXATTRIBDIVISORPROC;
#endif
	PFNGLVERTEXATTRIBDIVISORPROC _VertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISORPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::VertexAttribDivisor);
	return _VertexAttribDivisor(index, divisor);
}
inline void DYNAMICGLES_FUNCTION(BindTransformFeedback)(GLenum target, GLuint id)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glBindTransformFeedback) PFNGLBINDTRANSFORMFEEDBACKPROC;
#endif
	PFNGLBINDTRANSFORMFEEDBACKPROC _BindTransformFeedback = (PFNGLBINDTRANSFORMFEEDBACKPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::BindTransformFeedback);
	return _BindTransformFeedback(target, id);
}
inline void DYNAMICGLES_FUNCTION(DeleteTransformFeedbacks)(GLsizei n, const GLuint* ids)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glDeleteTransformFeedbacks) PFNGLDELETETRANSFORMFEEDBACKSPROC;
#endif
	PFNGLDELETETRANSFORMFEEDBACKSPROC _DeleteTransformFeedbacks = (PFNGLDELETETRANSFORMFEEDBACKSPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::DeleteTransformFeedbacks);
	return _DeleteTransformFeedbacks(n, ids);
}
inline void DYNAMICGLES_FUNCTION(GenTransformFeedbacks)(GLsizei n, GLuint* ids)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGenTransformFeedbacks) PFNGLGENTRANSFORMFEEDBACKSPROC;
#endif
	PFNGLGENTRANSFORMFEEDBACKSPROC _GenTransformFeedbacks = (PFNGLGENTRANSFORMFEEDBACKSPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::GenTransformFeedbacks);
	return _GenTransformFeedbacks(n, ids);
}
inline GLboolean DYNAMICGLES_FUNCTION(IsTransformFeedback)(GLuint id)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glIsTransformFeedback) PFNGLISTRANSFORMFEEDBACKPROC;
#endif
	PFNGLISTRANSFORMFEEDBACKPROC _IsTransformFeedback = (PFNGLISTRANSFORMFEEDBACKPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::IsTransformFeedback);
	return _IsTransformFeedback(id);
}
inline void DYNAMICGLES_FUNCTION(PauseTransformFeedback)(void)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glPauseTransformFeedback) PFNGLPAUSETRANSFORMFEEDBACKPROC;
#endif
	PFNGLPAUSETRANSFORMFEEDBACKPROC _PauseTransformFeedback = (PFNGLPAUSETRANSFORMFEEDBACKPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::PauseTransformFeedback);
	return _PauseTransformFeedback();
}
inline void DYNAMICGLES_FUNCTION(ResumeTransformFeedback)(void)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glResumeTransformFeedback) PFNGLRESUMETRANSFORMFEEDBACKPROC;
#endif
	PFNGLRESUMETRANSFORMFEEDBACKPROC _ResumeTransformFeedback = (PFNGLRESUMETRANSFORMFEEDBACKPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::ResumeTransformFeedback);
	return _ResumeTransformFeedback();
}
inline void DYNAMICGLES_FUNCTION(GetProgramBinary)(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, void* binary)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetProgramBinary) PFNGLGETPROGRAMBINARYPROC;
#endif
	PFNGLGETPROGRAMBINARYPROC _GetProgramBinary = (PFNGLGETPROGRAMBINARYPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::GetProgramBinary);
	return _GetProgramBinary(program, bufSize, length, binaryFormat, binary);
}
inline void DYNAMICGLES_FUNCTION(ProgramBinary)(GLuint program, GLenum binaryFormat, const void* binary, GLsizei length)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glProgramBinary) PFNGLPROGRAMBINARYPROC;
#endif
	PFNGLPROGRAMBINARYPROC _ProgramBinary = (PFNGLPROGRAMBINARYPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::ProgramBinary);
	return _ProgramBinary(program, binaryFormat, binary, length);
}
inline void DYNAMICGLES_FUNCTION(ProgramParameteri)(GLuint program, GLenum pname, GLint value)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glProgramParameteri) PFNGLPROGRAMPARAMETERIPROC;
#endif
	PFNGLPROGRAMPARAMETERIPROC _ProgramParameteri = (PFNGLPROGRAMPARAMETERIPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::ProgramParameteri);
	return _ProgramParameteri(program, pname, value);
}
inline void DYNAMICGLES_FUNCTION(InvalidateFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum* attachments)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glInvalidateFramebuffer) PFNGLINVALIDATEFRAMEBUFFERPROC;
#endif
	PFNGLINVALIDATEFRAMEBUFFERPROC _InvalidateFramebuffer = (PFNGLINVALIDATEFRAMEBUFFERPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::InvalidateFramebuffer);
	return _InvalidateFramebuffer(target, numAttachments, attachments);
}
inline void DYNAMICGLES_FUNCTION(InvalidateSubFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glInvalidateSubFramebuffer) PFNGLINVALIDATESUBFRAMEBUFFERPROC;
#endif
	PFNGLINVALIDATESUBFRAMEBUFFERPROC _InvalidateSubFramebuffer = (PFNGLINVALIDATESUBFRAMEBUFFERPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::InvalidateSubFramebuffer);
	return _InvalidateSubFramebuffer(target, numAttachments, attachments, x, y, width, height);
}
inline void DYNAMICGLES_FUNCTION(TexStorage2D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glTexStorage2D) PFNGLTEXSTORAGE2DPROC;
#endif
	PFNGLTEXSTORAGE2DPROC _TexStorage2D = (PFNGLTEXSTORAGE2DPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::TexStorage2D);
	return _TexStorage2D(target, levels, internalformat, width, height);
}
inline void DYNAMICGLES_FUNCTION(TexStorage3D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glTexStorage3D) PFNGLTEXSTORAGE3DPROC;
#endif
	PFNGLTEXSTORAGE3DPROC _TexStorage3D = (PFNGLTEXSTORAGE3DPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::TexStorage3D);
	return _TexStorage3D(target, levels, internalformat, width, height, depth);
}
inline void DYNAMICGLES_FUNCTION(GetInternalformativ)(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetInternalformativ) PFNGLGETINTERNALFORMATIVPROC;
#endif
	PFNGLGETINTERNALFORMATIVPROC _GetInternalformativ = (PFNGLGETINTERNALFORMATIVPROC)gl::internals::getEs3Function(gl::internals::Gl3FuncName::GetInternalformativ);
	return _GetInternalformativ(target, internalformat, pname, bufSize, params);
}
#ifndef DYNAMICGLES_NO_NAMESPACE
}
#elif TARGET_OS_IPHONE
}
}
#endif

namespace gl {
namespace internals {
namespace Gl2FuncName {
enum OpenGLES2FunctionName
{
	ActiveTexture,
	AttachShader,
	BindAttribLocation,
	BindBuffer,
	BindFramebuffer,
	BindRenderbuffer,
	BindTexture,
	BlendColor,
	BlendEquation,
	BlendEquationSeparate,
	BlendFunc,
	BlendFuncSeparate,
	BufferData,
	BufferSubData,
	CheckFramebufferStatus,
	Clear,
	ClearColor,
	ClearDepthf,
	ClearStencil,
	ColorMask,
	CompileShader,
	CompressedTexImage2D,
	CompressedTexSubImage2D,
	CopyTexImage2D,
	CopyTexSubImage2D,
	CreateProgram,
	CreateShader,
	CullFace,
	DeleteBuffers,
	DeleteFramebuffers,
	DeleteProgram,
	DeleteRenderbuffers,
	DeleteShader,
	DeleteTextures,
	DepthFunc,
	DepthMask,
	DepthRangef,
	DetachShader,
	Disable,
	DisableVertexAttribArray,
	DrawArrays,
	DrawElements,
	Enable,
	EnableVertexAttribArray,
	Finish,
	Flush,
	FramebufferRenderbuffer,
	FramebufferTexture2D,
	FrontFace,
	GenBuffers,
	GenerateMipmap,
	GenFramebuffers,
	GenRenderbuffers,
	GenTextures,
	GetActiveAttrib,
	GetActiveUniform,
	GetAttachedShaders,
	GetAttribLocation,
	GetBooleanv,
	GetBufferParameteriv,
	GetError,
	GetFloatv,
	GetFramebufferAttachmentParameteriv,
	GetIntegerv,
	GetProgramiv,
	GetProgramInfoLog,
	GetRenderbufferParameteriv,
	GetShaderiv,
	GetShaderInfoLog,
	GetShaderPrecisionFormat,
	GetShaderSource,
	GetString,
	GetTexParameterfv,
	GetTexParameteriv,
	GetUniformfv,
	GetUniformiv,
	GetUniformLocation,
	GetVertexAttribfv,
	GetVertexAttribiv,
	GetVertexAttribPointerv,
	Hint,
	IsBuffer,
	IsEnabled,
	IsFramebuffer,
	IsProgram,
	IsRenderbuffer,
	IsShader,
	IsTexture,
	LineWidth,
	LinkProgram,
	PixelStorei,
	PolygonOffset,
	ReadPixels,
	ReleaseShaderCompiler,
	RenderbufferStorage,
	SampleCoverage,
	Scissor,
	ShaderBinary,
	ShaderSource,
	StencilFunc,
	StencilFuncSeparate,
	StencilMask,
	StencilMaskSeparate,
	StencilOp,
	StencilOpSeparate,
	TexImage2D,
	TexParameterf,
	TexParameterfv,
	TexParameteri,
	TexParameteriv,
	TexSubImage2D,
	Uniform1f,
	Uniform1fv,
	Uniform1i,
	Uniform1iv,
	Uniform2f,
	Uniform2fv,
	Uniform2i,
	Uniform2iv,
	Uniform3f,
	Uniform3fv,
	Uniform3i,
	Uniform3iv,
	Uniform4f,
	Uniform4fv,
	Uniform4i,
	Uniform4iv,
	UniformMatrix2fv,
	UniformMatrix3fv,
	UniformMatrix4fv,
	UseProgram,
	ValidateProgram,
	VertexAttrib1f,
	VertexAttrib1fv,
	VertexAttrib2f,
	VertexAttrib2fv,
	VertexAttrib3f,
	VertexAttrib3fv,
	VertexAttrib4f,
	VertexAttrib4fv,
	VertexAttribPointer,
	Viewport,
	NUMBER_OF_OPENGLES2_FUNCTIONS
};
}

// Pre-loads the OpenGL ES 2.0 function pointers the first time any OpenGL ES 2.0 function call is made
inline void* getEs2Function(gl::internals::Gl2FuncName::OpenGLES2FunctionName funcname)
{
	static void* FunctionTable[Gl2FuncName::NUMBER_OF_OPENGLES2_FUNCTIONS];

	// Retrieve the OpenGL ES 2.0 functions pointers once
	if (!FunctionTable[0])
	{
#if !TARGET_OS_IPHONE
		pvr::lib::LIBTYPE lib = pvr::lib::openlib(gl::internals::libName);
		if (!lib) { Log_Error("OpenGL ES Bindings: Failed to open library %s\n", internals::libName); }
		else
		{
			Log_Info("OpenGL ES Bindings: Successfully loaded library %s for OpenGL ES 2.0\n", libName);
		}

		FunctionTable[Gl2FuncName::ActiveTexture] = pvr::lib::getLibFunctionChecked<void*>(lib, "glActiveTexture");
		FunctionTable[Gl2FuncName::AttachShader] = pvr::lib::getLibFunctionChecked<void*>(lib, "glAttachShader");
		FunctionTable[Gl2FuncName::BindAttribLocation] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBindAttribLocation");
		FunctionTable[Gl2FuncName::BindBuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBindBuffer");
		FunctionTable[Gl2FuncName::BindFramebuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBindFramebuffer");
		FunctionTable[Gl2FuncName::BindRenderbuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBindRenderbuffer");
		FunctionTable[Gl2FuncName::BindTexture] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBindTexture");
		FunctionTable[Gl2FuncName::BlendColor] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBlendColor");
		FunctionTable[Gl2FuncName::BlendEquation] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBlendEquation");
		FunctionTable[Gl2FuncName::BlendEquationSeparate] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBlendEquationSeparate");
		FunctionTable[Gl2FuncName::BlendFunc] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBlendFunc");
		FunctionTable[Gl2FuncName::BlendFuncSeparate] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBlendFuncSeparate");
		FunctionTable[Gl2FuncName::BufferData] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBufferData");
		FunctionTable[Gl2FuncName::BufferSubData] = pvr::lib::getLibFunctionChecked<void*>(lib, "glBufferSubData");
		FunctionTable[Gl2FuncName::CheckFramebufferStatus] = pvr::lib::getLibFunctionChecked<void*>(lib, "glCheckFramebufferStatus");
		FunctionTable[Gl2FuncName::Clear] = pvr::lib::getLibFunctionChecked<void*>(lib, "glClear");
		FunctionTable[Gl2FuncName::ClearColor] = pvr::lib::getLibFunctionChecked<void*>(lib, "glClearColor");
		FunctionTable[Gl2FuncName::ClearDepthf] = pvr::lib::getLibFunctionChecked<void*>(lib, "glClearDepthf");
		FunctionTable[Gl2FuncName::ClearStencil] = pvr::lib::getLibFunctionChecked<void*>(lib, "glClearStencil");
		FunctionTable[Gl2FuncName::ColorMask] = pvr::lib::getLibFunctionChecked<void*>(lib, "glColorMask");
		FunctionTable[Gl2FuncName::CompileShader] = pvr::lib::getLibFunctionChecked<void*>(lib, "glCompileShader");
		FunctionTable[Gl2FuncName::CompressedTexImage2D] = pvr::lib::getLibFunctionChecked<void*>(lib, "glCompressedTexImage2D");
		FunctionTable[Gl2FuncName::CompressedTexSubImage2D] = pvr::lib::getLibFunctionChecked<void*>(lib, "glCompressedTexSubImage2D");
		FunctionTable[Gl2FuncName::CopyTexImage2D] = pvr::lib::getLibFunctionChecked<void*>(lib, "glCopyTexImage2D");
		FunctionTable[Gl2FuncName::CopyTexSubImage2D] = pvr::lib::getLibFunctionChecked<void*>(lib, "glCopyTexSubImage2D");
		FunctionTable[Gl2FuncName::CreateProgram] = pvr::lib::getLibFunctionChecked<void*>(lib, "glCreateProgram");
		FunctionTable[Gl2FuncName::CreateShader] = pvr::lib::getLibFunctionChecked<void*>(lib, "glCreateShader");
		FunctionTable[Gl2FuncName::CullFace] = pvr::lib::getLibFunctionChecked<void*>(lib, "glCullFace");
		FunctionTable[Gl2FuncName::DeleteBuffers] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDeleteBuffers");
		FunctionTable[Gl2FuncName::DeleteFramebuffers] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDeleteFramebuffers");
		FunctionTable[Gl2FuncName::DeleteProgram] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDeleteProgram");
		FunctionTable[Gl2FuncName::DeleteRenderbuffers] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDeleteRenderbuffers");
		FunctionTable[Gl2FuncName::DeleteShader] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDeleteShader");
		FunctionTable[Gl2FuncName::DeleteTextures] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDeleteTextures");
		FunctionTable[Gl2FuncName::DepthFunc] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDepthFunc");
		FunctionTable[Gl2FuncName::DepthMask] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDepthMask");
		FunctionTable[Gl2FuncName::DepthRangef] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDepthRangef");
		FunctionTable[Gl2FuncName::DetachShader] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDetachShader");
		FunctionTable[Gl2FuncName::Disable] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDisable");
		FunctionTable[Gl2FuncName::DisableVertexAttribArray] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDisableVertexAttribArray");
		FunctionTable[Gl2FuncName::DrawArrays] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDrawArrays");
		FunctionTable[Gl2FuncName::DrawElements] = pvr::lib::getLibFunctionChecked<void*>(lib, "glDrawElements");
		FunctionTable[Gl2FuncName::Enable] = pvr::lib::getLibFunctionChecked<void*>(lib, "glEnable");
		FunctionTable[Gl2FuncName::EnableVertexAttribArray] = pvr::lib::getLibFunctionChecked<void*>(lib, "glEnableVertexAttribArray");
		FunctionTable[Gl2FuncName::Finish] = pvr::lib::getLibFunctionChecked<void*>(lib, "glFinish");
		FunctionTable[Gl2FuncName::Flush] = pvr::lib::getLibFunctionChecked<void*>(lib, "glFlush");
		FunctionTable[Gl2FuncName::FramebufferRenderbuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "glFramebufferRenderbuffer");
		FunctionTable[Gl2FuncName::FramebufferTexture2D] = pvr::lib::getLibFunctionChecked<void*>(lib, "glFramebufferTexture2D");
		FunctionTable[Gl2FuncName::FrontFace] = pvr::lib::getLibFunctionChecked<void*>(lib, "glFrontFace");
		FunctionTable[Gl2FuncName::GenBuffers] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGenBuffers");
		FunctionTable[Gl2FuncName::GenerateMipmap] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGenerateMipmap");
		FunctionTable[Gl2FuncName::GenFramebuffers] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGenFramebuffers");
		FunctionTable[Gl2FuncName::GenRenderbuffers] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGenRenderbuffers");
		FunctionTable[Gl2FuncName::GenTextures] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGenTextures");
		FunctionTable[Gl2FuncName::GetActiveAttrib] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetActiveAttrib");
		FunctionTable[Gl2FuncName::GetActiveUniform] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetActiveUniform");
		FunctionTable[Gl2FuncName::GetAttachedShaders] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetAttachedShaders");
		FunctionTable[Gl2FuncName::GetAttribLocation] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetAttribLocation");
		FunctionTable[Gl2FuncName::GetBooleanv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetBooleanv");
		FunctionTable[Gl2FuncName::GetBufferParameteriv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetBufferParameteriv");
		FunctionTable[Gl2FuncName::GetError] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetError");
		FunctionTable[Gl2FuncName::GetFloatv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetFloatv");
		FunctionTable[Gl2FuncName::GetFramebufferAttachmentParameteriv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetFramebufferAttachmentParameteriv");
		FunctionTable[Gl2FuncName::GetIntegerv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetIntegerv");
		FunctionTable[Gl2FuncName::GetProgramiv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetProgramiv");
		FunctionTable[Gl2FuncName::GetProgramInfoLog] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetProgramInfoLog");
		FunctionTable[Gl2FuncName::GetRenderbufferParameteriv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetRenderbufferParameteriv");
		FunctionTable[Gl2FuncName::GetShaderiv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetShaderiv");
		FunctionTable[Gl2FuncName::GetShaderInfoLog] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetShaderInfoLog");
		FunctionTable[Gl2FuncName::GetShaderPrecisionFormat] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetShaderPrecisionFormat");
		FunctionTable[Gl2FuncName::GetShaderSource] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetShaderSource");
		FunctionTable[Gl2FuncName::GetString] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetString");
		FunctionTable[Gl2FuncName::GetTexParameterfv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetTexParameterfv");
		FunctionTable[Gl2FuncName::GetTexParameteriv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetTexParameteriv");
		FunctionTable[Gl2FuncName::GetUniformfv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetUniformfv");
		FunctionTable[Gl2FuncName::GetUniformiv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetUniformiv");
		FunctionTable[Gl2FuncName::GetUniformLocation] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetUniformLocation");
		FunctionTable[Gl2FuncName::GetVertexAttribfv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetVertexAttribfv");
		FunctionTable[Gl2FuncName::GetVertexAttribiv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetVertexAttribiv");
		FunctionTable[Gl2FuncName::GetVertexAttribPointerv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glGetVertexAttribPointerv");
		FunctionTable[Gl2FuncName::Hint] = pvr::lib::getLibFunctionChecked<void*>(lib, "glHint");
		FunctionTable[Gl2FuncName::IsBuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "glIsBuffer");
		FunctionTable[Gl2FuncName::IsEnabled] = pvr::lib::getLibFunctionChecked<void*>(lib, "glIsEnabled");
		FunctionTable[Gl2FuncName::IsFramebuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "glIsFramebuffer");
		FunctionTable[Gl2FuncName::IsProgram] = pvr::lib::getLibFunctionChecked<void*>(lib, "glIsProgram");
		FunctionTable[Gl2FuncName::IsRenderbuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "glIsRenderbuffer");
		FunctionTable[Gl2FuncName::IsShader] = pvr::lib::getLibFunctionChecked<void*>(lib, "glIsShader");
		FunctionTable[Gl2FuncName::IsTexture] = pvr::lib::getLibFunctionChecked<void*>(lib, "glIsTexture");
		FunctionTable[Gl2FuncName::LineWidth] = pvr::lib::getLibFunctionChecked<void*>(lib, "glLineWidth");
		FunctionTable[Gl2FuncName::LinkProgram] = pvr::lib::getLibFunctionChecked<void*>(lib, "glLinkProgram");
		FunctionTable[Gl2FuncName::PixelStorei] = pvr::lib::getLibFunctionChecked<void*>(lib, "glPixelStorei");
		FunctionTable[Gl2FuncName::PolygonOffset] = pvr::lib::getLibFunctionChecked<void*>(lib, "glPolygonOffset");
		FunctionTable[Gl2FuncName::ReadPixels] = pvr::lib::getLibFunctionChecked<void*>(lib, "glReadPixels");
		FunctionTable[Gl2FuncName::ReleaseShaderCompiler] = pvr::lib::getLibFunctionChecked<void*>(lib, "glReleaseShaderCompiler");
		FunctionTable[Gl2FuncName::RenderbufferStorage] = pvr::lib::getLibFunctionChecked<void*>(lib, "glRenderbufferStorage");
		FunctionTable[Gl2FuncName::SampleCoverage] = pvr::lib::getLibFunctionChecked<void*>(lib, "glSampleCoverage");
		FunctionTable[Gl2FuncName::Scissor] = pvr::lib::getLibFunctionChecked<void*>(lib, "glScissor");
		FunctionTable[Gl2FuncName::ShaderBinary] = pvr::lib::getLibFunctionChecked<void*>(lib, "glShaderBinary");
		FunctionTable[Gl2FuncName::ShaderSource] = pvr::lib::getLibFunctionChecked<void*>(lib, "glShaderSource");
		FunctionTable[Gl2FuncName::StencilFunc] = pvr::lib::getLibFunctionChecked<void*>(lib, "glStencilFunc");
		FunctionTable[Gl2FuncName::StencilFuncSeparate] = pvr::lib::getLibFunctionChecked<void*>(lib, "glStencilFuncSeparate");
		FunctionTable[Gl2FuncName::StencilMask] = pvr::lib::getLibFunctionChecked<void*>(lib, "glStencilMask");
		FunctionTable[Gl2FuncName::StencilMaskSeparate] = pvr::lib::getLibFunctionChecked<void*>(lib, "glStencilMaskSeparate");
		FunctionTable[Gl2FuncName::StencilOp] = pvr::lib::getLibFunctionChecked<void*>(lib, "glStencilOp");
		FunctionTable[Gl2FuncName::StencilOpSeparate] = pvr::lib::getLibFunctionChecked<void*>(lib, "glStencilOpSeparate");
		FunctionTable[Gl2FuncName::TexImage2D] = pvr::lib::getLibFunctionChecked<void*>(lib, "glTexImage2D");
		FunctionTable[Gl2FuncName::TexParameterf] = pvr::lib::getLibFunctionChecked<void*>(lib, "glTexParameterf");
		FunctionTable[Gl2FuncName::TexParameterfv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glTexParameterfv");
		FunctionTable[Gl2FuncName::TexParameteri] = pvr::lib::getLibFunctionChecked<void*>(lib, "glTexParameteri");
		FunctionTable[Gl2FuncName::TexParameteriv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glTexParameteriv");
		FunctionTable[Gl2FuncName::TexSubImage2D] = pvr::lib::getLibFunctionChecked<void*>(lib, "glTexSubImage2D");
		FunctionTable[Gl2FuncName::Uniform1f] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform1f");
		FunctionTable[Gl2FuncName::Uniform1fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform1fv");
		FunctionTable[Gl2FuncName::Uniform1i] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform1i");
		FunctionTable[Gl2FuncName::Uniform1iv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform1iv");
		FunctionTable[Gl2FuncName::Uniform2f] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform2f");
		FunctionTable[Gl2FuncName::Uniform2fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform2fv");
		FunctionTable[Gl2FuncName::Uniform2i] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform2i");
		FunctionTable[Gl2FuncName::Uniform2iv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform2iv");
		FunctionTable[Gl2FuncName::Uniform3f] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform3f");
		FunctionTable[Gl2FuncName::Uniform3fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform3fv");
		FunctionTable[Gl2FuncName::Uniform3i] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform3i");
		FunctionTable[Gl2FuncName::Uniform3iv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform3iv");
		FunctionTable[Gl2FuncName::Uniform4f] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform4f");
		FunctionTable[Gl2FuncName::Uniform4fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform4fv");
		FunctionTable[Gl2FuncName::Uniform4i] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform4i");
		FunctionTable[Gl2FuncName::Uniform4iv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniform4iv");
		FunctionTable[Gl2FuncName::UniformMatrix2fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniformMatrix2fv");
		FunctionTable[Gl2FuncName::UniformMatrix3fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniformMatrix3fv");
		FunctionTable[Gl2FuncName::UniformMatrix4fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUniformMatrix4fv");
		FunctionTable[Gl2FuncName::UseProgram] = pvr::lib::getLibFunctionChecked<void*>(lib, "glUseProgram");
		FunctionTable[Gl2FuncName::ValidateProgram] = pvr::lib::getLibFunctionChecked<void*>(lib, "glValidateProgram");
		FunctionTable[Gl2FuncName::VertexAttrib1f] = pvr::lib::getLibFunctionChecked<void*>(lib, "glVertexAttrib1f");
		FunctionTable[Gl2FuncName::VertexAttrib1fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glVertexAttrib1fv");
		FunctionTable[Gl2FuncName::VertexAttrib2f] = pvr::lib::getLibFunctionChecked<void*>(lib, "glVertexAttrib2f");
		FunctionTable[Gl2FuncName::VertexAttrib2fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glVertexAttrib2fv");
		FunctionTable[Gl2FuncName::VertexAttrib3f] = pvr::lib::getLibFunctionChecked<void*>(lib, "glVertexAttrib3f");
		FunctionTable[Gl2FuncName::VertexAttrib3fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glVertexAttrib3fv");
		FunctionTable[Gl2FuncName::VertexAttrib4f] = pvr::lib::getLibFunctionChecked<void*>(lib, "glVertexAttrib4f");
		FunctionTable[Gl2FuncName::VertexAttrib4fv] = pvr::lib::getLibFunctionChecked<void*>(lib, "glVertexAttrib4fv");
		FunctionTable[Gl2FuncName::VertexAttribPointer] = pvr::lib::getLibFunctionChecked<void*>(lib, "glVertexAttribPointer");
		FunctionTable[Gl2FuncName::Viewport] = pvr::lib::getLibFunctionChecked<void*>(lib, "glViewport");
#else
		FunctionTable[Gl2FuncName::ActiveTexture] = (void*)&glActiveTexture;
		FunctionTable[Gl2FuncName::AttachShader] = (void*)&glAttachShader;
		FunctionTable[Gl2FuncName::BindAttribLocation] = (void*)&glBindAttribLocation;
		FunctionTable[Gl2FuncName::BindBuffer] = (void*)&glBindBuffer;
		FunctionTable[Gl2FuncName::BindFramebuffer] = (void*)&glBindFramebuffer;
		FunctionTable[Gl2FuncName::BindRenderbuffer] = (void*)&glBindRenderbuffer;
		FunctionTable[Gl2FuncName::BindTexture] = (void*)&glBindTexture;
		FunctionTable[Gl2FuncName::BlendColor] = (void*)&glBlendColor;
		FunctionTable[Gl2FuncName::BlendEquation] = (void*)&glBlendEquation;
		FunctionTable[Gl2FuncName::BlendEquationSeparate] = (void*)&glBlendEquationSeparate;
		FunctionTable[Gl2FuncName::BlendFunc] = (void*)&glBlendFunc;
		FunctionTable[Gl2FuncName::BlendFuncSeparate] = (void*)&glBlendFuncSeparate;
		FunctionTable[Gl2FuncName::BufferData] = (void*)&glBufferData;
		FunctionTable[Gl2FuncName::BufferSubData] = (void*)&glBufferSubData;
		FunctionTable[Gl2FuncName::CheckFramebufferStatus] = (void*)&glCheckFramebufferStatus;
		FunctionTable[Gl2FuncName::Clear] = (void*)&glClear;
		FunctionTable[Gl2FuncName::ClearColor] = (void*)&glClearColor;
		FunctionTable[Gl2FuncName::ClearDepthf] = (void*)&glClearDepthf;
		FunctionTable[Gl2FuncName::ClearStencil] = (void*)&glClearStencil;
		FunctionTable[Gl2FuncName::ColorMask] = (void*)&glColorMask;
		FunctionTable[Gl2FuncName::CompileShader] = (void*)&glCompileShader;
		FunctionTable[Gl2FuncName::CompressedTexImage2D] = (void*)&glCompressedTexImage2D;
		FunctionTable[Gl2FuncName::CompressedTexSubImage2D] = (void*)&glCompressedTexSubImage2D;
		FunctionTable[Gl2FuncName::CopyTexImage2D] = (void*)&glCopyTexImage2D;
		FunctionTable[Gl2FuncName::CopyTexSubImage2D] = (void*)&glCopyTexSubImage2D;
		FunctionTable[Gl2FuncName::CreateProgram] = (void*)&glCreateProgram;
		FunctionTable[Gl2FuncName::CreateShader] = (void*)&glCreateShader;
		FunctionTable[Gl2FuncName::CullFace] = (void*)&glCullFace;
		FunctionTable[Gl2FuncName::DeleteBuffers] = (void*)&glDeleteBuffers;
		FunctionTable[Gl2FuncName::DeleteFramebuffers] = (void*)&glDeleteFramebuffers;
		FunctionTable[Gl2FuncName::DeleteProgram] = (void*)&glDeleteProgram;
		FunctionTable[Gl2FuncName::DeleteRenderbuffers] = (void*)&glDeleteRenderbuffers;
		FunctionTable[Gl2FuncName::DeleteShader] = (void*)&glDeleteShader;
		FunctionTable[Gl2FuncName::DeleteTextures] = (void*)&glDeleteTextures;
		FunctionTable[Gl2FuncName::DepthFunc] = (void*)&glDepthFunc;
		FunctionTable[Gl2FuncName::DepthMask] = (void*)&glDepthMask;
		FunctionTable[Gl2FuncName::DepthRangef] = (void*)&glDepthRangef;
		FunctionTable[Gl2FuncName::DetachShader] = (void*)&glDetachShader;
		FunctionTable[Gl2FuncName::Disable] = (void*)&glDisable;
		FunctionTable[Gl2FuncName::DisableVertexAttribArray] = (void*)&glDisableVertexAttribArray;
		FunctionTable[Gl2FuncName::DrawArrays] = (void*)&glDrawArrays;
		FunctionTable[Gl2FuncName::DrawElements] = (void*)&glDrawElements;
		FunctionTable[Gl2FuncName::Enable] = (void*)&glEnable;
		FunctionTable[Gl2FuncName::EnableVertexAttribArray] = (void*)&glEnableVertexAttribArray;
		FunctionTable[Gl2FuncName::Finish] = (void*)&glFinish;
		FunctionTable[Gl2FuncName::Flush] = (void*)&glFlush;
		FunctionTable[Gl2FuncName::FramebufferRenderbuffer] = (void*)&glFramebufferRenderbuffer;
		FunctionTable[Gl2FuncName::FramebufferTexture2D] = (void*)&glFramebufferTexture2D;
		FunctionTable[Gl2FuncName::FrontFace] = (void*)&glFrontFace;
		FunctionTable[Gl2FuncName::GenBuffers] = (void*)&glGenBuffers;
		FunctionTable[Gl2FuncName::GenerateMipmap] = (void*)&glGenerateMipmap;
		FunctionTable[Gl2FuncName::GenFramebuffers] = (void*)&glGenFramebuffers;
		FunctionTable[Gl2FuncName::GenRenderbuffers] = (void*)&glGenRenderbuffers;
		FunctionTable[Gl2FuncName::GenTextures] = (void*)&glGenTextures;
		FunctionTable[Gl2FuncName::GetActiveAttrib] = (void*)&glGetActiveAttrib;
		FunctionTable[Gl2FuncName::GetActiveUniform] = (void*)&glGetActiveUniform;
		FunctionTable[Gl2FuncName::GetAttachedShaders] = (void*)&glGetAttachedShaders;
		FunctionTable[Gl2FuncName::GetAttribLocation] = (void*)&glGetAttribLocation;
		FunctionTable[Gl2FuncName::GetBooleanv] = (void*)&glGetBooleanv;
		FunctionTable[Gl2FuncName::GetBufferParameteriv] = (void*)&glGetBufferParameteriv;
		FunctionTable[Gl2FuncName::GetError] = (void*)&glGetError;
		FunctionTable[Gl2FuncName::GetFloatv] = (void*)&glGetFloatv;
		FunctionTable[Gl2FuncName::GetFramebufferAttachmentParameteriv] = (void*)&glGetFramebufferAttachmentParameteriv;
		FunctionTable[Gl2FuncName::GetIntegerv] = (void*)&glGetIntegerv;
		FunctionTable[Gl2FuncName::GetProgramiv] = (void*)&glGetProgramiv;
		FunctionTable[Gl2FuncName::GetProgramInfoLog] = (void*)&glGetProgramInfoLog;
		FunctionTable[Gl2FuncName::GetRenderbufferParameteriv] = (void*)&glGetRenderbufferParameteriv;
		FunctionTable[Gl2FuncName::GetShaderiv] = (void*)&glGetShaderiv;
		FunctionTable[Gl2FuncName::GetShaderInfoLog] = (void*)&glGetShaderInfoLog;
		FunctionTable[Gl2FuncName::GetShaderPrecisionFormat] = (void*)&glGetShaderPrecisionFormat;
		FunctionTable[Gl2FuncName::GetShaderSource] = (void*)&glGetShaderSource;
		FunctionTable[Gl2FuncName::GetString] = (void*)&glGetString;
		FunctionTable[Gl2FuncName::GetTexParameterfv] = (void*)&glGetTexParameterfv;
		FunctionTable[Gl2FuncName::GetTexParameteriv] = (void*)&glGetTexParameteriv;
		FunctionTable[Gl2FuncName::GetUniformfv] = (void*)&glGetUniformfv;
		FunctionTable[Gl2FuncName::GetUniformiv] = (void*)&glGetUniformiv;
		FunctionTable[Gl2FuncName::GetUniformLocation] = (void*)&glGetUniformLocation;
		FunctionTable[Gl2FuncName::GetVertexAttribfv] = (void*)&glGetVertexAttribfv;
		FunctionTable[Gl2FuncName::GetVertexAttribiv] = (void*)&glGetVertexAttribiv;
		FunctionTable[Gl2FuncName::GetVertexAttribPointerv] = (void*)&glGetVertexAttribPointerv;
		FunctionTable[Gl2FuncName::Hint] = (void*)&glHint;
		FunctionTable[Gl2FuncName::IsBuffer] = (void*)&glIsBuffer;
		FunctionTable[Gl2FuncName::IsEnabled] = (void*)&glIsEnabled;
		FunctionTable[Gl2FuncName::IsFramebuffer] = (void*)&glIsFramebuffer;
		FunctionTable[Gl2FuncName::IsProgram] = (void*)&glIsProgram;
		FunctionTable[Gl2FuncName::IsRenderbuffer] = (void*)&glIsRenderbuffer;
		FunctionTable[Gl2FuncName::IsShader] = (void*)&glIsShader;
		FunctionTable[Gl2FuncName::IsTexture] = (void*)&glIsTexture;
		FunctionTable[Gl2FuncName::LineWidth] = (void*)&glLineWidth;
		FunctionTable[Gl2FuncName::LinkProgram] = (void*)&glLinkProgram;
		FunctionTable[Gl2FuncName::PixelStorei] = (void*)&glPixelStorei;
		FunctionTable[Gl2FuncName::PolygonOffset] = (void*)&glPolygonOffset;
		FunctionTable[Gl2FuncName::ReadPixels] = (void*)&glReadPixels;
		FunctionTable[Gl2FuncName::ReleaseShaderCompiler] = (void*)&glReleaseShaderCompiler;
		FunctionTable[Gl2FuncName::RenderbufferStorage] = (void*)&glRenderbufferStorage;
		FunctionTable[Gl2FuncName::SampleCoverage] = (void*)&glSampleCoverage;
		FunctionTable[Gl2FuncName::Scissor] = (void*)&glScissor;
		FunctionTable[Gl2FuncName::ShaderBinary] = (void*)&glShaderBinary;
		FunctionTable[Gl2FuncName::ShaderSource] = (void*)&glShaderSource;
		FunctionTable[Gl2FuncName::StencilFunc] = (void*)&glStencilFunc;
		FunctionTable[Gl2FuncName::StencilFuncSeparate] = (void*)&glStencilFuncSeparate;
		FunctionTable[Gl2FuncName::StencilMask] = (void*)&glStencilMask;
		FunctionTable[Gl2FuncName::StencilMaskSeparate] = (void*)&glStencilMaskSeparate;
		FunctionTable[Gl2FuncName::StencilOp] = (void*)&glStencilOp;
		FunctionTable[Gl2FuncName::StencilOpSeparate] = (void*)&glStencilOpSeparate;
		FunctionTable[Gl2FuncName::TexImage2D] = (void*)&glTexImage2D;
		FunctionTable[Gl2FuncName::TexParameterf] = (void*)&glTexParameterf;
		FunctionTable[Gl2FuncName::TexParameterfv] = (void*)&glTexParameterfv;
		FunctionTable[Gl2FuncName::TexParameteri] = (void*)&glTexParameteri;
		FunctionTable[Gl2FuncName::TexParameteriv] = (void*)&glTexParameteriv;
		FunctionTable[Gl2FuncName::TexSubImage2D] = (void*)&glTexSubImage2D;
		FunctionTable[Gl2FuncName::Uniform1f] = (void*)&glUniform1f;
		FunctionTable[Gl2FuncName::Uniform1fv] = (void*)&glUniform1fv;
		FunctionTable[Gl2FuncName::Uniform1i] = (void*)&glUniform1i;
		FunctionTable[Gl2FuncName::Uniform1iv] = (void*)&glUniform1iv;
		FunctionTable[Gl2FuncName::Uniform2f] = (void*)&glUniform2f;
		FunctionTable[Gl2FuncName::Uniform2fv] = (void*)&glUniform2fv;
		FunctionTable[Gl2FuncName::Uniform2i] = (void*)&glUniform2i;
		FunctionTable[Gl2FuncName::Uniform2iv] = (void*)&glUniform2iv;
		FunctionTable[Gl2FuncName::Uniform3f] = (void*)&glUniform3f;
		FunctionTable[Gl2FuncName::Uniform3fv] = (void*)&glUniform3fv;
		FunctionTable[Gl2FuncName::Uniform3i] = (void*)&glUniform3i;
		FunctionTable[Gl2FuncName::Uniform3iv] = (void*)&glUniform3iv;
		FunctionTable[Gl2FuncName::Uniform4f] = (void*)&glUniform4f;
		FunctionTable[Gl2FuncName::Uniform4fv] = (void*)&glUniform4fv;
		FunctionTable[Gl2FuncName::Uniform4i] = (void*)&glUniform4i;
		FunctionTable[Gl2FuncName::Uniform4iv] = (void*)&glUniform4iv;
		FunctionTable[Gl2FuncName::UniformMatrix2fv] = (void*)&glUniformMatrix2fv;
		FunctionTable[Gl2FuncName::UniformMatrix3fv] = (void*)&glUniformMatrix3fv;
		FunctionTable[Gl2FuncName::UniformMatrix4fv] = (void*)&glUniformMatrix4fv;
		FunctionTable[Gl2FuncName::UseProgram] = (void*)&glUseProgram;
		FunctionTable[Gl2FuncName::ValidateProgram] = (void*)&glValidateProgram;
		FunctionTable[Gl2FuncName::VertexAttrib1f] = (void*)&glVertexAttrib1f;
		FunctionTable[Gl2FuncName::VertexAttrib1fv] = (void*)&glVertexAttrib1fv;
		FunctionTable[Gl2FuncName::VertexAttrib2f] = (void*)&glVertexAttrib2f;
		FunctionTable[Gl2FuncName::VertexAttrib2fv] = (void*)&glVertexAttrib2fv;
		FunctionTable[Gl2FuncName::VertexAttrib3f] = (void*)&glVertexAttrib3f;
		FunctionTable[Gl2FuncName::VertexAttrib3fv] = (void*)&glVertexAttrib3fv;
		FunctionTable[Gl2FuncName::VertexAttrib4f] = (void*)&glVertexAttrib4f;
		FunctionTable[Gl2FuncName::VertexAttrib4fv] = (void*)&glVertexAttrib4fv;
		FunctionTable[Gl2FuncName::VertexAttribPointer] = (void*)&glVertexAttribPointer;
		FunctionTable[Gl2FuncName::Viewport] = (void*)&glViewport;

#endif
	}
	return FunctionTable[funcname];
}
} // namespace internals
} // namespace gl

#ifndef DYNAMICGLES_NO_NAMESPACE
namespace gl {
#elif TARGET_OS_IPHONE
namespace gl {
namespace internals {
#endif

inline void DYNAMICGLES_FUNCTION(ActiveTexture)(GLenum texture)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glActiveTexture) PFNGLACTIVETEXTUREPROC;
#endif
	PFNGLACTIVETEXTUREPROC _ActiveTexture = (PFNGLACTIVETEXTUREPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::ActiveTexture);
	return _ActiveTexture(texture);
}

inline void DYNAMICGLES_FUNCTION(AttachShader)(GLuint program, GLuint shader)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glAttachShader) PFNGLATTACHSHADERPROC;
#endif
	PFNGLATTACHSHADERPROC _AttachShader = (PFNGLATTACHSHADERPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::AttachShader);
	return _AttachShader(program, shader);
}
inline void DYNAMICGLES_FUNCTION(BindAttribLocation)(GLuint program, GLuint index, const GLchar* name)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glBindAttribLocation) PFNGLBINDATTRIBLOCATIONPROC;
#endif
	PFNGLBINDATTRIBLOCATIONPROC _BindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::BindAttribLocation);
	return _BindAttribLocation(program, index, name);
}
inline void DYNAMICGLES_FUNCTION(BindBuffer)(GLenum target, GLuint buffer)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glBindBuffer) PFNGLBINDBUFFERPROC;
#endif
	PFNGLBINDBUFFERPROC _BindBuffer = (PFNGLBINDBUFFERPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::BindBuffer);
	return _BindBuffer(target, buffer);
}
inline void DYNAMICGLES_FUNCTION(BindFramebuffer)(GLenum target, GLuint framebuffer)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glBindFramebuffer) PFNGLBINDFRAMEBUFFERPROC;
#endif
	PFNGLBINDFRAMEBUFFERPROC _BindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::BindFramebuffer);
	return _BindFramebuffer(target, framebuffer);
}
inline void DYNAMICGLES_FUNCTION(BindRenderbuffer)(GLenum target, GLuint renderbuffer)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glBindRenderbuffer) PFNGLBINDRENDERBUFFERPROC;
#endif
	PFNGLBINDRENDERBUFFERPROC _BindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::BindRenderbuffer);
	return _BindRenderbuffer(target, renderbuffer);
}
inline void DYNAMICGLES_FUNCTION(BindTexture)(GLenum target, GLuint texture)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glBindTexture) PFNGLBINDTEXTUREPROC;
#endif
	PFNGLBINDTEXTUREPROC _BindTexture = (PFNGLBINDTEXTUREPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::BindTexture);
	return _BindTexture(target, texture);
}
inline void DYNAMICGLES_FUNCTION(BlendColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glBlendColor) PFNGLBLENDCOLORPROC;
#endif
	PFNGLBLENDCOLORPROC _BlendColor = (PFNGLBLENDCOLORPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::BlendColor);
	return _BlendColor(red, green, blue, alpha);
}
inline void DYNAMICGLES_FUNCTION(BlendEquation)(GLenum mode)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glBlendEquation) PFNGLBLENDEQUATIONPROC;
#endif
	PFNGLBLENDEQUATIONPROC _BlendEquation = (PFNGLBLENDEQUATIONPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::BlendEquation);
	return _BlendEquation(mode);
}
inline void DYNAMICGLES_FUNCTION(BlendEquationSeparate)(GLenum modeRGB, GLenum modeAlpha)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glBlendEquationSeparate) PFNGLBLENDEQUATIONSEPARATEPROC;
#endif
	PFNGLBLENDEQUATIONSEPARATEPROC _BlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATEPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::BlendEquationSeparate);
	return _BlendEquationSeparate(modeRGB, modeAlpha);
}
inline void DYNAMICGLES_FUNCTION(BlendFunc)(GLenum sfactor, GLenum dfactor)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glBlendFunc) PFNGLBLENDFUNCPROC;
#endif
	PFNGLBLENDFUNCPROC _BlendFunc = (PFNGLBLENDFUNCPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::BlendFunc);
	return _BlendFunc(sfactor, dfactor);
}
inline void DYNAMICGLES_FUNCTION(BlendFuncSeparate)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glBlendFuncSeparate) PFNGLBLENDFUNCSEPARATEPROC;
#endif
	PFNGLBLENDFUNCSEPARATEPROC _BlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::BlendFuncSeparate);
	return _BlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
}
inline void DYNAMICGLES_FUNCTION(BufferData)(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glBufferData) PFNGLBUFFERDATAPROC;
#endif
	PFNGLBUFFERDATAPROC _BufferData = (PFNGLBUFFERDATAPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::BufferData);
	return _BufferData(target, size, data, usage);
}
inline void DYNAMICGLES_FUNCTION(BufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const void* data)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glBufferSubData) PFNGLBUFFERSUBDATAPROC;
#endif
	PFNGLBUFFERSUBDATAPROC _BufferSubData = (PFNGLBUFFERSUBDATAPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::BufferSubData);
	return _BufferSubData(target, offset, size, data);
}
inline GLenum DYNAMICGLES_FUNCTION(CheckFramebufferStatus)(GLenum target)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glCheckFramebufferStatus) PFNGLCHECKFRAMEBUFFERSTATUSPROC;
#endif
	PFNGLCHECKFRAMEBUFFERSTATUSPROC _CheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::CheckFramebufferStatus);
	return _CheckFramebufferStatus(target);
}
inline void DYNAMICGLES_FUNCTION(Clear)(GLbitfield mask)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glClear) PFNGLCLEARPROC;
#endif
	PFNGLCLEARPROC _Clear = (PFNGLCLEARPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::Clear);
	return _Clear(mask);
}
inline void DYNAMICGLES_FUNCTION(ClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glClearColor) PFNGLCLEARCOLORPROC;
#endif
	PFNGLCLEARCOLORPROC _ClearColor = (PFNGLCLEARCOLORPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::ClearColor);
	return _ClearColor(red, green, blue, alpha);
}
inline void DYNAMICGLES_FUNCTION(ClearDepthf)(GLfloat d)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glClearDepthf) PFNGLCLEARDEPTHFPROC;
#endif
	PFNGLCLEARDEPTHFPROC _ClearDepthf = (PFNGLCLEARDEPTHFPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::ClearDepthf);
	return _ClearDepthf(d);
}
inline void DYNAMICGLES_FUNCTION(ClearStencil)(GLint s)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glClearStencil) PFNGLCLEARSTENCILPROC;
#endif
	PFNGLCLEARSTENCILPROC _ClearStencil = (PFNGLCLEARSTENCILPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::ClearStencil);
	return _ClearStencil(s);
}
inline void DYNAMICGLES_FUNCTION(ColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glColorMask) PFNGLCOLORMASKPROC;
#endif
	PFNGLCOLORMASKPROC _ColorMask = (PFNGLCOLORMASKPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::ColorMask);
	return _ColorMask(red, green, blue, alpha);
}
inline void DYNAMICGLES_FUNCTION(CompileShader)(GLuint shader)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glCompileShader) PFNGLCOMPILESHADERPROC;
#endif
	PFNGLCOMPILESHADERPROC _CompileShader = (PFNGLCOMPILESHADERPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::CompileShader);
	return _CompileShader(shader);
}
inline void DYNAMICGLES_FUNCTION(CompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glCompressedTexImage2D) PFNGLCOMPRESSEDTEXIMAGE2DPROC;
#endif
	PFNGLCOMPRESSEDTEXIMAGE2DPROC _CompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::CompressedTexImage2D);
	return _CompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
}
inline void DYNAMICGLES_FUNCTION(CompressedTexSubImage2D)(
	GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glCompressedTexSubImage2D) PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC;
#endif
	PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC _CompressedTexSubImage2D = (PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::CompressedTexSubImage2D);
	return _CompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
}
inline void DYNAMICGLES_FUNCTION(CopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glCopyTexImage2D) PFNGLCOPYTEXIMAGE2DPROC;
#endif
	PFNGLCOPYTEXIMAGE2DPROC _CopyTexImage2D = (PFNGLCOPYTEXIMAGE2DPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::CopyTexImage2D);
	return _CopyTexImage2D(target, level, internalformat, x, y, width, height, border);
}
inline void DYNAMICGLES_FUNCTION(CopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glCopyTexSubImage2D) PFNGLCOPYTEXSUBIMAGE2DPROC;
#endif
	PFNGLCOPYTEXSUBIMAGE2DPROC _CopyTexSubImage2D = (PFNGLCOPYTEXSUBIMAGE2DPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::CopyTexSubImage2D);
	return _CopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
}
inline GLenum DYNAMICGLES_FUNCTION(CreateProgram)()
{
#if TARGET_OS_IPHONE
	typedef decltype(&glCreateProgram) PFNGLCREATEPROGRAMPROC;
#endif
	PFNGLCREATEPROGRAMPROC _CreateProgram = (PFNGLCREATEPROGRAMPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::CreateProgram);
	return _CreateProgram();
}
inline GLenum DYNAMICGLES_FUNCTION(CreateShader)(GLenum target)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glCreateShader) PFNGLCREATESHADERPROC;
#endif
	PFNGLCREATESHADERPROC _CreateShader = (PFNGLCREATESHADERPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::CreateShader);
	return _CreateShader(target);
}
inline void DYNAMICGLES_FUNCTION(CullFace)(GLenum mode)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glCullFace) PFNGLCULLFACEPROC;
#endif
	PFNGLCULLFACEPROC _CullFace = (PFNGLCULLFACEPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::CullFace);
	return _CullFace(mode);
}
inline void DYNAMICGLES_FUNCTION(DeleteBuffers)(GLsizei n, const GLuint* buffers)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glDeleteBuffers) PFNGLDELETEBUFFERSPROC;
#endif
	PFNGLDELETEBUFFERSPROC _DeleteBuffers = (PFNGLDELETEBUFFERSPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::DeleteBuffers);
	return _DeleteBuffers(n, buffers);
}
inline void DYNAMICGLES_FUNCTION(DeleteFramebuffers)(GLsizei n, const GLuint* framebuffers)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glDeleteFramebuffers) PFNGLDELETEFRAMEBUFFERSPROC;
#endif
	PFNGLDELETEFRAMEBUFFERSPROC _DeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::DeleteFramebuffers);
	return _DeleteFramebuffers(n, framebuffers);
}
inline void DYNAMICGLES_FUNCTION(DeleteProgram)(GLuint program)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glDeleteProgram) PFNGLDELETEPROGRAMPROC;
#endif
	PFNGLDELETEPROGRAMPROC _DeleteProgram = (PFNGLDELETEPROGRAMPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::DeleteProgram);
	return _DeleteProgram(program);
}
inline void DYNAMICGLES_FUNCTION(DeleteRenderbuffers)(GLsizei n, const GLuint* renderbuffers)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glDeleteRenderbuffers) PFNGLDELETERENDERBUFFERSPROC;
#endif
	PFNGLDELETERENDERBUFFERSPROC _DeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::DeleteRenderbuffers);
	return _DeleteRenderbuffers(n, renderbuffers);
}
inline void DYNAMICGLES_FUNCTION(DeleteShader)(GLuint shader)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glDeleteShader) PFNGLDELETESHADERPROC;
#endif
	PFNGLDELETESHADERPROC _DeleteShader = (PFNGLDELETESHADERPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::DeleteShader);
	return _DeleteShader(shader);
}
inline void DYNAMICGLES_FUNCTION(DeleteTextures)(GLsizei n, const GLuint* textures)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glDeleteTextures) PFNGLDELETETEXTURESPROC;
#endif
	PFNGLDELETETEXTURESPROC _DeleteTextures = (PFNGLDELETETEXTURESPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::DeleteTextures);
	return _DeleteTextures(n, textures);
}
inline void DYNAMICGLES_FUNCTION(DepthFunc)(GLenum func)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glDepthFunc) PFNGLDEPTHFUNCPROC;
#endif
	PFNGLDEPTHFUNCPROC _DepthFunc = (PFNGLDEPTHFUNCPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::DepthFunc);
	return _DepthFunc(func);
}
inline void DYNAMICGLES_FUNCTION(DepthMask)(GLboolean flag)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glDepthMask) PFNGLDEPTHMASKPROC;
#endif
	PFNGLDEPTHMASKPROC _DepthMask = (PFNGLDEPTHMASKPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::DepthMask);
	return _DepthMask(flag);
}
inline void DYNAMICGLES_FUNCTION(DepthRangef)(GLfloat n, GLfloat f)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glDepthRangef) PFNGLDEPTHRANGEFPROC;
#endif
	PFNGLDEPTHRANGEFPROC _DepthRangef = (PFNGLDEPTHRANGEFPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::DepthRangef);
	return _DepthRangef(n, f);
}
inline void DYNAMICGLES_FUNCTION(DetachShader)(GLuint program, GLuint shader)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glDetachShader) PFNGLDETACHSHADERPROC;
#endif
	PFNGLDETACHSHADERPROC _DetachShader = (PFNGLDETACHSHADERPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::DetachShader);
	return _DetachShader(program, shader);
}
inline void DYNAMICGLES_FUNCTION(Disable)(GLenum cap)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glDisable) PFNGLDISABLEPROC;
#endif
	PFNGLDISABLEPROC _Disable = (PFNGLDISABLEPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::Disable);
	return _Disable(cap);
}
inline void DYNAMICGLES_FUNCTION(DisableVertexAttribArray)(GLuint index)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glDisableVertexAttribArray) PFNGLDISABLEVERTEXATTRIBARRAYPROC;
#endif
	PFNGLDISABLEVERTEXATTRIBARRAYPROC _DisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::DisableVertexAttribArray);
	return _DisableVertexAttribArray(index);
}
inline void DYNAMICGLES_FUNCTION(DrawArrays)(GLenum mode, GLint first, GLsizei count)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glDrawArrays) PFNGLDRAWARRAYSPROC;
#endif
	PFNGLDRAWARRAYSPROC _DrawArrays = (PFNGLDRAWARRAYSPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::DrawArrays);
	return _DrawArrays(mode, first, count);
}
inline void DYNAMICGLES_FUNCTION(DrawElements)(GLenum mode, GLsizei count, GLenum type, const void* indices)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glDrawElements) PFNGLDRAWELEMENTSPROC;
#endif
	PFNGLDRAWELEMENTSPROC _DrawElements = (PFNGLDRAWELEMENTSPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::DrawElements);
	return _DrawElements(mode, count, type, indices);
}
inline void DYNAMICGLES_FUNCTION(Enable)(GLenum cap)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glEnable) PFNGLENABLEPROC;
#endif
	PFNGLENABLEPROC _Enable = (PFNGLENABLEPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::Enable);
	return _Enable(cap);
}
inline void DYNAMICGLES_FUNCTION(EnableVertexAttribArray)(GLuint index)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glEnableVertexAttribArray) PFNGLENABLEVERTEXATTRIBARRAYPROC;
#endif
	PFNGLENABLEVERTEXATTRIBARRAYPROC _EnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::EnableVertexAttribArray);
	return _EnableVertexAttribArray(index);
}
inline void DYNAMICGLES_FUNCTION(Finish)(void)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glFinish) PFNGLFINISHPROC;
#endif
	PFNGLFINISHPROC _Finish = (PFNGLFINISHPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::Finish);
	return _Finish();
}
inline void DYNAMICGLES_FUNCTION(Flush)(void)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glFlush) PFNGLFLUSHPROC;
#endif
	PFNGLFLUSHPROC _Flush = (PFNGLFLUSHPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::Flush);
	return _Flush();
}
inline void DYNAMICGLES_FUNCTION(FramebufferRenderbuffer)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glFramebufferRenderbuffer) PFNGLFRAMEBUFFERRENDERBUFFERPROC;
#endif
	PFNGLFRAMEBUFFERRENDERBUFFERPROC _FramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::FramebufferRenderbuffer);
	return _FramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
}
inline void DYNAMICGLES_FUNCTION(FramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glFramebufferTexture2D) PFNGLFRAMEBUFFERTEXTURE2DPROC;
#endif
	PFNGLFRAMEBUFFERTEXTURE2DPROC _FramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::FramebufferTexture2D);
	return _FramebufferTexture2D(target, attachment, textarget, texture, level);
}
inline void DYNAMICGLES_FUNCTION(FrontFace)(GLenum mode)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glFrontFace) PFNGLFRONTFACEPROC;
#endif
	PFNGLFRONTFACEPROC _FrontFace = (PFNGLFRONTFACEPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::FrontFace);
	return _FrontFace(mode);
}
inline void DYNAMICGLES_FUNCTION(GenBuffers)(GLsizei n, GLuint* buffers)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGenBuffers) PFNGLGENBUFFERSPROC;
#endif
	PFNGLGENBUFFERSPROC _GenBuffers = (PFNGLGENBUFFERSPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GenBuffers);
	return _GenBuffers(n, buffers);
}
inline void DYNAMICGLES_FUNCTION(GenerateMipmap)(GLenum target)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGenerateMipmap) PFNGLGENERATEMIPMAPPROC;
#endif
	PFNGLGENERATEMIPMAPPROC _GenerateMipmap = (PFNGLGENERATEMIPMAPPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GenerateMipmap);
	return _GenerateMipmap(target);
}
inline void DYNAMICGLES_FUNCTION(GenFramebuffers)(GLsizei n, GLuint* framebuffers)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGenFramebuffers) PFNGLGENFRAMEBUFFERSPROC;
#endif
	PFNGLGENFRAMEBUFFERSPROC _GenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GenFramebuffers);
	return _GenFramebuffers(n, framebuffers);
}
inline void DYNAMICGLES_FUNCTION(GenRenderbuffers)(GLsizei n, GLuint* renderbuffers)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGenRenderbuffers) PFNGLGENRENDERBUFFERSPROC;
#endif
	PFNGLGENRENDERBUFFERSPROC _GenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GenRenderbuffers);
	return _GenRenderbuffers(n, renderbuffers);
}
inline void DYNAMICGLES_FUNCTION(GenTextures)(GLsizei n, GLuint* textures)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGenTextures) PFNGLGENTEXTURESPROC;
#endif
	PFNGLGENTEXTURESPROC _GenTextures = (PFNGLGENTEXTURESPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GenTextures);
	return _GenTextures(n, textures);
}
inline void DYNAMICGLES_FUNCTION(GetActiveAttrib)(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetActiveAttrib) PFNGLGETACTIVEATTRIBPROC;
#endif
	PFNGLGETACTIVEATTRIBPROC _GetActiveAttrib = (PFNGLGETACTIVEATTRIBPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GetActiveAttrib);
	return _GetActiveAttrib(program, index, bufSize, length, size, type, name);
}
inline void DYNAMICGLES_FUNCTION(GetActiveUniform)(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetActiveUniform) PFNGLGETACTIVEUNIFORMPROC;
#endif
	PFNGLGETACTIVEUNIFORMPROC _GetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GetActiveUniform);
	return _GetActiveUniform(program, index, bufSize, length, size, type, name);
}
inline void DYNAMICGLES_FUNCTION(GetAttachedShaders)(GLuint program, GLsizei maxCount, GLsizei* count, GLuint* shaders)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetAttachedShaders) PFNGLGETATTACHEDSHADERSPROC;
#endif
	PFNGLGETATTACHEDSHADERSPROC _GetAttachedShaders = (PFNGLGETATTACHEDSHADERSPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GetAttachedShaders);
	return _GetAttachedShaders(program, maxCount, count, shaders);
}
inline GLint DYNAMICGLES_FUNCTION(GetAttribLocation)(GLuint program, const GLchar* name)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetAttribLocation) PFNGLGETATTRIBLOCATIONPROC;
#endif
	PFNGLGETATTRIBLOCATIONPROC _GetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GetAttribLocation);
	return _GetAttribLocation(program, name);
}
inline void DYNAMICGLES_FUNCTION(GetBooleanv)(GLenum pname, GLboolean* data)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetBooleanv) PFNGLGETBOOLEANVPROC;
#endif
	PFNGLGETBOOLEANVPROC _GetBooleanv = (PFNGLGETBOOLEANVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GetBooleanv);
	return _GetBooleanv(pname, data);
}
inline void DYNAMICGLES_FUNCTION(GetBufferParameteriv)(GLenum target, GLenum pname, GLint* params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetBufferParameteriv) PFNGLGETBUFFERPARAMETERIVPROC;
#endif
	PFNGLGETBUFFERPARAMETERIVPROC _GetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GetBufferParameteriv);
	return _GetBufferParameteriv(target, pname, params);
}
inline GLenum DYNAMICGLES_FUNCTION(GetError)(void)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetError) PFNGLGETERRORPROC;
#endif
	PFNGLGETERRORPROC _GetError = (PFNGLGETERRORPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GetError);
	return _GetError();
}
inline void DYNAMICGLES_FUNCTION(GetFloatv)(GLenum pname, GLfloat* data)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetFloatv) PFNGLGETFLOATVPROC;
#endif
	PFNGLGETFLOATVPROC _GetFloatv = (PFNGLGETFLOATVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GetFloatv);
	return _GetFloatv(pname, data);
}
inline void DYNAMICGLES_FUNCTION(GetFramebufferAttachmentParameteriv)(GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetFramebufferAttachmentParameteriv) PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC;
#endif
	PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC _GetFramebufferAttachmentParameteriv =
		(PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GetFramebufferAttachmentParameteriv);
	return _GetFramebufferAttachmentParameteriv(target, attachment, pname, params);
}
inline void DYNAMICGLES_FUNCTION(GetIntegerv)(GLenum pname, GLint* data)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetIntegerv) PFNGLGETINTEGERVPROC;
#endif
	PFNGLGETINTEGERVPROC _GetIntegerv = (PFNGLGETINTEGERVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GetIntegerv);
	return _GetIntegerv(pname, data);
}
inline void DYNAMICGLES_FUNCTION(GetProgramiv)(GLuint program, GLenum pname, GLint* params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetProgramiv) PFNGLGETPROGRAMIVPROC;
#endif
	PFNGLGETPROGRAMIVPROC _GetProgramiv = (PFNGLGETPROGRAMIVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GetProgramiv);
	return _GetProgramiv(program, pname, params);
}
inline void DYNAMICGLES_FUNCTION(GetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetProgramInfoLog) PFNGLGETPROGRAMINFOLOGPROC;
#endif
	PFNGLGETPROGRAMINFOLOGPROC _GetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GetProgramInfoLog);
	return _GetProgramInfoLog(program, bufSize, length, infoLog);
}
inline void DYNAMICGLES_FUNCTION(GetRenderbufferParameteriv)(GLenum target, GLenum pname, GLint* params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetRenderbufferParameteriv) PFNGLGETRENDERBUFFERPARAMETERIVPROC;
#endif
	PFNGLGETRENDERBUFFERPARAMETERIVPROC _GetRenderbufferParameteriv =
		(PFNGLGETRENDERBUFFERPARAMETERIVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GetRenderbufferParameteriv);
	return _GetRenderbufferParameteriv(target, pname, params);
}
inline void DYNAMICGLES_FUNCTION(GetShaderiv)(GLuint shader, GLenum pname, GLint* params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetShaderiv) PFNGLGETSHADERIVPROC;
#endif
	PFNGLGETSHADERIVPROC _GetShaderiv = (PFNGLGETSHADERIVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GetShaderiv);
	return _GetShaderiv(shader, pname, params);
}
inline void DYNAMICGLES_FUNCTION(GetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetShaderInfoLog) PFNGLGETSHADERINFOLOGPROC;
#endif
	PFNGLGETSHADERINFOLOGPROC _GetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GetShaderInfoLog);
	return _GetShaderInfoLog(shader, bufSize, length, infoLog);
}
inline void DYNAMICGLES_FUNCTION(GetShaderPrecisionFormat)(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetShaderPrecisionFormat) PFNGLGETSHADERPRECISIONFORMATPROC;
#endif
	PFNGLGETSHADERPRECISIONFORMATPROC _GetShaderPrecisionFormat = (PFNGLGETSHADERPRECISIONFORMATPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GetShaderPrecisionFormat);
	return _GetShaderPrecisionFormat(shadertype, precisiontype, range, precision);
}
inline void DYNAMICGLES_FUNCTION(GetShaderSource)(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* source)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetShaderSource) PFNGLGETSHADERSOURCEPROC;
#endif
	PFNGLGETSHADERSOURCEPROC _GetShaderSource = (PFNGLGETSHADERSOURCEPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GetShaderSource);
	return _GetShaderSource(shader, bufSize, length, source);
}
inline const GLubyte* DYNAMICGLES_FUNCTION(GetString)(GLenum name)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetString) PFNGLGETSTRINGPROC;
#endif
	PFNGLGETSTRINGPROC _GetString = (PFNGLGETSTRINGPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GetString);
	return _GetString(name);
}
inline void DYNAMICGLES_FUNCTION(GetTexParameterfv)(GLenum target, GLenum pname, GLfloat* params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetTexParameterfv) PFNGLGETTEXPARAMETERFVPROC;
#endif
	PFNGLGETTEXPARAMETERFVPROC _GetTexParameterfv = (PFNGLGETTEXPARAMETERFVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GetTexParameterfv);
	return _GetTexParameterfv(target, pname, params);
}
inline void DYNAMICGLES_FUNCTION(GetTexParameteriv)(GLenum target, GLenum pname, GLint* params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetTexParameteriv) PFNGLGETTEXPARAMETERIVPROC;
#endif
	PFNGLGETTEXPARAMETERIVPROC _GetTexParameteriv = (PFNGLGETTEXPARAMETERIVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GetTexParameteriv);
	return _GetTexParameteriv(target, pname, params);
}
inline void DYNAMICGLES_FUNCTION(GetUniformfv)(GLuint program, GLint location, GLfloat* params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetUniformfv) PFNGLGETUNIFORMFVPROC;
#endif
	PFNGLGETUNIFORMFVPROC _GetUniformfv = (PFNGLGETUNIFORMFVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GetUniformfv);
	return _GetUniformfv(program, location, params);
}
inline void DYNAMICGLES_FUNCTION(GetUniformiv)(GLuint program, GLint location, GLint* params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetUniformiv) PFNGLGETUNIFORMIVPROC;
#endif
	PFNGLGETUNIFORMIVPROC _GetUniformiv = (PFNGLGETUNIFORMIVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GetUniformiv);
	return _GetUniformiv(program, location, params);
}
inline GLint DYNAMICGLES_FUNCTION(GetUniformLocation)(GLuint program, const GLchar* name)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetUniformLocation) PFNGLGETUNIFORMLOCATIONPROC;
#endif
	PFNGLGETUNIFORMLOCATIONPROC _GetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GetUniformLocation);
	return _GetUniformLocation(program, name);
}
inline void DYNAMICGLES_FUNCTION(GetVertexAttribfv)(GLuint index, GLenum pname, GLfloat* params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetVertexAttribfv) PFNGLGETVERTEXATTRIBFVPROC;
#endif
	PFNGLGETVERTEXATTRIBFVPROC _GetVertexAttribfv = (PFNGLGETVERTEXATTRIBFVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GetVertexAttribfv);
	return _GetVertexAttribfv(index, pname, params);
}
inline void DYNAMICGLES_FUNCTION(GetVertexAttribiv)(GLuint index, GLenum pname, GLint* params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetVertexAttribiv) PFNGLGETVERTEXATTRIBIVPROC;
#endif
	PFNGLGETVERTEXATTRIBIVPROC _GetVertexAttribiv = (PFNGLGETVERTEXATTRIBIVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GetVertexAttribiv);
	return _GetVertexAttribiv(index, pname, params);
}
inline void DYNAMICGLES_FUNCTION(GetVertexAttribPointerv)(GLuint index, GLenum pname, void** pointer)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetVertexAttribPointerv) PFNGLGETVERTEXATTRIBPOINTERVPROC;
#endif
	PFNGLGETVERTEXATTRIBPOINTERVPROC _GetVertexAttribPointerv = (PFNGLGETVERTEXATTRIBPOINTERVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::GetVertexAttribPointerv);
	return _GetVertexAttribPointerv(index, pname, pointer);
}
inline void DYNAMICGLES_FUNCTION(Hint)(GLenum target, GLenum mode)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glHint) PFNGLHINTPROC;
#endif
	PFNGLHINTPROC _Hint = (PFNGLHINTPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::Hint);
	return _Hint(target, mode);
}
inline GLboolean DYNAMICGLES_FUNCTION(IsBuffer)(GLuint buffer)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glIsBuffer) PFNGLISBUFFERPROC;
#endif
	PFNGLISBUFFERPROC _IsBuffer = (PFNGLISBUFFERPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::IsBuffer);
	return _IsBuffer(buffer);
}
inline GLboolean DYNAMICGLES_FUNCTION(IsEnabled)(GLenum cap)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glIsEnabled) PFNGLISENABLEDPROC;
#endif
	PFNGLISENABLEDPROC _IsEnabled = (PFNGLISENABLEDPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::IsEnabled);
	return _IsEnabled(cap);
}
inline GLboolean DYNAMICGLES_FUNCTION(IsFramebuffer)(GLuint framebuffer)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glIsFramebuffer) PFNGLISFRAMEBUFFERPROC;
#endif
	PFNGLISFRAMEBUFFERPROC _IsFramebuffer = (PFNGLISFRAMEBUFFERPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::IsFramebuffer);
	return _IsFramebuffer(framebuffer);
}
inline GLboolean DYNAMICGLES_FUNCTION(IsProgram)(GLuint program)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glIsProgram) PFNGLISPROGRAMPROC;
#endif
	PFNGLISPROGRAMPROC _IsProgram = (PFNGLISPROGRAMPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::IsProgram);
	return _IsProgram(program);
}
inline GLboolean DYNAMICGLES_FUNCTION(IsRenderbuffer)(GLuint renderbuffer)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glIsRenderbuffer) PFNGLISRENDERBUFFERPROC;
#endif
	PFNGLISRENDERBUFFERPROC _IsRenderbuffer = (PFNGLISRENDERBUFFERPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::IsRenderbuffer);
	return _IsRenderbuffer(renderbuffer);
}
inline GLboolean DYNAMICGLES_FUNCTION(IsShader)(GLuint shader)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glIsShader) PFNGLISSHADERPROC;
#endif
	PFNGLISSHADERPROC _IsShader = (PFNGLISSHADERPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::IsShader);
	return _IsShader(shader);
}
inline GLboolean DYNAMICGLES_FUNCTION(IsTexture)(GLuint texture)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glIsTexture) PFNGLISTEXTUREPROC;
#endif
	PFNGLISTEXTUREPROC _IsTexture = (PFNGLISTEXTUREPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::IsTexture);
	return _IsTexture(texture);
}
inline void DYNAMICGLES_FUNCTION(LineWidth)(GLfloat width)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glLineWidth) PFNGLLINEWIDTHPROC;
#endif
	PFNGLLINEWIDTHPROC _LineWidth = (PFNGLLINEWIDTHPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::LineWidth);
	return _LineWidth(width);
}
inline void DYNAMICGLES_FUNCTION(LinkProgram)(GLuint program)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glLinkProgram) PFNGLLINKPROGRAMPROC;
#endif
	PFNGLLINKPROGRAMPROC _LinkProgram = (PFNGLLINKPROGRAMPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::LinkProgram);
	return _LinkProgram(program);
}
inline void DYNAMICGLES_FUNCTION(PixelStorei)(GLenum pname, GLint param)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glPixelStorei) PFNGLPIXELSTOREIPROC;
#endif
	PFNGLPIXELSTOREIPROC _PixelStorei = (PFNGLPIXELSTOREIPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::PixelStorei);
	return _PixelStorei(pname, param);
}
inline void DYNAMICGLES_FUNCTION(PolygonOffset)(GLfloat factor, GLfloat units)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glPolygonOffset) PFNGLPOLYGONOFFSETPROC;
#endif
	PFNGLPOLYGONOFFSETPROC _PolygonOffset = (PFNGLPOLYGONOFFSETPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::PolygonOffset);
	return _PolygonOffset(factor, units);
}
inline void DYNAMICGLES_FUNCTION(ReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glReadPixels) PFNGLREADPIXELSPROC;
#endif
	PFNGLREADPIXELSPROC _ReadPixels = (PFNGLREADPIXELSPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::ReadPixels);
	return _ReadPixels(x, y, width, height, format, type, pixels);
}
inline void DYNAMICGLES_FUNCTION(ReleaseShaderCompiler)(void)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glReleaseShaderCompiler) PFNGLRELEASESHADERCOMPILERPROC;
#endif
	PFNGLRELEASESHADERCOMPILERPROC _ReleaseShaderCompiler = (PFNGLRELEASESHADERCOMPILERPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::ReleaseShaderCompiler);
	return _ReleaseShaderCompiler();
}
inline void DYNAMICGLES_FUNCTION(RenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glRenderbufferStorage) PFNGLRENDERBUFFERSTORAGEPROC;
#endif
	PFNGLRENDERBUFFERSTORAGEPROC _RenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::RenderbufferStorage);
	return _RenderbufferStorage(target, internalformat, width, height);
}
inline void DYNAMICGLES_FUNCTION(SampleCoverage)(GLfloat value, GLboolean invert)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glSampleCoverage) PFNGLSAMPLECOVERAGEPROC;
#endif
	PFNGLSAMPLECOVERAGEPROC _SampleCoverage = (PFNGLSAMPLECOVERAGEPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::SampleCoverage);
	return _SampleCoverage(value, invert);
}
inline void DYNAMICGLES_FUNCTION(Scissor)(GLint x, GLint y, GLsizei width, GLsizei height)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glScissor) PFNGLSCISSORPROC;
#endif
	PFNGLSCISSORPROC _Scissor = (PFNGLSCISSORPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::Scissor);
	return _Scissor(x, y, width, height);
}
inline void DYNAMICGLES_FUNCTION(ShaderBinary)(GLsizei count, const GLuint* shaders, GLenum binaryformat, const void* binary, GLsizei length)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glShaderBinary) PFNGLSHADERBINARYPROC;
#endif
	PFNGLSHADERBINARYPROC _ShaderBinary = (PFNGLSHADERBINARYPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::ShaderBinary);
	return _ShaderBinary(count, shaders, binaryformat, binary, length);
}
inline void DYNAMICGLES_FUNCTION(ShaderSource)(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glShaderSource) PFNGLSHADERSOURCEPROC;
#endif
	PFNGLSHADERSOURCEPROC _ShaderSource = (PFNGLSHADERSOURCEPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::ShaderSource);
	return _ShaderSource(shader, count, string, length);
}
inline void DYNAMICGLES_FUNCTION(StencilFunc)(GLenum func, GLint ref, GLuint mask)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glStencilFunc) PFNGLSTENCILFUNCPROC;
#endif
	PFNGLSTENCILFUNCPROC _StencilFunc = (PFNGLSTENCILFUNCPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::StencilFunc);
	return _StencilFunc(func, ref, mask);
}
inline void DYNAMICGLES_FUNCTION(StencilFuncSeparate)(GLenum face, GLenum func, GLint ref, GLuint mask)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glStencilFuncSeparate) PFNGLSTENCILFUNCSEPARATEPROC;
#endif
	PFNGLSTENCILFUNCSEPARATEPROC _StencilFuncSeparate = (PFNGLSTENCILFUNCSEPARATEPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::StencilFuncSeparate);
	return _StencilFuncSeparate(face, func, ref, mask);
}
inline void DYNAMICGLES_FUNCTION(StencilMask)(GLuint mask)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glStencilMask) PFNGLSTENCILMASKPROC;
#endif
	PFNGLSTENCILMASKPROC _StencilMask = (PFNGLSTENCILMASKPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::StencilMask);
	return _StencilMask(mask);
}
inline void DYNAMICGLES_FUNCTION(StencilMaskSeparate)(GLenum face, GLuint mask)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glStencilMaskSeparate) PFNGLSTENCILMASKSEPARATEPROC;
#endif
	PFNGLSTENCILMASKSEPARATEPROC _StencilMaskSeparate = (PFNGLSTENCILMASKSEPARATEPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::StencilMaskSeparate);
	return _StencilMaskSeparate(face, mask);
}
inline void DYNAMICGLES_FUNCTION(StencilOp)(GLenum fail, GLenum zfail, GLenum zpass)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glStencilOp) PFNGLSTENCILOPPROC;
#endif
	PFNGLSTENCILOPPROC _StencilOp = (PFNGLSTENCILOPPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::StencilOp);
	return _StencilOp(fail, zfail, zpass);
}
inline void DYNAMICGLES_FUNCTION(StencilOpSeparate)(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glStencilOpSeparate) PFNGLSTENCILOPSEPARATEPROC;
#endif
	PFNGLSTENCILOPSEPARATEPROC _StencilOpSeparate = (PFNGLSTENCILOPSEPARATEPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::StencilOpSeparate);
	return _StencilOpSeparate(face, sfail, dpfail, dppass);
}
inline void DYNAMICGLES_FUNCTION(TexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glTexImage2D) PFNGLTEXIMAGE2DPROC;
#endif
	PFNGLTEXIMAGE2DPROC _TexImage2D = (PFNGLTEXIMAGE2DPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::TexImage2D);
	return _TexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
}
inline void DYNAMICGLES_FUNCTION(TexParameterf)(GLenum target, GLenum pname, GLfloat param)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glTexParameterf) PFNGLTEXPARAMETERFPROC;
#endif
	PFNGLTEXPARAMETERFPROC _TexParameterf = (PFNGLTEXPARAMETERFPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::TexParameterf);
	return _TexParameterf(target, pname, param);
}
inline void DYNAMICGLES_FUNCTION(TexParameterfv)(GLenum target, GLenum pname, const GLfloat* params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glTexParameterfv) PFNGLTEXPARAMETERFVPROC;
#endif
	PFNGLTEXPARAMETERFVPROC _TexParameterfv = (PFNGLTEXPARAMETERFVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::TexParameterfv);
	return _TexParameterfv(target, pname, params);
}
inline void DYNAMICGLES_FUNCTION(TexParameteri)(GLenum target, GLenum pname, GLint param)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glTexParameteri) PFNGLTEXPARAMETERIPROC;
#endif
	PFNGLTEXPARAMETERIPROC _TexParameteri = (PFNGLTEXPARAMETERIPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::TexParameteri);
	return _TexParameteri(target, pname, param);
}
inline void DYNAMICGLES_FUNCTION(TexParameteriv)(GLenum target, GLenum pname, const GLint* params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glTexParameteriv) PFNGLTEXPARAMETERIVPROC;
#endif
	PFNGLTEXPARAMETERIVPROC _TexParameteriv = (PFNGLTEXPARAMETERIVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::TexParameteriv);
	return _TexParameteriv(target, pname, params);
}
inline void DYNAMICGLES_FUNCTION(TexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glTexSubImage2D) PFNGLTEXSUBIMAGE2DPROC;
#endif
	PFNGLTEXSUBIMAGE2DPROC _TexSubImage2D = (PFNGLTEXSUBIMAGE2DPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::TexSubImage2D);
	return _TexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}
inline void DYNAMICGLES_FUNCTION(Uniform1f)(GLint location, GLfloat v0)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniform1f) PFNGLUNIFORM1FPROC;
#endif
	PFNGLUNIFORM1FPROC _Uniform1f = (PFNGLUNIFORM1FPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::Uniform1f);
	return _Uniform1f(location, v0);
}
inline void DYNAMICGLES_FUNCTION(Uniform1fv)(GLint location, GLsizei count, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniform1fv) PFNGLUNIFORM1FVPROC;
#endif
	PFNGLUNIFORM1FVPROC _Uniform1fv = (PFNGLUNIFORM1FVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::Uniform1fv);
	return _Uniform1fv(location, count, value);
}
inline void DYNAMICGLES_FUNCTION(Uniform1i)(GLint location, GLint v0)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniform1i) PFNGLUNIFORM1IPROC;
#endif
	PFNGLUNIFORM1IPROC _Uniform1i = (PFNGLUNIFORM1IPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::Uniform1i);
	return _Uniform1i(location, v0);
}
inline void DYNAMICGLES_FUNCTION(Uniform1iv)(GLint location, GLsizei count, const GLint* value)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniform1iv) PFNGLUNIFORM1IVPROC;
#endif
	PFNGLUNIFORM1IVPROC _Uniform1iv = (PFNGLUNIFORM1IVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::Uniform1iv);
	return _Uniform1iv(location, count, value);
}
inline void DYNAMICGLES_FUNCTION(Uniform2f)(GLint location, GLfloat v0, GLfloat v1)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniform2f) PFNGLUNIFORM2FPROC;
#endif
	PFNGLUNIFORM2FPROC _Uniform2f = (PFNGLUNIFORM2FPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::Uniform2f);
	return _Uniform2f(location, v0, v1);
}
inline void DYNAMICGLES_FUNCTION(Uniform2fv)(GLint location, GLsizei count, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniform2fv) PFNGLUNIFORM2FVPROC;
#endif
	PFNGLUNIFORM2FVPROC _Uniform2fv = (PFNGLUNIFORM2FVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::Uniform2fv);
	return _Uniform2fv(location, count, value);
}
inline void DYNAMICGLES_FUNCTION(Uniform2i)(GLint location, GLint v0, GLint v1)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniform2i) PFNGLUNIFORM2IPROC;
#endif
	PFNGLUNIFORM2IPROC _Uniform2i = (PFNGLUNIFORM2IPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::Uniform2i);
	return _Uniform2i(location, v0, v1);
}
inline void DYNAMICGLES_FUNCTION(Uniform2iv)(GLint location, GLsizei count, const GLint* value)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniform2iv) PFNGLUNIFORM2IVPROC;
#endif
	PFNGLUNIFORM2IVPROC _Uniform2iv = (PFNGLUNIFORM2IVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::Uniform2iv);
	return _Uniform2iv(location, count, value);
}
inline void DYNAMICGLES_FUNCTION(Uniform3f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniform3f) PFNGLUNIFORM3FPROC;
#endif
	PFNGLUNIFORM3FPROC _Uniform3f = (PFNGLUNIFORM3FPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::Uniform3f);
	return _Uniform3f(location, v0, v1, v2);
}
inline void DYNAMICGLES_FUNCTION(Uniform3fv)(GLint location, GLsizei count, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniform3fv) PFNGLUNIFORM3FVPROC;
#endif
	PFNGLUNIFORM3FVPROC _Uniform3fv = (PFNGLUNIFORM3FVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::Uniform3fv);
	return _Uniform3fv(location, count, value);
}
inline void DYNAMICGLES_FUNCTION(Uniform3i)(GLint location, GLint v0, GLint v1, GLint v2)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniform3i) PFNGLUNIFORM3IPROC;
#endif
	PFNGLUNIFORM3IPROC _Uniform3i = (PFNGLUNIFORM3IPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::Uniform3i);
	return _Uniform3i(location, v0, v1, v2);
}
inline void DYNAMICGLES_FUNCTION(Uniform3iv)(GLint location, GLsizei count, const GLint* value)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniform3iv) PFNGLUNIFORM3IVPROC;
#endif
	PFNGLUNIFORM3IVPROC _Uniform3iv = (PFNGLUNIFORM3IVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::Uniform3iv);
	return _Uniform3iv(location, count, value);
}
inline void DYNAMICGLES_FUNCTION(Uniform4f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniform4f) PFNGLUNIFORM4FPROC;
#endif
	PFNGLUNIFORM4FPROC _Uniform4f = (PFNGLUNIFORM4FPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::Uniform4f);
	return _Uniform4f(location, v0, v1, v2, v3);
}
inline void DYNAMICGLES_FUNCTION(Uniform4fv)(GLint location, GLsizei count, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniform4fv) PFNGLUNIFORM4FVPROC;
#endif
	PFNGLUNIFORM4FVPROC _Uniform4fv = (PFNGLUNIFORM4FVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::Uniform4fv);
	return _Uniform4fv(location, count, value);
}
inline void DYNAMICGLES_FUNCTION(Uniform4i)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniform4i) PFNGLUNIFORM4IPROC;
#endif
	PFNGLUNIFORM4IPROC _Uniform4i = (PFNGLUNIFORM4IPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::Uniform4i);
	return _Uniform4i(location, v0, v1, v2, v3);
}
inline void DYNAMICGLES_FUNCTION(Uniform4iv)(GLint location, GLsizei count, const GLint* value)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniform4iv) PFNGLUNIFORM4IVPROC;
#endif
	PFNGLUNIFORM4IVPROC _Uniform4iv = (PFNGLUNIFORM4IVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::Uniform4iv);
	return _Uniform4iv(location, count, value);
}
inline void DYNAMICGLES_FUNCTION(UniformMatrix2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniformMatrix2fv) PFNGLUNIFORMMATRIX2FVPROC;
#endif
	PFNGLUNIFORMMATRIX2FVPROC _UniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::UniformMatrix2fv);
	return _UniformMatrix2fv(location, count, transpose, value);
}
inline void DYNAMICGLES_FUNCTION(UniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniformMatrix3fv) PFNGLUNIFORMMATRIX3FVPROC;
#endif
	PFNGLUNIFORMMATRIX3FVPROC _UniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::UniformMatrix3fv);
	return _UniformMatrix3fv(location, count, transpose, value);
}
inline void DYNAMICGLES_FUNCTION(UniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUniformMatrix4fv) PFNGLUNIFORMMATRIX4FVPROC;
#endif
	PFNGLUNIFORMMATRIX4FVPROC _UniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::UniformMatrix4fv);
	return _UniformMatrix4fv(location, count, transpose, value);
}
inline void DYNAMICGLES_FUNCTION(UseProgram)(GLuint program)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUseProgram) PFNGLUSEPROGRAMPROC;
#endif
	PFNGLUSEPROGRAMPROC _UseProgram = (PFNGLUSEPROGRAMPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::UseProgram);
	return _UseProgram(program);
}
inline void DYNAMICGLES_FUNCTION(ValidateProgram)(GLuint program)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glValidateProgram) PFNGLVALIDATEPROGRAMPROC;
#endif
	PFNGLVALIDATEPROGRAMPROC _ValidateProgram = (PFNGLVALIDATEPROGRAMPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::ValidateProgram);
	return _ValidateProgram(program);
}
inline void DYNAMICGLES_FUNCTION(VertexAttrib1f)(GLuint index, GLfloat x)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glVertexAttrib1f) PFNGLVERTEXATTRIB1FPROC;
#endif
	PFNGLVERTEXATTRIB1FPROC _VertexAttrib1f = (PFNGLVERTEXATTRIB1FPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::VertexAttrib1f);
	return _VertexAttrib1f(index, x);
}
inline void DYNAMICGLES_FUNCTION(VertexAttrib1fv)(GLuint index, const GLfloat* v)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glVertexAttrib1fv) PFNGLVERTEXATTRIB1FVPROC;
#endif
	PFNGLVERTEXATTRIB1FVPROC _VertexAttrib1fv = (PFNGLVERTEXATTRIB1FVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::VertexAttrib1fv);
	return _VertexAttrib1fv(index, v);
}
inline void DYNAMICGLES_FUNCTION(VertexAttrib2f)(GLuint index, GLfloat x, GLfloat y)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glVertexAttrib2f) PFNGLVERTEXATTRIB2FPROC;
#endif
	PFNGLVERTEXATTRIB2FPROC _VertexAttrib2f = (PFNGLVERTEXATTRIB2FPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::VertexAttrib2f);
	return _VertexAttrib2f(index, x, y);
}
inline void DYNAMICGLES_FUNCTION(VertexAttrib2fv)(GLuint index, const GLfloat* v)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glVertexAttrib2fv) PFNGLVERTEXATTRIB2FVPROC;
#endif
	PFNGLVERTEXATTRIB2FVPROC _VertexAttrib2fv = (PFNGLVERTEXATTRIB2FVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::VertexAttrib2fv);
	return _VertexAttrib2fv(index, v);
}
inline void DYNAMICGLES_FUNCTION(VertexAttrib3f)(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glVertexAttrib3f) PFNGLVERTEXATTRIB3FPROC;
#endif
	PFNGLVERTEXATTRIB3FPROC _VertexAttrib3f = (PFNGLVERTEXATTRIB3FPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::VertexAttrib3f);
	return _VertexAttrib3f(index, x, y, z);
}
inline void DYNAMICGLES_FUNCTION(VertexAttrib3fv)(GLuint index, const GLfloat* v)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glVertexAttrib3fv) PFNGLVERTEXATTRIB3FVPROC;
#endif
	PFNGLVERTEXATTRIB3FVPROC _VertexAttrib3fv = (PFNGLVERTEXATTRIB3FVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::VertexAttrib3fv);
	return _VertexAttrib3fv(index, v);
}
inline void DYNAMICGLES_FUNCTION(VertexAttrib4f)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glVertexAttrib4f) PFNGLVERTEXATTRIB4FPROC;
#endif
	PFNGLVERTEXATTRIB4FPROC _VertexAttrib4f = (PFNGLVERTEXATTRIB4FPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::VertexAttrib4f);
	return _VertexAttrib4f(index, x, y, z, w);
}
inline void DYNAMICGLES_FUNCTION(VertexAttrib4fv)(GLuint index, const GLfloat* v)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glVertexAttrib4fv) PFNGLVERTEXATTRIB4FVPROC;
#endif
	PFNGLVERTEXATTRIB4FVPROC _VertexAttrib4fv = (PFNGLVERTEXATTRIB4FVPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::VertexAttrib4fv);
	return _VertexAttrib4fv(index, v);
}
inline void DYNAMICGLES_FUNCTION(VertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glVertexAttribPointer) PFNGLVERTEXATTRIBPOINTERPROC;
#endif
	PFNGLVERTEXATTRIBPOINTERPROC _VertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::VertexAttribPointer);
	return _VertexAttribPointer(index, size, type, normalized, stride, pointer);
}
inline void DYNAMICGLES_FUNCTION(Viewport)(GLint x, GLint y, GLsizei width, GLsizei height)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glViewport) PFNGLVIEWPORTPROC;
#endif
	PFNGLVIEWPORTPROC _Viewport = (PFNGLVIEWPORTPROC)gl::internals::getEs2Function(gl::internals::Gl2FuncName::Viewport);
	return _Viewport(x, y, width, height);
}
#ifndef DYNAMICGLES_NO_NAMESPACE
}
#elif TARGET_OS_IPHONE
}
}
#endif

namespace gl {
namespace internals {
namespace GlExtFuncName {
enum OpenGLESExtFunctionName
{
	// Extensions
	MultiDrawArraysEXT,
	MultiDrawElementsEXT,
	DiscardFramebufferEXT,
	MapBufferOES,
	UnmapBufferOES,
	GetBufferPointervOES,
	BindVertexArrayOES,
	DeleteVertexArraysOES,
	GenVertexArraysOES,
	IsVertexArrayOES,
	DeleteFencesNV,
	GenFencesNV,
	IsFenceNV,
	TestFenceNV,
	GetFenceivNV,
	FinishFenceNV,
	SetFenceNV,
#if !TARGET_OS_IPHONE
	EGLImageTargetTexture2DOES,
	EGLImageTargetRenderbufferStorageOES,
#endif
	RenderbufferStorageMultisampleIMG,
	FramebufferTexture2DMultisampleIMG,
	GetPerfMonitorGroupsAMD,
	GetPerfMonitorCountersAMD,
	GetPerfMonitorGroupStringAMD,
	GetPerfMonitorCounterStringAMD,
	GetPerfMonitorCounterInfoAMD,
	GenPerfMonitorsAMD,
	DeletePerfMonitorsAMD,
	SelectPerfMonitorCountersAMD,
	BeginPerfMonitorAMD,
	EndPerfMonitorAMD,
	GetPerfMonitorCounterDataAMD,
	BlitFramebufferANGLE,
	RenderbufferStorageMultisampleANGLE,
	CoverageMaskNV,
	CoverageOperationNV,
	GetDriverControlsQCOM,
	GetDriverControlStringQCOM,
	EnableDriverControlQCOM,
	DisableDriverControlQCOM,
	ExtGetTexturesQCOM,
	ExtGetBuffersQCOM,
	ExtGetRenderbuffersQCOM,
	ExtGetFramebuffersQCOM,
	ExtGetTexLevelParameterivQCOM,
	ExtTexObjectStateOverrideiQCOM,
	ExtGetTexSubImageQCOM,
	ExtGetBufferPointervQCOM,
	ExtGetShadersQCOM,
	ExtGetProgramsQCOM,
	ExtIsProgramBinaryQCOM,
	ExtGetProgramBinarySourceQCOM,
	StartTilingQCOM,
	EndTilingQCOM,
	GetProgramBinaryOES,
	ProgramBinaryOES,
	TexImage3DOES,
	TexSubImage3DOES,
	CopyTexSubImage3DOES,
	CompressedTexImage3DOES,
	CompressedTexSubImage3DOES,
	FramebufferTexture3DOES,
	BlendEquationSeparateOES,
	BlendFuncSeparateOES,
	BlendEquationOES,
	QueryMatrixxOES,
	CopyTextureLevelsAPPLE,
	RenderbufferStorageMultisampleAPPLE,
	ResolveMultisampleFramebufferAPPLE,
	FenceSyncAPPLE,
	IsSyncAPPLE,
	DeleteSyncAPPLE,
	ClientWaitSyncAPPLE,
	WaitSyncAPPLE,
	GetInteger64vAPPLE,
	GetSyncivAPPLE,
	MapBufferRangeEXT,
	FlushMappedBufferRangeEXT,
	RenderbufferStorageMultisampleEXT,
	FramebufferTexture2DMultisampleEXT,
	GetGraphicsResetStatusEXT,
	ReadnPixelsEXT,
	GetnUniformfvEXT,
	GetnUniformivEXT,
	TexStorage1DEXT,
	TexStorage2DEXT,
	TexStorage3DEXT,
	TextureStorage1DEXT,
	TextureStorage2DEXT,
	TextureStorage3DEXT,
#if !defined(GL_KHR_debug)
	GLDEBUGPROCKHR,
#endif
	DebugMessageControlKHR,
	DebugMessageInsertKHR,
	DebugMessageCallbackKHR,
	GetDebugMessageLogKHR,
	PushDebugGroupKHR,
	PopDebugGroupKHR,
	ObjectLabelKHR,
	GetObjectLabelKHR,
	ObjectPtrLabelKHR,
	GetObjectPtrLabelKHR,
	GetPointervKHR,
	DrawArraysInstancedANGLE,
	DrawElementsInstancedANGLE,
	VertexAttribDivisorANGLE,
	GetTranslatedShaderSourceANGLE,
	LabelObjectEXT,
	GetObjectLabelEXT,
	InsertEventMarkerEXT,
	PushGroupMarkerEXT,
	PopGroupMarkerEXT,
	GenQueriesEXT,
	DeleteQueriesEXT,
	IsQueryEXT,
	BeginQueryEXT,
	EndQueryEXT,
	GetQueryivEXT,
	GetQueryObjectuivEXT,
	UseProgramStagesEXT,
	ActiveShaderProgramEXT,
	CreateShaderProgramvEXT,
	BindProgramPipelineEXT,
	DeleteProgramPipelinesEXT,
	GenProgramPipelinesEXT,
	IsProgramPipelineEXT,
	ProgramParameteriEXT,
	GetProgramPipelineivEXT,
	ProgramUniform1iEXT,
	ProgramUniform2iEXT,
	ProgramUniform3iEXT,
	ProgramUniform4iEXT,
	ProgramUniform1fEXT,
	ProgramUniform2fEXT,
	ProgramUniform3fEXT,
	ProgramUniform4fEXT,
	ProgramUniform1ivEXT,
	ProgramUniform2ivEXT,
	ProgramUniform3ivEXT,
	ProgramUniform4ivEXT,
	ProgramUniform1fvEXT,
	ProgramUniform2fvEXT,
	ProgramUniform3fvEXT,
	ProgramUniform4fvEXT,
	ProgramUniformMatrix2fvEXT,
	ProgramUniformMatrix3fvEXT,
	ProgramUniformMatrix4fvEXT,
	ValidateProgramPipelineEXT,
	GetProgramPipelineInfoLogEXT,
	ProgramUniform1uiEXT,
	ProgramUniform2uiEXT,
	ProgramUniform3uiEXT,
	ProgramUniform4uiEXT,
	ProgramUniform1uivEXT,
	ProgramUniform2uivEXT,
	ProgramUniform3uivEXT,
	ProgramUniform4uivEXT,
	ProgramUniformMatrix2x3fvEXT,
	ProgramUniformMatrix3x2fvEXT,
	ProgramUniformMatrix2x4fvEXT,
	ProgramUniformMatrix4x2fvEXT,
	ProgramUniformMatrix3x4fvEXT,
	ProgramUniformMatrix4x3fvEXT,
	AlphaFuncQCOM,
	ReadBufferNV,
	DrawBuffersNV,
	ReadBufferIndexedEXT,
	DrawBuffersIndexedEXT,
	GetIntegeri_vEXT,
	DrawBuffersEXT,
	BlendEquationEXT,
	BlendBarrierKHR,
	TexStorage3DMultisampleOES,
	FramebufferTextureMultiviewOVR,
	FramebufferPixelLocalStorageSizeEXT,
	ClearPixelLocalStorageuiEXT,
	GetFramebufferPixelLocalStorageSizeEXT,
	BufferStorageEXT,
	ClearTexImageIMG,
	ClearTexSubImageIMG,
	ClearTexImageEXT,
	ClearTexSubImageEXT,
	FramebufferTexture2DDownsampleIMG,
	FramebufferTextureLayerDownsampleIMG,
	PatchParameteriEXT,

#ifdef GL_IMG_bindless_texture
	GetTextureHandleIMG,
	GetTextureSamplerHandleIMG,
	UniformHandleui64IMG,
	UniformHandleui64vIMG,
	ProgramUniformHandleui64IMG,
	ProgramUniformHandleui64vIMG,
#endif

	NUMBER_OF_OPENGLEXT_FUNCTIONS
};
} // namespace GlExtFuncName

namespace GlExtFuncName {

static const std::pair<uint32_t, const char* const> OpenGLESExtFunctionNamePairs[] = { { MultiDrawArraysEXT, "glMultiDrawArraysEXT" },
	{ MultiDrawElementsEXT, "glMultiDrawElementsEXT" }, { DiscardFramebufferEXT, "glDiscardFramebufferEXT" }, { MapBufferOES, "glMapBufferOES" }, { UnmapBufferOES, "glUnmapBufferOES" },
	{ GetBufferPointervOES, "glGetBufferPointervOES" }, { BindVertexArrayOES, "glBindVertexArrayOES" }, { DeleteVertexArraysOES, "glDeleteVertexArraysOES" },
	{ GenVertexArraysOES, "glGenVertexArraysOES" }, { IsVertexArrayOES, "glIsVertexArrayOES" }, { DeleteFencesNV, "glDeleteFencesNV" }, { GenFencesNV, "glGenFencesNV" },
	{ IsFenceNV, "glIsFenceNV" }, { TestFenceNV, "glTestFenceNV" }, { GetFenceivNV, "glGetFenceivNV" }, { FinishFenceNV, "glFinishFenceNV" }, { SetFenceNV, "glSetFenceNV" },
#if !TARGET_OS_IPHONE
	{ EGLImageTargetTexture2DOES, "glEGLImageTargetTexture2DOES" }, { EGLImageTargetRenderbufferStorageOES, "glEGLImageTargetRenderbufferStorageOES" },
#endif
	{ RenderbufferStorageMultisampleIMG, "glRenderbufferStorageMultisampleIMG" }, { FramebufferTexture2DMultisampleIMG, "glFramebufferTexture2DMultisampleIMG" },
	{ GetPerfMonitorGroupsAMD, "glGetPerfMonitorGroupsAMD" }, { GetPerfMonitorCountersAMD, "glGetPerfMonitorCountersAMD" },
	{ GetPerfMonitorGroupStringAMD, "glGetPerfMonitorGroupStringAMD" }, { GetPerfMonitorCounterStringAMD, "glGetPerfMonitorCounterStringAMD" },
	{ GetPerfMonitorCounterInfoAMD, "glGetPerfMonitorCounterInfoAMD" }, { GenPerfMonitorsAMD, "glGenPerfMonitorsAMD" }, { DeletePerfMonitorsAMD, "glDeletePerfMonitorsAMD" },
	{ SelectPerfMonitorCountersAMD, "glSelectPerfMonitorCountersAMD" }, { BeginPerfMonitorAMD, "glBeginPerfMonitorAMD" }, { EndPerfMonitorAMD, "glEndPerfMonitorAMD" },
	{ GetPerfMonitorCounterDataAMD, "glGetPerfMonitorCounterDataAMD" }, { BlitFramebufferANGLE, "glBlitFramebufferANGLE" },
	{ RenderbufferStorageMultisampleANGLE, "glRenderbufferStorageMultisampleANGLE" }, { CoverageMaskNV, "glCoverageMaskNV" }, { CoverageOperationNV, "glCoverageOperationNV" },
	{ GetDriverControlsQCOM, "glGetDriverControlsQCOM" }, { GetDriverControlStringQCOM, "glGetDriverControlStringQCOM" }, { EnableDriverControlQCOM, "glEnableDriverControlQCOM" },
	{ DisableDriverControlQCOM, "glDisableDriverControlQCOM" }, { ExtGetTexturesQCOM, "glExtGetTexturesQCOM" }, { ExtGetBuffersQCOM, "glExtGetBuffersQCOM" },
	{ ExtGetRenderbuffersQCOM, "glExtGetRenderbuffersQCOM" }, { ExtGetFramebuffersQCOM, "glExtGetFramebuffersQCOM" },
	{ ExtGetTexLevelParameterivQCOM, "glExtGetTexLevelParameterivQCOM" }, { ExtTexObjectStateOverrideiQCOM, "glExtTexObjectStateOverrideiQCOM" },
	{ ExtGetTexSubImageQCOM, "glExtGetTexSubImageQCOM" }, { ExtGetBufferPointervQCOM, "glExtGetBufferPointervQCOM" }, { ExtGetShadersQCOM, "glExtGetShadersQCOM" },
	{ ExtGetProgramsQCOM, "glExtGetProgramsQCOM" }, { ExtIsProgramBinaryQCOM, "glExtIsProgramBinaryQCOM" }, { ExtGetProgramBinarySourceQCOM, "glExtGetProgramBinarySourceQCOM" },
	{ StartTilingQCOM, "glStartTilingQCOM" }, { EndTilingQCOM, "glEndTilingQCOM" }, { GetProgramBinaryOES, "glGetProgramBinaryOES" }, { ProgramBinaryOES, "glProgramBinaryOES" },
	{ TexImage3DOES, "glTexImage3DOES" }, { TexSubImage3DOES, "glTexSubImage3DOES" }, { CopyTexSubImage3DOES, "glCopyTexSubImage3DOES" },
	{ CompressedTexImage3DOES, "glCompressedTexImage3DOES" }, { CompressedTexSubImage3DOES, "glCompressedTexSubImage3DOES" }, { FramebufferTexture3DOES, "glFramebufferTexture3DOES" },
	{ BlendEquationSeparateOES, "glBlendEquationSeparateOES" }, { BlendFuncSeparateOES, "glBlendFuncSeparateOES" }, { BlendEquationOES, "glBlendEquationOES" },
	{ QueryMatrixxOES, "glQueryMatrixxOES" }, { CopyTextureLevelsAPPLE, "glCopyTextureLevelsAPPLE" }, { RenderbufferStorageMultisampleAPPLE, "glRenderbufferStorageMultisampleAPPLE" },
	{ ResolveMultisampleFramebufferAPPLE, "glResolveMultisampleFramebufferAPPLE" }, { FenceSyncAPPLE, "glFenceSyncAPPLE" }, { IsSyncAPPLE, "glIsSyncAPPLE" },
	{ DeleteSyncAPPLE, "glDeleteSyncAPPLE" }, { ClientWaitSyncAPPLE, "glClientWaitSyncAPPLE" }, { WaitSyncAPPLE, "glWaitSyncAPPLE" }, { GetInteger64vAPPLE, "glGetInteger64vAPPLE" },
	{ GetSyncivAPPLE, "glGetSyncivAPPLE" }, { MapBufferRangeEXT, "glMapBufferRangeEXT" }, { FlushMappedBufferRangeEXT, "glFlushMappedBufferRangeEXT" },
	{ RenderbufferStorageMultisampleEXT, "glRenderbufferStorageMultisampleEXT" }, { FramebufferTexture2DMultisampleEXT, "glFramebufferTexture2DMultisampleEXT" },
	{ GetGraphicsResetStatusEXT, "glGetGraphicsResetStatusEXT" }, { ReadnPixelsEXT, "glReadnPixelsEXT" }, { GetnUniformfvEXT, "glGetnUniformfvEXT" },
	{ GetnUniformivEXT, "glGetnUniformivEXT" }, { TexStorage1DEXT, "glTexStorage1DEXT" }, { TexStorage2DEXT, "glTexStorage2DEXT" }, { TexStorage3DEXT, "glTexStorage3DEXT" },
	{ TextureStorage1DEXT, "glTextureStorage1DEXT" }, { TextureStorage2DEXT, "glTextureStorage2DEXT" }, { TextureStorage3DEXT, "glTextureStorage3DEXT" },
#if !defined(GL_KHR_debug)
	{ GLDEBUGPROCKHR, "glGLDEBUGPROCKHR" },
#endif
	{ DebugMessageControlKHR, "glDebugMessageControlKHR" }, { DebugMessageInsertKHR, "glDebugMessageInsertKHR" }, { DebugMessageCallbackKHR, "glDebugMessageCallbackKHR" },
	{ GetDebugMessageLogKHR, "glGetDebugMessageLogKHR" }, { PushDebugGroupKHR, "glPushDebugGroupKHR" }, { PopDebugGroupKHR, "glPopDebugGroupKHR" },
	{ ObjectLabelKHR, "glObjectLabelKHR" }, { GetObjectLabelKHR, "glGetObjectLabelKHR" }, { ObjectPtrLabelKHR, "glObjectPtrLabelKHR" },
	{ GetObjectPtrLabelKHR, "glGetObjectPtrLabelKHR" }, { GetPointervKHR, "glGetPointervKHR" }, { DrawArraysInstancedANGLE, "glDrawArraysInstancedANGLE" },
	{ DrawElementsInstancedANGLE, "glDrawElementsInstancedANGLE" }, { VertexAttribDivisorANGLE, "glVertexAttribDivisorANGLE" },
	{ GetTranslatedShaderSourceANGLE, "glGetTranslatedShaderSourceANGLE" }, { LabelObjectEXT, "glLabelObjectEXT" }, { GetObjectLabelEXT, "glGetObjectLabelEXT" },
	{ InsertEventMarkerEXT, "glInsertEventMarkerEXT" }, { PushGroupMarkerEXT, "glPushGroupMarkerEXT" }, { PopGroupMarkerEXT, "glPopGroupMarkerEXT" },
	{ GenQueriesEXT, "glGenQueriesEXT" }, { DeleteQueriesEXT, "glDeleteQueriesEXT" }, { IsQueryEXT, "glIsQueryEXT" }, { BeginQueryEXT, "glBeginQueryEXT" },
	{ EndQueryEXT, "glEndQueryEXT" }, { GetQueryivEXT, "glGetQueryivEXT" }, { GetQueryObjectuivEXT, "glGetQueryObjectuivEXT" }, { UseProgramStagesEXT, "glUseProgramStagesEXT" },
	{ ActiveShaderProgramEXT, "glActiveShaderProgramEXT" }, { CreateShaderProgramvEXT, "glCreateShaderProgramvEXT" }, { BindProgramPipelineEXT, "glBindProgramPipelineEXT" },
	{ DeleteProgramPipelinesEXT, "glDeleteProgramPipelinesEXT" }, { GenProgramPipelinesEXT, "glGenProgramPipelinesEXT" }, { IsProgramPipelineEXT, "glIsProgramPipelineEXT" },
	{ ProgramParameteriEXT, "glProgramParameteriEXT" }, { GetProgramPipelineivEXT, "glGetProgramPipelineivEXT" }, { ProgramUniform1iEXT, "glProgramUniform1iEXT" },
	{ ProgramUniform2iEXT, "glProgramUniform2iEXT" }, { ProgramUniform3iEXT, "glProgramUniform3iEXT" }, { ProgramUniform4iEXT, "glProgramUniform4iEXT" },
	{ ProgramUniform1fEXT, "glProgramUniform1fEXT" }, { ProgramUniform2fEXT, "glProgramUniform2fEXT" }, { ProgramUniform3fEXT, "glProgramUniform3fEXT" },
	{ ProgramUniform4fEXT, "glProgramUniform4fEXT" }, { ProgramUniform1ivEXT, "glProgramUniform1ivEXT" }, { ProgramUniform2ivEXT, "glProgramUniform2ivEXT" },
	{ ProgramUniform3ivEXT, "glProgramUniform3ivEXT" }, { ProgramUniform4ivEXT, "glProgramUniform4ivEXT" }, { ProgramUniform1fvEXT, "glProgramUniform1fvEXT" },
	{ ProgramUniform2fvEXT, "glProgramUniform2fvEXT" }, { ProgramUniform3fvEXT, "glProgramUniform3fvEXT" }, { ProgramUniform4fvEXT, "glProgramUniform4fvEXT" },
	{ ProgramUniformMatrix2fvEXT, "glProgramUniformMatrix2fvEXT" }, { ProgramUniformMatrix3fvEXT, "glProgramUniformMatrix3fvEXT" },
	{ ProgramUniformMatrix4fvEXT, "glProgramUniformMatrix4fvEXT" }, { ValidateProgramPipelineEXT, "glValidateProgramPipelineEXT" },
	{ GetProgramPipelineInfoLogEXT, "glGetProgramPipelineInfoLogEXT" }, { ProgramUniform1uiEXT, "glProgramUniform1uiEXT" }, { ProgramUniform2uiEXT, "glProgramUniform2uiEXT" },
	{ ProgramUniform3uiEXT, "glProgramUniform3uiEXT" }, { ProgramUniform4uiEXT, "glProgramUniform4uiEXT" }, { ProgramUniform1uivEXT, "glProgramUniform1uivEXT" },
	{ ProgramUniform2uivEXT, "glProgramUniform2uivEXT" }, { ProgramUniform3uivEXT, "glProgramUniform3uivEXT" }, { ProgramUniform4uivEXT, "glProgramUniform4uivEXT" },
	{ ProgramUniformMatrix2x3fvEXT, "glProgramUniformMatrix2x3fvEXT" }, { ProgramUniformMatrix3x2fvEXT, "glProgramUniformMatrix3x2fvEXT" },
	{ ProgramUniformMatrix2x4fvEXT, "glProgramUniformMatrix2x4fvEXT" }, { ProgramUniformMatrix4x2fvEXT, "glProgramUniformMatrix4x2fvEXT" },
	{ ProgramUniformMatrix3x4fvEXT, "glProgramUniformMatrix3x4fvEXT" }, { ProgramUniformMatrix4x3fvEXT, "glProgramUniformMatrix4x3fvEXT" }, { AlphaFuncQCOM, "glAlphaFuncQCOM" },
	{ ReadBufferNV, "glReadBufferNV" }, { DrawBuffersNV, "glDrawBuffersNV" }, { ReadBufferIndexedEXT, "glReadBufferIndexedEXT" },
	{ DrawBuffersIndexedEXT, "glDrawBuffersIndexedEXT" }, { GetIntegeri_vEXT, "glGetIntegeri_vEXT" }, { DrawBuffersEXT, "glDrawBuffersEXT" },
	{ BlendEquationEXT, "glBlendEquationEXT" }, { BlendBarrierKHR, "glBlendBarrierKHR" }, { TexStorage3DMultisampleOES, "glTexStorage3DMultisampleOES" },
	{ FramebufferTextureMultiviewOVR, "glFramebufferTextureMultiviewOVR" }, { FramebufferPixelLocalStorageSizeEXT, "glFramebufferPixelLocalStorageSizeEXT" },
	{ ClearPixelLocalStorageuiEXT, "glClearPixelLocalStorageuiEXT" }, { GetFramebufferPixelLocalStorageSizeEXT, "glGetFramebufferPixelLocalStorageSize" },
	{ BufferStorageEXT, "glBufferStorageEXT" }, { ClearTexImageIMG, "glClearTexImageIMG" }, { ClearTexSubImageIMG, "glClearTexSubImageIMG" },
	{ ClearTexImageEXT, "glClearTexImageEXT" }, { ClearTexSubImageEXT, "glClearTexSubImageEXT" }, { FramebufferTexture2DDownsampleIMG, "glFramebufferTexture2DDownsampleIMG" },
	{ FramebufferTextureLayerDownsampleIMG, "glFramebufferTextureLayerDownsampleIMG" }, { PatchParameteriEXT, "glPatchParameteriEXT" },

#ifdef GL_IMG_bindless_texture
	{ GetTextureHandleIMG, "glGetTextureHandleIMG" }, { GetTextureSamplerHandleIMG, "glGetTextureSamplerHandleIMG" }, { UniformHandleui64IMG, "glUniformHandleui64IMG" },
	{ UniformHandleui64vIMG, "glUniformHandleui64vIMG" }, { ProgramUniformHandleui64IMG, "glProgramUniformHandleui64IMG" },
	{ ProgramUniformHandleui64vIMG, "glProgramUniformHandleui64vIMG" }
#endif

};
} // namespace GlExtFuncName

#if !TARGET_OS_IPHONE
static inline void* GetGlesExtensionFunction(const char* funcName) { return (void*)DYNAMICEGL_CALL_FUNCTION(GetProcAddress)(funcName); }
#endif

static inline bool isExtensionSupported(const unsigned char* const extensionString, const char* const extension)
{
	if (!extensionString) { return false; }

	// The recommended technique for querying OpenGL extensions;
	// from http://opengl.org/resources/features/OGLextensions/
	const char* start = (const char*)extensionString;
	char *position, *terminator;

	// Extension names should not have spaces.
	position = (char*)strchr(extension, ' ');

	if (position || *extension == '\0') { return 0; }

	/* It takes a bit of care to be fool-proof about parsing the
	OpenGL extensions string. Don't be fooled by sub-strings, etc. */
	for (;;)
	{
		position = (char*)strstr((char*)start, extension);

		if (!position) { break; }

		terminator = position + strlen(extension);

		if (position == start || *(position - 1) == ' ')
		{
			if (*terminator == ' ' || *terminator == '\0') { return true; }
		}

		start = terminator;
	}

	return false;
}

// Pre-loads the OpenGL ES extension function pointers the first time any OpenGL ES extension function call is made
inline void* getGlesExtFunction(gl::internals::GlExtFuncName::OpenGLESExtFunctionName funcname, bool reset = false)
{
	static void* FunctionTable[GlExtFuncName::NUMBER_OF_OPENGLEXT_FUNCTIONS + 1] = { 0 };

#if TARGET_OS_IPHONE
	FunctionTable[GlExtFuncName::DiscardFramebufferEXT] = (void*)&glDiscardFramebufferEXT;
	FunctionTable[GlExtFuncName::MapBufferOES] = (void*)&glMapBufferOES;
	FunctionTable[GlExtFuncName::UnmapBufferOES] = (void*)&glUnmapBufferOES;
	FunctionTable[GlExtFuncName::GetBufferPointervOES] = (void*)&glGetBufferPointervOES;
	FunctionTable[GlExtFuncName::BindVertexArrayOES] = (void*)&glBindVertexArrayOES;
	FunctionTable[GlExtFuncName::DeleteVertexArraysOES] = (void*)&glDeleteVertexArraysOES;
	FunctionTable[GlExtFuncName::GenVertexArraysOES] = (void*)&glGenVertexArraysOES;
	FunctionTable[GlExtFuncName::IsVertexArrayOES] = (void*)&glIsVertexArrayOES;

	FunctionTable[GlExtFuncName::GetProgramBinaryOES] = (void*)&glGetProgramBinary;
	FunctionTable[GlExtFuncName::ProgramBinaryOES] = (void*)&glProgramBinary;
	FunctionTable[GlExtFuncName::TexImage3DOES] = (void*)&glTexImage3D;
	FunctionTable[GlExtFuncName::TexSubImage3DOES] = (void*)&glTexSubImage3D;
	FunctionTable[GlExtFuncName::CopyTexSubImage3DOES] = (void*)&glCopyTexSubImage3D;
	FunctionTable[GlExtFuncName::CompressedTexImage3DOES] = (void*)&glCompressedTexImage3D;
	FunctionTable[GlExtFuncName::CompressedTexSubImage3DOES] = (void*)&glCompressedTexSubImage3D;
	FunctionTable[GlExtFuncName::BlendEquationSeparateOES] = (void*)&glBlendEquationSeparate;
	FunctionTable[GlExtFuncName::BlendFuncSeparateOES] = (void*)&glBlendFuncSeparate;
	FunctionTable[GlExtFuncName::BlendEquationOES] = (void*)&glBlendEquation;

	FunctionTable[GlExtFuncName::CopyTextureLevelsAPPLE] = (void*)&glCopyTextureLevelsAPPLE;
	FunctionTable[GlExtFuncName::RenderbufferStorageMultisampleAPPLE] = (void*)&glRenderbufferStorageMultisampleAPPLE;
	FunctionTable[GlExtFuncName::ResolveMultisampleFramebufferAPPLE] = (void*)&glResolveMultisampleFramebufferAPPLE;
	FunctionTable[GlExtFuncName::FenceSyncAPPLE] = (void*)&glFenceSyncAPPLE;
	FunctionTable[GlExtFuncName::IsSyncAPPLE] = (void*)&glIsSyncAPPLE;
	FunctionTable[GlExtFuncName::DeleteSyncAPPLE] = (void*)&glDeleteSyncAPPLE;
	FunctionTable[GlExtFuncName::ClientWaitSyncAPPLE] = (void*)&glClientWaitSyncAPPLE;
	FunctionTable[GlExtFuncName::WaitSyncAPPLE] = (void*)&glWaitSyncAPPLE;
	FunctionTable[GlExtFuncName::GetInteger64vAPPLE] = (void*)&glGetInteger64vAPPLE;
	FunctionTable[GlExtFuncName::GetSyncivAPPLE] = (void*)&glGetSyncivAPPLE;
	FunctionTable[GlExtFuncName::MapBufferRangeEXT] = (void*)&glMapBufferRangeEXT;
	FunctionTable[GlExtFuncName::FlushMappedBufferRangeEXT] = (void*)&glFlushMappedBufferRangeEXT;
	FunctionTable[GlExtFuncName::RenderbufferStorageMultisampleEXT] = (void*)&glRenderbufferStorageMultisample;
	FunctionTable[GlExtFuncName::TexStorage2DEXT] = (void*)&glTexStorage2DEXT;
	FunctionTable[GlExtFuncName::TextureStorage2DEXT] = (void*)&glTexStorage2DEXT;
	FunctionTable[GlExtFuncName::GetObjectLabelKHR] = (void*)&glGetObjectLabelEXT;
	FunctionTable[GlExtFuncName::LabelObjectEXT] = (void*)&glLabelObjectEXT;
	FunctionTable[GlExtFuncName::GetObjectLabelEXT] = (void*)&glGetObjectLabelEXT;
	FunctionTable[GlExtFuncName::InsertEventMarkerEXT] = (void*)&glInsertEventMarkerEXT;
	FunctionTable[GlExtFuncName::PushGroupMarkerEXT] = (void*)&glPushGroupMarkerEXT;
	FunctionTable[GlExtFuncName::PopGroupMarkerEXT] = (void*)&glPopGroupMarkerEXT;
	FunctionTable[GlExtFuncName::GenQueriesEXT] = (void*)&glGenQueriesEXT;
	FunctionTable[GlExtFuncName::DeleteQueriesEXT] = (void*)&glDeleteQueriesEXT;
	FunctionTable[GlExtFuncName::IsQueryEXT] = (void*)&glIsQueryEXT;
	FunctionTable[GlExtFuncName::BeginQueryEXT] = (void*)&glBeginQueryEXT;
	FunctionTable[GlExtFuncName::EndQueryEXT] = (void*)&glEndQueryEXT;
	FunctionTable[GlExtFuncName::GetQueryivEXT] = (void*)&glGetQueryivEXT;
	FunctionTable[GlExtFuncName::GetQueryObjectuivEXT] = (void*)&glGetQueryObjectuivEXT;
	FunctionTable[GlExtFuncName::UseProgramStagesEXT] = (void*)&glUseProgramStagesEXT;
	FunctionTable[GlExtFuncName::ActiveShaderProgramEXT] = (void*)&glActiveShaderProgramEXT;
	FunctionTable[GlExtFuncName::CreateShaderProgramvEXT] = (void*)&glCreateShaderProgramvEXT;
	FunctionTable[GlExtFuncName::BindProgramPipelineEXT] = (void*)&glBindProgramPipelineEXT;
	FunctionTable[GlExtFuncName::DeleteProgramPipelinesEXT] = (void*)&glDeleteProgramPipelinesEXT;
	FunctionTable[GlExtFuncName::GenProgramPipelinesEXT] = (void*)&glGenProgramPipelinesEXT;
	FunctionTable[GlExtFuncName::IsProgramPipelineEXT] = (void*)&glIsProgramPipelineEXT;
	FunctionTable[GlExtFuncName::ProgramParameteriEXT] = (void*)&glProgramParameteriEXT;
	FunctionTable[GlExtFuncName::GetProgramPipelineivEXT] = (void*)&glGetProgramPipelineivEXT;
	FunctionTable[GlExtFuncName::ProgramUniform1iEXT] = (void*)&glProgramUniform1iEXT;
	FunctionTable[GlExtFuncName::ProgramUniform2iEXT] = (void*)&glProgramUniform2iEXT;
	FunctionTable[GlExtFuncName::ProgramUniform3iEXT] = (void*)&glProgramUniform3iEXT;
	FunctionTable[GlExtFuncName::ProgramUniform4iEXT] = (void*)&glProgramUniform4iEXT;
	FunctionTable[GlExtFuncName::ProgramUniform1fEXT] = (void*)&glProgramUniform1fEXT;
	FunctionTable[GlExtFuncName::ProgramUniform2fEXT] = (void*)&glProgramUniform2fEXT;
	FunctionTable[GlExtFuncName::ProgramUniform3fEXT] = (void*)&glProgramUniform3fEXT;
	FunctionTable[GlExtFuncName::ProgramUniform4fEXT] = (void*)&glProgramUniform4fEXT;
	FunctionTable[GlExtFuncName::ProgramUniform1ivEXT] = (void*)&glProgramUniform1ivEXT;
	FunctionTable[GlExtFuncName::ProgramUniform2ivEXT] = (void*)&glProgramUniform2ivEXT;
	FunctionTable[GlExtFuncName::ProgramUniform3ivEXT] = (void*)&glProgramUniform3ivEXT;
	FunctionTable[GlExtFuncName::ProgramUniform4ivEXT] = (void*)&glProgramUniform4ivEXT;
	FunctionTable[GlExtFuncName::ProgramUniform1fvEXT] = (void*)&glProgramUniform1fvEXT;
	FunctionTable[GlExtFuncName::ProgramUniform2fvEXT] = (void*)&glProgramUniform2fvEXT;
	FunctionTable[GlExtFuncName::ProgramUniform3fvEXT] = (void*)&glProgramUniform3fvEXT;
	FunctionTable[GlExtFuncName::ProgramUniform4fvEXT] = (void*)&glProgramUniform4fvEXT;
	FunctionTable[GlExtFuncName::ProgramUniformMatrix2fvEXT] = (void*)&glProgramUniformMatrix2fvEXT;
	FunctionTable[GlExtFuncName::ProgramUniformMatrix3fvEXT] = (void*)&glProgramUniformMatrix3fvEXT;
	FunctionTable[GlExtFuncName::ProgramUniformMatrix4fvEXT] = (void*)&glProgramUniformMatrix4fvEXT;
	FunctionTable[GlExtFuncName::ValidateProgramPipelineEXT] = (void*)&glValidateProgramPipelineEXT;
	FunctionTable[GlExtFuncName::GetProgramPipelineInfoLogEXT] = (void*)&glGetProgramPipelineInfoLogEXT;
	FunctionTable[GlExtFuncName::ProgramUniform1uiEXT] = (void*)&glProgramUniform1uiEXT;
	FunctionTable[GlExtFuncName::ProgramUniform2uiEXT] = (void*)&glProgramUniform2uiEXT;
	FunctionTable[GlExtFuncName::ProgramUniform3uiEXT] = (void*)&glProgramUniform3uiEXT;
	FunctionTable[GlExtFuncName::ProgramUniform4uiEXT] = (void*)&glProgramUniform4uiEXT;
	FunctionTable[GlExtFuncName::ProgramUniform1uivEXT] = (void*)&glProgramUniform1uivEXT;
	FunctionTable[GlExtFuncName::ProgramUniform2uivEXT] = (void*)&glProgramUniform2uivEXT;
	FunctionTable[GlExtFuncName::ProgramUniform3uivEXT] = (void*)&glProgramUniform3uivEXT;
	FunctionTable[GlExtFuncName::ProgramUniform4uivEXT] = (void*)&glProgramUniform4uivEXT;
	FunctionTable[GlExtFuncName::ProgramUniformMatrix2x3fvEXT] = (void*)&glProgramUniformMatrix2x3fvEXT;
	FunctionTable[GlExtFuncName::ProgramUniformMatrix3x2fvEXT] = (void*)&glProgramUniformMatrix3x2fvEXT;
	FunctionTable[GlExtFuncName::ProgramUniformMatrix2x4fvEXT] = (void*)&glProgramUniformMatrix2x4fvEXT;
	FunctionTable[GlExtFuncName::ProgramUniformMatrix4x2fvEXT] = (void*)&glProgramUniformMatrix4x2fvEXT;
	FunctionTable[GlExtFuncName::ProgramUniformMatrix3x4fvEXT] = (void*)&glProgramUniformMatrix3x4fvEXT;
	FunctionTable[GlExtFuncName::ProgramUniformMatrix4x3fvEXT] = (void*)&glProgramUniformMatrix4x3fvEXT;
	FunctionTable[GlExtFuncName::GetIntegeri_vEXT] = (void*)&glGetIntegeri_v;
	FunctionTable[GlExtFuncName::DrawBuffersEXT] = (void*)&glDrawBuffers;
	FunctionTable[GlExtFuncName::BlendEquationEXT] = (void*)&glBlendEquation;
#else
	// Retrieve the OpenGL ES extension functions pointers once
	if (!FunctionTable[GlExtFuncName::NUMBER_OF_OPENGLEXT_FUNCTIONS] || reset)
	{
		const uint32_t numExtensionStrings = sizeof(GlExtFuncName::OpenGLESExtFunctionNamePairs) / sizeof(GlExtFuncName::OpenGLESExtFunctionNamePairs[0]);

		// Set the last element of the function table to avoid issues
		FunctionTable[GlExtFuncName::NUMBER_OF_OPENGLEXT_FUNCTIONS] = (void*)1;

		for (uint32_t i = 0; i < numExtensionStrings; i++) { FunctionTable[i] = GetGlesExtensionFunction(GlExtFuncName::OpenGLESExtFunctionNamePairs[i].second); }
	}
#endif
	return FunctionTable[funcname];
}
} // namespace internals
} // namespace gl

#ifndef DYNAMICGLES_NO_NAMESPACE
namespace gl {
namespace ext {
#elif TARGET_OS_IPHONE
namespace gl {
namespace internals {
#endif
inline void resetExtensionFunctionPointers() { gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::OpenGLESExtFunctionName(0), true); }

inline void DYNAMICGLES_FUNCTION(MultiDrawElementsEXT)(GLenum mode, const GLsizei* count, GLenum type, const GLvoid** indices, GLsizei primcount)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLMULTIDRAWELEMENTSEXTPROC _MultiDrawElementsEXT = (PFNGLMULTIDRAWELEMENTSEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::MultiDrawElementsEXT);
	return _MultiDrawElementsEXT(mode, count, type, indices, primcount);
#endif
}
inline void DYNAMICGLES_FUNCTION(MultiDrawArraysEXT)(GLenum mode, const GLint* first, const GLsizei* count, GLsizei primcount)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLMULTIDRAWARRAYSEXTPROC _MultiDrawArraysEXT = (PFNGLMULTIDRAWARRAYSEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::MultiDrawArraysEXT);
	return _MultiDrawArraysEXT(mode, first, count, primcount);
#endif
}
inline void DYNAMICGLES_FUNCTION(DiscardFramebufferEXT)(GLenum target, GLsizei numAttachments, const GLenum* attachments)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glDiscardFramebufferEXT) PFNGLDISCARDFRAMEBUFFEREXTPROC;
#endif
	PFNGLDISCARDFRAMEBUFFEREXTPROC _DiscardFramebufferEXT = (PFNGLDISCARDFRAMEBUFFEREXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::DiscardFramebufferEXT);
	return _DiscardFramebufferEXT(target, numAttachments, attachments);
}
inline void* DYNAMICGLES_FUNCTION(MapBufferOES)(GLenum target, GLenum access)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glMapBufferOES) PFNGLMAPBUFFEROESPROC;
#endif
	PFNGLMAPBUFFEROESPROC _MapBufferOES = (PFNGLMAPBUFFEROESPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::MapBufferOES);
	return _MapBufferOES(target, access);
}
inline GLboolean DYNAMICGLES_FUNCTION(UnmapBufferOES)(GLenum target)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUnmapBufferOES) PFNGLUNMAPBUFFEROESPROC;
#endif
	PFNGLUNMAPBUFFEROESPROC _UnmapBufferOES = (PFNGLUNMAPBUFFEROESPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::UnmapBufferOES);
	return _UnmapBufferOES(target);
}
inline void DYNAMICGLES_FUNCTION(GetBufferPointervOES)(GLenum target, GLenum pname, void** params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetBufferPointervOES) PFNGLGETBUFFERPOINTERVOESPROC;
#endif
	PFNGLGETBUFFERPOINTERVOESPROC _GetBufferPointervOES = (PFNGLGETBUFFERPOINTERVOESPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetBufferPointervOES);
	return _GetBufferPointervOES(target, pname, params);
}
inline void DYNAMICGLES_FUNCTION(BindVertexArrayOES)(GLuint vertexarray)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glBindVertexArrayOES) PFNGLBINDVERTEXARRAYOESPROC;
#endif
	PFNGLBINDVERTEXARRAYOESPROC _BindVertexArrayOES = (PFNGLBINDVERTEXARRAYOESPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::BindVertexArrayOES);
	return _BindVertexArrayOES(vertexarray);
}
inline void DYNAMICGLES_FUNCTION(DeleteVertexArraysOES)(GLsizei n, const GLuint* vertexarrays)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glDeleteVertexArraysOES) PFNGLDELETEVERTEXARRAYSOESPROC;
#endif
	PFNGLDELETEVERTEXARRAYSOESPROC _DeleteVertexArraysOES = (PFNGLDELETEVERTEXARRAYSOESPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::DeleteVertexArraysOES);
	return _DeleteVertexArraysOES(n, vertexarrays);
}
inline void DYNAMICGLES_FUNCTION(GenVertexArraysOES)(GLsizei n, GLuint* vertexarrays)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGenVertexArraysOES) PFNGLGENVERTEXARRAYSOESPROC;
#endif
	PFNGLGENVERTEXARRAYSOESPROC _GenVertexArraysOES = (PFNGLGENVERTEXARRAYSOESPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GenVertexArraysOES);
	return _GenVertexArraysOES(n, vertexarrays);
}
inline GLboolean DYNAMICGLES_FUNCTION(IsVertexArrayOES)(GLuint vertexarray)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glIsVertexArrayOES) PFNGLISVERTEXARRAYOESPROC;
#endif
	PFNGLISVERTEXARRAYOESPROC _IsVertexArrayOES = (PFNGLISVERTEXARRAYOESPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::IsVertexArrayOES);
	return _IsVertexArrayOES(vertexarray);
}
inline void DYNAMICGLES_FUNCTION(DeleteFencesNV)(GLsizei n, const GLuint* fences)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLDELETEFENCESNVPROC _DeleteFencesNV = (PFNGLDELETEFENCESNVPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::DeleteFencesNV);
	return _DeleteFencesNV(n, fences);
#endif
}
inline void DYNAMICGLES_FUNCTION(GenFencesNV)(GLsizei n, GLuint* fences)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGENFENCESNVPROC _GenFencesNV = (PFNGLGENFENCESNVPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GenFencesNV);
	return _GenFencesNV(n, fences);
#endif
}
inline GLboolean DYNAMICGLES_FUNCTION(IsFenceNV)(GLuint fence)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLISFENCENVPROC _IsFenceNV = (PFNGLISFENCENVPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::IsFenceNV);
	return _IsFenceNV(fence);
#endif
}
inline GLboolean DYNAMICGLES_FUNCTION(TestFenceNV)(GLuint fence)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLTESTFENCENVPROC _TestFenceNV = (PFNGLTESTFENCENVPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::TestFenceNV);
	return _TestFenceNV(fence);
#endif
}
inline void DYNAMICGLES_FUNCTION(GetFenceivNV)(GLuint fence, GLenum pname, GLint* params)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETFENCEIVNVPROC _GetFenceivNV = (PFNGLGETFENCEIVNVPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetFenceivNV);
	return _GetFenceivNV(fence, pname, params);
#endif
}
inline void DYNAMICGLES_FUNCTION(FinishFenceNV)(GLuint fence)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLFINISHFENCENVPROC _FinishFenceNV = (PFNGLFINISHFENCENVPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::FinishFenceNV);
	return _FinishFenceNV(fence);
#endif
}
inline void DYNAMICGLES_FUNCTION(SetFenceNV)(GLuint fence, GLenum condition)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLSETFENCENVPROC _SetFenceNV = (PFNGLSETFENCENVPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::SetFenceNV);
	return _SetFenceNV(fence, condition);
#endif
}
#if !TARGET_OS_IPHONE
inline void DYNAMICGLES_FUNCTION(EGLImageTargetTexture2DOES)(GLenum target, GLeglImageOES image)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLEGLIMAGETARGETTEXTURE2DOESPROC _EGLImageTargetTexture2DOES =
		(PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::EGLImageTargetTexture2DOES);
	return _EGLImageTargetTexture2DOES(target, image);
#endif
}
inline void DYNAMICGLES_FUNCTION(EGLImageTargetRenderbufferStorageOES)(GLenum target, GLeglImageOES image)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC _EGLImageTargetRenderbufferStorageOES =
		(PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::EGLImageTargetRenderbufferStorageOES);
	return _EGLImageTargetRenderbufferStorageOES(target, image);
#endif
}
#endif
inline void DYNAMICGLES_FUNCTION(RenderbufferStorageMultisampleIMG)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLRENDERBUFFERSTORAGEMULTISAMPLEIMGPROC _RenderbufferStorageMultisampleIMG =
		(PFNGLRENDERBUFFERSTORAGEMULTISAMPLEIMGPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::RenderbufferStorageMultisampleIMG);
	return _RenderbufferStorageMultisampleIMG(target, samples, internalformat, width, height);
#endif
}
inline void DYNAMICGLES_FUNCTION(FramebufferTexture2DMultisampleIMG)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEIMGPROC _FramebufferTexture2DMultisampleIMG =
		(PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEIMGPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::FramebufferTexture2DMultisampleIMG);
	return _FramebufferTexture2DMultisampleIMG(target, attachment, textarget, texture, level, samples);
#endif
}
inline void DYNAMICGLES_FUNCTION(GetPerfMonitorGroupsAMD)(GLint* numGroups, GLsizei groupsSize, GLuint* groups)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETPERFMONITORGROUPSAMDPROC _GetPerfMonitorGroupsAMD =
		(PFNGLGETPERFMONITORGROUPSAMDPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetPerfMonitorGroupsAMD);
	return _GetPerfMonitorGroupsAMD(numGroups, groupsSize, groups);
#endif
}
inline void DYNAMICGLES_FUNCTION(GetPerfMonitorCountersAMD)(GLuint group, GLint* numCounters, GLint* maxActiveCounters, GLsizei counterSize, GLuint* counters)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETPERFMONITORCOUNTERSAMDPROC _GetPerfMonitorCountersAMD =
		(PFNGLGETPERFMONITORCOUNTERSAMDPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetPerfMonitorCountersAMD);
	return _GetPerfMonitorCountersAMD(group, numCounters, maxActiveCounters, counterSize, counters);
#endif
}
inline void DYNAMICGLES_FUNCTION(GetPerfMonitorGroupStringAMD)(GLuint group, GLsizei bufSize, GLsizei* length, char* groupString)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETPERFMONITORGROUPSTRINGAMDPROC _GetPerfMonitorGroupStringAMD =
		(PFNGLGETPERFMONITORGROUPSTRINGAMDPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetPerfMonitorGroupStringAMD);
	return _GetPerfMonitorGroupStringAMD(group, bufSize, length, groupString);
#endif
}
inline void DYNAMICGLES_FUNCTION(GetPerfMonitorCounterStringAMD)(GLuint group, GLuint counter, GLsizei bufSize, GLsizei* length, char* counterString)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETPERFMONITORCOUNTERSTRINGAMDPROC _GetPerfMonitorCounterStringAMD =
		(PFNGLGETPERFMONITORCOUNTERSTRINGAMDPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetPerfMonitorCounterStringAMD);
	return _GetPerfMonitorCounterStringAMD(group, counter, bufSize, length, counterString);
#endif
}
inline void DYNAMICGLES_FUNCTION(GetPerfMonitorCounterInfoAMD)(GLuint group, GLuint counter, GLenum pname, GLvoid* data)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETPERFMONITORCOUNTERINFOAMDPROC _GetPerfMonitorCounterInfoAMD =
		(PFNGLGETPERFMONITORCOUNTERINFOAMDPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetPerfMonitorCounterInfoAMD);
	return _GetPerfMonitorCounterInfoAMD(group, counter, pname, data);
#endif
}
inline void DYNAMICGLES_FUNCTION(GenPerfMonitorsAMD)(GLsizei n, GLuint* monitors)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGENPERFMONITORSAMDPROC _GenPerfMonitorsAMD = (PFNGLGENPERFMONITORSAMDPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GenPerfMonitorsAMD);
	return _GenPerfMonitorsAMD(n, monitors);
#endif
}
inline void DYNAMICGLES_FUNCTION(DeletePerfMonitorsAMD)(GLsizei n, GLuint* monitors)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLDELETEPERFMONITORSAMDPROC _DeletePerfMonitorsAMD = (PFNGLDELETEPERFMONITORSAMDPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::DeletePerfMonitorsAMD);
	return _DeletePerfMonitorsAMD(n, monitors);
#endif
}
inline void DYNAMICGLES_FUNCTION(SelectPerfMonitorCountersAMD)(GLuint monitor, GLboolean enable, GLuint group, GLint numCounters, GLuint* countersList)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLSELECTPERFMONITORCOUNTERSAMDPROC _SelectPerfMonitorCountersAMD =
		(PFNGLSELECTPERFMONITORCOUNTERSAMDPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::SelectPerfMonitorCountersAMD);
	return _SelectPerfMonitorCountersAMD(monitor, enable, group, numCounters, countersList);
#endif
}
inline void DYNAMICGLES_FUNCTION(BeginPerfMonitorAMD)(GLuint monitor)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLBEGINPERFMONITORAMDPROC _BeginPerfMonitorAMD = (PFNGLBEGINPERFMONITORAMDPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::BeginPerfMonitorAMD);
	return _BeginPerfMonitorAMD(monitor);
#endif
}
inline void DYNAMICGLES_FUNCTION(EndPerfMonitorAMD)(GLuint monitor)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLENDPERFMONITORAMDPROC _EndPerfMonitorAMD = (PFNGLENDPERFMONITORAMDPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::EndPerfMonitorAMD);
	return _EndPerfMonitorAMD(monitor);
#endif
}
inline void DYNAMICGLES_FUNCTION(GetPerfMonitorCounterDataAMD)(GLuint monitor, GLenum pname, GLsizei dataSize, GLuint* data, GLint* bytesWritten)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETPERFMONITORCOUNTERDATAAMDPROC _GetPerfMonitorCounterDataAMD =
		(PFNGLGETPERFMONITORCOUNTERDATAAMDPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetPerfMonitorCounterDataAMD);
	return _GetPerfMonitorCounterDataAMD(monitor, pname, dataSize, data, bytesWritten);
#endif
}
inline void DYNAMICGLES_FUNCTION(BlitFramebufferANGLE)(
	GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLBLITFRAMEBUFFERANGLEPROC _BlitFramebufferANGLE = (PFNGLBLITFRAMEBUFFERANGLEPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::BlitFramebufferANGLE);
	return _BlitFramebufferANGLE(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
#endif
}
inline void DYNAMICGLES_FUNCTION(RenderbufferStorageMultisampleANGLE)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLRENDERBUFFERSTORAGEMULTISAMPLEANGLEPROC _RenderbufferStorageMultisampleANGLE =
		(PFNGLRENDERBUFFERSTORAGEMULTISAMPLEANGLEPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::RenderbufferStorageMultisampleANGLE);
	return _RenderbufferStorageMultisampleANGLE(target, samples, internalformat, width, height);
#endif
}
inline void DYNAMICGLES_FUNCTION(RenderbufferStorageMultisampleAPPLE)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glRenderbufferStorageMultisampleAPPLE) PFNGLRENDERBUFFERSTORAGEMULTISAMPLEAPPLEPROC;
#endif
	PFNGLRENDERBUFFERSTORAGEMULTISAMPLEAPPLEPROC _RenderbufferStorageMultisampleAPPLE =
		(PFNGLRENDERBUFFERSTORAGEMULTISAMPLEAPPLEPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::RenderbufferStorageMultisampleAPPLE);
	return _RenderbufferStorageMultisampleAPPLE(target, samples, internalformat, width, height);
}
inline void DYNAMICGLES_FUNCTION(ResolveMultisampleFramebufferAPPLE)(void)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glResolveMultisampleFramebufferAPPLE) PFNGLRESOLVEMULTISAMPLEFRAMEBUFFERAPPLEPROC;
#endif
	PFNGLRESOLVEMULTISAMPLEFRAMEBUFFERAPPLEPROC _ResolveMultisampleFramebufferAPPLE =
		(PFNGLRESOLVEMULTISAMPLEFRAMEBUFFERAPPLEPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ResolveMultisampleFramebufferAPPLE);
	return _ResolveMultisampleFramebufferAPPLE();
}
inline void DYNAMICGLES_FUNCTION(CoverageMaskNV)(GLboolean mask)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLCOVERAGEMASKNVPROC _CoverageMaskNV = (PFNGLCOVERAGEMASKNVPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::CoverageMaskNV);
	return _CoverageMaskNV(mask);
#endif
}
inline void DYNAMICGLES_FUNCTION(CoverageOperationNV)(GLenum operation)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLCOVERAGEOPERATIONNVPROC _CoverageOperationNV = (PFNGLCOVERAGEOPERATIONNVPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::CoverageOperationNV);
	return _CoverageOperationNV(operation);
#endif
}
inline void DYNAMICGLES_FUNCTION(GetDriverControlsQCOM)(GLint* num, GLsizei size, GLuint* driverControls)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETDRIVERCONTROLSQCOMPROC _GetDriverControlsQCOM = (PFNGLGETDRIVERCONTROLSQCOMPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetDriverControlsQCOM);
	return _GetDriverControlsQCOM(num, size, driverControls);
#endif
}
inline void DYNAMICGLES_FUNCTION(GetDriverControlStringQCOM)(GLuint driverControl, GLsizei bufSize, GLsizei* length, char* driverControlString)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETDRIVERCONTROLSTRINGQCOMPROC _GetDriverControlStringQCOM =
		(PFNGLGETDRIVERCONTROLSTRINGQCOMPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetDriverControlStringQCOM);
	return _GetDriverControlStringQCOM(driverControl, bufSize, length, driverControlString);
#endif
}
inline void DYNAMICGLES_FUNCTION(EnableDriverControlQCOM)(GLuint driverControl)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLENABLEDRIVERCONTROLQCOMPROC _EnableDriverControlQCOM =
		(PFNGLENABLEDRIVERCONTROLQCOMPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::EnableDriverControlQCOM);
	return _EnableDriverControlQCOM(driverControl);
#endif
}
inline void DYNAMICGLES_FUNCTION(DisableDriverControlQCOM)(GLuint driverControl)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLDISABLEDRIVERCONTROLQCOMPROC _DisableDriverControlQCOM =
		(PFNGLDISABLEDRIVERCONTROLQCOMPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::DisableDriverControlQCOM);
	return _DisableDriverControlQCOM(driverControl);
#endif
}
inline void DYNAMICGLES_FUNCTION(ExtGetTexturesQCOM)(GLuint* textures, GLint maxTextures, GLint* numTextures)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLEXTGETTEXTURESQCOMPROC _ExtGetTexturesQCOM = (PFNGLEXTGETTEXTURESQCOMPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ExtGetTexturesQCOM);
	return _ExtGetTexturesQCOM(textures, maxTextures, numTextures);
#endif
}
inline void DYNAMICGLES_FUNCTION(ExtGetBuffersQCOM)(GLuint* buffers, GLint maxBuffers, GLint* numBuffers)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLEXTGETBUFFERSQCOMPROC _ExtGetBuffersQCOM = (PFNGLEXTGETBUFFERSQCOMPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ExtGetBuffersQCOM);
	return _ExtGetBuffersQCOM(buffers, maxBuffers, numBuffers);
#endif
}
inline void DYNAMICGLES_FUNCTION(ExtGetRenderbuffersQCOM)(GLuint* renderbuffers, GLint maxRenderbuffers, GLint* numRenderbuffers)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLEXTGETRENDERBUFFERSQCOMPROC _ExtGetRenderbuffersQCOM =
		(PFNGLEXTGETRENDERBUFFERSQCOMPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ExtGetRenderbuffersQCOM);
	return _ExtGetRenderbuffersQCOM(renderbuffers, maxRenderbuffers, numRenderbuffers);
#endif
}
inline void DYNAMICGLES_FUNCTION(ExtGetFramebuffersQCOM)(GLuint* framebuffers, GLint maxFramebuffers, GLint* numFramebuffers)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLEXTGETFRAMEBUFFERSQCOMPROC _ExtGetFramebuffersQCOM = (PFNGLEXTGETFRAMEBUFFERSQCOMPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ExtGetFramebuffersQCOM);
	return _ExtGetFramebuffersQCOM(framebuffers, maxFramebuffers, numFramebuffers);
#endif
}
inline void DYNAMICGLES_FUNCTION(ExtGetTexLevelParameterivQCOM)(GLuint texture, GLenum face, GLint level, GLenum pname, GLint* params)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLEXTGETTEXLEVELPARAMETERIVQCOMPROC _ExtGetTexLevelParameterivQCOM =
		(PFNGLEXTGETTEXLEVELPARAMETERIVQCOMPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ExtGetTexLevelParameterivQCOM);
	return _ExtGetTexLevelParameterivQCOM(texture, face, level, pname, params);
#endif
}
inline void DYNAMICGLES_FUNCTION(ExtTexObjectStateOverrideiQCOM)(GLenum target, GLenum pname, GLint param)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLEXTTEXOBJECTSTATEOVERRIDEIQCOMPROC _ExtTexObjectStateOverrideiQCOM =
		(PFNGLEXTTEXOBJECTSTATEOVERRIDEIQCOMPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ExtTexObjectStateOverrideiQCOM);
	return _ExtTexObjectStateOverrideiQCOM(target, pname, param);
#endif
}
inline void DYNAMICGLES_FUNCTION(ExtGetTexSubImageQCOM)(
	GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLvoid* texels)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLEXTGETTEXSUBIMAGEQCOMPROC _ExtGetTexSubImageQCOM = (PFNGLEXTGETTEXSUBIMAGEQCOMPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ExtGetTexSubImageQCOM);
	return _ExtGetTexSubImageQCOM(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, texels);
#endif
}
inline void DYNAMICGLES_FUNCTION(ExtGetBufferPointervQCOM)(GLenum target, GLvoid** params)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLEXTGETBUFFERPOINTERVQCOMPROC _ExtGetBufferPointervQCOM =
		(PFNGLEXTGETBUFFERPOINTERVQCOMPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ExtGetBufferPointervQCOM);
	return _ExtGetBufferPointervQCOM(target, params);
#endif
}
inline void DYNAMICGLES_FUNCTION(ExtGetShadersQCOM)(GLuint* shaders, GLint maxShaders, GLint* numShaders)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLEXTGETSHADERSQCOMPROC _ExtGetShadersQCOM = (PFNGLEXTGETSHADERSQCOMPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ExtGetShadersQCOM);
	return _ExtGetShadersQCOM(shaders, maxShaders, numShaders);
#endif
}
inline void DYNAMICGLES_FUNCTION(ExtGetProgramsQCOM)(GLuint* programs, GLint maxPrograms, GLint* numPrograms)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLEXTGETPROGRAMSQCOMPROC _ExtGetProgramsQCOM = (PFNGLEXTGETPROGRAMSQCOMPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ExtGetProgramsQCOM);
	return _ExtGetProgramsQCOM(programs, maxPrograms, numPrograms);
#endif
}
inline GLboolean DYNAMICGLES_FUNCTION(ExtIsProgramBinaryQCOM)(GLuint program)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLEXTISPROGRAMBINARYQCOMPROC _ExtIsProgramBinaryQCOM = (PFNGLEXTISPROGRAMBINARYQCOMPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ExtIsProgramBinaryQCOM);
	return _ExtIsProgramBinaryQCOM(program);
#endif
}
inline void DYNAMICGLES_FUNCTION(ExtGetProgramBinarySourceQCOM)(GLuint program, GLenum shadertype, char* source, GLint* length)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLEXTGETPROGRAMBINARYSOURCEQCOMPROC _ExtGetProgramBinarySourceQCOM =
		(PFNGLEXTGETPROGRAMBINARYSOURCEQCOMPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ExtGetProgramBinarySourceQCOM);
	return _ExtGetProgramBinarySourceQCOM(program, shadertype, source, length);
#endif
}
inline void DYNAMICGLES_FUNCTION(StartTilingQCOM)(GLuint x, GLuint y, GLuint width, GLuint height, GLbitfield preserveMask)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLSTARTTILINGQCOMPROC _StartTilingQCOM = (PFNGLSTARTTILINGQCOMPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::StartTilingQCOM);
	return _StartTilingQCOM(x, y, width, height, preserveMask);
#endif
}
inline void DYNAMICGLES_FUNCTION(EndTilingQCOM)(GLbitfield preserveMask)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLENDTILINGQCOMPROC _EndTilingQCOM = (PFNGLENDTILINGQCOMPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::EndTilingQCOM);
	return _EndTilingQCOM(preserveMask);
#endif
}
inline void DYNAMICGLES_FUNCTION(GetProgramBinaryOES)(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETPROGRAMBINARYOESPROC _GetProgramBinaryOES = (PFNGLGETPROGRAMBINARYOESPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetProgramBinaryOES);
	return _GetProgramBinaryOES(program, bufSize, length, binaryFormat, binary);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramBinaryOES)(GLuint program, GLenum binaryFormat, const GLvoid* binary, GLint length)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMBINARYOESPROC _ProgramBinaryOES = (PFNGLPROGRAMBINARYOESPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramBinaryOES);
	return _ProgramBinaryOES(program, binaryFormat, binary, length);
#endif
}
inline void DYNAMICGLES_FUNCTION(TexImage3DOES)(
	GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLTEXIMAGE3DOESPROC _TexImage3DOES = (PFNGLTEXIMAGE3DOESPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::TexImage3DOES);
	return _TexImage3DOES(target, level, internalformat, width, height, depth, border, format, type, pixels);
#endif
}
inline void DYNAMICGLES_FUNCTION(TexSubImage3DOES)(
	GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLTEXSUBIMAGE3DOESPROC _TexSubImage3DOES = (PFNGLTEXSUBIMAGE3DOESPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::TexSubImage3DOES);
	return _TexSubImage3DOES(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
#endif
}
inline void DYNAMICGLES_FUNCTION(CopyTexSubImage3DOES)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLCOPYTEXSUBIMAGE3DOESPROC _CopyTexSubImage3DOES = (PFNGLCOPYTEXSUBIMAGE3DOESPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::CopyTexSubImage3DOES);
	return _CopyTexSubImage3DOES(target, level, xoffset, yoffset, zoffset, x, y, width, height);
#endif
}
inline void DYNAMICGLES_FUNCTION(CompressedTexImage3DOES)(
	GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLCOMPRESSEDTEXIMAGE3DOESPROC _CompressedTexImage3DOES =
		(PFNGLCOMPRESSEDTEXIMAGE3DOESPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::CompressedTexImage3DOES);
	return _CompressedTexImage3DOES(target, level, internalformat, width, height, depth, border, imageSize, data);
#endif
}
inline void DYNAMICGLES_FUNCTION(CompressedTexSubImage3DOES)(
	GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLCOMPRESSEDTEXSUBIMAGE3DOESPROC _CompressedTexSubImage3DOES =
		(PFNGLCOMPRESSEDTEXSUBIMAGE3DOESPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::CompressedTexSubImage3DOES);
	return _CompressedTexSubImage3DOES(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
#endif
}
inline void DYNAMICGLES_FUNCTION(FramebufferTexture3DOES)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLFRAMEBUFFERTEXTURE3DOESPROC _FramebufferTexture3DOES =
		(PFNGLFRAMEBUFFERTEXTURE3DOESPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::FramebufferTexture3DOES);
	return _FramebufferTexture3DOES(target, attachment, textarget, texture, level, zoffset);
#endif
}
inline void DYNAMICGLES_FUNCTION(BlendEquationSeparateOES)(GLenum modeRGB, GLenum modeAlpha)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLBLENDEQUATIONSEPARATEPROC _BlendEquationSeparateOES = (PFNGLBLENDEQUATIONSEPARATEPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::BlendEquationSeparateOES);
	return _BlendEquationSeparateOES(modeRGB, modeAlpha);
#endif
}

inline void DYNAMICGLES_FUNCTION(CopyTextureLevelsAPPLE)(GLuint destinationTexture, GLuint sourceTexture, GLint sourceBaseLevel, GLsizei sourceLevelCount)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glCopyTextureLevelsAPPLE) PFNGLCOPYTEXTURELEVELSAPPLEPROC;
#endif
	PFNGLCOPYTEXTURELEVELSAPPLEPROC _CopyTextureLevelsAPPLE = (PFNGLCOPYTEXTURELEVELSAPPLEPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::CopyTextureLevelsAPPLE);
	return _CopyTextureLevelsAPPLE(destinationTexture, sourceTexture, sourceBaseLevel, sourceLevelCount);
}
inline GLsync DYNAMICGLES_FUNCTION(FenceSyncAPPLE)(GLenum condition, GLbitfield flags)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glFenceSyncAPPLE) PFNGLFENCESYNCAPPLEPROC;
#endif
	PFNGLFENCESYNCAPPLEPROC _FenceSyncAPPLE = (PFNGLFENCESYNCAPPLEPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::FenceSyncAPPLE);
	return _FenceSyncAPPLE(condition, flags);
}
inline GLboolean DYNAMICGLES_FUNCTION(IsSyncAPPLE)(GLsync sync)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glIsSyncAPPLE) PFNGLISSYNCAPPLEPROC;
#endif
	PFNGLISSYNCAPPLEPROC _IsSyncAPPLE = (PFNGLISSYNCAPPLEPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::IsSyncAPPLE);
	return _IsSyncAPPLE(sync);
}
inline void DYNAMICGLES_FUNCTION(DeleteSyncAPPLE)(GLsync sync)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glDeleteSyncAPPLE) PFNGLDELETESYNCAPPLEPROC;
#endif
	PFNGLDELETESYNCAPPLEPROC _DeleteSyncAPPLE = (PFNGLDELETESYNCAPPLEPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::DeleteSyncAPPLE);
	return _DeleteSyncAPPLE(sync);
}
inline GLenum DYNAMICGLES_FUNCTION(ClientWaitSyncAPPLE)(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glClientWaitSyncAPPLE) PFNGLCLIENTWAITSYNCAPPLEPROC;
#endif
	PFNGLCLIENTWAITSYNCAPPLEPROC _ClientWaitSyncAPPLE = (PFNGLCLIENTWAITSYNCAPPLEPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ClientWaitSyncAPPLE);
	return _ClientWaitSyncAPPLE(sync, flags, timeout);
}
inline void DYNAMICGLES_FUNCTION(WaitSyncAPPLE)(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glWaitSyncAPPLE) PFNGLWAITSYNCAPPLEPROC;
#endif
	PFNGLWAITSYNCAPPLEPROC _WaitSyncAPPLE = (PFNGLWAITSYNCAPPLEPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::WaitSyncAPPLE);
	return _WaitSyncAPPLE(sync, flags, timeout);
}
inline void DYNAMICGLES_FUNCTION(GetInteger64vAPPLE)(GLenum pname, GLint64* params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetInteger64vAPPLE) PFNGLGETINTEGER64VAPPLEPROC;
#endif
	PFNGLGETINTEGER64VAPPLEPROC _GetInteger64vAPPLE = (PFNGLGETINTEGER64VAPPLEPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetInteger64vAPPLE);
	return _GetInteger64vAPPLE(pname, params);
}
inline void DYNAMICGLES_FUNCTION(GetSyncivAPPLE)(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetSyncivAPPLE) PFNGLGETSYNCIVAPPLEPROC;
#endif
	PFNGLGETSYNCIVAPPLEPROC _GetSyncivAPPLE = (PFNGLGETSYNCIVAPPLEPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetSyncivAPPLE);
	return _GetSyncivAPPLE(sync, pname, bufSize, length, values);
}
inline void* DYNAMICGLES_FUNCTION(MapBufferRangeEXT)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glMapBufferRangeEXT) PFNGLMAPBUFFERRANGEEXTPROC;
#endif
	PFNGLMAPBUFFERRANGEEXTPROC _MapBufferRangeEXT = (PFNGLMAPBUFFERRANGEEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::MapBufferRangeEXT);
	return _MapBufferRangeEXT(target, offset, length, access);
}
inline void DYNAMICGLES_FUNCTION(FlushMappedBufferRangeEXT)(GLenum target, GLintptr offset, GLsizeiptr length)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glFlushMappedBufferRangeEXT) PFNGLFLUSHMAPPEDBUFFERRANGEEXTPROC;
#endif
	PFNGLFLUSHMAPPEDBUFFERRANGEEXTPROC _FlushMappedBufferRangeEXT =
		(PFNGLFLUSHMAPPEDBUFFERRANGEEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::FlushMappedBufferRangeEXT);
	return _FlushMappedBufferRangeEXT(target, offset, length);
}
inline void DYNAMICGLES_FUNCTION(RenderbufferStorageMultisampleEXT)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC _RenderbufferStorageMultisampleEXT =
		(PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::RenderbufferStorageMultisampleEXT);
	return _RenderbufferStorageMultisampleEXT(target, samples, internalformat, width, height);
#endif
}
inline void DYNAMICGLES_FUNCTION(FramebufferTexture2DMultisampleEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC _FramebufferTexture2DMultisampleEXT =
		(PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::FramebufferTexture2DMultisampleEXT);
	return _FramebufferTexture2DMultisampleEXT(target, attachment, textarget, texture, level, samples);
#endif
}
inline GLenum DYNAMICGLES_FUNCTION(GetGraphicsResetStatusEXT)(void)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETGRAPHICSRESETSTATUSEXTPROC _GetGraphicsResetStatusEXT =
		(PFNGLGETGRAPHICSRESETSTATUSEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetGraphicsResetStatusEXT);
	return _GetGraphicsResetStatusEXT();
#endif
}
inline void DYNAMICGLES_FUNCTION(ReadnPixelsEXT)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void* data)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLREADNPIXELSEXTPROC _ReadnPixelsEXT = (PFNGLREADNPIXELSEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ReadnPixelsEXT);
	return _ReadnPixelsEXT(x, y, width, height, format, type, bufSize, data);
#endif
}
inline void DYNAMICGLES_FUNCTION(GetnUniformfvEXT)(GLuint program, GLint location, GLsizei bufSize, float* params)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETNUNIFORMFVEXTPROC _GetnUniformfvEXT = (PFNGLGETNUNIFORMFVEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetnUniformfvEXT);
	return _GetnUniformfvEXT(program, location, bufSize, params);
#endif
}
inline void DYNAMICGLES_FUNCTION(GetnUniformivEXT)(GLuint program, GLint location, GLsizei bufSize, GLint* params)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETNUNIFORMIVEXTPROC _GetnUniformivEXT = (PFNGLGETNUNIFORMIVEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetnUniformivEXT);
	return _GetnUniformivEXT(program, location, bufSize, params);
#endif
}
inline void DYNAMICGLES_FUNCTION(TexStorage1DEXT)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLTEXSTORAGE1DEXTPROC _TexStorage1DEXT = (PFNGLTEXSTORAGE1DEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::TexStorage1DEXT);
	return _TexStorage1DEXT(target, levels, internalformat, width);
#endif
}
inline void DYNAMICGLES_FUNCTION(TexStorage2DEXT)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLTEXSTORAGE2DEXTPROC _TexStorage2DEXT = (PFNGLTEXSTORAGE2DEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::TexStorage2DEXT);
	return _TexStorage2DEXT(target, levels, internalformat, width, height);
#endif
}
inline void DYNAMICGLES_FUNCTION(TexStorage3DEXT)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLTEXSTORAGE3DEXTPROC _TexStorage3DEXT = (PFNGLTEXSTORAGE3DEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::TexStorage3DEXT);
	return _TexStorage3DEXT(target, levels, internalformat, width, height, depth);
#endif
}
inline void DYNAMICGLES_FUNCTION(TextureStorage1DEXT)(GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLTEXTURESTORAGE1DEXTPROC _TextureStorage1DEXT = (PFNGLTEXTURESTORAGE1DEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::TextureStorage1DEXT);
	return _TextureStorage1DEXT(texture, target, levels, internalformat, width);
#endif
}
inline void DYNAMICGLES_FUNCTION(TextureStorage2DEXT)(GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLTEXTURESTORAGE2DEXTPROC _TextureStorage2DEXT = (PFNGLTEXTURESTORAGE2DEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::TextureStorage2DEXT);
	return _TextureStorage2DEXT(texture, target, levels, internalformat, width, height);
#endif
}
inline void DYNAMICGLES_FUNCTION(TextureStorage3DEXT)(GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLTEXTURESTORAGE3DEXTPROC _TextureStorage3DEXT = (PFNGLTEXTURESTORAGE3DEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::TextureStorage3DEXT);
	return _TextureStorage3DEXT(texture, target, levels, internalformat, width, height, depth);
#endif
}
inline void DYNAMICGLES_FUNCTION(DebugMessageControlKHR)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLDEBUGMESSAGECONTROLKHRPROC _DebugMessageControlKHR = (PFNGLDEBUGMESSAGECONTROLKHRPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::DebugMessageControlKHR);
	return _DebugMessageControlKHR(source, type, severity, count, ids, enabled);
#endif
}
inline void DYNAMICGLES_FUNCTION(DebugMessageInsertKHR)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLDEBUGMESSAGEINSERTKHRPROC _DebugMessageInsertKHR = (PFNGLDEBUGMESSAGEINSERTKHRPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::DebugMessageInsertKHR);
	return _DebugMessageInsertKHR(source, type, id, severity, length, buf);
#endif
}
#if !TARGET_OS_IPHONE
inline void DYNAMICGLES_FUNCTION(DebugMessageCallbackKHR)(GLDEBUGPROCKHR callback, const void* userParam)
{
	PFNGLDEBUGMESSAGECALLBACKKHRPROC _DebugMessageCallbackKHR =
		(PFNGLDEBUGMESSAGECALLBACKKHRPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::DebugMessageCallbackKHR);
	return _DebugMessageCallbackKHR(callback, userParam);
}
#endif
inline GLuint DYNAMICGLES_FUNCTION(GetDebugMessageLogKHR)(
	GLuint count, GLsizei bufsize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETDEBUGMESSAGELOGKHRPROC _GetDebugMessageLogKHR = (PFNGLGETDEBUGMESSAGELOGKHRPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetDebugMessageLogKHR);
	return _GetDebugMessageLogKHR(count, bufsize, sources, types, ids, severities, lengths, messageLog);
#endif
}
inline void DYNAMICGLES_FUNCTION(PushDebugGroupKHR)(GLenum source, GLuint id, GLsizei length, const GLchar* message)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPUSHDEBUGGROUPKHRPROC _PushDebugGroupKHR = (PFNGLPUSHDEBUGGROUPKHRPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::PushDebugGroupKHR);
	return _PushDebugGroupKHR(source, id, length, message);
#endif
}
inline void DYNAMICGLES_FUNCTION(PopDebugGroupKHR)(void)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPOPDEBUGGROUPKHRPROC _PopDebugGroupKHR = (PFNGLPOPDEBUGGROUPKHRPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::PopDebugGroupKHR);
	return _PopDebugGroupKHR();
#endif
}
inline void DYNAMICGLES_FUNCTION(ObjectLabelKHR)(GLenum identifier, GLuint name, GLsizei length, const GLchar* label)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLOBJECTLABELKHRPROC _ObjectLabelKHR = (PFNGLOBJECTLABELKHRPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ObjectLabelKHR);
	return _ObjectLabelKHR(identifier, name, length, label);
#endif
}
inline void DYNAMICGLES_FUNCTION(GetObjectLabelKHR)(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei* length, GLchar* label)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETOBJECTLABELKHRPROC _GetObjectLabelKHR = (PFNGLGETOBJECTLABELKHRPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetObjectLabelKHR);
	return _GetObjectLabelKHR(identifier, name, bufSize, length, label);
#endif
}
inline void DYNAMICGLES_FUNCTION(ObjectPtrLabelKHR)(const void* ptr, GLsizei length, const GLchar* label)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLOBJECTPTRLABELKHRPROC _ObjectPtrLabelKHR = (PFNGLOBJECTPTRLABELKHRPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ObjectPtrLabelKHR);
	return _ObjectPtrLabelKHR(ptr, length, label);
#endif
}
inline void DYNAMICGLES_FUNCTION(GetObjectPtrLabelKHR)(const void* ptr, GLsizei bufSize, GLsizei* length, GLchar* label)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETOBJECTPTRLABELKHRPROC _GetObjectPtrLabelKHR = (PFNGLGETOBJECTPTRLABELKHRPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetObjectPtrLabelKHR);
	return _GetObjectPtrLabelKHR(ptr, bufSize, length, label);
#endif
}
inline void DYNAMICGLES_FUNCTION(GetPointervKHR)(GLenum pname, void** params)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETPOINTERVKHRPROC _GetPointervKHR = (PFNGLGETPOINTERVKHRPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetPointervKHR);
	return _GetPointervKHR(pname, params);
#endif
}
inline void DYNAMICGLES_FUNCTION(DrawArraysInstancedANGLE)(GLenum mode, GLint first, GLsizei count, GLsizei primcount)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLDRAWARRAYSINSTANCEDANGLEPROC _DrawArraysInstancedANGLE =
		(PFNGLDRAWARRAYSINSTANCEDANGLEPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::DrawArraysInstancedANGLE);
	return _DrawArraysInstancedANGLE(mode, first, count, primcount);
#endif
}
inline void DYNAMICGLES_FUNCTION(DrawElementsInstancedANGLE)(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei primcount)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLDRAWELEMENTSINSTANCEDANGLEPROC _DrawElementsInstancedANGLE =
		(PFNGLDRAWELEMENTSINSTANCEDANGLEPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::DrawElementsInstancedANGLE);
	return _DrawElementsInstancedANGLE(mode, count, type, indices, primcount);
#endif
}
inline void DYNAMICGLES_FUNCTION(VertexAttribDivisorANGLE)(GLuint index, GLuint divisor)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLVERTEXATTRIBDIVISORANGLEPROC _VertexAttribDivisorANGLE =
		(PFNGLVERTEXATTRIBDIVISORANGLEPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::VertexAttribDivisorANGLE);
	return _VertexAttribDivisorANGLE(index, divisor);
#endif
}
inline void DYNAMICGLES_FUNCTION(GetTranslatedShaderSourceANGLE)(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETTRANSLATEDSHADERSOURCEANGLEPROC _GetTranslatedShaderSourceANGLE =
		(PFNGLGETTRANSLATEDSHADERSOURCEANGLEPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetTranslatedShaderSourceANGLE);
	return _GetTranslatedShaderSourceANGLE(shader, bufsize, length, source);
#endif
}
inline void DYNAMICGLES_FUNCTION(LabelObjectEXT)(GLenum type, GLuint object, GLsizei length, const GLchar* label)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLLABELOBJECTEXTPROC _LabelObjectEXT = (PFNGLLABELOBJECTEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::LabelObjectEXT);
	return _LabelObjectEXT(type, object, length, label);
#endif
}
inline void DYNAMICGLES_FUNCTION(GetObjectLabelEXT)(GLenum type, GLuint object, GLsizei bufSize, GLsizei* length, GLchar* label)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETOBJECTLABELEXTPROC _GetObjectLabelEXT = (PFNGLGETOBJECTLABELEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetObjectLabelEXT);
	return _GetObjectLabelEXT(type, object, bufSize, length, label);
#endif
}
inline void DYNAMICGLES_FUNCTION(InsertEventMarkerEXT)(GLsizei length, const GLchar* marker)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLINSERTEVENTMARKEREXTPROC _InsertEventMarkerEXT = (PFNGLINSERTEVENTMARKEREXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::InsertEventMarkerEXT);
	return _InsertEventMarkerEXT(length, marker);
#endif
}
inline void DYNAMICGLES_FUNCTION(PushGroupMarkerEXT)(GLsizei length, const GLchar* marker)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPUSHGROUPMARKEREXTPROC _PushGroupMarkerEXT = (PFNGLPUSHGROUPMARKEREXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::PushGroupMarkerEXT);
	return _PushGroupMarkerEXT(length, marker);
#endif
}
inline void DYNAMICGLES_FUNCTION(PopGroupMarkerEXT)(void)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPOPGROUPMARKEREXTPROC _PopGroupMarkerEXT = (PFNGLPOPGROUPMARKEREXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::PopGroupMarkerEXT);
	return _PopGroupMarkerEXT();
#endif
}
inline void DYNAMICGLES_FUNCTION(GenQueriesEXT)(GLsizei n, GLuint* ids)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGENQUERIESEXTPROC _GenQueriesEXT = (PFNGLGENQUERIESEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GenQueriesEXT);
	return _GenQueriesEXT(n, ids);
#endif
}
inline void DYNAMICGLES_FUNCTION(DeleteQueriesEXT)(GLsizei n, const GLuint* ids)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLDELETEQUERIESEXTPROC _DeleteQueriesEXT = (PFNGLDELETEQUERIESEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::DeleteQueriesEXT);
	return _DeleteQueriesEXT(n, ids);
#endif
}
inline GLboolean DYNAMICGLES_FUNCTION(IsQueryEXT)(GLuint id)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLISQUERYEXTPROC _IsQueryEXT = (PFNGLISQUERYEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::IsQueryEXT);
	return _IsQueryEXT(id);
#endif
}
inline void DYNAMICGLES_FUNCTION(BeginQueryEXT)(GLenum target, GLuint id)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLBEGINQUERYEXTPROC _BeginQueryEXT = (PFNGLBEGINQUERYEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::BeginQueryEXT);
	return _BeginQueryEXT(target, id);
#endif
}
inline void DYNAMICGLES_FUNCTION(EndQueryEXT)(GLenum target)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glEndQueryEXT) PFNGLENDQUERYEXTPROC;
#endif
	PFNGLENDQUERYEXTPROC _EndQueryEXT = (PFNGLENDQUERYEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::EndQueryEXT);
	return _EndQueryEXT(target);
}
inline void DYNAMICGLES_FUNCTION(GetQueryivEXT)(GLenum target, GLenum pname, GLint* params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetQueryivEXT) PFNGLGETQUERYIVEXTPROC;
#endif
	PFNGLGETQUERYIVEXTPROC _GetQueryivEXT = (PFNGLGETQUERYIVEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetQueryivEXT);
	return _GetQueryivEXT(target, pname, params);
}
inline void DYNAMICGLES_FUNCTION(GetQueryObjectuivEXT)(GLuint id, GLenum pname, GLuint* params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetQueryObjectuivEXT) PFNGLGETQUERYOBJECTUIVEXTPROC;
#endif
	PFNGLGETQUERYOBJECTUIVEXTPROC _GetQueryObjectuivEXT = (PFNGLGETQUERYOBJECTUIVEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetQueryObjectuivEXT);
	return _GetQueryObjectuivEXT(id, pname, params);
}
inline void DYNAMICGLES_FUNCTION(UseProgramStagesEXT)(GLuint pipeline, GLbitfield stages, GLuint program)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glUseProgramStagesEXT) PFNGLUSEPROGRAMSTAGESEXTPROC;
#endif
	PFNGLUSEPROGRAMSTAGESEXTPROC _UseProgramStagesEXT = (PFNGLUSEPROGRAMSTAGESEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::UseProgramStagesEXT);
	return _UseProgramStagesEXT(pipeline, stages, program);
}
inline void DYNAMICGLES_FUNCTION(ActiveShaderProgramEXT)(GLuint pipeline, GLuint program)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glActiveShaderProgramEXT) PFNGLACTIVESHADERPROGRAMEXTPROC;
#endif
	PFNGLACTIVESHADERPROGRAMEXTPROC _ActiveShaderProgramEXT = (PFNGLACTIVESHADERPROGRAMEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ActiveShaderProgramEXT);
	return _ActiveShaderProgramEXT(pipeline, program);
}
inline GLuint DYNAMICGLES_FUNCTION(CreateShaderProgramvEXT)(GLenum type, GLsizei count, const GLchar** strings)
{
#if TARGET_OS_IPHONE
	assert(0);
	return 0;
#else
	PFNGLCREATESHADERPROGRAMVEXTPROC _CreateShaderProgramvEXT =
		(PFNGLCREATESHADERPROGRAMVEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::CreateShaderProgramvEXT);
	return _CreateShaderProgramvEXT(type, count, strings);
#endif
}
inline void DYNAMICGLES_FUNCTION(BindProgramPipelineEXT)(GLuint pipeline)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glBindProgramPipelineEXT) PFNGLBINDPROGRAMPIPELINEEXTPROC;
#endif
	PFNGLBINDPROGRAMPIPELINEEXTPROC _BindProgramPipelineEXT = (PFNGLBINDPROGRAMPIPELINEEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::BindProgramPipelineEXT);
	return _BindProgramPipelineEXT(pipeline);
}
inline void DYNAMICGLES_FUNCTION(DeleteProgramPipelinesEXT)(GLsizei n, const GLuint* pipelines)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glDeleteProgramPipelinesEXT) PFNGLDELETEPROGRAMPIPELINESEXTPROC;
#endif
	PFNGLDELETEPROGRAMPIPELINESEXTPROC _DeleteProgramPipelinesEXT =
		(PFNGLDELETEPROGRAMPIPELINESEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::DeleteProgramPipelinesEXT);
	return _DeleteProgramPipelinesEXT(n, pipelines);
}
inline void DYNAMICGLES_FUNCTION(GenProgramPipelinesEXT)(GLsizei n, GLuint* pipelines)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGenProgramPipelinesEXT) PFNGLGENPROGRAMPIPELINESEXTPROC;
#endif
	PFNGLGENPROGRAMPIPELINESEXTPROC _GenProgramPipelinesEXT = (PFNGLGENPROGRAMPIPELINESEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GenProgramPipelinesEXT);
	return _GenProgramPipelinesEXT(n, pipelines);
}
inline GLboolean DYNAMICGLES_FUNCTION(IsProgramPipelineEXT)(GLuint pipeline)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glIsProgramPipelineEXT) PFNGLISPROGRAMPIPELINEEXTPROC;
#endif
	PFNGLISPROGRAMPIPELINEEXTPROC _IsProgramPipelineEXT = (PFNGLISPROGRAMPIPELINEEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::IsProgramPipelineEXT);
	return _IsProgramPipelineEXT(pipeline);
}
inline void DYNAMICGLES_FUNCTION(ProgramParameteriEXT)(GLuint program, GLenum pname, GLint value)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glProgramParameteriEXT) PFNGLPROGRAMPARAMETERIEXTPROC;
#endif
	PFNGLPROGRAMPARAMETERIEXTPROC _ProgramParameteriEXT = (PFNGLPROGRAMPARAMETERIEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramParameteriEXT);
	return _ProgramParameteriEXT(program, pname, value);
}
inline void DYNAMICGLES_FUNCTION(GetProgramPipelineivEXT)(GLuint pipeline, GLenum pname, GLint* params)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetProgramPipelineivEXT) PFNGLGETPROGRAMPIPELINEIVEXTPROC;
#endif
	PFNGLGETPROGRAMPIPELINEIVEXTPROC _GetProgramPipelineivEXT =
		(PFNGLGETPROGRAMPIPELINEIVEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetProgramPipelineivEXT);
	return _GetProgramPipelineivEXT(pipeline, pname, params);
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform1iEXT)(GLuint program, GLint location, GLint x)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM1IEXTPROC _ProgramUniform1iEXT = (PFNGLPROGRAMUNIFORM1IEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniform1iEXT);
	return _ProgramUniform1iEXT(program, location, x);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform2iEXT)(GLuint program, GLint location, GLint x, GLint y)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM2IEXTPROC _ProgramUniform2iEXT = (PFNGLPROGRAMUNIFORM2IEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniform2iEXT);
	return _ProgramUniform2iEXT(program, location, x, y);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform3iEXT)(GLuint program, GLint location, GLint x, GLint y, GLint z)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM3IEXTPROC _ProgramUniform3iEXT = (PFNGLPROGRAMUNIFORM3IEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniform3iEXT);
	return _ProgramUniform3iEXT(program, location, x, y, z);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform4iEXT)(GLuint program, GLint location, GLint x, GLint y, GLint z, GLint w)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM4IEXTPROC _ProgramUniform4iEXT = (PFNGLPROGRAMUNIFORM4IEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniform4iEXT);
	return _ProgramUniform4iEXT(program, location, x, y, z, w);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform1fEXT)(GLuint program, GLint location, GLfloat x)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM1FEXTPROC _ProgramUniform1fEXT = (PFNGLPROGRAMUNIFORM1FEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniform1fEXT);
	return _ProgramUniform1fEXT(program, location, x);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform2fEXT)(GLuint program, GLint location, GLfloat x, GLfloat y)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM2FEXTPROC _ProgramUniform2fEXT = (PFNGLPROGRAMUNIFORM2FEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniform2fEXT);
	return _ProgramUniform2fEXT(program, location, x, y);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform3fEXT)(GLuint program, GLint location, GLfloat x, GLfloat y, GLfloat z)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM3FEXTPROC _ProgramUniform3fEXT = (PFNGLPROGRAMUNIFORM3FEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniform3fEXT);
	return _ProgramUniform3fEXT(program, location, x, y, z);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform4fEXT)(GLuint program, GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM4FEXTPROC _ProgramUniform4fEXT = (PFNGLPROGRAMUNIFORM4FEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniform4fEXT);
	return _ProgramUniform4fEXT(program, location, x, y, z, w);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform1ivEXT)(GLuint program, GLint location, GLsizei count, const GLint* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM1IVEXTPROC _ProgramUniform1ivEXT = (PFNGLPROGRAMUNIFORM1IVEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniform1ivEXT);
	return _ProgramUniform1ivEXT(program, location, count, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform2ivEXT)(GLuint program, GLint location, GLsizei count, const GLint* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM2IVEXTPROC _ProgramUniform2ivEXT = (PFNGLPROGRAMUNIFORM2IVEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniform2ivEXT);
	return _ProgramUniform2ivEXT(program, location, count, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform3ivEXT)(GLuint program, GLint location, GLsizei count, const GLint* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM3IVEXTPROC _ProgramUniform3ivEXT = (PFNGLPROGRAMUNIFORM3IVEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniform3ivEXT);
	return _ProgramUniform3ivEXT(program, location, count, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform4ivEXT)(GLuint program, GLint location, GLsizei count, const GLint* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM4IVEXTPROC _ProgramUniform4ivEXT = (PFNGLPROGRAMUNIFORM4IVEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniform4ivEXT);
	return _ProgramUniform4ivEXT(program, location, count, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform1fvEXT)(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM1FVEXTPROC _ProgramUniform1fvEXT = (PFNGLPROGRAMUNIFORM1FVEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniform1fvEXT);
	return _ProgramUniform1fvEXT(program, location, count, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform2fvEXT)(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM2FVEXTPROC _ProgramUniform2fvEXT = (PFNGLPROGRAMUNIFORM2FVEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniform2fvEXT);
	return _ProgramUniform2fvEXT(program, location, count, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform3fvEXT)(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM3FVEXTPROC _ProgramUniform3fvEXT = (PFNGLPROGRAMUNIFORM3FVEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniform3fvEXT);
	return _ProgramUniform3fvEXT(program, location, count, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform4fvEXT)(GLuint program, GLint location, GLsizei count, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM4FVEXTPROC _ProgramUniform4fvEXT = (PFNGLPROGRAMUNIFORM4FVEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniform4fvEXT);
	return _ProgramUniform4fvEXT(program, location, count, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniformMatrix2fvEXT)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORMMATRIX2FVEXTPROC _ProgramUniformMatrix2fvEXT =
		(PFNGLPROGRAMUNIFORMMATRIX2FVEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniformMatrix2fvEXT);
	return _ProgramUniformMatrix2fvEXT(program, location, count, transpose, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniformMatrix3fvEXT)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORMMATRIX3FVEXTPROC _ProgramUniformMatrix3fvEXT =
		(PFNGLPROGRAMUNIFORMMATRIX3FVEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniformMatrix3fvEXT);
	return _ProgramUniformMatrix3fvEXT(program, location, count, transpose, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniformMatrix4fvEXT)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC _ProgramUniformMatrix4fvEXT =
		(PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniformMatrix4fvEXT);
	return _ProgramUniformMatrix4fvEXT(program, location, count, transpose, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ValidateProgramPipelineEXT)(GLuint pipeline)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glValidateProgramPipelineEXT) PFNGLVALIDATEPROGRAMPIPELINEEXTPROC;
#endif
	PFNGLVALIDATEPROGRAMPIPELINEEXTPROC _ValidateProgramPipelineEXT =
		(PFNGLVALIDATEPROGRAMPIPELINEEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ValidateProgramPipelineEXT);
	return _ValidateProgramPipelineEXT(pipeline);
}
inline void DYNAMICGLES_FUNCTION(GetProgramPipelineInfoLogEXT)(GLuint pipeline, GLsizei bufSize, GLsizei* length, GLchar* infoLog)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetProgramPipelineInfoLogEXT) PFNGLGETPROGRAMPIPELINEINFOLOGEXTPROC;
#endif
	PFNGLGETPROGRAMPIPELINEINFOLOGEXTPROC _GetProgramPipelineInfoLogEXT =
		(PFNGLGETPROGRAMPIPELINEINFOLOGEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetProgramPipelineInfoLogEXT);
	return _GetProgramPipelineInfoLogEXT(pipeline, bufSize, length, infoLog);
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform1uiEXT)(GLuint program, GLint location, GLuint v0)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM1UIEXTPROC _ProgramUniform1uiEXT = (PFNGLPROGRAMUNIFORM1UIEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniform1uiEXT);
	return _ProgramUniform1uiEXT(program, location, v0);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform2uiEXT)(GLuint program, GLint location, GLuint v0, GLuint v1)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM2UIEXTPROC _ProgramUniform2uiEXT = (PFNGLPROGRAMUNIFORM2UIEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniform2uiEXT);
	return _ProgramUniform2uiEXT(program, location, v0, v1);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform3uiEXT)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM3UIEXTPROC _ProgramUniform3uiEXT = (PFNGLPROGRAMUNIFORM3UIEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniform3uiEXT);
	return _ProgramUniform3uiEXT(program, location, v0, v1, v2);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform4uiEXT)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM4UIEXTPROC _ProgramUniform4uiEXT = (PFNGLPROGRAMUNIFORM4UIEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniform4uiEXT);
	return _ProgramUniform4uiEXT(program, location, v0, v1, v2, v3);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform1uivEXT)(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM1UIVEXTPROC _ProgramUniform1uivEXT = (PFNGLPROGRAMUNIFORM1UIVEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniform1uivEXT);
	return _ProgramUniform1uivEXT(program, location, count, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform2uivEXT)(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM2UIVEXTPROC _ProgramUniform2uivEXT = (PFNGLPROGRAMUNIFORM2UIVEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniform2uivEXT);
	return _ProgramUniform2uivEXT(program, location, count, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform3uivEXT)(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM3UIVEXTPROC _ProgramUniform3uivEXT = (PFNGLPROGRAMUNIFORM3UIVEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniform3uivEXT);
	return _ProgramUniform3uivEXT(program, location, count, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniform4uivEXT)(GLuint program, GLint location, GLsizei count, const GLuint* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORM4UIVEXTPROC _ProgramUniform4uivEXT = (PFNGLPROGRAMUNIFORM4UIVEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniform4uivEXT);
	return _ProgramUniform4uivEXT(program, location, count, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniformMatrix2x3fvEXT)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORMMATRIX2X3FVEXTPROC _ProgramUniformMatrix2x3fvEXT =
		(PFNGLPROGRAMUNIFORMMATRIX2X3FVEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniformMatrix2x3fvEXT);
	return _ProgramUniformMatrix2x3fvEXT(program, location, count, transpose, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniformMatrix3x2fvEXT)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORMMATRIX3X2FVEXTPROC _ProgramUniformMatrix3x2fvEXT =
		(PFNGLPROGRAMUNIFORMMATRIX3X2FVEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniformMatrix3x2fvEXT);
	return _ProgramUniformMatrix3x2fvEXT(program, location, count, transpose, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniformMatrix2x4fvEXT)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORMMATRIX2X4FVEXTPROC _ProgramUniformMatrix2x4fvEXT =
		(PFNGLPROGRAMUNIFORMMATRIX2X4FVEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniformMatrix2x4fvEXT);
	return _ProgramUniformMatrix2x4fvEXT(program, location, count, transpose, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniformMatrix4x2fvEXT)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORMMATRIX4X2FVEXTPROC _ProgramUniformMatrix4x2fvEXT =
		(PFNGLPROGRAMUNIFORMMATRIX4X2FVEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniformMatrix4x2fvEXT);
	return _ProgramUniformMatrix4x2fvEXT(program, location, count, transpose, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniformMatrix3x4fvEXT)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORMMATRIX3X4FVEXTPROC _ProgramUniformMatrix3x4fvEXT =
		(PFNGLPROGRAMUNIFORMMATRIX3X4FVEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniformMatrix3x4fvEXT);
	return _ProgramUniformMatrix3x4fvEXT(program, location, count, transpose, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(ProgramUniformMatrix4x3fvEXT)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPROGRAMUNIFORMMATRIX4X3FVEXTPROC _ProgramUniformMatrix4x3fvEXT =
		(PFNGLPROGRAMUNIFORMMATRIX4X3FVEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniformMatrix4x3fvEXT);
	return _ProgramUniformMatrix4x3fvEXT(program, location, count, transpose, value);
#endif
}
inline void DYNAMICGLES_FUNCTION(AlphaFuncQCOM)(GLenum func, GLclampf ref)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLALPHAFUNCQCOMPROC _AlphaFuncQCOM = (PFNGLALPHAFUNCQCOMPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::AlphaFuncQCOM);
	return _AlphaFuncQCOM(func, ref);
#endif
}
inline void DYNAMICGLES_FUNCTION(ReadBufferNV)(GLenum mode)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLREADBUFFERNVPROC _ReadBufferNV = (PFNGLREADBUFFERNVPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ReadBufferNV);
	return _ReadBufferNV(mode);
#endif
}
inline void DYNAMICGLES_FUNCTION(DrawBuffersNV)(GLsizei n, const GLenum* bufs)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLDRAWBUFFERSNVPROC _DrawBuffersNV = (PFNGLDRAWBUFFERSNVPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::DrawBuffersNV);
	return _DrawBuffersNV(n, bufs);
#endif
}
inline void DYNAMICGLES_FUNCTION(ReadBufferIndexedEXT)(GLenum src, GLint index)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLREADBUFFERINDEXEDEXTPROC _ReadBufferIndexedEXT = (PFNGLREADBUFFERINDEXEDEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ReadBufferIndexedEXT);
	return _ReadBufferIndexedEXT(src, index);
#endif
}
inline void DYNAMICGLES_FUNCTION(DrawBuffersIndexedEXT)(GLint n, const GLenum* location, const GLint* indices)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLDRAWBUFFERSINDEXEDEXTPROC _DrawBuffersIndexedEXT = (PFNGLDRAWBUFFERSINDEXEDEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::DrawBuffersIndexedEXT);
	return _DrawBuffersIndexedEXT(n, location, indices);
#endif
}
inline void DYNAMICGLES_FUNCTION(GetIntegeri_vEXT)(GLenum target, GLuint index, GLint* data)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLGETINTEGERI_VEXTPROC _GetIntegeri_vEXT = (PFNGLGETINTEGERI_VEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetIntegeri_vEXT);
	return _GetIntegeri_vEXT(target, index, data);
#endif
}
inline void DYNAMICGLES_FUNCTION(DrawBuffersEXT)(GLsizei n, const GLenum* bufs)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLDRAWBUFFERSEXTPROC _DrawBuffersEXT = (PFNGLDRAWBUFFERSEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::DrawBuffersEXT);
	return _DrawBuffersEXT(n, bufs);
#endif
}
inline void DYNAMICGLES_FUNCTION(BlendBarrierKHR)(void)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLBLENDBARRIERKHRPROC _BlendBarrierKHR = (PFNGLBLENDBARRIERKHRPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::BlendBarrierKHR);
	return _BlendBarrierKHR();
#endif
}
inline void DYNAMICGLES_FUNCTION(TexStorage3DMultisampleOES)(
	GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLTEXSTORAGE3DMULTISAMPLEOESPROC _TexStorage3DMultisampleOES =
		(PFNGLTEXSTORAGE3DMULTISAMPLEOESPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::TexStorage3DMultisampleOES);
	return _TexStorage3DMultisampleOES(target, samples, internalformat, width, height, depth, fixedsamplelocations);
#endif
}
inline void DYNAMICGLES_FUNCTION(FramebufferTextureMultiviewOVR)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint baseViewIndex, GLsizei numViews)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC _FramebufferTextureMultiviewOVR =
		(PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::FramebufferTextureMultiviewOVR);
	return _FramebufferTextureMultiviewOVR(target, attachment, texture, level, baseViewIndex, numViews);
#endif
}
inline void DYNAMICGLES_FUNCTION(FramebufferPixelLocalStorageSizeEXT)(GLuint target, GLsizei storageSize)
{
	typedef void(GL_APIENTRY * PFNGLFRAMEBUFFERPIXELLOCALSTORAGESIZEPROC)(GLuint target, GLsizei storageSize);
	PFNGLFRAMEBUFFERPIXELLOCALSTORAGESIZEPROC _FramebufferPixelLocalStorageSize =
		(PFNGLFRAMEBUFFERPIXELLOCALSTORAGESIZEPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::FramebufferPixelLocalStorageSizeEXT);
	return _FramebufferPixelLocalStorageSize(target, storageSize);
}
inline void DYNAMICGLES_FUNCTION(ClearPixelLocalStorageuiEXT)(GLsizei offset, GLsizei n, const GLuint* values)
{
	typedef void(GL_APIENTRY * PFNGLCLEARPIXELLOCALSTORAGEUIPROC)(GLsizei offset, GLsizei n, const GLuint* values);
	PFNGLCLEARPIXELLOCALSTORAGEUIPROC _ClearPixelLocalStorageui =
		(PFNGLCLEARPIXELLOCALSTORAGEUIPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ClearPixelLocalStorageuiEXT);
	return _ClearPixelLocalStorageui(offset, n, values);
}
inline void DYNAMICGLES_FUNCTION(GetFramebufferPixelLocalStorageSizeEXT)(GLuint target)
{
	typedef void(GL_APIENTRY * PFNGLGETFRAMEBUFFERPIXELLOCALSTORAGESIZEPROC)(GLuint target);
	PFNGLGETFRAMEBUFFERPIXELLOCALSTORAGESIZEPROC _GetFramebufferPixelLocalStorageSize =
		(PFNGLGETFRAMEBUFFERPIXELLOCALSTORAGESIZEPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetFramebufferPixelLocalStorageSizeEXT);
	return _GetFramebufferPixelLocalStorageSize(target);
}
inline void DYNAMICGLES_FUNCTION(BufferStorageEXT)(GLenum target, GLsizei size, const void* data, GLbitfield flags)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLBUFFERSTORAGEEXTPROC _BufferStorageEXT = (PFNGLBUFFERSTORAGEEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::BufferStorageEXT);
	return _BufferStorageEXT(target, size, data, flags);
#endif
}
inline void DYNAMICGLES_FUNCTION(ClearTexImageEXT)(GLuint texture, GLint level, GLenum format, GLenum type, const GLvoid* data)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLCLEARTEXIMAGEEXTPROC _ClearTexImageEXT = (PFNGLCLEARTEXIMAGEEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ClearTexImageEXT);
	return _ClearTexImageEXT(texture, level, format, type, data);
#endif
}
inline void DYNAMICGLES_FUNCTION(ClearTexSubImageEXT)(
	GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* data)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLCLEARTEXSUBIMAGEEXTPROC _ClearTexSubImageEXT = (PFNGLCLEARTEXSUBIMAGEEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ClearTexSubImageEXT);
	return _ClearTexSubImageEXT(texture, level, xoffset, yoffset, zoffset, width, height, depth, format, type, data);
#endif
}
inline void DYNAMICGLES_FUNCTION(ClearTexSubImageIMG)(
	GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* data)
{
	typedef void(GL_APIENTRY * PFNGLCLEARTEXSUBIMAGEIMGPROC)(
		GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* data);
	PFNGLCLEARTEXSUBIMAGEIMGPROC _ClearTexSubImageIMG = (PFNGLCLEARTEXSUBIMAGEIMGPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ClearTexSubImageIMG);
	return _ClearTexSubImageIMG(texture, level, xoffset, yoffset, zoffset, width, height, depth, format, type, data);
}
inline void DYNAMICGLES_FUNCTION(FramebufferTexture2DDownsampleIMG)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint xscale, GLint yscale)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLFRAMEBUFFERTEXTURE2DDOWNSAMPLEIMGPROC _FramebufferTexture2DDownsampleIMG =
		(PFNGLFRAMEBUFFERTEXTURE2DDOWNSAMPLEIMGPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::FramebufferTexture2DDownsampleIMG);
	return _FramebufferTexture2DDownsampleIMG(target, attachment, textarget, texture, level, xscale, yscale);
#endif
}
inline void DYNAMICGLES_FUNCTION(FramebufferTextureLayerDownsampleIMG)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer, GLint xscale, GLint yscale)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLFRAMEBUFFERTEXTURELAYERDOWNSAMPLEIMGPROC _FramebufferTextureLayerDownsampleIMG =
		(PFNGLFRAMEBUFFERTEXTURELAYERDOWNSAMPLEIMGPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::FramebufferTextureLayerDownsampleIMG);
	return _FramebufferTextureLayerDownsampleIMG(target, attachment, texture, level, layer, xscale, yscale);
#endif
}
inline void DYNAMICGLES_FUNCTION(PatchParameteriEXT)(GLenum pname, GLint val)
{
#if TARGET_OS_IPHONE
	assert(0);
#else
	PFNGLPATCHPARAMETERIEXTPROC _PatchParameteriEXT = (PFNGLPATCHPARAMETERIEXTPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::PatchParameteriEXT);
	return _PatchParameteriEXT(pname, val);
#endif
}

#ifdef GL_IMG_bindless_texture

inline GLuint64 DYNAMICGLES_FUNCTION(GetTextureHandleIMG)(GLuint texture)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetTextureHandleIMG) PFNGLGETTEXTUREHANDLEIMGPROC;
#endif
	PFNGLGETTEXTUREHANDLEIMGPROC _GetTextureHandleIMG = (PFNGLGETTEXTUREHANDLEIMGPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetTextureHandleIMG);
	return _GetTextureHandleIMG(texture);
}
inline GLuint64 DYNAMICGLES_FUNCTION(GetTextureSamplerHandleIMG)(GLuint texture, GLuint sampler)
{
#if TARGET_OS_IPHONE
	typedef decltype(&glGetTextureSamplerHandleIMG) PFNGLGETTEXTURESAMPLERHANDLEIMGPROC;
#endif
	PFNGLGETTEXTURESAMPLERHANDLEIMGPROC _GetTextureSamplerHandleIMG =
		(PFNGLGETTEXTURESAMPLERHANDLEIMGPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::GetTextureSamplerHandleIMG);
	return _GetTextureSamplerHandleIMG(texture, sampler);
}
inline void DYNAMICGLES_FUNCTION(UniformHandleui64IMG)(GLint location, GLuint64 value)
{
#if TARGET_OS_IPHONE
	assert(0);
#endif
	PFNGLUNIFORMHANDLEUI64IMGPROC _UniformHandleui64IMG = (PFNGLUNIFORMHANDLEUI64IMGPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::UniformHandleui64IMG);
	return _UniformHandleui64IMG(location, value);
}
inline void DYNAMICGLES_FUNCTION(UniformHandleui64vIMG)(GLint location, GLsizei count, const GLuint64* value)
{
#if TARGET_OS_IPHONE
	assert(0);
#endif
	PFNGLUNIFORMHANDLEUI64VIMGPROC _UniformHandleui64vIMG = (PFNGLUNIFORMHANDLEUI64VIMGPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::UniformHandleui64vIMG);
	return _UniformHandleui64vIMG(location, count, value);
}
inline void DYNAMICGLES_FUNCTION(ProgramUniformHandleui64IMG)(GLuint program, GLint location, GLuint64 value)
{
#if TARGET_OS_IPHONE
	assert(0);
#endif
	PFNGLPROGRAMUNIFORMHANDLEUI64IMGPROC _ProgramUniformHandleui64IMG =
		(PFNGLPROGRAMUNIFORMHANDLEUI64IMGPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniformHandleui64IMG);
	return _ProgramUniformHandleui64IMG(program, location, value);
}
inline void DYNAMICGLES_FUNCTION(ProgramUniformHandleui64vIMG)(GLuint program, GLint location, GLsizei count, const GLuint64* values)
{
#if TARGET_OS_IPHONE
	assert(0);
#endif
	PFNGLPROGRAMUNIFORMHANDLEUI64VIMGPROC _ProgramUniformHandleui64vIMG =
		(PFNGLPROGRAMUNIFORMHANDLEUI64VIMGPROC)gl::internals::getGlesExtFunction(gl::internals::GlExtFuncName::ProgramUniformHandleui64vIMG);
	return _ProgramUniformHandleui64vIMG(program, location, count, values);
}
#endif
#ifndef DYNAMICGLES_NO_NAMESPACE
}
#elif TARGET_OS_IPHONE
}
}
#endif

#if TARGET_OS_IPHONE
#pragma clang diagnostic pop
#endif

inline bool isGlExtensionSupported(const char* extensionName)
{
#ifndef DYNAMICGLES_NO_NAMESPACE
	const unsigned char* extensionString = GetString(GL_EXTENSIONS);
#else
	const unsigned char* extensionString = glGetString(GL_EXTENSIONS);
#endif
	return gl::internals::isExtensionSupported(extensionString, extensionName);
}

#ifndef DYNAMICGLES_NO_NAMESPACE
}
#endif
