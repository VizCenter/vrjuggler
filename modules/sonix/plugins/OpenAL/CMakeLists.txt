find_package(ALUT REQUIRED)

include_directories(${ALUT_INCLUDE_DIRS})

add_library(OpenAL_snd
	MODULE
	OpenALSoundImplementation.cpp
	OpenALSoundImplementation.h
	openal.rc)

target_link_libraries(OpenAL_snd sonix ${ALUT_LIBRARIES})

install(TARGETS OpenAL_snd
	LIBRARY DESTINATION lib/${SNX_LIB_DIR}/plugins COMPONENT sonix-runtime-openal
	RUNTIME DESTINATION lib/${SNX_LIB_DIR}/plugins COMPONENT sonix-runtime-openal)