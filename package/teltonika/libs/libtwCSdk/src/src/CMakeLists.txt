# Recurse through tomcrypt directory to collect all source files
file(GLOB_RECURSE tomcrypt_src thirdParty/tomcrypt/src/*.c)
# Recurse through source directories to collect all header files
file(GLOB_RECURSE headers
		${CMAKE_CURRENT_SOURCE_DIR}/api/*.h
		${CMAKE_CURRENT_SOURCE_DIR}/config/*.h
		${CMAKE_CURRENT_SOURCE_DIR}/fileTransfer/*.h
		${CMAKE_CURRENT_SOURCE_DIR}/messaging/*.h
		${CMAKE_CURRENT_SOURCE_DIR}/porting/*.h
		${CMAKE_CURRENT_SOURCE_DIR}/tls/*.h
		${CMAKE_CURRENT_SOURCE_DIR}/utils/*.h
		${CMAKE_CURRENT_SOURCE_DIR}/websocket/*.h
		${CMAKE_CURRENT_SOURCE_DIR}/tunneling/*.h
		${CMAKE_CURRENT_SOURCE_DIR}/subscribedProps/*.h
		${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/cJSON/*.h
		${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/wildcard/*.h
		${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/tomcrypt/src/headers/*.h
		${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/ntlm/*.h
		${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/wildcard/*.h
		${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/libcfu/*.h
		${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/zlib/*.h
		${CMAKE_CURRENT_SOURCE_DIR}/stubs/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/shapes/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../build/platforms/win32-dll/*.h
		${CMAKE_CURRENT_SOURCE_DIR}/primitiveUtils/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/standardProps/*.h
		${CMAKE_CURRENT_SOURCE_DIR}/threadUtils/*.h
		)

# Include os-specific porting layer source
if (${OS} MATCHES "macos")
	set(twPortingLayer
			porting/twApiWrapper.c
			porting/twPThreads.c
			porting/twLinux.c)
elseif (${OS} MATCHES "linux")
	set(twPortingLayer
			porting/twApiWrapper.c
			porting/twPThreads.c
			porting/twLinux.c)
elseif (${OS} MATCHES "windows")
	set(twPortingLayer
			porting/twApiWrapper.c
			porting/twWin32Threads.c
			porting/twWindows.c)
endif ()



# Include TLS library source and include directories
set(tls_include_dirs ${OPENSSL_INCLUDE_DIR})

# Set C SDK source files
set(twCSdk_src
		api/twApi.c
		api/twPasswds.c
		api/twProperties.c
		api/twServices.c
		fileTransfer/twFileManager.c
		fileTransfer/twFileTransferCallbacks.c
		messaging/twBaseTypes.c
		messaging/twInfoTable.c
		messaging/twMessages.c
		messaging/twMessaging.c
		primitiveUtils/twPrimitiveUtils.c
		shapes/twShapes.c
		standardProps/twStandardProps.c
		stubs/twApiStubs.c
		subscribedProps/twSubscribedProps.c
		thirdParty/cJSON/cJSON.c
		thirdParty/ntlm/ntlm.c
		thirdParty/wildcard/wildcard.c
		threadUtils/twThreadUtils.c
		tls/twConnectionInfo.c
		tls/twTls.c
		tunneling/twTunnelManager.c
		utils/crypto_wrapper.c
		utils/list.c
		utils/twMap.c
		utils/twDict.c
		utils/stringUtils.c
		utils/twTasker.c
		utils/twHttpProxy.c
		utils/twLogger.c
		utils/twMap.c
		utils/twNtlm.c
		utils/twOfflineMsgStore.c
		utils/twPath.c
		websocket/twWebsocket.c
		tunneling/twTunnelManager.c
		subscribedProps/twSubscribedProps.c
		thirdParty/ntlm/ntlm.c
		thirdParty/cJSON/cJSON.c
		thirdParty/wildcard/wildcard.c
		thirdParty/libcfu/src/cfuhash.c
		thirdParty/zlib/adler32.c
		thirdParty/zlib/crc32.c
		thirdParty/zlib/deflate.c
		thirdParty/zlib/inflate.c
		thirdParty/zlib/inffast.c
		thirdParty/zlib/inftrees.c
		thirdParty/zlib/trees.c
		thirdParty/zlib/zutil.c
		stubs/twApiStubs.c
		${twPortingLayer}
        ${tomcrypt_src}
        ${tls_src}
        ${headers}
		${dotnet_wrapper}
		thirdParty/zlib/gzlib.c
		thirdParty/zlib/gzclose.c
		thirdParty/zlib/gzread.c
		utils/twZipTar.c
		thirdParty/zlib/contrib/untgz/untgz.c
		thirdParty/zlib/contrib/untgz/untgz.c
		thirdParty/zlib/contrib/minizip/unzip.c
		thirdParty/zlib/contrib/minizip/ioapi.c
		tls/twSig.c
		)

		# Set C SDK include directories
set( twCSdk_include_dirs
		${CMAKE_CURRENT_SOURCE_DIR}/api
		${CMAKE_CURRENT_SOURCE_DIR}/config
		${CMAKE_CURRENT_SOURCE_DIR}/fileTransfer
		${CMAKE_CURRENT_SOURCE_DIR}/messaging
		${CMAKE_CURRENT_SOURCE_DIR}/porting
		${CMAKE_CURRENT_SOURCE_DIR}/tls
		${CMAKE_CURRENT_SOURCE_DIR}/utils
		${CMAKE_CURRENT_SOURCE_DIR}/websocket
		${CMAKE_CURRENT_SOURCE_DIR}/tunneling
		${CMAKE_CURRENT_SOURCE_DIR}/shapes
		${CMAKE_CURRENT_SOURCE_DIR}/primitiveUtils
        ${CMAKE_CURRENT_SOURCE_DIR}/standardProps
		${CMAKE_CURRENT_SOURCE_DIR}/threadUtils
		${CMAKE_CURRENT_SOURCE_DIR}/subscribedProps
		${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/cJSON
		${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/wildcard
		${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/tomcrypt/src/headers
		${CMAKE_CURRENT_SOURCE_DIR}/stubs
		${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/ntlm
		${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/libcfu/src
		${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/zlib
		${CMAKE_CURRENT_SOURCE_DIR}/thirdParty/zlib/contrib/minizip
		${tls_include_dirs}
)

configure_file(
		${CMAKE_CURRENT_SOURCE_DIR}/version.rc.in
		${CMAKE_CURRENT_BINARY_DIR}/version.rc
		@ONLY)

# Link
if(BUILD_AND_INSTALL_SHARED)
	add_library(twCSdk SHARED ${twCSdk_src} ${CMAKE_CURRENT_BINARY_DIR}/version.rc api/twExt.h)
	target_include_directories(twCSdk PUBLIC ${twCSdk_include_dirs})
endif()

if(BUILD_AND_INSTALL_STATIC)
 add_library(twCSdk_static STATIC ${twCSdk_src} ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
 target_include_directories(twCSdk_static PUBLIC ${twCSdk_include_dirs})
endif()

# Link openssl static library stubs
if(BUILD_AND_INSTALL_SHARED)
	target_link_libraries(twCSdk ${OPENSSL_LIBRARIES})
endif()
if(BUILD_AND_INSTALL_STATIC)
	target_link_libraries(twCSdk_static ${OPENSSL_LIBRARIES})
endif()

if (${OS} MATCHES "windows")
	# Link windows ws libraries (openssl needs ws2_32 to link correctly)
	if(BUILD_AND_INSTALL_SHARED)
		target_link_libraries(twCSdk wsock32 ws2_32)
	endif()
    if(BUILD_AND_INSTALL_STATIC)
	    target_link_libraries(twCSdk_static wsock32 ws2_32)
    endif()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
endif ()

# Doxygen
# Add a target to generate API documentation using Doxygen with "make doc"
find_package(Doxygen)
if(DOXYGEN_FOUND)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
	add_custom_target(doc
			${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMENT "Generating API documentation with Doxygen" VERBATIM
			)
endif(DOXYGEN_FOUND)

# CPACK
# Configuration
set(SDK_VERSION_STRING ${SDK_PRODUCT_MAJOR}.${SDK_PRODUCT_MINOR}.${SDK_PRODUCT_REV})
set(CPACK_PACKAGE_VERSION_MAJOR "${SDK_PRODUCT_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${SDK_PRODUCT_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${SDK_PRODUCT_REV}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "tw-c-sdk")
set(CPACK_SOURCE_GENERATOR "ZIP")
if(BUILD_AND_INSTALL_SHARED)
	set_target_properties(twCSdk PROPERTIES VERSION ${SDK_VERSION_STRING} SOVERSION ${SDK_VERSION_STRING})
endif()
if(BUILD_AND_INSTALL_STATIC)
	set_target_properties(twCSdk_static PROPERTIES VERSION ${SDK_VERSION_STRING} SOVERSION ${SDK_VERSION_STRING})
endif()
if(BRANCH_NAME)
	set(CPACK_SOURCE_PACKAGE_FILE_NAME "c-sdk-${C_SDK_VERSION}-${BRANCH_NAME}")
else()
	set(CPACK_SOURCE_PACKAGE_FILE_NAME "c-sdk-${C_SDK_VERSION}-local-build")
endif()
set(CPACK_SOURCE_IGNORE_FILES
		build.*\\\\.sh
		\\\\.git.*
		\\\\.idea.*
		api/twVersion.h.template
		.*\\\\.o
		.*\\\\.a
		.*\\\\.swp
		.*\\\\.tags
		.*/obj/.*
		.*/linux/bin.*
		build/
		examples/Old.*
		examples/Regression.*
		test/integration/RegressionIntegrationTests\\\\.c
		porting/twMarvel.*
		porting/twTiSimplelink.*
		tls/twMarvel.*
		tls/twTiSimpleLink.*
		test/etc/testconfig\\\\.json
		test/etc/docker/
		tutorial/
		${CMAKE_BINARY_DIR}
		README\\\\.md
		ReleaseNotes\\\\.txt)

