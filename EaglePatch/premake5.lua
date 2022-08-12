workspace "EaglePatch"
	configurations
	{
		"Debug",
		"Release",
	}

	location "build"

project "EaglePatchAC1"
	kind "SharedLib"
	language "C++"
	targetname "EaglePatchAC1"
	targetdir "bin/%{cfg.buildcfg}"
	targetextension ".asi"

	files { "../patcher/patcher.cpp" }
	files { "../patcher/patcher.h" }
	includedirs { "../patcher" }

	files { "shared/console.h" }
	files { "shared/ini_reader.h" }
	files { "shared/console.cpp" }
	files { "shared/ini_reader.cpp" }
	files { "src/ac1.cpp" }

	characterset ("MBCS")
	toolset ("v141_xp")
	floatingpoint "Fast"
	buildoptions { "/Zc:sizedDealloc-" }
	links { "legacy_stdio_definitions" }
	linkoptions { "/SAFESEH:NO /IGNORE:4222" }
	defines { "WIN32_LEAN_AND_MEAN", "_CRT_SECURE_NO_WARNINGS", "_CRT_NONSTDC_NO_DEPRECATE", "_USE_32BIT_TIME_T", "NOMINMAX", "DLL_NAME=\"$(ProjectName)\"" }

	filter "configurations:Debug"
		defines { "_DEBUG" }
		symbols "full"
		optimize "off"
		runtime "debug"
		editAndContinue "off"
		flags { "NoIncrementalLink" }
		staticruntime "on"

	filter "configurations:Release"
		defines { "NDEBUG" }
		symbols "on"
		optimize "speed"
		runtime "release"
		staticruntime "on"
		flags { "LinkTimeOptimization" }
		linkoptions { "/OPT:NOICF" }


project "EaglePatchAC2"
	kind "SharedLib"
	language "C++"
	targetname "EaglePatchAC2"
	targetdir "bin/%{cfg.buildcfg}"
	targetextension ".asi"

	files { "../patcher/patcher.cpp" }
	files { "../patcher/patcher.h" }
	includedirs { "../patcher" }

	files { "shared/console.h" }
	files { "shared/ini_reader.h" }
	files { "shared/console.cpp" }
	files { "shared/ini_reader.cpp" }
	files { "src/ac2.cpp" }

	characterset ("MBCS")
	toolset ("v141_xp")
	floatingpoint "Fast"
	buildoptions { "/Zc:sizedDealloc-" }
	links { "legacy_stdio_definitions" }
	linkoptions { "/SAFESEH:NO /IGNORE:4222" }
	defines { "WIN32_LEAN_AND_MEAN", "_CRT_SECURE_NO_WARNINGS", "_CRT_NONSTDC_NO_DEPRECATE", "_USE_32BIT_TIME_T", "NOMINMAX", "DLL_NAME=\"$(ProjectName)\"" }

	filter "configurations:Debug"
		defines { "_DEBUG" }
		symbols "full"
		optimize "off"
		runtime "debug"
		editAndContinue "off"
		flags { "NoIncrementalLink" }
		staticruntime "on"

	filter "configurations:Release"
		defines { "NDEBUG" }
		symbols "on"
		optimize "speed"
		runtime "release"
		staticruntime "on"
		flags { "LinkTimeOptimization" }
		linkoptions { "/OPT:NOICF" }
	