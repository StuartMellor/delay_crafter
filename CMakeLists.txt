cmake_minimum_required(VERSION 3.25)

# This tells cmake we have goodies in the /cmake folder
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include (PamplejuceVersion)

# Modern concise way to add dependencies to your project
include (CPM)

# Configures universal binaries and decides which version of macOS to support
include(PamplejuceMacOS)

# Couple tweaks that IMO should be JUCE defaults
include(JUCEDefaults)

# Change me!
# This is the internal name of the project and the name of JUCE's shared code target
# Note: This cannot have spaces (it may be 2024, but you can't have it all!)
# Worry not, JUCE's PRODUCT_NAME can have spaces (and is what DAWs display)
set(PROJECT_NAME "DelayCrafter")

# Worry not, JUCE's PRODUCT_NAME can have spaces (and is what DAWs will display)
# You can also just have it be the same thing as PROJECT_NAME
# You may want to append the major version on the end of this (and PROJECT_NAME) ala:
#   set(PROJECT_NAME "MyPlugin_v${MAJOR_VERSION}")
# Doing so enables major versions to show up in IDEs and DAWs as separate plugins
# allowing you to change parameters and behavior without breaking existing user projects
set(PRODUCT_NAME "Delay Crafter")

# Change me! Used for the MacOS bundle name and Installers
set(COMPANY_NAME "Stuart Mellor")

# Change me! Used for the MacOS bundle identifier (and signing)
#TODO: Use a domain you own here
set(BUNDLE_ID "com.stuartmellor.delaycrafter")

# Change me! Set the plugin formats you want built
# Valid choices: AAX Unity VST VST3 AU AUv3 Standalone
# TODO: Just building for VST3 for now... remove comments if needed.
set(FORMATS Standalone VST3)
#AUv3  AU )

# Force JUCE to work with sccache
# instruct MSVC to embed debug info (/Z7) instead of emitting a .pdb
set(CMAKE_POLICY_DEFAULT_CMP0141        NEW      CACHE STRING "" FORCE)
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT Embedded CACHE STRING "" FORCE)

# For simplicity, the name of the CMake project is also the name of the target
project(${PROJECT_NAME} VERSION ${CURRENT_VERSION})

# JUCE is setup as a submodule in the /JUCE folder
# Locally, you must run `git submodule update --init --recursive` once
# and later `git submodule update --remote --merge` to keep it up to date
# On Github Actions, this is done as a part of actions/checkout
add_subdirectory(JUCE)

# Add CLAP format
add_subdirectory(modules/clap-juce-extensions EXCLUDE_FROM_ALL)

# Add any other modules you want modules here, before the juce_add_plugin call
# juce_add_module(modules/my_module)

# This adds the melatonin inspector module
add_subdirectory (modules/melatonin_inspector)

add_library(bx STATIC
    extern/bx/src/amalgamated.cpp
)

target_include_directories(bx PUBLIC
    extern/bx/include
    extern/bx/3rdparty
)

if (WIN32)
    target_include_directories(bx PUBLIC extern/bx/include/compat/msvc)
endif()

target_compile_features(bx PUBLIC cxx_std_20)

target_compile_definitions(bx PUBLIC
    BX_CONFIG_DEBUG=$<IF:$<CONFIG:Debug>,1,0>
)

if (MSVC)
    target_compile_options(bx PUBLIC /Zc:__cplusplus /Zc:preprocessor /permissive)
    target_compile_definitions(bx PUBLIC 
        __STDC_FORMAT_MACROS
        ENTRY_CONFIG_IMPLEMENT_MAIN=0
    )
endif()

add_library(bimg STATIC
    extern/bimg/src/image.cpp
    extern/bimg/src/image_gnf.cpp
    extern/bimg/3rdparty/astc-encoder/source/astcenc_averages_and_directions.cpp
    extern/bimg/3rdparty/astc-encoder/source/astcenc_block_sizes.cpp
    extern/bimg/3rdparty/astc-encoder/source/astcenc_color_quantize.cpp
    extern/bimg/3rdparty/astc-encoder/source/astcenc_color_unquantize.cpp
    extern/bimg/3rdparty/astc-encoder/source/astcenc_compress_symbolic.cpp
    extern/bimg/3rdparty/astc-encoder/source/astcenc_compute_variance.cpp
    extern/bimg/3rdparty/astc-encoder/source/astcenc_decompress_symbolic.cpp
    extern/bimg/3rdparty/astc-encoder/source/astcenc_diagnostic_trace.cpp
    extern/bimg/3rdparty/astc-encoder/source/astcenc_entry.cpp
    extern/bimg/3rdparty/astc-encoder/source/astcenc_find_best_partitioning.cpp
    extern/bimg/3rdparty/astc-encoder/source/astcenc_ideal_endpoints_and_weights.cpp
    extern/bimg/3rdparty/astc-encoder/source/astcenc_image.cpp
    extern/bimg/3rdparty/astc-encoder/source/astcenc_integer_sequence.cpp
    extern/bimg/3rdparty/astc-encoder/source/astcenc_mathlib.cpp
    extern/bimg/3rdparty/astc-encoder/source/astcenc_mathlib_softfloat.cpp
    extern/bimg/3rdparty/astc-encoder/source/astcenc_partition_tables.cpp
    extern/bimg/3rdparty/astc-encoder/source/astcenc_percentile_tables.cpp
    extern/bimg/3rdparty/astc-encoder/source/astcenc_pick_best_endpoint_format.cpp
    extern/bimg/3rdparty/astc-encoder/source/astcenc_quantization.cpp
    extern/bimg/3rdparty/astc-encoder/source/astcenc_symbolic_physical.cpp
    extern/bimg/3rdparty/astc-encoder/source/astcenc_weight_align.cpp
    extern/bimg/3rdparty/astc-encoder/source/astcenc_weight_quant_xfer_tables.cpp
)

