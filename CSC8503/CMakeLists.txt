set(PROJECT_NAME CSC8503)

################################################################################
# Source groups
################################################################################
set(Networking
    "NetworkedGame.h"
    "NetworkedGame.cpp"
    "NetworkPlayer.h"
    "NetworkPlayer.cpp"
)

source_group("Networking" FILES ${Networking})

set(Image
    "imconfig.h"
    "imgui.cpp"
     "imgui.h"
     "imgui_demo.cpp"
     "imgui_draw.cpp"
     "imgui_impl_opengl3.cpp"
     "imgui_impl_opengl3.h"
     "imgui_impl_opengl3_loader.h"
     "imgui_impl_win32.cpp"
     "imgui_impl_win32.h"
     "imgui_internal.h"
     "imgui_tables.cpp"
     "imgui_widgets.cpp"
     "imstb_rectpack.h"
     "imstb_textedit.h"
     "imstb_truetype.h"
)

source_group("Image" FILES ${Image})

set(Header_Files
    "GameTechRenderer.h"
    "StateGameObject.h"
    "TutorialGame.h"
    "playerTracking.h"
    "ObjectPool.h"
    "Projectile.h"
    "PropSystem.h"
    "Item.h"
    "RespawnPoint.h"

    "AreaTex.h"
    "SearchTex.h"

	"GameManager.h"
	"AudioSystem.h"

    "RayMarchSphere.h"
    "AudioSource.h"



)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "GameTechRenderer.cpp"
    "Main.cpp"
    "StateGameObject.cpp"
    "TutorialGame.cpp"
    "playerTracking.cpp"
    "Projectile.cpp"
	"PropSystem.cpp"
	"Item.cpp"
	"GameManager.cpp"
    "AudioSystem.cpp"


    "RespawnPoint.cpp"

    "RayMarchSphere.cpp"
     "AudioSource.cpp"

)

if(USE_VULKAN)
list(APPEND Header_Files "GameTechVulkanRenderer.h")
list(APPEND Source_Files "GameTechVulkanRenderer.cpp")



