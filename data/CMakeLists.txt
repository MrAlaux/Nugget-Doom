string(TIMESTAMP PROJECT_DATE "%Y-%m-%d" UTC)

configure_file(io.github.MrAlaux.Nugget-Doom.metainfo.in
               io.github.MrAlaux.Nugget-Doom.metainfo.xml
               ESCAPE_QUOTES @ONLY)

if(UNIX AND NOT APPLE)
    install(FILES io.github.MrAlaux.Nugget-Doom.desktop DESTINATION share/applications)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/io.github.MrAlaux.Nugget-Doom.metainfo.xml"
            DESTINATION share/metainfo)
    install(FILES io.github.MrAlaux.Nugget-Doom-setup.desktop DESTINATION share/applications)
    install(FILES nugget-doom.png DESTINATION share/icons/hicolor/128x128/apps)
    install(FILES nugget-doom-setup.png DESTINATION share/icons/hicolor/128x128/apps)
endif()
