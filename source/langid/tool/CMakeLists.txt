add_executable(langid_create_profile langid_create_profile.cpp)
target_link_libraries(langid_create_profile ${LIBS} )

add_executable(langid_measure_profile langid_measure_profile.cpp)
target_link_libraries(langid_measure_profile ${LIBS})

add_executable(langid_select_profile langid_select_profile.cpp)
target_link_libraries(langid_select_profile ${LIBS})

add_executable(langid_build_encoding_model langid_build_encoding_model.cpp)
target_link_libraries(langid_build_encoding_model ${LIBS})

add_executable(langid_build_language_model langid_build_language_model.cpp)
target_link_libraries(langid_build_language_model ${LIBS})
