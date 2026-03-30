# ─────────────────────────────────────────────────────────────────────────────
# CapTable — mUI demo for Cap Table & Progression Analytics
# ─────────────────────────────────────────────────────────────────────────────
project(CapTable)

file(GLOB_RECURSE SOURCES "*.cpp" "*.hpp")

# ── Require nlohmann/json ─────────────────────────────────────────────────────
# Point NLOHMANN_JSON_DIR to the folder that contains the "nlohmann/" sub-directory, e.g.:
#   cmake -DNLOHMANN_JSON_DIR=M:/Source/JSON/nlohmann/install/include ...
set(NLOHMANN_JSON_DIR "" CACHE PATH "Path to the nlohmann/json single-include root (must contain nlohmann/json.hpp)")

if(NOT NLOHMANN_JSON_DIR)
    message(FATAL_ERROR
        "[CapTable] NLOHMANN_JSON_DIR is not set.\n"
        "Please configure CMake with:\n"
        "  -DNLOHMANN_JSON_DIR=<path>/nlohmann_json/single_include\n"
        "(the directory must contain nlohmann/json.hpp)")
endif()

if(NOT EXISTS "${NLOHMANN_JSON_DIR}/nlohmann/json.hpp")
    message(FATAL_ERROR
        "[CapTable] nlohmann/json.hpp not found under NLOHMANN_JSON_DIR='${NLOHMANN_JSON_DIR}'.\n"
        "Make sure the path points to the single-include root.")
endif()

add_executable(${PROJECT_NAME} ${SOURCES})

# ── Include paths ─────────────────────────────────────────────────────────────
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${NLOHMANN_JSON_DIR}
)

# ── Link libraries ────────────────────────────────────────────────────────────
target_link_libraries(${PROJECT_NAME} PRIVATE
    Pulse::Pulse
    parser Imp
    mUI::InputsManager mUI::EventsManager mUI::WindowManager
    mUI::SceneManager  mUI::Renderer      mUI::Parser
    mUI::Typerino      mUI::Widgets       mUI::mUI
)

# ── Compiler settings ─────────────────────────────────────────────────────────
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W3 /utf-8)
    set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Demo/mUI")
endif()

# ── Install ───────────────────────────────────────────────────────────────────
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin/$<CONFIG>)
