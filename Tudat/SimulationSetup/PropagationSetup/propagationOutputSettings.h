/*    Copyright (c) 2010-2016, Delft University of Technology
 *    All rigths reserved
 *
 *    This file is part of the Tudat. Redistribution and use in source and
 *    binary forms, with or without modification, are permitted exclusively
 *    under the terms of the Modified BSD license. You should have received
 *    a copy of the license with this file. If not, please or visit:
 *    http://tudat.tudelft.nl/LICENSE.
 */

#ifndef TUDAT_PROPAGATIONOUTPUTSETTINGS_H
#define TUDAT_PROPAGATIONOUTPUTSETTINGS_H

#include <string>

#include "Tudat/Astrodynamics/BasicAstrodynamics/accelerationModelTypes.h"
#include "Tudat/Astrodynamics/ReferenceFrames/aerodynamicAngleCalculator.h"

namespace tudat
{

namespace propagators
{


//! Enum listing the dependent variables that can be saved during the propagation
enum PropagationDependentVariables
{
    mach_number_dependent_variable = 0,
    altitude_dependent_variable = 1,
    airspeed_dependent_variable = 2,
    local_density_dependent_variable = 3,
    relative_speed_dependent_variable = 4,
    relative_position_dependent_variable = 5,
    relative_distance_dependent_variable = 6,
    relative_velocity_dependent_variable = 7,
    radiation_pressure_dependent_variable = 8,
    total_acceleration_norm_dependent_variable = 9,
    single_acceleration_norm_dependent_variable = 10,
    total_acceleration_dependent_variable = 11,
    single_acceleration_dependent_variable = 12,
    aerodynamic_force_coefficients_dependent_variable = 13,
    aerodynamic_moment_coefficients_dependent_variable = 14,
    rotation_matrix_to_body_fixed_frame_variable = 15,
    intermediate_aerodynamic_rotation_matrix_variable = 16,
    relative_body_aerodynamic_orientation_angle_variable = 17,
    body_fixed_airspeed_based_velocity_variable = 18,
    thrust_force_dependent_variable = 19

};

//! Functional base class for defining settings for dependent variables that are to be saved during propagation
/*!
 *  Functional base class for defining settings for dependent variables that are to be saved during propagation.
 *  Any dependent variable that requires additional information in addition to what can be provided here, should be
 *  defined by a dedicated derived class.
 */
class SingleDependentVariableSaveSettings
{
public:

    //! Constructor
    /*!
     * Constructor
     * \param variableType Type of dependent variable that is to be saved.
     * \param associatedBody Body associated with dependent variable
     * \param secondaryBody Secondary body (not necessarilly required) w.r.t. which parameter is defined (e.g. relative
     * position, velocity etc. is defined of associatedBody w.r.t. secondaryBody).
     */
    SingleDependentVariableSaveSettings(
            const PropagationDependentVariables variableType,
            const std::string& associatedBody,
            const std::string& secondaryBody = "" ):
        variableType_( variableType ), associatedBody_( associatedBody ), secondaryBody_( secondaryBody ){ }

    //! Destructor.
    virtual ~SingleDependentVariableSaveSettings( ){ }

    //! Type of dependent variable that is to be saved.
    PropagationDependentVariables variableType_;

    //! Body associated with dependent variable
    std::string associatedBody_;

    //! Secondary body (not necessarilly required) w.r.t. which parameter is defined (e.g. relative  position,
    //! velocity etc. is defined of associatedBody w.r.t. secondaryBody).
    std::string secondaryBody_;

};

//! Class to define settings for saving a single acceleration (norm or vector) during propagation
class SingleAccelerationDependentVariableSaveSettings: public SingleDependentVariableSaveSettings
{
public:

    //! Constructor
    /*!
     * Constructor
     * \param accelerationModeType Type of acceleration that is to be saved.
     * \param bodyUndergoingAcceleration Name of body undergoing the acceleration.
     * \param bodyExertingAcceleration Name of body exerting the acceleration.
     * \param useNorm Boolean denoting whether to use the norm (if true) or the vector (if false) of the acceleration.
     */
    SingleAccelerationDependentVariableSaveSettings(
            const basic_astrodynamics::AvailableAcceleration accelerationModeType,
            const std::string& bodyUndergoingAcceleration,
            const std::string& bodyExertingAcceleration,
            const bool useNorm = 0 ):
        SingleDependentVariableSaveSettings(
            ( useNorm == 1 ) ? ( single_acceleration_norm_dependent_variable ) : ( single_acceleration_dependent_variable ),
            bodyUndergoingAcceleration, bodyExertingAcceleration ),
        accelerationModeType_( accelerationModeType )
    { }

