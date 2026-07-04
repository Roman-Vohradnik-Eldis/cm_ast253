#ifndef _RDF_HEADER_H
#define _RDF_HEADER_H

#include "CMSHeader.hh"
#include <sstream>
#include <iomanip> 
#include <memory>
#include <iostream>
#include "CMSDebug.hh"
using namespace std;

#define ELDIS_RDF_VERSION "1.16.64"

#define METER_TO_NM (1 / 1852.0)
#define NM_TO_METER 1852

#define METER_TO_FEET (1 / 0.3048)
#define FEET_TO_METER 0.3048

#define SPEED_TO_KNOT (3.6 / 1.852)
#define KNOT_TO_SPEED (1.852 / 3.6)

#ifndef RAD_TO_DEG
#define RAD_TO_DEG	57.29577951308232
#endif

#ifndef DEG_TO_RAD
#define DEG_TO_RAD	.0174532925199432958
#endif

#define	SCF_CAT			17
#define	DLF_CAT			18

#ifndef M_TWOPI
#define M_TWOPI (M_PI + M_PI)
#endif

#endif /* _RDF_HEADER_H */
