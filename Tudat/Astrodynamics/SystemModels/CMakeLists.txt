 #    Copyright (c) 2010-2016, Delft University of Technology
 #    All rigths reserved
 #
 #    This file is part of the Tudat. Redistribution and use in source and
 #    binary forms, with or without modification, are permitted exclusively
 #    under the terms of the Modified BSD license. You should have received
 #    a copy of the license with this file. If not, please or visit:
 #    http://tudat.tudelft.nl/LICENSE.
 #

# Set the source files.
set(SYSTEMMODELS_SOURCES
  "${SRCROOT}${SYSTEMMODELSDIR}/engineModel.cpp"
)

# Set the header files.
set(SYSTEMMODELS_HEADERS 
  "${SRCROOT}${SYSTEMMODELSDIR}/engineModel.h"
  "${SRCROOT}${SYSTEMMODELSDIR}/vehicleSystems.h"
)

# Add static libraries.
add_library(tudat_system_models STATIC ${SYSTEMMODELS_SOURCES} ${SYSTEMMODELS_HEADERS})
setup_tudat_library_target(tudat_system_models "${SRCROOT}${SYSTEMMODELSDIR}")