target_include_directories(bimg PUBLIC
    extern/bimg/include
    extern/bimg/3rdparty
    # Add the include folder so astcenc.h is found
    extern/bimg/3rdparty/astc-encoder/include 
    extern/bimg/3rdparty/astc-encoder/source
)

target_link_libraries(bimg PUBLIC bx)
target_compile_features(bimg PUBLIC cxx_std_20)

target_compile_definitions(bimg PUBLIC
    BX_CONFIG_DEBUG=$<IF:$<CONFIG:Debug>,1,0>
)

if (MSVC)
    target_compile_options(bimg PUBLIC /Zc:__cplusplus /Zc:preprocessor)
endif()

# Add meshoptimizer
add_library(meshoptimizer STATIC
    extern/bgfx/3rdparty/meshoptimizer/src/meshoptimizer.h
    extern/bgfx/3rdparty/meshoptimizer/src/allocator.cpp
    extern/bgfx/3rdparty/meshoptimizer/src/clusterizer.cpp
    extern/bgfx/3rdparty/meshoptimizer/src/indexanalyzer.cpp
    extern/bgfx/3rdparty/meshoptimizer/src/indexcodec.cpp
    extern/bgfx/3rdparty/meshoptimizer/src/indexgenerator.cpp
    extern/bgfx/3rdparty/meshoptimizer/src/overdrawoptimizer.cpp
    extern/bgfx/3rdparty/meshoptimizer/src/partition.cpp
    extern/bgfx/3rdparty/meshoptimizer/src/quantization.cpp
    extern/bgfx/3rdparty/meshoptimizer/src/rasterizer.cpp
    extern/bgfx/3rdparty/meshoptimizer/src/simplifier.cpp
    extern/bgfx/3rdparty/meshoptimizer/src/spatialorder.cpp
    extern/bgfx/3rdparty/meshoptimizer/src/stripifier.cpp
    extern/bgfx/3rdparty/meshoptimizer/src/vcacheoptimizer.cpp
    extern/bgfx/3rdparty/meshoptimizer/src/vertexcodec.cpp
    extern/bgfx/3rdparty/meshoptimizer/src/vertexfilter.cpp
    extern/bgfx/3rdparty/meshoptimizer/src/vfetchoptimizer.cpp
)

target_include_directories(meshoptimizer PUBLIC
    extern/bgfx/3rdparty/meshoptimizer/src
)

add_library(bgfx_amalgamated STATIC
    extern/bgfx/src/amalgamated.cpp
)

target_compile_features(bgfx_amalgamated PUBLIC cxx_std_20)

target_include_directories(bgfx_amalgamated PUBLIC
    extern/bgfx/include
    extern/bgfx/3rdparty
    extern/bgfx/3rdparty/khronos
)

target_link_libraries(bgfx_amalgamated PUBLIC bx bimg)

if (MSVC)
    target_compile_options(bgfx_amalgamated PUBLIC /Zc:__cplusplus /Zc:preprocessor)
endif()

target_compile_definitions(bgfx_amalgamated PUBLIC
    BX_CONFIG_DEBUG=$<IF:$<CONFIG:Debug>,1,0>
    BGFX_CONFIG_RENDERER_OPENGL=0
    BGFX_CONFIG_RENDERER_DIRECT3D11=0
    BGFX_CONFIG_RENDERER_VULKAN=1
)

if (WIN32)
    # Find Vulkan
    find_package(Vulkan REQUIRED)
    
    target_link_libraries(bgfx_amalgamated PUBLIC
        gdi32
        psapi
        Vulkan::Vulkan
    )
endif()


# See `docs/CMake API.md` in the JUCE repo for all config options
juce_add_plugin("${PROJECT_NAME}"
    # Icons for the standalone app
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/packaging/icon.png"

    # Change me!
    COMPANY_NAME "${COMPANY_NAME}"
    BUNDLE_ID "${BUNDLE_ID}"

    # On MacOS, plugin is copied to /Users/yourname/Library/Audio/Plug-Ins/
    COPY_PLUGIN_AFTER_BUILD FALSE

    # Change me!
    # A four-character plugin id
    # First character MUST be uppercase for AU formats
    PLUGIN_MANUFACTURER_CODE Pamp

    # Change me!
    # A unique four-character plugin id
    # Note: this must have at least one upper-case character
    PLUGIN_CODE P001
    FORMATS "${FORMATS}"

    # The name of your final executable
    # This is how it's listed in the DAW
    # This can be different from PROJECT_NAME and can have spaces!
    # You might want to use v${MAJOR_VERSION} here once you go to v2...
    PRODUCT_NAME "${PRODUCT_NAME}")

