# Orion

Star hunting software. Contains a planning component for loading astrometric catalogs and selecting suitable stars.
The other component is a tracker which transforms catalog coordinates into a useable format and broadcasts them to client(s) over TATS.
The [Novas 3.1](http://aa.usno.navy.mil/software/novas/novas_info.php) library is used to perform the transformations.
The first part of [FK5](http://www-kpno.kpno.noao.edu/Info/Caches/Catalogs/FK5/fk5.html) or [FK6](http://cdsarc.u-strasbg.fr/viz-bin/Cat?I/264) is currently used as a catalog.
The front end is still in the conceptual phases, however a [UI experiment](https://caseyshields.github.io/starlog/index.html) has been written in [D3](https://d3js.org/).  

[documentation](http://caseyshields.github.io/Orion/docs/doxygen/html/index.html)


## Components
 - vmath : utility library of vector and spherical math routines
 - novas : USNO's astrometric software package
 - fk6 : loads raw FK6 catalogs into a catalog
 - catalog : module for filtering and sorting desired stars.
 - tracker : performs coordinate transforms according to the current time and location on earth 
 - sensor : a dummy server which simulates a slaved sensor
 - orion : main program which integrates all features

## Requirements
 - Orion is developed in CLion using MinGW
