#ifndef _RDF_GEO_H
#define _RDF_GEO_H

#include "RDFHeader.hh"

struct RDFCoorGeographic
{
	double Latitude;	// zemepisna sirka (jih - sever)
	double Longitude;	// zemepisna delka (zapad - vychod)
	
	RDFCoorGeographic ();
	RDFCoorGeographic (double latitude, double longitude);
	RDFCoorGeographic (const RDFCoorGeographic & geographic);
	RDFCoorGeographic (const RDFCoorGeographic * geographic);

	void init ();
	bool equal (const RDFCoorGeographic & geographic);
	bool operator == (const RDFCoorGeographic & geographic) { return equal (geographic); }
	bool operator != (const RDFCoorGeographic & geographic) { return !equal (geographic); }
};

struct RDFCoorCartesian
{
	double X;
	double Y;
	
	RDFCoorCartesian ();
	RDFCoorCartesian (double x, double y);
	RDFCoorCartesian (const RDFCoorCartesian & cartesian);
	RDFCoorCartesian (const RDFCoorCartesian * cartesian);

	void init ();
	bool equal (const RDFCoorCartesian & cartesian);
	bool operator == (const RDFCoorCartesian & cartesian) { return equal (cartesian); }
	bool operator != (const RDFCoorCartesian & cartesian) { return !equal (cartesian); }
};

struct RDFCoorPolar
{
	double Rho;
	double Theta;
	
	RDFCoorPolar ();
	RDFCoorPolar (double rho, double theta);
	RDFCoorPolar (const RDFCoorPolar & polar);
	RDFCoorPolar (const RDFCoorPolar * polar);

	void init ();
	bool equal (const RDFCoorPolar & polar);
	bool operator == (const RDFCoorPolar & polar) { return equal (polar); }
	bool operator != (const RDFCoorPolar & polar) { return !equal (polar); }
};

void rdf_xy2ra (double x, double y, double & rho, double & angle);
void rdf_xy2rt (double x, double y, double & rho, double & theta);
void rdf_rt2xy (double rho, double theta, double & x, double & y);
void rdf_ra2xy (double rho, double angle, double & x, double & y);
bool rdf_dms2rad (int way, int deg, int min, double sec, double & rad);
bool rdf_rad2dms (double rad, int & way, int & deg, int & min, int & sec);
bool rdf_rad2dms (double rad, int & way, int & deg, int & min, double & sec);
void rdf_geo2strgeo (double lat, double lon, char * strlat, char * strlon);
void rdf_geo2strgeoref (const RDFCoorGeographic & geo, char * strgeoref);
void rdf_strgeoref2geo (const char * strgeoref, RDFCoorGeographic & geo);
bool rdf_geo2strgeoatc (const RDFCoorGeographic & geo, char * strgeoatc);
bool rdf_strgeoatc2geo (const char * strgeoatc, RDFCoorGeographic & geo);
void rdf_spherical2xyz (double rho, double theta, double psi, 
	double & x, double & y, double & z, double deviation);
void rdf_xyz2ra (double x, double y, double z, double & rho, double & angle);
void rdf_xyz2rt (double x, double y, double z, double & rho, double & theta);

#endif /* _RDF_GEO_H */
