/*    Copyright (c) 2010-2016, Delft University of Technology
 *    All rigths reserved
 *
 *    This file is part of the Tudat. Redistribution and use in source and
 *    binary forms, with or without modification, are permitted exclusively
 *    under the terms of the Modified BSD license. You should have received
 *    a copy of the license with this file. If not, please or visit:
 *    http://tudat.tudelft.nl/LICENSE.
 */

#include <algorithm>

#include <boost/make_shared.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include "Tudat/Astrodynamics/Aerodynamics/flightConditions.h"
#include "Tudat/Astrodynamics/Ephemerides/frameManager.h"
#include "Tudat/Astrodynamics/Gravitation/sphericalHarmonicsGravityField.h"
#include "Tudat/Astrodynamics/Propulsion/thrustMagnitudeWrapper.h"
#include "Tudat/Astrodynamics/ReferenceFrames/aerodynamicAngleCalculator.h"
#include "Tudat/Astrodynamics/ReferenceFrames/referenceFrameTransformations.h"
#include "Tudat/Basics/utilities.h"
#include "Tudat/SimulationSetup/PropagationSetup/accelerationSettings.h"
#include "Tudat/SimulationSetup/PropagationSetup/createAccelerationModels.h"
#include "Tudat/SimulationSetup/EnvironmentSetup/createFlightConditions.h"

