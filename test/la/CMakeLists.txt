IF( USE_WISEKMA )
    ADD_DEFINITIONS( -DWISEKMA=$(WISEKMA) )
ENDIF( USE_WISEKMA )

IF( USE_IZENECMA )
    ADD_DEFINITIONS( -DIZENECMA=$(IZENECMA) )
ENDIF( USE_IZENECMA )

SET(Boost_USE_STATIC_LIBS OFF)
FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS unit_test_framework)

IF(Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)
  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
  )

#  add_executable(t_LA
#	Runner.cpp
#	t_Tokenizer.cpp
#	t_Analyzer.cpp
#	t_CharAnalyzer.cpp
#	t_TokenAnalyzer.cpp
#	t_EnglishAnalyzer.cpp
#	t_ChineseAnalyzer.cpp
#	t_KoreanAnalyzer.cpp
#	t_MultiLanguageAnalyzer.cpp
#	)

  set(la_libs
	ilplib
	${Boost_THREAD_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_SERIALIZATION_LIBRARY}
	${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
	${izenelib_LIBRARIES}
	${izenecma_LIBRARIES}
	${izenejma_LIBRARIES}	
	${wisekma_LIBRARIES}
	${Glog_LIBRARIES}
	${TokyoCabinet_LIBRARIES}
 )

#  target_link_libraries(t_LA ${la_libs})
ENDIF(Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)