# This lets us use our code in both the JUCE targets and our Test target
# Without running into ODR violations
add_library(SharedCode INTERFACE)

clap_juce_extensions_plugin(TARGET "${PROJECT_NAME}"
    CLAP_ID "${BUNDLE_ID}"
    CLAP_FEATURES audio-effect)

# Enable fast math, C++20 and a few other target defaults
include(SharedCodeDefaults)

# Manually list all .h and .cpp files for the plugin
# If you are like me, you'll use globs for your sanity.
# Just ensure you employ CONFIGURE_DEPENDS so the build system picks up changes
# If you want to appease the CMake gods and avoid globs, manually add files like so:
# set(SourceFiles Source/PluginEditor.h Source/PluginProcessor.h Source/PluginEditor.cpp Source/PluginProcessor.cpp)
file(GLOB_RECURSE SourceFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/source/*.h")
target_sources(SharedCode INTERFACE ${SourceFiles})

# Add include directories to SharedCode
target_include_directories(SharedCode INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/source"
    "${CMAKE_CURRENT_SOURCE_DIR}/extern/bx/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/extern/bx/3rdparty"
    "${CMAKE_CURRENT_SOURCE_DIR}/extern/bgfx/examples/common" # for entry/entry.h etc
)

# Adds a BinaryData target for embedding assets into the binary
include(Assets)

# Compile bgfx shaders
include(CompileShaders)

set(SHADER_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/compiled_shaders")

# Compile vertex and fragment shaders
compile_shader(
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/vs_triangle.sc"
    "vertex"
    "${SHADER_OUTPUT_DIR}"
)

compile_shader(
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/fs_triangle.sc"
    "fragment"
    "${SHADER_OUTPUT_DIR}"
)

# Create a custom target for shaders
add_custom_target(CompiledShaders ALL
    DEPENDS 
        "${SHADER_OUTPUT_DIR}/vs_triangle.bin"
        "${SHADER_OUTPUT_DIR}/fs_triangle.bin"
    COMMENT "Building shaders"
)

juce_add_binary_data(CompiledShaderData
    SOURCES
        "${SHADER_OUTPUT_DIR}/vs_triangle.bin"
        "${SHADER_OUTPUT_DIR}/fs_triangle.bin"
    # 1. This generates the file as "CompiledShaderData.h" instead of "BinaryData.h"
    HEADER_NAME "CompiledShaderData.h"
    # 2. This puts the variables in the "ShaderData" namespace (e.g. ShaderData::vs_triangle_bin)
    #    so they don't conflict with the standard "BinaryData" namespace used by Assets.
    NAMESPACE "ShaderData"
)

add_dependencies(CompiledShaderData CompiledShaders)
set_target_properties(CompiledShaderData PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

# Make sure SharedCode can find the generated header
target_include_directories(SharedCode INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/juce_binarydata_CompiledShaderData/JuceLibraryCode")

# MacOS only: Cleans up folder and target organization on Xcode.
include(XcodePrettify)

# This is where you can set preprocessor definitions for JUCE and your plugin
target_compile_definitions(SharedCode
    INTERFACE

    # JUCE_WEB_BROWSER and JUCE_USE_CURL off by default
    JUCE_WEB_BROWSER=0  # If you set this to 1, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
    JUCE_USE_CURL=0     # If you set this to 1, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
    JUCE_VST3_CAN_REPLACE_VST2=0

    # Uncomment if you are paying for a an Indie/Pro license or releasing under GPLv3
    # JUCE_DISPLAY_SPLASH_SCREEN=0

    # lets the app known if we're Debug or Release
    CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
    VERSION="${CURRENT_VERSION}"

    # JucePlugin_Name is for some reason doesn't use the nicer PRODUCT_NAME
    PRODUCT_NAME_WITHOUT_VERSION="Pamplejuce"
)

# Link to any other modules you added (with juce_add_module) here!
# Usually JUCE modules must have PRIVATE visibility
# See https://github.com/juce-framework/JUCE/blob/master/docs/CMake%20API.md#juce_add_module
# However, with Pamplejuce, you'll link modules to SharedCode with INTERFACE visibility
# This allows the JUCE plugin targets and the Tests target to link against it
target_link_libraries(SharedCode
    INTERFACE
    Assets
    CompiledShaderData
    melatonin_inspector
    juce_audio_utils
    juce_audio_processors
    juce_dsp
    juce_gui_basics
    juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
    bx
    bimg
    bgfx_amalgamated
    meshoptimizer
)

# Link the JUCE plugin targets our SharedCode target
target_link_libraries("${PROJECT_NAME}" PRIVATE SharedCode)

# IPP support, comment out to disable
include(PamplejuceIPP)

# Everything related to the tests target
include(Tests)

# A separate target for Benchmarks (keeps the Tests target fast)
include(Benchmarks)

# Output some config for CI (like our PRODUCT_NAME)
include(GitHubENV)
