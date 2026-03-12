cmake_minimum_required(VERSION 3.22)

project(BloomVerb VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
include(CTest)

set(BLOOMVERB_JUCE_GIT_TAG 29396c22c93392d6738e021b83196283d6e4d850 CACHE STRING
    "Pinned JUCE commit (JUCE 8.0.12)")

FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG ${BLOOMVERB_JUCE_GIT_TAG}
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(juce)

juce_add_plugin(BloomVerb
    COMPANY_NAME "BloomVerb"
    BUNDLE_ID com.bloomverb.plugin
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE BlmV
    PLUGIN_CODE BlmR
    FORMATS VST3 Standalone
    PRODUCT_NAME "BloomVerb"
)

target_sources(BloomVerb
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/Parameters/BloomVerbParameters.cpp
        Source/Parameters/BloomVerbParameters.h
        Source/Presets/BloomVerbPreset.cpp
        Source/Presets/BloomVerbPreset.h
        Source/Presets/BloomVerbPresetBank.cpp
        Source/Presets/BloomVerbPresetBank.h
        Source/DSP/BloomVerbEngine.cpp
        Source/DSP/BloomVerbEngine.h
        Source/DSP/BloomVerbTypeVoicing.cpp
        Source/DSP/BloomVerbTypeVoicing.h
)

target_compile_definitions(BloomVerb
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(BloomVerb
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

juce_add_gui_app(BloomVerbLoopTester
    PRODUCT_NAME "BloomVerb Loop Tester"
    COMPANY_NAME "BloomVerb"
    BUNDLE_ID com.bloomverb.looptester
)

target_sources(BloomVerbLoopTester
    PRIVATE
        Source/Tester/LoopTesterMain.cpp
        Source/DSP/BloomVerbEngine.cpp
        Source/DSP/BloomVerbEngine.h
        Source/DSP/BloomVerbTypeVoicing.cpp
        Source/DSP/BloomVerbTypeVoicing.h
)

target_compile_definitions(BloomVerbLoopTester
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

target_link_libraries(BloomVerbLoopTester
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

if(BUILD_TESTING)
    set(BLOOMVERB_TEST_COMMON_SOURCES
        Source/DSP/BloomVerbEngine.cpp
        Source/DSP/BloomVerbTypeVoicing.cpp
        Source/Parameters/BloomVerbParameters.cpp
        Source/Presets/BloomVerbPreset.cpp
        Source/Presets/BloomVerbPresetBank.cpp
    )

    function(bloomverb_add_test test_name test_source)
        add_executable(${test_name}
            ${test_source}
            ${BLOOMVERB_TEST_COMMON_SOURCES}
        )

        target_include_directories(${test_name}
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}
                ${CMAKE_CURRENT_SOURCE_DIR}/Source
        )

        target_compile_definitions(${test_name}
            PRIVATE
                JUCE_WEB_BROWSER=0
                JUCE_USE_CURL=0
                JUCE_VST3_CAN_REPLACE_VST2=0
        )

        target_link_libraries(${test_name}
            PRIVATE
                juce::juce_audio_basics
                juce::juce_audio_processors
                juce::juce_dsp
                juce::juce_events
                juce::juce_data_structures
                juce::juce_core
            PUBLIC
                juce::juce_recommended_config_flags
                juce::juce_recommended_warning_flags
        )

        add_test(NAME ${test_name} COMMAND ${test_name})
    endfunction()

    bloomverb_add_test(BloomVerbEngineTests Tests/BloomVerbEngineTests.cpp)
    bloomverb_add_test(BloomVerbEngineInvarianceTests Tests/BloomVerbEngineInvarianceTests.cpp)
    bloomverb_add_test(BloomVerbStateTests Tests/BloomVerbStateTests.cpp)
    bloomverb_add_test(BloomVerbPresetRecallTests Tests/BloomVerbPresetRecallTests.cpp)

    target_sources(BloomVerbPresetRecallTests
        PRIVATE
            Source/PluginProcessor.cpp
            Source/PluginEditor.cpp
    )

    target_compile_definitions(BloomVerbPresetRecallTests
        PRIVATE
            JucePlugin_Name="BloomVerb"
    )

    target_link_libraries(BloomVerbPresetRecallTests
        PRIVATE
            juce::juce_audio_utils
            juce::juce_gui_basics
            juce::juce_gui_extra
    )
endif()
