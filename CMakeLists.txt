# 3.18 is latest bundled by VS 2019 as of Jan 2021
# 3.19 is latest bundled CLion as of 2021.1
cmake_minimum_required(VERSION 3.18)

# Change this to your project name
set(TARGET_NAME "MIDIVibrato")
set(PLUGIN_NAME "MIDI Vibrato")

# Set the plugin formats you'll be building here. Valid formats: AAX Unity VST AU AUv3 Standalone
set(FORMATS AU VST3 AUv3)

# TODO: Automate/coordinate version with GitHub tagging?
project(${TARGET_NAME} VERSION 0.0.1)

# Create a /Modules directory in the IDE with the JUCE Module code
option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Show all module sources in IDE projects" ON)

# JUCE is setup as a submodule in the /JUCE folder
# Locally, you need to run `git submodule update --init --recursive` once
# On Github Actions, it's managed by actions/checkout
add_subdirectory(JUCE)

# find_package(JUCE CONFIG REQUIRED)

# Check the readme at `docs/CMake API.md` in the JUCE repo for full config
juce_add_plugin("${TARGET_NAME}"
    # VERSION ...                               # Set this if the plugin version is different to the project version
    # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
    # ICON_SMALL ...
    COMPANY_NAME MagnusHodne
    # IS_SYNTH TRUE/FALSE                       # Is this a synth or an effect?
    NEEDS_MIDI_INPUT TRUE               # Does the plugin need midi input?
    NEEDS_MIDI_OUTPUT TRUE/FALSE              # Does the plugin need midi output?
    IS_MIDI_EFFECT TRUE                         # Is this plugin a MIDI effect?
    # EDITOR_WANTS_KEYBOARD_FOCUS TRUE/FALSE    # Does the editor need keyboard focus?
    COPY_PLUGIN_AFTER_BUILD TRUE # On MacOS, plugin will be copied to /Users/you/Library/Audio/Plug-Ins/
    PLUGIN_MANUFACTURER_CODE MaHo 
    PLUGIN_CODE MiVi                            # A unique four-character plugin id with at least one upper-case character
    FORMATS "${FORMATS}"
    PRODUCT_NAME "${PLUGIN_NAME}")        # The name of the final executable, which can differ from the target name

# C++20 please 
target_compile_features("${TARGET_NAME}" PRIVATE cxx_std_20)

# Manually list all .h and .cpp files for the plugin, don't use globs:
set(SourceFiles
    Source/Component/MidiMeter.h
    Source/VibratoDetector.h
    Source/Utility/MidiSineGenerator.h
    Source/PluginEditor.h
    Source/PluginEditor.cpp
    Source/PluginProcessor.h
    Source/PluginProcessor.cpp)

target_sources("${TARGET_NAME}" PRIVATE ${SourceFiles})

# No, we don't want our source buried in extra nested folders
set_target_properties("${TARGET_NAME}" PROPERTIES FOLDER "")

# The source tree should uhhh, still look like the source tree, yo
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Source PREFIX "" FILES ${SourceFiles})

# Setup our binary data as a target
# juce_add_binary_data(Assets SOURCES pamplejuce.png)

# We'll need to link to these from our plugin as well as our tests
set(JUCE_DEPENDENCIES 
#    juce::juce_audio_basics
    juce::juce_audio_devices
#    juce::juce_audio_formats
#    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
#    juce::juce_events
#    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra)

target_compile_definitions("${TARGET_NAME}"
    PUBLIC
    # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
    JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
    JUCE_VST3_CAN_REPLACE_VST2=0)
    
target_link_libraries("${TARGET_NAME}"
    PRIVATE
    # Assets
    ${JUCE_DEPENDENCIES}
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

add_subdirectory(Tests)