namespace tudat
{

namespace simulation_setup
{

using namespace aerodynamics;
using namespace gravitation;
using namespace basic_astrodynamics;
using namespace electro_magnetism;
using namespace ephemerides;


//! Function to create a direct (i.e. not third-body) gravitational acceleration (of any type)
boost::shared_ptr< basic_astrodynamics::AccelerationModel< Eigen::Vector3d > > createDirectGravitationalAcceleration(
        const boost::shared_ptr< Body > bodyUndergoingAcceleration,
        const boost::shared_ptr< Body > bodyExertingAcceleration,
        const std::string& nameOfBodyUndergoingAcceleration,
        const std::string& nameOfBodyExertingAcceleration,
        const boost::shared_ptr< AccelerationSettings > accelerationSettings,
        const std::string& nameOfCentralBody,
        const bool isCentralBody )
{
    // Check if sum of gravitational parameters (i.e. inertial force w.r.t. central body) should be used.
    bool sumGravitationalParameters = 0;
    if( ( nameOfCentralBody == nameOfBodyExertingAcceleration ) && bodyUndergoingAcceleration != NULL )
    {
        sumGravitationalParameters = 1;
    }


    // Check type of acceleration model and create.
    boost::shared_ptr< basic_astrodynamics::AccelerationModel< Eigen::Vector3d > > accelerationModel;
    switch( accelerationSettings->accelerationType_ )
    {
    case central_gravity:
        accelerationModel = createCentralGravityAcceleratioModel(
                    bodyUndergoingAcceleration,
                    bodyExertingAcceleration,
                    nameOfBodyUndergoingAcceleration,
                    nameOfBodyExertingAcceleration,
                    sumGravitationalParameters );
        break;
    case spherical_harmonic_gravity:
        accelerationModel = createSphericalHarmonicsGravityAcceleration(
                    bodyUndergoingAcceleration,
                    bodyExertingAcceleration,
                    nameOfBodyUndergoingAcceleration,
                    nameOfBodyExertingAcceleration,
                    accelerationSettings,
                    sumGravitationalParameters );
        break;
    case mutual_spherical_harmonic_gravity:
        accelerationModel = createMutualSphericalHarmonicsGravityAcceleration(
                    bodyUndergoingAcceleration,
                    bodyExertingAcceleration,
                    nameOfBodyUndergoingAcceleration,
                    nameOfBodyExertingAcceleration,
                    accelerationSettings,
                    sumGravitationalParameters,
                    isCentralBody );
        break;
    default:

        std::string errorMessage = "Error when making gravitional acceleration model, cannot parse type " +
                boost::lexical_cast< std::string >( accelerationSettings->accelerationType_ );
        throw std::runtime_error( errorMessage );
    }
    return accelerationModel;
}

//! Function to create a third-body gravitational acceleration (of any type)
boost::shared_ptr< basic_astrodynamics::AccelerationModel< Eigen::Vector3d > > createThirdBodyGravitationalAcceleration(
        const boost::shared_ptr< Body > bodyUndergoingAcceleration,
        const boost::shared_ptr< Body > bodyExertingAcceleration,
        const boost::shared_ptr< Body > centralBody,
        const std::string& nameOfBodyUndergoingAcceleration,
        const std::string& nameOfBodyExertingAcceleration,
        const std::string& nameOfCentralBody,
        const boost::shared_ptr< AccelerationSettings > accelerationSettings )
{
    // Check type of acceleration model and create.
    boost::shared_ptr< basic_astrodynamics::AccelerationModel< Eigen::Vector3d > > accelerationModel;
    switch( accelerationSettings->accelerationType_ )
    {
    case central_gravity:
        accelerationModel = boost::make_shared< ThirdBodyCentralGravityAcceleration >(
                    boost::dynamic_pointer_cast< CentralGravitationalAccelerationModel3d >(
                        createDirectGravitationalAcceleration(
                            bodyUndergoingAcceleration, bodyExertingAcceleration,
                            nameOfBodyUndergoingAcceleration, nameOfBodyExertingAcceleration,
                            accelerationSettings, "", 0 ) ),
                    boost::dynamic_pointer_cast< CentralGravitationalAccelerationModel3d >(
                        createDirectGravitationalAcceleration(
                            centralBody, bodyExertingAcceleration,
                            nameOfCentralBody, nameOfBodyExertingAcceleration,
                            accelerationSettings, "", 1 ) ), nameOfCentralBody );
        break;
    case spherical_harmonic_gravity:
        accelerationModel = boost::make_shared< ThirdBodySphericalHarmonicsGravitationalAccelerationModel >(
                    boost::dynamic_pointer_cast< SphericalHarmonicsGravitationalAccelerationModel >(
                        createDirectGravitationalAcceleration(
                            bodyUndergoingAcceleration, bodyExertingAcceleration,
                            nameOfBodyUndergoingAcceleration, nameOfBodyExertingAcceleration,
                            accelerationSettings, "", 0 ) ),
                    boost::dynamic_pointer_cast< SphericalHarmonicsGravitationalAccelerationModel >(
                        createDirectGravitationalAcceleration(
                            centralBody, bodyExertingAcceleration, nameOfCentralBody, nameOfBodyExertingAcceleration,
                            accelerationSettings, "", 1 ) ), nameOfCentralBody );
        break;
    case mutual_spherical_harmonic_gravity:
        accelerationModel = boost::make_shared< ThirdBodyMutualSphericalHarmonicsGravitationalAccelerationModel >(
                    boost::dynamic_pointer_cast< MutualSphericalHarmonicsGravitationalAccelerationModel >(
                        createDirectGravitationalAcceleration(
                            bodyUndergoingAcceleration, bodyExertingAcceleration,
                            nameOfBodyUndergoingAcceleration, nameOfBodyExertingAcceleration,
                            accelerationSettings, "", 0 ) ),
                    boost::dynamic_pointer_cast< MutualSphericalHarmonicsGravitationalAccelerationModel >(
                        createDirectGravitationalAcceleration(
                            centralBody, bodyExertingAcceleration, nameOfCentralBody, nameOfBodyExertingAcceleration,
                            accelerationSettings, "", 1 ) ), nameOfCentralBody );
        break;
    default:

        std::string errorMessage = "Error when making third-body gravitional acceleration model, cannot parse type " +
                boost::lexical_cast< std::string >( accelerationSettings->accelerationType_ );
        throw std::runtime_error( errorMessage );
    }
    return accelerationModel;
}

//! Function to create gravitational acceleration (of any type)
boost::shared_ptr< AccelerationModel< Eigen::Vector3d > > createGravitationalAccelerationModel(
        const boost::shared_ptr< Body > bodyUndergoingAcceleration,
        const boost::shared_ptr< Body > bodyExertingAcceleration,
        const boost::shared_ptr< AccelerationSettings > accelerationSettings,
        const std::string& nameOfBodyUndergoingAcceleration,
        const std::string& nameOfBodyExertingAcceleration,
        const boost::shared_ptr< Body > centralBody,
        const std::string& nameOfCentralBody )
{

    boost::shared_ptr< AccelerationModel< Eigen::Vector3d > > accelerationModelPointer;
    if( accelerationSettings->accelerationType_ != central_gravity &&
            accelerationSettings->accelerationType_ != spherical_harmonic_gravity &&
            accelerationSettings->accelerationType_ != mutual_spherical_harmonic_gravity )
    {
        throw std::runtime_error( "Error when making gravitational acceleration, type is inconsistent" );
    }

    if( nameOfCentralBody == nameOfBodyExertingAcceleration || ephemerides::isFrameInertial( nameOfCentralBody ) )
    {
        accelerationModelPointer = createDirectGravitationalAcceleration( bodyUndergoingAcceleration,
                                                                          bodyExertingAcceleration,
                                                                          nameOfBodyUndergoingAcceleration,
                                                                          nameOfBodyExertingAcceleration,
                                                                          accelerationSettings,
                                                                          nameOfCentralBody, false );
    }
    else
    {
        accelerationModelPointer = createThirdBodyGravitationalAcceleration( bodyUndergoingAcceleration,
                                                                             bodyExertingAcceleration,
                                                                             centralBody,
                                                                             nameOfBodyUndergoingAcceleration,
                                                                             nameOfBodyExertingAcceleration,
                                                                             nameOfCentralBody, accelerationSettings );
    }

    return accelerationModelPointer;
}


//! Function to create central gravity acceleration model.
boost::shared_ptr< CentralGravitationalAccelerationModel3d > createCentralGravityAcceleratioModel(
        const boost::shared_ptr< Body > bodyUndergoingAcceleration,
        const boost::shared_ptr< Body > bodyExertingAcceleration,
        const std::string& nameOfBodyUndergoingAcceleration,
        const std::string& nameOfBodyExertingAcceleration,
        const bool useCentralBodyFixedFrame )
{
    // Declare pointer to return object.
    boost::shared_ptr< CentralGravitationalAccelerationModel3d > accelerationModelPointer;

    // Check if body is endowed with a gravity field model (i.e. is capable of exerting
    // gravitation acceleration).
    if( bodyExertingAcceleration->getGravityFieldModel( ) == NULL )
    {
        throw std::runtime_error(
                    std::string( "Error, gravity field model not set when making central ") +
                    " gravitational acceleration of " + nameOfBodyExertingAcceleration + " on " +
                    nameOfBodyUndergoingAcceleration );
    }
    else
    {
        boost::function< double( ) > gravitationalParameterFunction;

        // Set correct value for gravitational parameter.
        if( useCentralBodyFixedFrame == 0  ||
                bodyUndergoingAcceleration->getGravityFieldModel( ) == NULL )
        {
            gravitationalParameterFunction =
                    boost::bind( &gravitation::GravityFieldModel::getGravitationalParameter,
                                 bodyExertingAcceleration->getGravityFieldModel( ) );
        }
        else
        {
            boost::function< double( ) > gravitationalParameterOfBodyExertingAcceleration =
                    boost::bind( &gravitation::GravityFieldModel::getGravitationalParameter,
                                 bodyExertingAcceleration->getGravityFieldModel( ) );
            boost::function< double( ) > gravitationalParameterOfBodyUndergoingAcceleration =
                    boost::bind( &gravitation::GravityFieldModel::getGravitationalParameter,
                                 bodyUndergoingAcceleration->getGravityFieldModel( ) );
            gravitationalParameterFunction =
                    boost::bind( &utilities::sumFunctionReturn< double >,
                                 gravitationalParameterOfBodyExertingAcceleration,
                                 gravitationalParameterOfBodyUndergoingAcceleration );
        }

        // Create acceleration object.
        accelerationModelPointer =
                boost::make_shared< CentralGravitationalAccelerationModel3d >(
                    boost::bind( &Body::getPosition, bodyUndergoingAcceleration ),
                    gravitationalParameterFunction,
                    boost::bind( &Body::getPosition, bodyExertingAcceleration ),
                    useCentralBodyFixedFrame );
    }


    return accelerationModelPointer;
}

//! Function to create spherical harmonic gravity acceleration model.
boost::shared_ptr< gravitation::SphericalHarmonicsGravitationalAccelerationModel >
createSphericalHarmonicsGravityAcceleration(
        const boost::shared_ptr< Body > bodyUndergoingAcceleration,
        const boost::shared_ptr< Body > bodyExertingAcceleration,
        const std::string& nameOfBodyUndergoingAcceleration,
        const std::string& nameOfBodyExertingAcceleration,
        const boost::shared_ptr< AccelerationSettings > accelerationSettings,
        const bool useCentralBodyFixedFrame )
{
    // Declare pointer to return object
    boost::shared_ptr< SphericalHarmonicsGravitationalAccelerationModel > accelerationModel;

    // Dynamic cast acceleration settings to required type and check consistency.
    boost::shared_ptr< SphericalHarmonicAccelerationSettings > sphericalHarmonicsSettings =
            boost::dynamic_pointer_cast< SphericalHarmonicAccelerationSettings >(
                accelerationSettings );
    if( sphericalHarmonicsSettings == NULL )
    {
        throw std::runtime_error(
                    std::string( "Error, acceleration settings inconsistent ") +
                    " making sh gravitational acceleration of " + nameOfBodyExertingAcceleration +
                    " on " + nameOfBodyUndergoingAcceleration );
    }
    else
    {
        // Get pointer to gravity field of central body and cast to required type.
        boost::shared_ptr< SphericalHarmonicsGravityField > sphericalHarmonicsGravityField =
                boost::dynamic_pointer_cast< SphericalHarmonicsGravityField >(
                    bodyExertingAcceleration->getGravityFieldModel( ) );

        boost::shared_ptr< RotationalEphemeris> rotationalEphemeris =
                bodyExertingAcceleration->getRotationalEphemeris( );
        if( sphericalHarmonicsGravityField == NULL )
        {
            throw std::runtime_error(
                        std::string( "Error, spherical harmonic gravity field model not set when ")
                        + " making sh gravitational acceleration of " +
                        nameOfBodyExertingAcceleration +
                        " on " + nameOfBodyUndergoingAcceleration );
        }
        else
        {
            if( rotationalEphemeris == NULL )
            {
                throw std::runtime_error( "Warning when making spherical harmonic acceleration on body " +
                                          nameOfBodyUndergoingAcceleration + ", no rotation model found for " +
                                          nameOfBodyExertingAcceleration );
            }

            if( rotationalEphemeris->getTargetFrameOrientation( ) !=
                    sphericalHarmonicsGravityField->getFixedReferenceFrame( ) )
            {
                throw std::runtime_error( "Warning when making spherical harmonic acceleration on body " +
                                          nameOfBodyUndergoingAcceleration + ", rotation model found for " +
                                          nameOfBodyExertingAcceleration + " is incompatible, frames are: " +
                                          rotationalEphemeris->getTargetFrameOrientation( ) + " and " +
                                          sphericalHarmonicsGravityField->getFixedReferenceFrame( ) );
            }

            boost::function< double( ) > gravitationalParameterFunction;

            // Check if mutual acceleration is to be used.
            if( useCentralBodyFixedFrame == false ||
                    bodyUndergoingAcceleration->getGravityFieldModel( ) == NULL )
            {
                gravitationalParameterFunction =
                        boost::bind( &SphericalHarmonicsGravityField::getGravitationalParameter,
                                     sphericalHarmonicsGravityField );
            }
            else
            {
                // Create function returning summed gravitational parameter of the two bodies.
                boost::function< double( ) > gravitationalParameterOfBodyExertingAcceleration =
                        boost::bind( &gravitation::GravityFieldModel::getGravitationalParameter,
                                     sphericalHarmonicsGravityField );
                boost::function< double( ) > gravitationalParameterOfBodyUndergoingAcceleration =
                        boost::bind( &gravitation::GravityFieldModel::getGravitationalParameter,
                                     bodyUndergoingAcceleration->getGravityFieldModel( ) );
                gravitationalParameterFunction =
                        boost::bind( &utilities::sumFunctionReturn< double >,
                                     gravitationalParameterOfBodyExertingAcceleration,
                                     gravitationalParameterOfBodyUndergoingAcceleration );
            }

            // Create acceleration object.
            accelerationModel =
                    boost::make_shared< SphericalHarmonicsGravitationalAccelerationModel >
                    ( boost::bind( &Body::getPosition, bodyUndergoingAcceleration ),
                      gravitationalParameterFunction,
                      sphericalHarmonicsGravityField->getReferenceRadius( ),
                      boost::bind( &SphericalHarmonicsGravityField::getCosineCoefficients,
                                   sphericalHarmonicsGravityField,
                                   sphericalHarmonicsSettings->maximumDegree_,
                                   sphericalHarmonicsSettings->maximumOrder_ ),
                      boost::bind( &SphericalHarmonicsGravityField::getSineCoefficients,
                                   sphericalHarmonicsGravityField,
                                   sphericalHarmonicsSettings->maximumDegree_,
                                   sphericalHarmonicsSettings->maximumOrder_ ),
                      boost::bind( &Body::getPosition, bodyExertingAcceleration ),
                      boost::bind( &Body::getCurrentRotationToGlobalFrame,
                                   bodyExertingAcceleration ), useCentralBodyFixedFrame );
        }
    }
    return accelerationModel;
}

//! Function to create mutual spherical harmonic gravity acceleration model.
boost::shared_ptr< gravitation::MutualSphericalHarmonicsGravitationalAccelerationModel >
createMutualSphericalHarmonicsGravityAcceleration(
        const boost::shared_ptr< Body > bodyUndergoingAcceleration,
        const boost::shared_ptr< Body > bodyExertingAcceleration,
        const std::string& nameOfBodyUndergoingAcceleration,
        const std::string& nameOfBodyExertingAcceleration,
        const boost::shared_ptr< AccelerationSettings > accelerationSettings,
        const bool useCentralBodyFixedFrame,
        const bool acceleratedBodyIsCentralBody )
{
    using namespace basic_astrodynamics;

    // Declare pointer to return object
    boost::shared_ptr< MutualSphericalHarmonicsGravitationalAccelerationModel > accelerationModel;

    // Dynamic cast acceleration settings to required type and check consistency.
    boost::shared_ptr< MutualSphericalHarmonicAccelerationSettings > mutualSphericalHarmonicsSettings =
            boost::dynamic_pointer_cast< MutualSphericalHarmonicAccelerationSettings >( accelerationSettings );
    if( mutualSphericalHarmonicsSettings == NULL )
    {
        std::string errorMessage = "Error, expected mutual spherical harmonics acceleration settings when making acceleration model on " +
                nameOfBodyUndergoingAcceleration + "due to " + nameOfBodyExertingAcceleration;
        throw std::runtime_error( errorMessage );
    }
    else
    {
        // Get pointer to gravity field of central body and cast to required type.
        boost::shared_ptr< SphericalHarmonicsGravityField > sphericalHarmonicsGravityFieldOfBodyExertingAcceleration =
                boost::dynamic_pointer_cast< SphericalHarmonicsGravityField >(
                    bodyExertingAcceleration->getGravityFieldModel( ) );
        boost::shared_ptr< SphericalHarmonicsGravityField > sphericalHarmonicsGravityFieldOfBodyUndergoingAcceleration =
                boost::dynamic_pointer_cast< SphericalHarmonicsGravityField >(
                    bodyUndergoingAcceleration->getGravityFieldModel( ) );

        if( sphericalHarmonicsGravityFieldOfBodyExertingAcceleration == NULL )
        {

            std::string errorMessage = "Error " + nameOfBodyExertingAcceleration + " does not have a spherical harmonics gravity field " +
                    "when making mutual spherical harmonics gravity acceleration on " +
                    nameOfBodyUndergoingAcceleration;
            throw std::runtime_error( errorMessage );

        }
        else if( sphericalHarmonicsGravityFieldOfBodyUndergoingAcceleration == NULL )
        {

            std::string errorMessage = "Error " + nameOfBodyUndergoingAcceleration + " does not have a spherical harmonics gravity field " +
                    "when making mutual spherical harmonics gravity acceleration on " +
                    nameOfBodyUndergoingAcceleration;
            throw std::runtime_error( errorMessage );
        }
        else
        {
            boost::function< double( ) > gravitationalParameterFunction;

            // Create function returning summed gravitational parameter of the two bodies.
            if( useCentralBodyFixedFrame == false )
            {
                gravitationalParameterFunction =
                        boost::bind( &SphericalHarmonicsGravityField::getGravitationalParameter,
                                     sphericalHarmonicsGravityFieldOfBodyExertingAcceleration );
            }
            else
            {
                // Create function returning summed gravitational parameter of the two bodies.
                boost::function< double( ) > gravitationalParameterOfBodyExertingAcceleration =
                        boost::bind( &gravitation::GravityFieldModel::getGravitationalParameter,
                                     sphericalHarmonicsGravityFieldOfBodyExertingAcceleration );
                boost::function< double( ) > gravitationalParameterOfBodyUndergoingAcceleration =
                        boost::bind( &gravitation::GravityFieldModel::getGravitationalParameter,
                                     sphericalHarmonicsGravityFieldOfBodyUndergoingAcceleration );
                gravitationalParameterFunction =
                        boost::bind( &utilities::sumFunctionReturn< double >,
                                     gravitationalParameterOfBodyExertingAcceleration,
                                     gravitationalParameterOfBodyUndergoingAcceleration );
            }

            // Create acceleration object.

            int maximumDegreeOfUndergoingBody, maximumOrderOfUndergoingBody;
            if( !acceleratedBodyIsCentralBody )
            {
                maximumDegreeOfUndergoingBody = mutualSphericalHarmonicsSettings->maximumDegreeOfBodyUndergoingAcceleration_;
                maximumOrderOfUndergoingBody = mutualSphericalHarmonicsSettings->maximumOrderOfBodyUndergoingAcceleration_;
            }
            else
            {
                maximumDegreeOfUndergoingBody = mutualSphericalHarmonicsSettings->maximumDegreeOfCentralBody_;
                maximumOrderOfUndergoingBody = mutualSphericalHarmonicsSettings->maximumOrderOfCentralBody_;
            }

            accelerationModel = boost::make_shared< MutualSphericalHarmonicsGravitationalAccelerationModel >(
                        boost::bind( &Body::getPosition, bodyUndergoingAcceleration ),
                        boost::bind( &Body::getPosition, bodyExertingAcceleration ),
                        gravitationalParameterFunction,
                        sphericalHarmonicsGravityFieldOfBodyExertingAcceleration->getReferenceRadius( ),
                        sphericalHarmonicsGravityFieldOfBodyUndergoingAcceleration->getReferenceRadius( ),
                        boost::bind( &SphericalHarmonicsGravityField::getCosineCoefficients,
                                     sphericalHarmonicsGravityFieldOfBodyExertingAcceleration,
                                     mutualSphericalHarmonicsSettings->maximumDegreeOfBodyExertingAcceleration_,
                                     mutualSphericalHarmonicsSettings->maximumOrderOfBodyExertingAcceleration_ ),
                        boost::bind( &SphericalHarmonicsGravityField::getSineCoefficients,
                                     sphericalHarmonicsGravityFieldOfBodyExertingAcceleration,
                                     mutualSphericalHarmonicsSettings->maximumDegreeOfBodyExertingAcceleration_,
                                     mutualSphericalHarmonicsSettings->maximumOrderOfBodyExertingAcceleration_ ),
                        boost::bind( &SphericalHarmonicsGravityField::getCosineCoefficients,
                                     sphericalHarmonicsGravityFieldOfBodyUndergoingAcceleration,
                                     maximumDegreeOfUndergoingBody,
                                     maximumOrderOfUndergoingBody ),
                        boost::bind( &SphericalHarmonicsGravityField::getSineCoefficients,
                                     sphericalHarmonicsGravityFieldOfBodyUndergoingAcceleration,
                                     maximumDegreeOfUndergoingBody,
                                     maximumOrderOfUndergoingBody ),
                        boost::bind( &Body::getCurrentRotationToGlobalFrame,
                                     bodyExertingAcceleration ),
                        boost::bind( &Body::getCurrentRotationToGlobalFrame,
                                     bodyUndergoingAcceleration ),
                        useCentralBodyFixedFrame );
        }
    }
    return accelerationModel;
}


//! Function to create a third body central gravity acceleration model.
boost::shared_ptr< gravitation::ThirdBodyCentralGravityAcceleration >
createThirdBodyCentralGravityAccelerationModel(
        const boost::shared_ptr< Body > bodyUndergoingAcceleration,
        const boost::shared_ptr< Body > bodyExertingAcceleration,
        const boost::shared_ptr< Body > centralBody,
        const std::string& nameOfBodyUndergoingAcceleration,
        const std::string& nameOfBodyExertingAcceleration,
        const std::string& nameOfCentralBody )
{
    // Declare pointer to return object.
    boost::shared_ptr< ThirdBodyCentralGravityAcceleration > accelerationModelPointer;

    // Create acceleration object.
    accelerationModelPointer =  boost::make_shared< ThirdBodyCentralGravityAcceleration >(
                boost::dynamic_pointer_cast< CentralGravitationalAccelerationModel3d >(
                    createCentralGravityAcceleratioModel( bodyUndergoingAcceleration,
                                                          bodyExertingAcceleration,
                                                          nameOfBodyUndergoingAcceleration,
                                                          nameOfBodyExertingAcceleration, 0 ) ),
                boost::dynamic_pointer_cast< CentralGravitationalAccelerationModel3d >(
                    createCentralGravityAcceleratioModel( centralBody, bodyExertingAcceleration,
                                                          nameOfCentralBody,
                                                          nameOfBodyExertingAcceleration, 0 ) ), nameOfCentralBody );

    return accelerationModelPointer;
}

//! Function to create a third body spheric harmonic gravity acceleration model.
boost::shared_ptr< gravitation::ThirdBodySphericalHarmonicsGravitationalAccelerationModel >
createThirdBodySphericalHarmonicGravityAccelerationModel(
        const boost::shared_ptr< Body > bodyUndergoingAcceleration,
        const boost::shared_ptr< Body > bodyExertingAcceleration,
        const boost::shared_ptr< Body > centralBody,
        const std::string& nameOfBodyUndergoingAcceleration,
        const std::string& nameOfBodyExertingAcceleration,
        const std::string& nameOfCentralBody,
        const boost::shared_ptr< AccelerationSettings > accelerationSettings )
{
    using namespace basic_astrodynamics;

    // Declare pointer to return object
    boost::shared_ptr< ThirdBodySphericalHarmonicsGravitationalAccelerationModel > accelerationModel;

    // Dynamic cast acceleration settings to required type and check consistency.
    boost::shared_ptr< SphericalHarmonicAccelerationSettings > sphericalHarmonicsSettings =
            boost::dynamic_pointer_cast< SphericalHarmonicAccelerationSettings >( accelerationSettings );
    if( sphericalHarmonicsSettings == NULL )
    {
        std::string errorMessage = "Error, expected spherical harmonics acceleration settings when making acceleration model on " +
                nameOfBodyUndergoingAcceleration + " due to " + nameOfBodyExertingAcceleration;
        throw std::runtime_error( errorMessage );
    }
    else
    {
        // Get pointer to gravity field of central body and cast to required type.
        boost::shared_ptr< SphericalHarmonicsGravityField > sphericalHarmonicsGravityField =
                boost::dynamic_pointer_cast< SphericalHarmonicsGravityField >(
                    bodyExertingAcceleration->getGravityFieldModel( ) );
        if( sphericalHarmonicsGravityField == NULL )
        {
            std::string errorMessage = "Error " + nameOfBodyExertingAcceleration + " does not have a spherical harmonics gravity field " +
                    "when making third body spherical harmonics gravity acceleration on " +
                    nameOfBodyUndergoingAcceleration;
            throw std::runtime_error( errorMessage );
        }
        else
        {

            accelerationModel =  boost::make_shared< ThirdBodySphericalHarmonicsGravitationalAccelerationModel >(
                        boost::dynamic_pointer_cast< SphericalHarmonicsGravitationalAccelerationModel >(
                            createSphericalHarmonicsGravityAcceleration(
                                bodyUndergoingAcceleration, bodyExertingAcceleration, nameOfBodyUndergoingAcceleration,
                                nameOfBodyExertingAcceleration, sphericalHarmonicsSettings, 0 ) ),
                        boost::dynamic_pointer_cast< SphericalHarmonicsGravitationalAccelerationModel >(
                            createSphericalHarmonicsGravityAcceleration(
                                centralBody, bodyExertingAcceleration, nameOfCentralBody,
                                nameOfBodyExertingAcceleration, sphericalHarmonicsSettings, 0 ) ), nameOfCentralBody );
        }
    }
    return accelerationModel;
}

//! Function to create a third body mutual spheric harmonic gravity acceleration model.
boost::shared_ptr< gravitation::ThirdBodyMutualSphericalHarmonicsGravitationalAccelerationModel >
createThirdBodyMutualSphericalHarmonicGravityAccelerationModel(
        const boost::shared_ptr< Body > bodyUndergoingAcceleration,
        const boost::shared_ptr< Body > bodyExertingAcceleration,
        const boost::shared_ptr< Body > centralBody,
        const std::string& nameOfBodyUndergoingAcceleration,
        const std::string& nameOfBodyExertingAcceleration,
        const std::string& nameOfCentralBody,
        const boost::shared_ptr< AccelerationSettings > accelerationSettings )
{
    // Declare pointer to return object
    boost::shared_ptr< ThirdBodyMutualSphericalHarmonicsGravitationalAccelerationModel > accelerationModel;

    // Dynamic cast acceleration settings to required type and check consistency.
    boost::shared_ptr< MutualSphericalHarmonicAccelerationSettings > mutualSphericalHarmonicsSettings =
            boost::dynamic_pointer_cast< MutualSphericalHarmonicAccelerationSettings >( accelerationSettings );
    if( mutualSphericalHarmonicsSettings == NULL )
    {

        std::string errorMessage = "Error, expected mutual spherical harmonics acceleration settings when making acceleration model on " +
                nameOfBodyUndergoingAcceleration +
                " due to " + nameOfBodyExertingAcceleration;
        throw std::runtime_error( errorMessage );
    }
    else
    {
        // Get pointer to gravity field of central body and cast to required type.
        boost::shared_ptr< SphericalHarmonicsGravityField > sphericalHarmonicsGravityFieldOfBodyExertingAcceleration =
                boost::dynamic_pointer_cast< SphericalHarmonicsGravityField >(
                    bodyExertingAcceleration->getGravityFieldModel( ) );
        boost::shared_ptr< SphericalHarmonicsGravityField > sphericalHarmonicsGravityFieldOfBodyUndergoingAcceleration =
                boost::dynamic_pointer_cast< SphericalHarmonicsGravityField >(
                    bodyUndergoingAcceleration->getGravityFieldModel( ) );
        boost::shared_ptr< SphericalHarmonicsGravityField > sphericalHarmonicsGravityFieldOfCentralBody =
                boost::dynamic_pointer_cast< SphericalHarmonicsGravityField >(
                    centralBody->getGravityFieldModel( ) );

        if( sphericalHarmonicsGravityFieldOfBodyExertingAcceleration == NULL )
        {
            std::string errorMessage = "Error " + nameOfBodyExertingAcceleration + " does not have a spherical harmonics gravity field " +
                    "when making mutual spherical harmonics gravity acceleration on " +
                    nameOfBodyUndergoingAcceleration;
            throw std::runtime_error( errorMessage );
        }
        else if( sphericalHarmonicsGravityFieldOfBodyUndergoingAcceleration == NULL )
        {
            std::string errorMessage = "Error " + nameOfBodyUndergoingAcceleration + " does not have a spherical harmonics gravity field " +
                    "when making mutual spherical harmonics gravity acceleration on " +
                    nameOfBodyUndergoingAcceleration;
            throw std::runtime_error( errorMessage );
        }
        else if( sphericalHarmonicsGravityFieldOfCentralBody == NULL )
        {
            std::string errorMessage = "Error " + nameOfCentralBody + " does not have a spherical harmonics gravity field " +
                    "when making mutual spherical harmonics gravity acceleration on " +
                    nameOfBodyUndergoingAcceleration;
            throw std::runtime_error( errorMessage );
        }
        else
        {
            boost::shared_ptr< MutualSphericalHarmonicAccelerationSettings > accelerationSettingsForCentralBodyAcceleration =
                    boost::make_shared< MutualSphericalHarmonicAccelerationSettings >(
                        mutualSphericalHarmonicsSettings->maximumDegreeOfBodyExertingAcceleration_,
                        mutualSphericalHarmonicsSettings->maximumOrderOfBodyExertingAcceleration_,
                        mutualSphericalHarmonicsSettings->maximumDegreeOfCentralBody_,
                        mutualSphericalHarmonicsSettings->maximumOrderOfCentralBody_ );
            accelerationModel =  boost::make_shared< ThirdBodyMutualSphericalHarmonicsGravitationalAccelerationModel >(
                        boost::dynamic_pointer_cast< MutualSphericalHarmonicsGravitationalAccelerationModel >(
                            createMutualSphericalHarmonicsGravityAcceleration(
                                bodyUndergoingAcceleration, bodyExertingAcceleration, nameOfBodyUndergoingAcceleration,
                                nameOfBodyExertingAcceleration, mutualSphericalHarmonicsSettings, 0, 0 ) ),
                        boost::dynamic_pointer_cast< MutualSphericalHarmonicsGravitationalAccelerationModel >(
                            createMutualSphericalHarmonicsGravityAcceleration(
                                centralBody, bodyExertingAcceleration, nameOfCentralBody,
                                nameOfBodyExertingAcceleration, accelerationSettingsForCentralBodyAcceleration, 0, 1 ) ),
                        nameOfCentralBody );
        }
    }
    return accelerationModel;
}

//! Function to create an aerodynamic acceleration model.
boost::shared_ptr< aerodynamics::AerodynamicAcceleration > createAerodynamicAcceleratioModel(
        const boost::shared_ptr< Body > bodyUndergoingAcceleration,
        const boost::shared_ptr< Body > bodyExertingAcceleration,
        const std::string& nameOfBodyUndergoingAcceleration,
        const std::string& nameOfBodyExertingAcceleration )
{
    // Check existence of required environment models
    if( bodyUndergoingAcceleration->getAerodynamicCoefficientInterface( ) == NULL )
    {
        throw std::runtime_error( "Error when making aerodynamic acceleration, body " +
                                  nameOfBodyUndergoingAcceleration +
                                  "has no aerodynamic coefficients." );
    }

    if( bodyExertingAcceleration->getAtmosphereModel( ) == NULL )
    {
        throw std::runtime_error(  "Error when making aerodynamic acceleration, central body " +
                                   nameOfBodyExertingAcceleration + " has no atmosphere model.");
    }

    if( bodyExertingAcceleration->getShapeModel( ) == NULL )
    {
        throw std::runtime_error( "Error when making aerodynamic acceleration, central body " +
                                  nameOfBodyExertingAcceleration + " has no shape model." );
    }

    // Retrieve flight conditions; create object if not yet extant.
    boost::shared_ptr< FlightConditions > bodyFlightConditions =
            bodyUndergoingAcceleration->getFlightConditions( );

    if( bodyFlightConditions == NULL )
    {
        bodyUndergoingAcceleration->setFlightConditions(
                    createFlightConditions( bodyUndergoingAcceleration,
                                            bodyExertingAcceleration,
                                            nameOfBodyUndergoingAcceleration,
                                            nameOfBodyExertingAcceleration ) );
        bodyFlightConditions = bodyUndergoingAcceleration->getFlightConditions( );
    }

    // Retrieve frame in which aerodynamic coefficients are defined.
    boost::shared_ptr< aerodynamics::AerodynamicCoefficientInterface > aerodynamicCoefficients =
            bodyUndergoingAcceleration->getAerodynamicCoefficientInterface( );
    reference_frames::AerodynamicsReferenceFrames accelerationFrame;
    if( aerodynamicCoefficients->getAreCoefficientsInAerodynamicFrame( ) )
    {
        accelerationFrame = reference_frames::aerodynamic_frame;
    }
    else
    {
        accelerationFrame = reference_frames::body_frame;
    }

    // Create function to transform from frame of aerodynamic coefficienrs to that of propagation.
    boost::function< Eigen::Vector3d( const Eigen::Vector3d& ) > toPropagationFrameTransformation;
    toPropagationFrameTransformation =
            reference_frames::getAerodynamicForceTransformationFunction(
                bodyFlightConditions->getAerodynamicAngleCalculator( ),
                accelerationFrame,
                boost::bind( &Body::getCurrentRotationToGlobalFrame, bodyExertingAcceleration ),
                reference_frames::inertial_frame );


    boost::function< Eigen::Vector3d( ) > coefficientFunction =
            boost::bind( &AerodynamicCoefficientInterface::getCurrentForceCoefficients,
                         aerodynamicCoefficients );
    boost::function< Eigen::Vector3d( ) > coefficientInPropagationFrameFunction =
            boost::bind( &reference_frames::transformVectorFunctionFromVectorFunctions,
                         coefficientFunction, toPropagationFrameTransformation );

    // Create acceleration model.
    return boost::make_shared< AerodynamicAcceleration >(
                coefficientInPropagationFrameFunction,
                boost::bind( &FlightConditions::getCurrentDensity, bodyFlightConditions ),
                boost::bind( &FlightConditions::getCurrentAirspeed, bodyFlightConditions ),
                boost::bind( &Body::getBodyMass, bodyUndergoingAcceleration ),
                boost::bind( &AerodynamicCoefficientInterface::getReferenceArea,
                             aerodynamicCoefficients ),
                aerodynamicCoefficients->getAreCoefficientsInNegativeAxisDirection( ) );
}

//! Function to create a cannonball radiation pressure acceleration model.
boost::shared_ptr< CannonBallRadiationPressureAcceleration >
createCannonballRadiationPressureAcceleratioModel(
        const boost::shared_ptr< Body > bodyUndergoingAcceleration,
        const boost::shared_ptr< Body > bodyExertingAcceleration,
        const std::string& nameOfBodyUndergoingAcceleration,
        const std::string& nameOfBodyExertingAcceleration )
{
    // Retrieve radiation pressure interface
    if( bodyUndergoingAcceleration->getRadiationPressureInterfaces( ).count(
                nameOfBodyExertingAcceleration ) == 0 )
    {
        throw std::runtime_error(
                    "Error when making radiation pressure, no radiation pressure interface found  in " +
                    nameOfBodyUndergoingAcceleration +
                    " for body " + nameOfBodyExertingAcceleration );
    }
    boost::shared_ptr< RadiationPressureInterface > radiationPressureInterface =
            bodyUndergoingAcceleration->getRadiationPressureInterfaces( ).at(
                nameOfBodyExertingAcceleration );

    // Create acceleration model.
    return boost::make_shared< CannonBallRadiationPressureAcceleration >(
                boost::bind( &Body::getPosition, bodyExertingAcceleration ),
                boost::bind( &Body::getPosition, bodyUndergoingAcceleration ),
                boost::bind( &RadiationPressureInterface::getCurrentRadiationPressure, radiationPressureInterface ),
                boost::bind( &RadiationPressureInterface::getRadiationPressureCoefficient, radiationPressureInterface ),
                boost::bind( &RadiationPressureInterface::getArea, radiationPressureInterface ),
                boost::bind( &Body::getBodyMass, bodyUndergoingAcceleration ) );

}

//! Function to create a thrust acceleration model.
boost::shared_ptr< propulsion::ThrustAcceleration >
createThrustAcceleratioModel(
        const boost::shared_ptr< AccelerationSettings > accelerationSettings,
        const NamedBodyMap& bodyMap,
        const std::string& nameOfBodyUndergoingThrust )
{
    // Check input consistency
    boost::shared_ptr< ThrustAccelerationSettings > thrustAccelerationSettings =
            boost::dynamic_pointer_cast< ThrustAccelerationSettings >( accelerationSettings );
    if( thrustAccelerationSettings == NULL )
    {
        throw std::runtime_error( "Error when creating thrust acceleration, input is inconsistent" );
    }

    std::map< propagators::EnvironmentModelsToUpdate, std::vector< std::string > > magnitudeUpdateSettings;
    std::map< propagators::EnvironmentModelsToUpdate, std::vector< std::string > > directionUpdateSettings;



    // Check if user-supplied interpolator for full thrust ius present.
    if( thrustAccelerationSettings->interpolatorInterface_ != NULL )
    {
        // Check input consisten
        if( thrustAccelerationSettings->thrustFrame_ == unspecified_thurst_frame )
        {
            throw std::runtime_error( "Error when creating thrust acceleration, input frame is inconsistent with interface" );
        }
        else if( thrustAccelerationSettings->thrustFrame_ != inertial_thurst_frame )
        {
            // Create rotation function from velocity-based LVLH thrust-frame to propagation frame.
            if( thrustAccelerationSettings->thrustFrame_ == lvlh_thrust_frame )
            {
                boost::function< basic_mathematics::Vector6d( ) > vehicleStateFunction =
                        boost::bind( &Body::getState, bodyMap.at( nameOfBodyUndergoingThrust ) );
                boost::function< basic_mathematics::Vector6d( ) > centralBodyStateFunction;

                if( ephemerides::isFrameInertial( thrustAccelerationSettings->centralBody_ ) )
                {
                    centralBodyStateFunction =  boost::lambda::constant( basic_mathematics::Vector6d::Zero( ) );
                }
                else
                {
                    if( bodyMap.count( thrustAccelerationSettings->centralBody_ ) == 0 )
                    {
                        throw std::runtime_error( "Error when creating thrust acceleration, input central body not found" );
                    }
                    centralBodyStateFunction =
                            boost::bind( &Body::getState, bodyMap.at( thrustAccelerationSettings->centralBody_ ) );
                }
                thrustAccelerationSettings->interpolatorInterface_->resetRotationFunction(
                            boost::bind( &reference_frames::getVelocityBasedLvlhToInertialRotationFromFunctions,
                                         vehicleStateFunction, centralBodyStateFunction, thrustAccelerationSettings->doesNaxisPointAwayFromCentralBody_ ) );
            }
            // Create rotation function from RTN thrust-frame to propagation frame.
            else if( thrustAccelerationSettings->thrustFrame_ == rtn_thrust_frame )
            {
                boost::function< basic_mathematics::Vector6d( ) > vehicleStateFunction =
                        boost::bind( &Body::getState, bodyMap.at( nameOfBodyUndergoingThrust ) );
                boost::function< basic_mathematics::Vector6d( ) > centralBodyStateFunction;

                if( ephemerides::isFrameInertial( thrustAccelerationSettings->centralBody_ ) )
                {
                    centralBodyStateFunction =  boost::lambda::constant( basic_mathematics::Vector6d::Zero( ) );
                }
                else
                {
                    if( bodyMap.count( thrustAccelerationSettings->centralBody_ ) == 0 )
                    {
                        throw std::runtime_error( "Error when creating thrust acceleration, input central body not found" );
                    }
                    centralBodyStateFunction =
                            boost::bind( &Body::getState, bodyMap.at( thrustAccelerationSettings->centralBody_ ) );
                }
                thrustAccelerationSettings->interpolatorInterface_->resetRotationFunction(
                            boost::bind( &reference_frames::getRtnToInertialRotationFromFunctions,
                                         vehicleStateFunction, centralBodyStateFunction ) );
            }
            else
            {
                throw std::runtime_error( "Error when creating thrust acceleration, input frame not recognized" );
            }
        }
    }

    // Create thrust direction model.
    boost::shared_ptr< propulsion::BodyFixedForceDirectionGuidance  > thrustDirectionGuidance = createThrustGuidanceModel(
                thrustAccelerationSettings->thrustDirectionGuidanceSettings_, bodyMap, nameOfBodyUndergoingThrust,
                getBodyFixedThrustDirection( thrustAccelerationSettings->thrustMagnitudeSettings_, bodyMap,
                                             nameOfBodyUndergoingThrust ), magnitudeUpdateSettings );

    // Create thrust magnitude model
    boost::shared_ptr< propulsion::ThrustMagnitudeWrapper > thrustMagnitude = createThrustMagnitudeWrapper(
                thrustAccelerationSettings->thrustMagnitudeSettings_, bodyMap, nameOfBodyUndergoingThrust,
                directionUpdateSettings );

    // Add required updates of environemt models.
    std::map< propagators::EnvironmentModelsToUpdate, std::vector< std::string > > totalUpdateSettings;
    propagators::addEnvironmentUpdates( totalUpdateSettings, magnitudeUpdateSettings );
    propagators::addEnvironmentUpdates( totalUpdateSettings, directionUpdateSettings );

    // Set DependentOrientationCalculator for body if required.
    if( !( thrustAccelerationSettings->thrustDirectionGuidanceSettings_->thrustDirectionType_ ==
            thrust_direction_from_existing_body_orientation ) )
    {
        bodyMap.at( nameOfBodyUndergoingThrust )->setDependentOrientationCalculator( thrustDirectionGuidance );
    }

    // Create and return thrust acceleration object.
    boost::function< void( const double ) > updateFunction =
            boost::bind( &updateThrustMagnitudeAndDirection, thrustMagnitude, thrustDirectionGuidance, _1 );
    boost::function< void( const double ) > timeResetFunction =
            boost::bind( &resetThrustMagnitudeAndDirectionTime, thrustMagnitude, thrustDirectionGuidance, _1 );
    return boost::make_shared< propulsion::ThrustAcceleration >(
                boost::bind( &propulsion::ThrustMagnitudeWrapper::getCurrentThrustMagnitude, thrustMagnitude ),
                boost::bind( &propulsion::BodyFixedForceDirectionGuidance ::getCurrentForceDirectionInPropagationFrame, thrustDirectionGuidance ),
                boost::bind( &Body::getBodyMass, bodyMap.at( nameOfBodyUndergoingThrust ) ),
                boost::bind( &propulsion::ThrustMagnitudeWrapper::getCurrentMassRate, thrustMagnitude ),
                thrustAccelerationSettings->thrustMagnitudeSettings_->thrustOriginId_,
                updateFunction, timeResetFunction, totalUpdateSettings );
}


//! Function to create acceleration model object.
boost::shared_ptr< AccelerationModel< Eigen::Vector3d > > createAccelerationModel(
        const boost::shared_ptr< Body > bodyUndergoingAcceleration,
        const boost::shared_ptr< Body > bodyExertingAcceleration,
        const boost::shared_ptr< AccelerationSettings > accelerationSettings,
        const std::string& nameOfBodyUndergoingAcceleration,
        const std::string& nameOfBodyExertingAcceleration,
        const boost::shared_ptr< Body > centralBody,
        const std::string& nameOfCentralBody,
        const NamedBodyMap& bodyMap )
{
    // Declare pointer to return object.
    boost::shared_ptr< AccelerationModel< Eigen::Vector3d > > accelerationModelPointer;

    // Switch to call correct acceleration model type factory function.
    switch( accelerationSettings->accelerationType_ )
    {
    case central_gravity:
        accelerationModelPointer = createGravitationalAccelerationModel(
                    bodyUndergoingAcceleration, bodyExertingAcceleration, accelerationSettings,
                    nameOfBodyUndergoingAcceleration, nameOfBodyExertingAcceleration,
                    centralBody, nameOfCentralBody );
        break;
    case spherical_harmonic_gravity:
        accelerationModelPointer = createGravitationalAccelerationModel(
                    bodyUndergoingAcceleration, bodyExertingAcceleration, accelerationSettings,
                    nameOfBodyUndergoingAcceleration, nameOfBodyExertingAcceleration,
                    centralBody, nameOfCentralBody );
        break;
    case mutual_spherical_harmonic_gravity:
        accelerationModelPointer = createGravitationalAccelerationModel(
                    bodyUndergoingAcceleration, bodyExertingAcceleration, accelerationSettings,
                    nameOfBodyUndergoingAcceleration, nameOfBodyExertingAcceleration,
                    centralBody, nameOfCentralBody );
        break;
    case aerodynamic:
        accelerationModelPointer = createAerodynamicAcceleratioModel(
                    bodyUndergoingAcceleration,
                    bodyExertingAcceleration,
                    nameOfBodyUndergoingAcceleration,
                    nameOfBodyExertingAcceleration );
        break;
    case cannon_ball_radiation_pressure:
        accelerationModelPointer = createCannonballRadiationPressureAcceleratioModel(
                    bodyUndergoingAcceleration,
                    bodyExertingAcceleration,
                    nameOfBodyUndergoingAcceleration,
                    nameOfBodyExertingAcceleration );
        break;
    case thrust_acceleration:
        accelerationModelPointer = createThrustAcceleratioModel(
                    accelerationSettings, bodyMap,
                    nameOfBodyUndergoingAcceleration );
        break;
    default:
        throw std::runtime_error(
                    std::string( "Error, acceleration model ") +
                    boost::lexical_cast< std::string >( accelerationSettings->accelerationType_ ) +
                    " not recognized when making acceleration model of" +
                    nameOfBodyExertingAcceleration + " on " +
                    nameOfBodyUndergoingAcceleration );
        break;
    }
    return accelerationModelPointer;
}

//! Function to put SelectedAccelerationMap in correct order, to ensure correct model creation
SelectedAccelerationMap orderSelectedAccelerationMap( const SelectedAccelerationMap& selectedAccelerationsPerBody )
{
    // Declare map of acceleration models acting on current body.
    SelectedAccelerationMap orderedAccelerationsPerBody;

    // Iterate over all bodies which are undergoing acceleration
    for( SelectedAccelerationMap::const_iterator bodyIterator =
         selectedAccelerationsPerBody.begin( ); bodyIterator != selectedAccelerationsPerBody.end( );
         bodyIterator++ )
    {
        // Retrieve name of body undergoing acceleration.
        std::string bodyUndergoingAcceleration = bodyIterator->first;

        // Retrieve list of required acceleration model types and bodies exerting accelerationd on
        // current body.
        std::map< std::string, std::vector< boost::shared_ptr< AccelerationSettings > > >
                accelerationsForBody = bodyIterator->second;

        // Iterate over all bodies exerting an acceleration
        for( std::map< std::string, std::vector< boost::shared_ptr< AccelerationSettings > > >::
             iterator body2Iterator = accelerationsForBody.begin( );
             body2Iterator != accelerationsForBody.end( ); body2Iterator++ )
        {
            // Retrieve name of body exerting acceleration.
            std::string bodyExertingAcceleration = body2Iterator->first;

            // Retrieve list of accelerations due to current body.
            std::vector< boost::shared_ptr< AccelerationSettings > > accelerationList =
                    body2Iterator->second;

            // Retrieve indices of all acceleration anf thrust models.
            std::vector< int > aerodynamicAccelerationIndex;
            std::vector< int > thrustAccelerationIndices;
            for( unsigned int i = 0; i < accelerationList.size( ); i++ )
            {
                if( accelerationList.at( i )->accelerationType_ == basic_astrodynamics::thrust_acceleration )
                {
                    thrustAccelerationIndices.push_back( i );
                }
                else if( accelerationList.at( i )->accelerationType_ == basic_astrodynamics::aerodynamic )
                {
                    aerodynamicAccelerationIndex.push_back( i );
                }
            }

            std::vector< boost::shared_ptr< AccelerationSettings > > orderedAccelerationList = accelerationList;

            // Put aerodynamic and thrust accelerations in correct order (ensure aerodynamic acceleration created first).
            if( ( thrustAccelerationIndices.size( ) > 0 ) && ( aerodynamicAccelerationIndex.size( ) > 0 ) )
            {
                if( aerodynamicAccelerationIndex.at( aerodynamicAccelerationIndex.size( ) - 1 ) >
                        thrustAccelerationIndices.at( 0 ) )
                {
                    if( ( aerodynamicAccelerationIndex.size( ) == 1 ) )
                    {
                        std::iter_swap( orderedAccelerationList.begin( ) + aerodynamicAccelerationIndex.at( 0 ),
                                        orderedAccelerationList.begin( ) + thrustAccelerationIndices.at(
                                            thrustAccelerationIndices.size( ) - 1 ) );
                    }
                    else
                    {
                        throw std::runtime_error(
                                    "Error when ordering accelerations, cannot yet handle multple aerodynamic and thrust accelerations" );
                    }
                }
            }

            orderedAccelerationsPerBody[ bodyUndergoingAcceleration ][ bodyExertingAcceleration ] = orderedAccelerationList;
        }
    }

    return orderedAccelerationsPerBody;
}

} // namespace simulation_setup

} // namespace tudat
