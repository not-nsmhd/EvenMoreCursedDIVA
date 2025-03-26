workspace "EvenMoreCursedDIVA"
	configurations { "Debug", "Release" }
	platforms { "Win32", "Win64" }

project "DIVA"
	kind "ConsoleApp"
	language "C++"
	targetname "game"
	targetdir "bin_%{cfg.architecture}/%{cfg.buildcfg}"
	cdialect "C17"
	cppdialect "C++17"

	buildDate = os.date("%Y.%m.%dT%H:%M:%S")

	newoption {
		trigger = "gfx_d3d9",
		description = "Enable Direct3D 9 graphics backend. (Windows only)"
	}

	files { 
		"src/**.h",
		"src/pcg/pcg_basic.c",
		"src/**.cpp",
		"src/*/**.h",
		"src/*/**.cpp",
		"src/*/*/**.h",
		"src/*/*/**.cpp",
		"lib/tinyxml2/tinyxml2.cpp"
	}
		
	includedirs {
		"lib/glm/include", 
		"lib/stb/include", 
		"lib/tinyxml2",
		"src/" }

	defines {
		"PREMAKE_BUILD_DATE=\"" .. buildDate .. "\""
	}
		
	filter { "platforms:Win32" }
		system "Windows"
		architecture "x86"

	filter { "platforms:Win64" }
		system "Windows"
		architecture "x86_64"

	filter { "system:windows" }
		links { "shell32", "shlwapi", "winmm" }

	filter { "options:gfx_d3d9" }
		defines { "STARSHINE_GFX_D3D9" }
		links { "d3d9" }
	
	filter { "configurations:Debug" }
		includedirs { "lib/glad_debug/glad/include" }
		files { "lib/glad_debug/glad.c" }

	filter { "configurations:Release" }
		includedirs { "lib/glad_release/glad/include" }
		files { "lib/glad_release/glad.c" }

	filter { "configurations:Debug" }
		defines { "_DEBUG" }
		symbols "On"
		buildoptions { "`sdl2-config --cflags`", "`pkg-config FAudio vorbisfile --cflags`", "-mconsole" }
		linkoptions { "`sdl2-config --libs`", "`pkg-config FAudio vorbisfile --libs`", "-mconsole" }
		
	filter { "configurations:Release" }
		defines { "_NDEBUG" }
		optimize "On"
		buildoptions { "`sdl2-config --cflags`", "`pkg-config FAudio vorbisfile --cflags`" }
		linkoptions { "`sdl2-config --libs`", "`pkg-config FAudio vorbisfile --libs`" }	