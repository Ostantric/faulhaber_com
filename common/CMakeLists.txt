### Compile and make available as libraries all the common cpp sources ###
#
# This exports the ${sew_common_libraries} list to the parent scope,
# so you can just add ${sew_common_libraries} to your target_link_libraries
# if you need these.
#
# NOTE[imo]: For now the individual libraries are not exported, but if
#   we need to we could add another set(... PARENT_SCOPE) in the foreach
file(GLOB common_cpp_files RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

set(sew_common_libraries "" PARENT_SCOPE)

foreach(cpp_file ${common_cpp_files})
	get_filename_component(cpp_file_basename ${cpp_file} NAME_WE)
	set(new_lib_name "SEW_COMMON_${cpp_file_basename}")
	add_library("${new_lib_name}" "${cpp_file}")
	list(APPEND sew_common_libraries "${new_lib_name}")
endforeach(cpp_file)

set(sew_common_libraries ${sew_common_libraries} PARENT_SCOPE)
