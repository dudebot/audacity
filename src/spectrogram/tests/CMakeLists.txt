#
# Audacity: A Digital Audio Editor
#

set(MODULE_TEST spectrogram_tests)

set(MODULE_TEST_SRC
    ${CMAKE_CURRENT_LIST_DIR}/stereopalette_tests.cpp
    )

set(MODULE_TEST_LINK
    spectrogram
    )

set(MODULE_TEST_INCLUDE
    ${CMAKE_CURRENT_LIST_DIR}/..
    )

set(MODULE_TEST_DATA_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(SetupGTest)
