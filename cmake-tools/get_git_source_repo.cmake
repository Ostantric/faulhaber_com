##### Download a source repository and make it available as source files to use in our build #####
#
# Instead of copy pasting third party code into our repository, we should prefer (in this order):
#   1. Installing the library system wide via our ansible provisioning system in sew/ops/
#   2, Using this helper function to make the repo's source available during the build
#
# This function clones the given third party repository into the ${CMAKE_BINARY_DIR}/third_party/
# directory, and exports ${repo_prefix}_INCLUDE_DIR and ${repo_prefix}_SOURCE_DIR for use
# in building our own internal executables.
#
# Arguments:
#   repo_prefix - the prefix used on the exported variables. EG if you use LABJACK_U6 then
#     LABJACK_U6_INCLUDE_DIR and LABJACK_U6_SOURCE_DIR are set in the parent scope for you.
#   repo_git_url - A valid url for direct use with git clone.
#   repo_git_rev - A valid revision that should be checked out in the repository (eg: master, ae5cff, etc)
#   repo_source_subdir - Within the repository, the directory containing the source files we
#     want to be able to use.
#   repo_include_subdir - Within the repository, the directory containing the header files
#     we want to be able to use.
###################################################################################################
function(get_git_source_repo repo_prefix repo_git_url repo_git_rev repo_source_subdir repo_include_subdir)
    if (repo_prefix STREQUAL "")
        message(FATAL "You must specify a repo_prefix")
    endif()

    if (DEFINED ${repo_prefix}_INCLUDE_DIR)
        message(FATAL "The variable '${repo_prefix}_INCLUDE_DIR' is already defined, so '${repo_prefix}' can't be used as a prefix.")
    endif()

    if (DEFINED ${repo_prefix}_SOURCE_DIR)
        message(FATAL "The variable '${repo_prefix}_SOURCE_DIR' is already defined, so '${repo_prefix}' can't be used as a prefix.")
    endif()

    if (repo_git_url STREQUAL "")
        message(FATAL "You must specify a git repo url")
    endif()

    if (repo_source_subdir STREQUAL "")
        message(FATAL "You must specify a directory within the repo to treat as the source containing directory")
    endif()

    if (repo_include_subdir STREQUAL "")
        message(FATAL "You must specify a directory within the repo to treat as the header containing directory")
    endif()

    set(build_third_party_dir "${CMAKE_BINARY_DIR}/third_party")
    set(repo_base_dir "${build_third_party_dir}/${repo_prefix}")
    file(MAKE_DIRECTORY "${build_third_party_dir}")

    if (NOT EXISTS "${repo_base_dir}")
        message(STATUS "Cloning ${repo_git_url} into ${repo_base_dir} for ${repo_prefix} sources.")
        execute_process(
            COMMAND git clone "${repo_git_url}" "${repo_base_dir}"
            RESULT_VARIABLE repo_clone_result
            ERROR_VARIABLE repo_clone_errors
        )

        if (NOT repo_clone_result EQUAL 0)
            message(FATAL "Could not clone repository ${repo_git_url}. Git output:\n${repo_clone_errors}")
        endif()
    endif()

    message(STATUS "Checking out ${repo_git_rev} revision of the ${repo_git_url} git repo.")
    execute_process(
        COMMAND git checkout "${repo_git_rev}"
        WORKING_DIRECTORY "${repo_base_dir}"
        RESULT_VARIABLE repo_checkout_result
        ERROR_VARIABLE repo_checkout_errors
    )
    if (NOT repo_checkout_result EQUAL 0)
        message(FATAL "Could not checkout revision '${repo_git_rev}' in ${repo_git_url}. Git output:\n${repo_checkout_errors}")
    endif()

    file(TO_CMAKE_PATH "${repo_base_dir}/${repo_include_subdir}" exported_include_dir)
    file(TO_CMAKE_PATH "${repo_base_dir}/${repo_source_subdir}" exported_source_dir)

    if (NOT EXISTS "${exported_include_dir}")
        message(FATAL "The include dir '${exported_include_dir}' does not exist, are you sure the subdir '${repo_include_subdir}' is correct?")
    endif()

    if (NOT EXISTS "${exported_source_dir}")
        message(FATAL "The source dir '${exported_source_dir}' does not exist, are you sure the subdir '${repo_source_subdir}' is correct?")
    endif()

    set("${repo_prefix}_INCLUDE_DIR" "${exported_include_dir}" PARENT_SCOPE)
    set("${repo_prefix}_SOURCE_DIR" "${exported_source_dir}" PARENT_SCOPE)
endfunction(get_git_source_repo)
