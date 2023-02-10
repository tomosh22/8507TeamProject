set(PROJECT_NAME Server)

################################################################################
# Source groups
################################################################################
set(Network
    "ServerConnect.h"
    "ServerConnect.cpp"
)

source_group("Network" FILES ${Network})

set(Datebase
    "MyDB.h"
    "MyDB.cpp"
)

source_group("Datebase" FILES ${Datebase})

set(Player
    "playerInfo.h"
    "playerInfo.cpp"
)

source_group("Player" FILES ${Player})

set(Source_Files
    main.cpp
)

source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Network}
    ${Datebase}
    ${Player}
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME}  ${ALL_FILES})
use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
)

