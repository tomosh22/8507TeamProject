set(PROJECT_NAME GameServer)

################################################################################
# Source groups
################################################################################

set(Header_Files
     "ServerObject.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "Main.cpp"
     "ServerObject.cpp"
)

source_group("Source Files" FILES ${Source_Files})

set(enet_Files
    #"./enet/callbacks.h"
    "./enet/callbacks.c"
    "./enet/list.h"
    "./enet/list.c"
    "./enet/protocol.h"
    "./enet/protocol.c"
    "./enet/win32.h"
    "./enet/win32.c"

    "./enet/enet.h"
    "./enet/time.h"
    "./enet/types.h"
    "./enet/utility.h"

    "./enet/compress.c"
    "./enet/host.c"
    "./enet/packet.c"
    "./enet/peer.c"
)
source_group("eNet" FILES ${enet_Files})

set(ALL_FILES
    ${Header_Files}
    ${enet_Files} 
    ${Source_Files}
)

set_source_files_properties(${ALL_FILES} PROPERTIES LANGUAGE CXX)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME}  ${ALL_FILES})

#set(CMAKE_CXX_SOURCE_FILE_EXTENSIONS c;C;M;c++;cc;cpp;cxx;m;mm;mpp;CPP;ixx;cppm)

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE GameServer)

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
    ";"
    "UNICODE"
    "_UNICODE"
    "WIN32_LEAN_AND_MEAN"
    "_WINSOCK_DEPRECATED_NO_WARNINGS"
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
endif()

set(ADDITIONAL_LIBRARY_DEPENDENCIES
    "CSC8503CoreClasses"
    "NCLCoreClasses"
    "ws2_32.lib"
)

include_directories("../NCLCoreClasses/")
include_directories("../CSC8503CoreClasses/")

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC NCLCoreClasses)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC CSC8503CoreClasses)

if(MSVC)
    target_link_libraries(${PROJECT_NAME} PRIVATE "ws2_32.lib")
endif()
