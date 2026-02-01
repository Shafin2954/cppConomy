cmake_minimum_required(VERSION 3.16)
project(cppConomy)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Let CMake auto-find wxWidgets in mingw64
set(wxWidgets_USE_STATIC ON)
find_package(wxWidgets REQUIRED COMPONENTS core base)

include(${wxWidgets_USE_FILE})

# Source files
set(SOURCES
    src/main.cpp
    
    # Core
    src/core/Person.cpp
    src/core/Firm.cpp
    src/core/Government.cpp
    src/core/Economy.cpp
    src/core/Market.cpp
    
    # Commands
    src/commands/CommandParser.cpp
    src/commands/CommandExecutor.cpp
    
    # GUI
    src/gui/MainFrame.cpp
    src/gui/DashboardPanel.cpp
    src/gui/EventLogPanel.cpp
    src/gui/CommandPanel.cpp
    src/gui/HistoryPanel.cpp
)

# Header files (for IDE support)
set(HEADERS
    # Utils
    src/utils/Config.h
    src/utils/Logger.h
    src/utils/Statistics.h
    
    # Core
    src/core/Agent.h
    src/core/Person.h
    src/core/Firm.h
    src/core/Government.h
    src/core/Economy.h
    src/core/Market.h
    
    # Models
    src/models/PhillipsCurve.h
    src/models/OkunLaw.h
    src/models/QuantityTheory.h
    src/models/UtilityFunction.h
    src/models/StochasticShock.h
    
    # Commands
    src/commands/CommandParser.h
    src/commands/CommandExecutor.h
    
    # GUI
    src/gui/MainFrame.h
    src/gui/DashboardPanel.h
    src/gui/EventLogPanel.h
    src/gui/CommandPanel.h
    src/gui/HistoryPanel.h
)

add_executable(cppConomy WIN32 ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(cppConomy PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(cppConomy ${wxWidgets_LIBRARIES})

# Static standalone EXE
if(MINGW)
    target_link_options(cppConomy PRIVATE 
        -static -static-libgcc -static-libstdc++
        -Wl,--subsystem,windows
    )
endif()
