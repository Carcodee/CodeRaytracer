project "Core"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   targetdir "Binaries/%{cfg.buildcfg}"
   staticruntime "off"

   files { "Source/**.h", "Source/**.cpp", "Source/**.hpp" }

   includedirs
   {
      "Source",
      "Source/Libraries/glfw-3.3.9.bin.WIN64/include",
      "Source/Libraries/glm",
      "C:/VulkanSDK/1.3.268.0/Include"
   }
   libdirs
   {
      "Source/Libraries/glfw-3.3.9.bin.WIN64/lib-vc2022",
      "C:/VulkanSDK/1.3.268.0/Lib",

   }

   links
   {
      "glfw3",
      "vulkan-1",
   }

   targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
   objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

   filter "system:windows"
       systemversion "latest"
       defines { }

   filter "configurations:Debug"
       defines { "DEBUG" }
       runtime "Debug"
       symbols "On"

   filter "configurations:Release"
       defines { "RELEASE" }
       runtime "Release"
       optimize "On"
       symbols "On"

   filter "configurations:Dist"
       defines { "DIST" }
       runtime "Release"
       optimize "On"
       symbols "Off"