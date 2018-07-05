
cmake_minimum_required (VERSION 2.6)

set (LIBS_DIR /home/irina/FRRR/jniLibs)

add_library( openalpr SHARED IMPORTED )
set_target_properties(openalpr PROPERTIES IMPORTED_LOCATION ${LIBS_DIR}/${ANDROID_ABI}/libopenalpr.so)

add_library( openalprjni SHARED IMPORTED )
set_target_properties(openalprjni PROPERTIES IMPORTED_LOCATION ${LIBS_DIR}/${ANDROID_ABI}/libopenalprjni.so)

add_library( tess SHARED IMPORTED )
set_target_properties(tess PROPERTIES IMPORTED_LOCATION ${LIBS_DIR}/${ANDROID_ABI}/libtess.so)

add_library( jpgt SHARED IMPORTED )
set_target_properties(jpgt PROPERTIES IMPORTED_LOCATION ${LIBS_DIR}/${ANDROID_ABI}/libjpgt.so)

add_library( pngt SHARED IMPORTED )
set_target_properties(pngt PROPERTIES IMPORTED_LOCATION ${LIBS_DIR}/${ANDROID_ABI}/libpngt.so)

add_library( lept SHARED IMPORTED )
set_target_properties(lept PROPERTIES IMPORTED_LOCATION ${LIBS_DIR}/${ANDROID_ABI}/liblept.so)

#add_library( opencv_java3 SHARED IMPORTED )
#set_target_properties(opencv_java3 PROPERTIES IMPORTED_LOCATION ${LIBS_DIR}/${ANDROID_ABI}/libopencv_java3.so)
