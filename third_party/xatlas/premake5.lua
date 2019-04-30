local SHADERS_DIR = "extra/shaders"
local SHADERS_BIN_DIR = "extra/shaders_bin"
local THIRDPARTY_DIR = "extra/thirdparty"
local BGFX_DIR = path.join(THIRDPARTY_DIR, "bgfx")
local BIMG_DIR = path.join(THIRDPARTY_DIR, "bimg")
local BX_DIR = path.join(THIRDPARTY_DIR, "bx")
local EIGEN_DIR = path.join(THIRDPARTY_DIR, "eigen")
local GLFW_DIR = path.join(THIRDPARTY_DIR, "glfw")
local IGL_DIR = path.join(THIRDPARTY_DIR, "libigl")
local OIDN_DIR = path.join(THIRDPARTY_DIR, "oidn")

newaction
{
	trigger = "shaders",
	description = "Compile shaders",
	onStart = function()
		dofile("extra/shaderc.lua")
		local shaders =
		{
			"fs_atomicCounterClear",
			"fs_chart",
			"fs_color",
			"fs_gui",
			"fs_lightmapAverage",
			"fs_lightmapOp",
			"fs_material",
			"fs_rayBundleClear",
			"fs_rayBundleIntegrate",
			"fs_rayBundleLightmapClear",
			"fs_rayBundleWrite",
			"vs_chart",
			"vs_chartTexcoordSpace",
			"vs_gui",
			"vs_model",
			"vs_position"
		}
		local renderers = nil
		if os.ishost("windows") then
			renderers = { "d3d11", "gl" }
		else
			renderers = { "gl" }
		end
		for _,renderer in pairs(renderers) do
			os.mkdir(path.join(SHADERS_BIN_DIR, renderer))
		end
		pcall(function()
			for _,shader in pairs(shaders) do
				for _,renderer in pairs(renderers) do
					io.write("Compiling " .. shader .. " " .. renderer .. "\n")
					io.flush()
					local shaderType = "vertex"
					if shader:sub(0, 2) == "fs" then shaderType = "fragment" end
					compileShader(
					{
						type = shaderType,
						renderer = renderer,
						inputFilename = path.join(SHADERS_DIR, shader) .. ".sc",
						includeDirs = path.join(BGFX_DIR, "src"),
						varyingFilename = path.join(SHADERS_DIR, "varying.def.sc"),
						outputFilename = path.join(SHADERS_BIN_DIR, renderer, shader) .. ".h",
						bin2c = true,
						variableName = shader .. "_" .. renderer
					})
				end
			end
			-- Write a header file that includes all the shader headers.
			local filename = path.join(SHADERS_BIN_DIR, "shaders.h")
			io.write("Writing " .. filename .. "\n")
			io.flush()
			local file = assert(io.open(filename, "w"))
			for _,renderer in pairs(renderers) do
				for _,shader in pairs(shaders) do
					file:write(string.format("#include \"%s.h\"\n", path.join(renderer, shader)))
				end
			end
			file:close()
		end)
	end
}

if _ACTION == nil then
	return
end

solution "xatlas"
	configurations { "Release", "Debug" }
	if _OPTIONS["cc"] ~= nil then
		location(path.join("build", _ACTION) .. "_" .. _OPTIONS["cc"])
	else
		location(path.join("build", _ACTION))
	end
	if os.is64bit() and not os.istarget("windows") then
		platforms { "x86_64", "x86" }
	else
		platforms { "x86", "x86_64" }
	end
	startproject "example"
	filter "platforms:x86"
		architecture "x86"
	filter "platforms:x86_64"
		architecture "x86_64"
	filter "configurations:Debug*"
		defines { "_DEBUG" }
		optimize "Debug"
		symbols "On"
	filter "configurations:Release"
		defines "NDEBUG"
		optimize "Full"
		
project "xatlas"
	kind "StaticLib"
	language "C++"
	cppdialect "C++11"
	exceptionhandling "Off"
	rtti "Off"
	warnings "Extra"
	files { "xatlas.cpp", "xatlas.h" }

project "example"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++11"
	exceptionhandling "Off"
	rtti "Off"
	warnings "Extra"
	files "extra/example.cpp"
	includedirs(THIRDPARTY_DIR)
	links { "stb_image_write", "tiny_obj_loader", "xatlas" }
	filter "system:linux"
		links { "pthread" }

