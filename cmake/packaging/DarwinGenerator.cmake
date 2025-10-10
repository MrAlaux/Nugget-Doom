set(staging_dir "${CPACK_TEMPORARY_DIRECTORY}/${CPACK_PACKAGING_INSTALL_PREFIX}")
set(packaged_dir "${CPACK_TEMPORARY_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}")
file(MAKE_DIRECTORY ${packaged_dir})

file(
    COPY_FILE
    ${staging_dir}/${CPACK_BIN_DIR}/nugget-doom
    ${packaged_dir}/nugget-doom
)
file(
    COPY_FILE
    ${staging_dir}/${CPACK_BIN_DIR}/nugget-doom-setup
    ${packaged_dir}/nugget-doom-setup
)

file(
    COPY_FILE
    ${staging_dir}/share/nugget-doom/nugget-doom.pk3
    ${packaged_dir}/nugget-doom.pk3
)
file(
    COPY ${staging_dir}/share/nugget-doom/soundfonts
    DESTINATION ${packaged_dir}
)

file(
    WRITE
    "${packaged_dir}/Troubleshooting.txt"
    CONTENT
    "If you are getting errors like 'libzip.5.5.dylib cant be opened because Apple cannot check it for malicious software.'\n"
    "Run the following command in the nugget-doom folder:\n\n"
    "xattr -dr com.apple.quarantine path/to/folder\n"
)

execute_process(
    COMMAND
    zip -r ${CPACK_PACKAGE_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}.zip
    ${CPACK_PACKAGE_FILE_NAME}
    WORKING_DIRECTORY
    ${CPACK_TEMPORARY_DIRECTORY}
)
