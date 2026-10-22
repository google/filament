# Compile internal irrXML only if system is not requested
if( NOT SYSTEM_IRRXML )
    add_subdirectory(irrXML)
endif( NOT SYSTEM_IRRXML )
