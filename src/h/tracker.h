//
// Created by Casey Shields on 4/27/2018.
//

#ifndef STARTRACK_TRACKER_H
#define STARTRACK_TRACKER_H

#include "novas.h"

#define SECONDS_IN_DAY 86400.0
#define DELTA_TT 32.184
#define REDUCED_ACCURACY 1

/** The Tracker module represents a sensor on the surface of the earth at a specific time, observing
 * a celestial object. It can transform star coordinates between celestial and horizon, using Novas
 * 3.1 to handle orbit, spin, precession, parallax, atmospheric refraction and relativistic
 * aberration, whatever that is.
 * @author Casey Shields*/
typedef struct {

    /** Julian hours; the number of hours since noon, January 1, 4713 BC- Which Joseph Justice Scaliger estimated to be the beginning of history. */
    double date;

    /** Current observed discrepancy between earth's non-uniform rotation and Universal Coordinated Time */
    double ut1_utc;

    /** Number of leap seconds added in International Atomic Time */
    double leap_secs;

    /** Novas structure holding the geodetic location of the tracker */
    on_surface site;

    /** Novas structure for planet earth */
    object earth;

} Tracker;

/** Initializes or allocates the given tracker structure
 * @param tracker A pointer to an existing structure or NULL. If NULL a structure is allocated.
 * @param ut1_utc Current difference between UT1 and UTC time, usually obtained from IERS bulletin A
 * @param leap_secs Current number of leap seconds in TAI, usually obtained from IERS bulletin A
 * @return a pointer to the initialized or allocated structure */
int tracker_create(Tracker *tracker, double ut1_utc, double leap_secs);

/** Sets the current time for the star tracker
 * @param utc_unix_seconds Seconds since the unix epoch(January 1, 1970) in Universal Coordinated Time */
void tracker_set_time(Tracker *tracker, double utc_unix_seconds); // set UTC

// todo julian hours is pretty obscure, might want to return in unix seconds...
/** @returns terrestrial time in julian hours, a somewhat obscure Novas convention. TT = UTC + leap_seconds + 32.184. */
double tracker_get_TT(Tracker *tracker);

/** @return The time in UT1, a time scale which depends on the non-uniform rotation of the earth.
 * It is derived by adding an empirically determined offset to UTC */
double tracker_get_UT1(Tracker *tracker);

/** @return The Universal Coordinated Time in Julian hours. */
double tracker_get_UTC(Tracker *tracker);

/** @return  */
double tracker_get_DeltaT(Tracker *tracker);

/** Sets the location of the tracker on earth geoid.
 * @param tracker object whose coordinates are set
 * @param latitude in degrees
 * @param longitude in degrees
 * @param height in meters */
void tracker_set_location(Tracker *tracker, double latitude, double longitude, double height);

/** @return The current geodetic location using the novas convention */
on_surface tracker_get_location(Tracker *tracker);

/** A simple model is used to estimate atmospheric refraction by novas. This sets it's coefficients.
 * @param temperature the temperature at the sensor in degrees Celsius.
 * @param pressure the air pressure at the sensor in millibars.*/
void tracker_set_weather(Tracker *tracker, double temperature, double pressure);

/** the local horizon coordinates of the target relative to the tracker
 * @param tracker The tracker to compute the direction from
 * @param target A novas catalog entry to point at
 * @param zenith_distance Output argument returning the angular offset from the local zenith in degrees
 * @param topocentric_azimuth Output argument returning the clockwise angular offset from north in degrees
 * @return Zero on success, otherwise a Novas error code. */
int tracker_to_horizon(Tracker *tracker, cat_entry *target, double *zenith_distance, double *topocentric_azimuth);

/** returns the current location of the given tracker's zenith in celestial coordinates.
 * @param tracker Location used to compute the zenith vector
 * @param right_ascension Celestial spherical coordinates of zenith in hours
 * @param declination Celestial spherical coordinates of zenith in degrees
 * @returns Zero on success, otherwise a Novas error code. */
int tracker_zenith(Tracker *tracker, double *right_ascension, double *declination);

// todo should take a stream
void tracker_print_time(Tracker *tracker);

void tracker_print_site(Tracker *tracker);

#endif //STARTRACK_TRACKER_H