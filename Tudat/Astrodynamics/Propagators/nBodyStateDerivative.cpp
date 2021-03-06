#include "Tudat/Astrodynamics/Propagators/nBodyStateDerivative.h"

namespace tudat
{

namespace propagators
{

//! Function to determine in which order the ephemerides are to be updated
std::vector< std::string > determineEphemerisUpdateorder( std::vector< std::string > integratedBodies,
                                                          std::vector< std::string > centralBodies,
                                                          std::vector< std::string > ephemerisOrigins )
{

    std::vector< std::string > updateOrder;

    // Declare variables
    bool isFinished = 0;
    int currentIndex = 0;
    int counter = 0;
    std::vector< std::string >::const_iterator centralBodyIterator;
    std::vector< std::string >::const_iterator ephemerisOriginIterator;

    // Continue iterating until all integratedBodies have been handled.
    while( !isFinished )
    {
        // Check if current central body or ephemeris origin is integratedBodies
        centralBodyIterator = std::find(
                    integratedBodies.begin( ), integratedBodies.end( ),
                    centralBodies.at( currentIndex ) );
        ephemerisOriginIterator = std::find(
                    integratedBodies.begin( ), integratedBodies.end( ),
                    ephemerisOrigins.at( currentIndex ) );

        // If neither is in list, there is no dependency, and current body can be added to update
        // list.
        if( centralBodyIterator == integratedBodies.end( )
            && ephemerisOriginIterator == integratedBodies.end( ) )
        {
            // Add to list.
            updateOrder.push_back( integratedBodies.at( currentIndex ) );

            // Remove from list of handled bodies.
            integratedBodies.erase( integratedBodies.begin( ) + currentIndex );
            centralBodies.erase( centralBodies.begin( ) + currentIndex );
            ephemerisOrigins.erase( ephemerisOrigins.begin( ) + currentIndex );

            // Handle first entry at next iteration.
            currentIndex = 0;

            // Check if any bodies left.
            if( integratedBodies.size( ) == 0 )
            {
                isFinished = 1;
            }
        }
        else
        {
            // If both are found in list, start next iteration at whichever is first in list.
            if( centralBodyIterator != integratedBodies.end( )
                && ephemerisOriginIterator != integratedBodies.end( ) )
            {

                currentIndex = std::min(
                            std::distance(
                                integratedBodies.begin( ),
                                std::find( integratedBodies.begin( ), integratedBodies.end( ),
                                           centralBodies.at( currentIndex ) ) ),
                            std::distance(
                                integratedBodies.begin( ),
                                std::find( integratedBodies.begin( ), integratedBodies.end( ),
                                           ephemerisOrigins.at( currentIndex ) ) ));

            }
            // If only central body is found, start with this central body in next iteration
            else if( centralBodyIterator != integratedBodies.end( ) )
            {
                currentIndex = std::distance(
                            integratedBodies.begin( ),
                            std::find( integratedBodies.begin( ), integratedBodies.end( ),
                                       centralBodies.at( currentIndex ) ) );
            }
            // If only ephemeris origin is found, start with this ephemeris origin in next iteration
            else if( ephemerisOriginIterator != integratedBodies.end( ) )
            {
                currentIndex = std::distance(
                            integratedBodies.begin( ),
                            std::find( integratedBodies.begin( ), integratedBodies.end( ),
                                       ephemerisOrigins.at( currentIndex ) ) );
            }
        }

        // Check to break circular dependency that occurs for inadmissible input data.
        counter++;
        if( counter > 10000 )
        {
            throw std::runtime_error( "Warning, ephemeris update order determination now at iteration " +
                                      boost::lexical_cast< std::string>( counter ) );
        }
    }

    return updateOrder;
}

}

}
