workspace "EvenMoreCursedDIVA"
	configurations { "Debug", "Release" }
	platforms { "Linux64" }

project "DIVA"
	kind "ConsoleApp"
	language "C++"
	targetname "game"
	targetdir "bin/%{cfg.platform}_%{cfg.buildcfg}"
	cdialect "C17"
	cppdialect "C++17"

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
		"lib/utf8cpp/include" 
	}

	filter { "platforms:Linux64" }
		system "Linux"
		architecture "x86_64"

	filter { "system:linux" }
		prebuildcommands { "python python/gen_build_info.py src/build/build_info.h" }
		links { "m" }
	
	filter { "configurations:Debug" }
		includedirs { "lib/glad_debug/glad/include" }
		files { "lib/glad_debug/glad.c" }

	filter { "configurations:Release" }
		includedirs { "lib/glad_release/glad/include" }
		files { "lib/glad_release/glad.c" }

	filter { "configurations:Debug" }
		defines { "_DEBUG" }
		symbols "On"
		buildoptions { "`sdl2-config --cflags`", "`pkg-config FAudio vorbisfile --cflags`" }
		linkoptions { "`sdl2-config --libs`", "`pkg-config FAudio vorbisfile --libs`" }
		
	filter { "configurations:Release" }
		defines { "_NDEBUG" }
		optimize "On"
		buildoptions { "`sdl2-config --cflags`", "`pkg-config FAudio vorbisfile --cflags`" }
		linkoptions { "`sdl2-config --libs`", "`pkg-config FAudio vorbisfile --libs`" }

project "SpritePack"
	kind "ConsoleApp"
	language "C++"
	targetdir "bin_%{cfg.architecture}/tools/%{cfg.buildcfg}"
	objdir "obj/tools"
	cdialect "C17"
	cppdialect "C++17"

	files { 
		"src_tools/SpritePack/**.cpp",
		"src_tools/SpritePack/**.h",
		"src_tools/SpritePack/*/**.cpp",
		"src_tools/SpritePack/*/**.h",
		"lib/fmt/src/format.cc"
	}
		
	includedirs {
		"lib/glm/include", 
		"lib/stb/include", 
		"lib/tinyxml2",
		"lib/fmt/include",
		"lib/qoi/include",
		"src_tools/SpritePack/" }

	filter { "configurations:Debug" }
		defines { "_DEBUG" }
		symbols "On"
		
	filter { "configurations:Release" }
		defines { "_NDEBUG" }
		optimize "On"