    //! Boolean denoting whether to use the norm (if true) or the vector (if false) of the acceleration.
    basic_astrodynamics::AvailableAcceleration accelerationModeType_;

};

//! Class to define settings for saving a rotation matrix between two AerodynamicsReferenceFrames
class IntermediateAerodynamicRotationVariableSaveSettings: public SingleDependentVariableSaveSettings
{
public:

    //! Constructor
    /*!
     * Constructor
     * \param associatedBody Body for which the rotation matrix is to be saved.
     * \param baseFrame Frame from which rotation is to take place.
     * \param targetFrame Frame to which the rotation is to take place.
     */
    IntermediateAerodynamicRotationVariableSaveSettings(
            const std::string& associatedBody,
            const reference_frames::AerodynamicsReferenceFrames baseFrame,
            const reference_frames::AerodynamicsReferenceFrames targetFrame ):
        SingleDependentVariableSaveSettings( intermediate_aerodynamic_rotation_matrix_variable, associatedBody ),
        baseFrame_( baseFrame ), targetFrame_( targetFrame ){ }

    //! Frame from which rotation is to take place.
    reference_frames::AerodynamicsReferenceFrames baseFrame_;

    //! Frame to which the rotation is to take place.
    reference_frames::AerodynamicsReferenceFrames targetFrame_;

};

//! Class to define settings for saving an aerodynamics orientation angle from AerodynamicsReferenceFrameAngles list.
class BodyAerodynamicAngleVariableSaveSettings: public SingleDependentVariableSaveSettings
{
public:

    //! Constructor
    /*!
     * Constructor
     * \param associatedBody Body for which the orientation angle is to be saved.
     * \param angle Orientation angle that is to be saved.
     */
    BodyAerodynamicAngleVariableSaveSettings(
            const std::string& associatedBody,
            const reference_frames::AerodynamicsReferenceFrameAngles angle ):
        SingleDependentVariableSaveSettings( relative_body_aerodynamic_orientation_angle_variable, associatedBody ),
        angle_( angle ){ }

    //! Orientation angle that is to be saved.
    reference_frames::AerodynamicsReferenceFrameAngles angle_;
};

//! Container class for settings of all dependent variables that are to be saved.
class DependentVariableSaveSettings
{
public:

    //! Constructor
    /*!
     * Constructor
     * \param dependentVariables List of settings for parameters that are to be saved.
     * \param printDependentVariableTypes Variable denoting whether to print the list and vector entries of
     * dependent variables when propagating.
     */
    DependentVariableSaveSettings(
            const std::vector< boost::shared_ptr< SingleDependentVariableSaveSettings > > dependentVariables,
            const bool printDependentVariableTypes = 1 ):
        dependentVariables_( dependentVariables ), printDependentVariableTypes_( printDependentVariableTypes ){ }

    //! List of settings for parameters that are to be saved.
    std::vector< boost::shared_ptr< SingleDependentVariableSaveSettings > > dependentVariables_;

    //! Variable denoting whether to print the list and vector entries of dependent variables when propagating.
    bool printDependentVariableTypes_;
};

//! Function to get a string representing a 'named identification' of a dependent variable type
/*!
 * Function to get a string representing a 'named identification' of a dependent variable type
 * \param propagationDependentVariables Dependent variable type
 * \return String with variable type id.
 */
std::string getDependentVariableName( const PropagationDependentVariables propagationDependentVariables );

//! Function to get a string representing a 'named identification' of a dependent variable
/*!
 * Function to get a string representing a 'named identification' of a dependent variable
 * \param dependentVariableSettings Dependent variable
 * \return String with variable id.
 */
std::string getDependentVariableId(
        const boost::shared_ptr< SingleDependentVariableSaveSettings > dependentVariableSettings );


} // namespace propagators

} // namespace tudat


#endif // TUDAT_PROPAGATIONOUTPUTSETTINGS_H