file(GLOB SHADER_FILES ${ASSET_ROOT}/Shaders/VK/*.*)

endif()

source_group("Source Files" FILES ${Source_Files})

set(Networking
    "NetworkedGame.h"
    "NetworkPlayer.h"
    "NetworkedGame.cpp"
    "NetworkPlayer.cpp"
)

source_group("Networking" FILES ${Networking})


set(ALL_FILES
    ${Header_Files}
     ${Networking}
     ${Image}
    ${Source_Files}
)

if(USE_VULKAN)
    set(ALL_FILES 
        ${Header_Files}
        ${Source_Files}
        ${SHADER_FILES}
    )

foreach (file ${SHADER_FILES})
    get_filename_component(file_name ${file} NAME)
	get_filename_component(file_ext ${file} EXT)
	
	if(file_ext STREQUAL ".h" OR file_ext STREQUAL ".cpp")
		continue()
	endif()
		
	if( file_ext STREQUAL  ".vert" OR
		file_ext STREQUAL  ".frag" OR
		file_ext STREQUAL  ".comp" OR
		file_ext STREQUAL  ".geom" OR
		file_ext STREQUAL  ".tesc" OR
		file_ext STREQUAL  ".tese" OR
		file_ext STREQUAL  ".rgen" OR
		file_ext STREQUAL  ".rint" OR
		file_ext STREQUAL  ".rahit" OR
		file_ext STREQUAL  ".rchit" OR
		file_ext STREQUAL  ".rmiss" OR
		file_ext STREQUAL  ".rcall" OR
		file_ext STREQUAL  ".task" OR
		file_ext STREQUAL  ".mesh"
	)
		message("Adding custom command to ${file}")
		get_filename_component(file_dir ${file} ABSOLUTE)
        set(SPIRV_OUTPUT ${file_name}.spv)
		set(SPIRV_ABS_INPUT ${file_dir})
		set(SPIRV_ABS_OUTPUT ${file_dir}.spv)
		
		add_custom_command(
			OUTPUT ${SPIRV_ABS_OUTPUT}
			
			COMMENT "Compiling GLSL shader:"
			COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} -V  ${SPIRV_ABS_INPUT} -o ${SPIRV_ABS_OUTPUT}
            DEPENDS ${file}
			VERBATIM
		)
        list(APPEND SPIRV_BINARY_FILES ${SPIRV_OUTPUT})   
	endif()
endforeach()

endif()

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME}  ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE CSC8503)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
)

################################################################################
# Compile definitions
################################################################################
if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "UNICODE;"
        "_UNICODE" 
        "WIN32_LEAN_AND_MEAN"
        "_WINSOCKAPI_"   
        "_WINSOCK2API_"
        "_WINSOCK_DEPRECATED_NO_WARNINGS"
          "_CRT_SECURE_NO_WARNINGS"
    )
endif()

target_precompile_headers(${PROJECT_NAME} PRIVATE
    <vector>
    <map>
    <stack>
    <string>
    <list>
    <thread>
    <atomic>
    <functional>
    <iostream>
    <set>
    "../NCLCoreClasses/Vector2.h"
    "../NCLCoreClasses/Vector3.h"
    "../NCLCoreClasses/Vector4.h"
    "../NCLCoreClasses/Quaternion.h"
    "../NCLCoreClasses/Plane.h"
    "../NCLCoreClasses/Matrix2.h"
    "../NCLCoreClasses/Matrix3.h"
    "../NCLCoreClasses/Matrix4.h"
    
)
 
if(USE_VULKAN)
target_precompile_headers(${PROJECT_NAME} PRIVATE
    "../VulkanRendering/SmartTypes.h"

    "../VulkanRendering/VulkanRenderer.h"
    "../VulkanRendering/VulkanShader.h"
    "../VulkanRendering/VulkanShaderBuilder.h"
    "../VulkanRendering/VulkanTexture.h"
    "../VulkanRendering/VulkanMesh.h"
    "../VulkanRendering/VulkanPipelineBuilder.h"
    "../VulkanRendering/VulkanDynamicRenderBuilder.h"

    "../VulkanRendering/VulkanDescriptorSetLayoutBuilder.h"
    "../VulkanRendering/VulkanRenderPassBuilder.h"

    "../VulkanRendering/VulkanCompute.h"
    "../VulkanRendering/VulkanComputePipelineBuilder.h"
)
endif()

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:
            /Oi;
            /Gy
        >
        /permissive-;
        /std:c++latest;
        /sdl;
        /W3;
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        ${DEFAULT_CXX_EXCEPTION_HANDLING};
        /Y-
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF
        >
    )
endif()

################################################################################
# Dependencies
################################################################################
if(MSVC)
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC  "Winmm.lib")
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC  "fmodstudio_vc.lib")
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC  "fmodstudioL_vc.lib")
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC  "fmod_vc.lib")
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC  "fmodL_vc.lib")
endif()

include_directories("../OpenGLRendering/")
include_directories("../NCLCoreClasses/")
include_directories("../CSC8503CoreClasses/")
include_directories("../FMOD/core/inc")
include_directories("../FMOD/studio/inc")

target_link_directories(${PROJECT_NAME} PUBLIC "../FMOD/studio/lib/x64")
target_link_directories(${PROJECT_NAME} PUBLIC "../FMOD/core/lib/x64")

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC NCLCoreClasses)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC CSC8503CoreClasses)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC OpenGLRendering)

if (OUTPUT_DIRECTORY STREQUAL "")
	message(FATAL_ERROR "OUTPUT_DIRECTORY is empty")
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		"${CMAKE_SOURCE_DIR}/FMOD/core/lib/x64/fmod.dll"
		"${CMAKE_SOURCE_DIR}/FMOD/studio/lib/x64/fmodstudio.dll"
		"${OUTPUT_DIRECTORY}"
)

if(USE_VULKAN)
include_directories("../VulkanRendering/")
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC VulkanRendering)


add_custom_target(
    SHADER_FILES
    DEPENDS ON ${SPIRV_BINARY_FILES}
)

add_dependencies(${PROJECT_NAME} SHADER_FILES)

endif()