project "test"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++11"
	exceptionhandling "Off"
	rtti "Off"
	warnings "Extra"
	includedirs(THIRDPARTY_DIR)
	files "extra/test.cpp"
	links { "tiny_obj_loader", "xatlas" }
	filter "system:linux"
		links { "pthread" }

project "viewer"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++14"
	exceptionhandling "Off"
	rtti "Off"
	warnings "Extra"
	files { "extra/viewer*", "extra/shaders/*.*" }
	includedirs
	{
		THIRDPARTY_DIR,
		path.join(BGFX_DIR, "include"),
		path.join(BX_DIR, "include"),
		EIGEN_DIR,
		path.join(GLFW_DIR, "include"),
		path.join(IGL_DIR, "include"),
		path.join(OIDN_DIR, "include")
	}
	links { "bgfx", "bimg", "bx", "glfw", "imgui", "nativefiledialog", "objzero", "stb_image", "stb_image_resize", "xatlas" }
	filter "system:windows"
		links { "gdi32", "ole32", "psapi", "uuid" }
	filter "system:linux"
		links { "dl", "GL", "gtk-3", "gobject-2.0", "glib-2.0", "pthread", "X11", "Xcursor", "Xinerama", "Xrandr" }
	filter "action:vs*"
		includedirs { path.join(BX_DIR, "include/compat/msvc") }
	filter { "system:windows", "action:gmake" }
		includedirs { path.join(BX_DIR, "include/compat/mingw") }

group "thirdparty"

project "bgfx"
	kind "StaticLib"
	language "C++"
	cppdialect "C++14"
	exceptionhandling "Off"
	rtti "Off"
	defines	{ "__STDC_FORMAT_MACROS" }
	files
	{
		path.join(BGFX_DIR, "include/bgfx/**.h"),
		path.join(BGFX_DIR, "src/*.cpp"),
		path.join(BGFX_DIR, "src/*.h")
	}
	excludes
	{
		path.join(BGFX_DIR, "src/amalgamated.cpp")
	}
	includedirs
	{
		path.join(BX_DIR, "include"),
		path.join(BIMG_DIR, "include"),
		path.join(BIMG_DIR, "3rdparty"),
		path.join(BIMG_DIR, "3rdparty/astc-codec/include"),
		path.join(BIMG_DIR, "3rdparty/iqa/include"),
		path.join(BGFX_DIR, "include"),
		path.join(BGFX_DIR, "3rdparty"),
		path.join(BGFX_DIR, "3rdparty/dxsdk/include"),
		path.join(BGFX_DIR, "3rdparty/khronos")
	}
	filter "configurations:Debug"
		defines "BGFX_CONFIG_DEBUG=1"
	filter "action:vs*"
		defines { "_CRT_SECURE_NO_WARNINGS" }
		includedirs { path.join(BX_DIR, "include/compat/msvc") }
		excludes
		{
			path.join(BGFX_DIR, "src/glcontext_glx.cpp"),
			path.join(BGFX_DIR, "src/glcontext_egl.cpp")
		}
	filter { "system:windows", "action:gmake" }
		includedirs { path.join(BX_DIR, "include/compat/mingw") }
		
project "bimg"
	kind "StaticLib"
	language "C++"
	cppdialect "C++14"
	exceptionhandling "Off"
	rtti "Off"
	files
	{
		path.join(BIMG_DIR, "include/bimg/*.h"),
		path.join(BIMG_DIR, "src/*.cpp"),
		path.join(BIMG_DIR, "src/*.h")
	}
	includedirs
	{
		path.join(BX_DIR, "include"),
		path.join(BIMG_DIR, "include")
	}
	filter "action:vs*"
		defines { "_CRT_SECURE_NO_WARNINGS" }
		includedirs { path.join(BX_DIR, "include/compat/msvc") }
	filter { "system:windows", "action:gmake" }
		includedirs { path.join(BX_DIR, "include/compat/mingw") }