# Install the C SDK with "sudo make install"
if(BUILD_AND_INSTALL_SHARED)
	# Install C SDK libraries
	install(TARGETS twCSdk
			LIBRARY DESTINATION lib COMPONENT libraries
			ARCHIVE DESTINATION lib COMPONENT libraries)
	if(${OS} MATCHES "windows")
		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/twCSdk.dll DESTINATION lib)
	endif()
	# Install OpenSSL libraries
	install(FILES ${OPENSSL_LIBRARIES} DESTINATION lib)
	if(${OS} MATCHES "windows")
		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/ssleay32.dll DESTINATION lib)
		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/libeay32.dll DESTINATION lib)
	endif()
	# Install included headers
	install(FILES ${headers} DESTINATION include/thingworx)
endif()
# Platform apropriate binary distribution
add_custom_target(dist COMMAND "cpack --config ${CMAKE_BINARY_DIR}/CPackConfig.cmake" WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
include(CPack)

if(BUILD_AND_INSTALL_SHARED)
	if(${OS} MATCHES "windows")
		add_custom_command(TARGET twCSdk POST_BUILD
				COMMAND cmake -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/twCSdk.dll" "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/twCSdk.dll"
				COMMENT "Copying C SDK dll to base directory.")
			# Copy over openssl dll files
			add_custom_command(TARGET twCSdk POST_BUILD
				COMMAND cmake -E copy_if_different "${OPENSSL_SSLEAY_BIN_PATH}" "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/ssleay32.dll"
				COMMENT "Copying ssleay dll to tests directory.")
			add_custom_command(TARGET twCSdk POST_BUILD
				COMMAND cmake -E copy_if_different "${OPENSSL_LIBEAY_BIN_PATH}" "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/libeay32.dll"
				COMMENT "Copying libeay dll to tests directory.")
	endif()
endif()

