workspace "dx11playground"
	architecture "x64"
	startproject "playground"

	configurations
	{
	"Debug",
	"Release"
	}
	
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
	
	
	
project "playground"
	location "playground"
	kind "WindowedApp"
	language "c++"
	cppdialect "c++17"
	staticruntime "on"
	
	targetdir ("Bin/" .. outputdir .. "/%{prj.name}")
	objdir ("Intermediate/" .. outputdir .. "/%{prj.name}")

	pchheader "pch.h"
	pchsource "playground/Source/pch.cpp"

	files
	{
		"%{prj.name}/Source/**.h",
		"%{prj.name}/Source/**.cpp",
		"%{prj.name}/Source/**.hlsl"
	}

	links
	{
		"d3d11.lib",
		"D3DCompiler.lib"
	}
	
	libdirs
	{
		"ThirdParty/Library/**"
	}
	
	includedirs
	{
		"%{prj.name}/Source",
		"%{prj.name}/Source/imgui",
		"ThirdParty/Header"
	}
	
	filter "files:**/imgui/**"
		flags {"NoPCH"}

--	filter("files:**.hlsl")
--		  flags("ExcludeFromBuild")
--		  shaderobjectfileoutput(shader_dir.."%{file.basename}"..".cso")
--		  shaderassembleroutput(shader_dir.."%{file.basename}"..".asm")

	filter("files:**_ps.hlsl")
		removeflags("ExcludeFromBuild")
		shadertype("Pixel")
		shadermodel("4.0")	

   filter("files:**_vs.hlsl")
		removeflags("ExcludeFromBuild")
		shadertype("Vertex")
		shadermodel("4.0")	

	filter "configurations:Debug"
		defines
		{
			"DRN_DEBUG"
		}
			
		runtime "Debug"
		symbols "on"			
		
	filter "configurations:Release"
		defines
		{
			"DRN_RELEASE"
		}
		
		runtime "Release"
		optimize "on"