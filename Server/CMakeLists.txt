set(PROJECT_NAME Server)

################################################################################
# Source groups
################################################################################

set(Network
    "ServerConnect.h"
    "ServerConnect.cpp"
)

source_group("(Network" FILES ${Network})

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
    ${Header_Files}
    ${Network}
    ${Datebase}
    ${Player}
    ${Source_Files}
)