project "bx"
	kind "StaticLib"
	language "C++"
	cppdialect "C++14"
	exceptionhandling "Off"
	rtti "Off"
	defines	{ "__STDC_FORMAT_MACROS" }
	files
	{
		path.join(BX_DIR, "include/bx/*.h"),
		path.join(BX_DIR, "include/bx/inline/*.inl"),
		path.join(BX_DIR, "include/tinystl/*.h"),
		path.join(BX_DIR, "src/*.cpp")
	}
	excludes
	{
		path.join(BX_DIR, "src/amalgamated.cpp"),
		path.join(BX_DIR, "src/crtnone.cpp")
	}
	includedirs
	{
		path.join(BX_DIR, "3rdparty"),
		path.join(BX_DIR, "include")
	}
	filter "action:vs*"
		defines { "_CRT_SECURE_NO_WARNINGS" }
		includedirs { path.join(BX_DIR, "include/compat/msvc") }
	filter { "system:windows", "action:gmake" }
		includedirs { path.join(BX_DIR, "include/compat/mingw") }


project "glfw"
	kind "StaticLib"
	language "C"
	files
	{
		path.join(GLFW_DIR, "include/*.h"),
		path.join(GLFW_DIR, "src/context.c"),
		path.join(GLFW_DIR, "src/egl_context.c"),
		path.join(GLFW_DIR, "src/init.c"),
		path.join(GLFW_DIR, "src/input.c"),
		path.join(GLFW_DIR, "src/monitor.c"),
		path.join(GLFW_DIR, "src/osmesa_context.c"),
		path.join(GLFW_DIR, "src/vulkan.c"),
		path.join(GLFW_DIR, "src/window.c"),
	}
	includedirs { path.join(GLFW_DIR, "include") }
	filter "system:windows"
		defines "_GLFW_WIN32"
		files
		{
			path.join(GLFW_DIR, "src/win32_init.c"),
			path.join(GLFW_DIR, "src/win32_joystick.c"),
			path.join(GLFW_DIR, "src/win32_monitor.c"),
			path.join(GLFW_DIR, "src/win32_thread.c"),
			path.join(GLFW_DIR, "src/win32_time.c"),
			path.join(GLFW_DIR, "src/win32_window.c"),
			path.join(GLFW_DIR, "src/wgl_context.c")
		}
	filter "system:linux"
		defines "_GLFW_X11"
		files
		{
			path.join(GLFW_DIR, "src/glx_context.c"),
			path.join(GLFW_DIR, "src/linux*.c"),
			path.join(GLFW_DIR, "src/posix*.c"),
			path.join(GLFW_DIR, "src/x11*.c"),
			path.join(GLFW_DIR, "src/xkb*.c")
		}
	filter "action:vs*"
		defines { "_CRT_SECURE_NO_WARNINGS" }
	filter {}
	
project "imgui"
	kind "StaticLib"
	language "C++"
	exceptionhandling "Off"
	rtti "Off"
	files(path.join(THIRDPARTY_DIR, "imgui/*.*"))
	
project "nativefiledialog"
	kind "StaticLib"
	language "C++"
	exceptionhandling "Off"
	rtti "Off"
	files(path.join(THIRDPARTY_DIR, "nativefiledialog/nfd_common.*"))
	filter "system:windows"
		files(path.join(THIRDPARTY_DIR, "nativefiledialog/nfd_win.cpp"))
	filter "system:linux"
		files(path.join(THIRDPARTY_DIR, "nativefiledialog/nfd_gtk.c"))
		buildoptions(os.outputof("pkg-config --cflags gtk+-3.0"))
	filter "action:vs*"
		defines { "_CRT_SECURE_NO_WARNINGS" }
	
project "objzero"
	kind "StaticLib"
	language "C"
	cdialect "C99"
	files(path.join(THIRDPARTY_DIR, "objzero/objzero.*"))
	
project "stb_image"
	kind "StaticLib"
	language "C"
	files(path.join(THIRDPARTY_DIR, "stb_image.*"))
	
project "stb_image_resize"
	kind "StaticLib"
	language "C"
	files(path.join(THIRDPARTY_DIR, "stb_image_resize.*"))
	
project "stb_image_write"
	kind "StaticLib"
	language "C"
	files(path.join(THIRDPARTY_DIR, "stb_image_write.*"))
	
project "tiny_obj_loader"
	kind "StaticLib"
	language "C++"
	exceptionhandling "Off"
	rtti "Off"
	files(path.join(THIRDPARTY_DIR, "tiny_obj_loader.*"))
