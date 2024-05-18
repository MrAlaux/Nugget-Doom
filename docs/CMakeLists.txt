set(NUGGETDOOM_DOCS
    boom.txt
    boomdeh.txt
    boomref.txt
    mbf.txt
    mbfedit.txt
    mbffaq.txt
    nughud.lmp
    nughud.md
    options.txt
    woofhud.lmp
    woofhud.md)

if(WIN32)
    install(FILES ${NUGGETDOOM_DOCS} DESTINATION docs)
else()
    install(FILES ${NUGGETDOOM_DOCS} DESTINATION "share/doc/${PROJECT_SHORTNAME}")
endif()
