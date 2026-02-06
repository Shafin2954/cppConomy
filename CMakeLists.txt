cmake_minimum_required(VERSION 3.16)
project(cppConomy)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Let CMake auto-find wxWidgets in mingw64
set(wxWidgets_USE_STATIC ON)
find_package(wxWidgets REQUIRED COMPONENTS core base)

include(${wxWidgets_USE_FILE})

# Source files (only .cpp files that exist)
set(SOURCES
    src/main.cpp
    
    # Core (new simplified implementation)
    src/core/Person.cpp
    src/core/Worker.cpp
    src/core/Farmer.cpp
    src/core/Owner.cpp
    src/core/Market.cpp
    src/core/Government.cpp
    src/core/Simulation.cpp
    
    # Commands
    src/commands/CommandParser.cpp
    src/commands/CommandExecutor.cpp
    
    # GUI
    src/gui/MainFrame.cpp
    src/gui/DashboardPanel.cpp
    src/gui/EventLogPanel.cpp
    src/gui/CommandPanel.cpp
    src/gui/SelectionPanel.cpp
    src/gui/CommandsPanel.cpp
)

# Header files (for IDE support)
set(HEADERS
    # Utils
    src/utils/Config.h
    src/utils/Logger.h
    
    # Core (New simplified structure)
    src/core/Product.h
    src/core/Person.h
    src/core/Worker.h
    src/core/Farmer.h
    src/core/Owner.h
    src/core/Market.h
    src/core/Government.h
    src/core/Simulation.h
    
    
    # Commands
    src/commands/CommandParser.h
    src/commands/CommandExecutor.h
    
    # GUI
    src/gui/MainFrame.h
    src/gui/DashboardPanel.h
    src/gui/EventLogPanel.h
    src/gui/CommandPanel.h
    src/gui/SelectionPanel.h
    src/gui/CommandsPanel.h
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
