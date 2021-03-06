/** @file catalog.h
 * @brief A catalog object for collecting, filtering and searching celestial targets
 * @author Casey Shields */

#ifndef STARTRACK_CATALOG_H
#define STARTRACK_CATALOG_H

#include <stdio.h>
#include <assert.h>

#include "../lib/novasc3.1/novas.h"
#include "../lib/cutest-1.5/CuTest.h"
#include "util/vmath.h"
#include "data/fk6.h"

/** Represents a celestial object in a star catalog. Extended from Novas's struct cat_entry to add
 * some more parameters and various transformed coordinates.
 * @author Casey Shields*/
typedef struct {

    /** The Novas structure which contains all the information needed to perform astrometric calculations. */
    cat_entry novas;

    /** The visual magnitude of the star. */
    double magnitude;

    /** Scratch-space for transformed horizon coordinates- only makes sense in the context of a Tracker */
    double zenith_distance, topocentric_azimuth;

    /** A pointing vector in geocentric coordinates, only makes sense in the context of a Tracker */
    double efg[3];
    // todo To follow better encapsulation principles we may want to extract transformed coordinates into a structure which references the catalog entry

} Entry;

void entry_print( Entry *e, FILE * file );

void entry_print_summary( Entry *star );

/** A catalog of stars. provides methods for loading, searching, filtering and printing entries.
 * @author Casey Shields*/
typedef struct {

    /** Current number of stars in the catalog */
    size_t size;

    /** Tracks space currently allocated for the catalog. */
    size_t allocated;

    /** An array of catalog entries. */
    Entry** stars;

} Catalog;

/** Creates a new catalog at the given pointer.
 * If the 'catalog' reference is NULL a new catalog and entries are allocated using the hint.
 * If the reference is valid and 'allocate' is positive, new entries are allocated and the old are freed.
 * Otherwise the catalog's pre-existing references are used. This allows you to reuse previously allocated catalogs.
 * Returns the initialized catalog. */
Catalog* catalog_create(Catalog* c, size_t s);

/** Adds the given entry to the catalog, doubling the allocated space if necessary. */
void catalog_add( Catalog* c, Entry *e );

/** Loads a FK5 catalog printout from the given file. */
Catalog* catalog_load_fk5(Catalog * c, FILE * f);

/**  */
Catalog * catalog_load_fk6(Catalog *catalog, FK6 *fk6, FILE *file);

typedef void (*EntryFunction)(Entry*);

/** Applies a void function to every entry in the catalog. */
void catalog_each( Catalog * c, EntryFunction f );

/** A predicate used in catalog filters to test individual entries. */
typedef int (*EntryPredicate)(Entry*);

/** Searches a catalog for Entries for which the predicate function returns true.
 * No effort is taken to remove duplicates from the results.
 * @param catalog The catalog to be searched.
 * @param predicate a pointer to a boolean function of an Entry
 * @param results A catalog to add the matches to. If NULL, a new Catalog is allocated. Don't forget to de-allocate it!
 * @return a pointer to the resulting catalog, regardless if the result parameter was set to NULL. */
Catalog* catalog_filter(
        const Catalog * catalog,
        EntryPredicate predicate,
        Catalog * results );
// a nice behavior might be if catalog == results, filter the catalog in place...

/** Searches a catalog for entries within the geometry and returns a catalog holding the results.
 * No effort is taken to remove duplicates from the results.
 * @param catalog The catalog to be searched.
 * @param right_ascension Right ascension of axis of search cone volume in hours.
 * @param declination Declination of axis of search cone volume in degrees.
 * @param radius Angle between axis and edge of search cone volume in degrees.
 * @param results A catalog to add the matches to. If NULL, a new Catalog is allocated. Don't forget to de-allocate it!
 * @return a pointer to the resulting catalog, regardless if the result parameter was set to NULL. */
Catalog* catalog_search_dome( Catalog* catalog, double right_ascension, double declination, double radius, Catalog* results );

/** Searches  */
Catalog * catalog_brighter(const Catalog *c, double min_mag, Catalog *results);

/** Searches a catalog for entries within the geometry and returns a catalog holding the results.
 * No effort is taken to remove duplicates from the results.
 * @param catalog The catalog to be searched.
 * @param ra_min Right ascension lower bound, inclusive.
 * @param ra_max Right ascension upper bound, inclusive.
 * @param dec_min Declination lower bound, inclusive.
 * @param dec_max Declination upper bound, inclusive.
 * @param results A catalog to add the matches to. If NULL, a new Catalog is allocated. Don't forget to de-allocate it!*/
Catalog* catalog_search_patch(
        const Catalog* catalog,
        double ra_min, double ra_max, double dec_min, double dec_max,
        Catalog* results );

/** Searched a catalog for entries with names that contain the given substring.
 * No effort is taken to remove duplicates from the results
 * @param catalog The catalog to be searched
 * @param substring A case sensitive sub-string to search entry names for
 * @param results A catalog to add the matches to. If NULL, a new Catalog is allocated. Don't forget to de-allocate it!*/
Catalog * catalog_search_name (
        const Catalog * catalog,
        const char * substring,
        Catalog * results );

/** Selects the first entry with the given Fundamental Catalog ID.
 * @param catalog the catalog to search
 * @param fkid The Hipparcos/Tycho catalog ID
 * @return a pointer to the star's entry, or NULL if it is not in the catalog */
Entry * catalog_get(const Catalog *catalog, unsigned long fkid);

/** Prints the given catalog to stdout in a default format. */
void catalog_print( Catalog *c );

/** A function for ordering catalog entries */
typedef void (*EntryComparison)(Entry*, Entry*);

//void catalog_sort( Catalog* c, int (*comparison)(Entry*, Entry*) );
//todo implement a sort function for ranking results

/** Clears all entries from the Catalog, without affecting the underlying Entries.
 * @param c The catalog to empty.*/
void catalog_clear( Catalog *c );

/** Releases the Catalog and it's directory, but not the actual Entries.*/
void catalog_free( Catalog *c );

/** Loads an example fk6 dataset, and only performs some crude sanity checks
 * @param test the CuTest structure which holds test results. */
void test_FK6( CuTest * test );

#endif //STARTRACK_CATALOG_H
