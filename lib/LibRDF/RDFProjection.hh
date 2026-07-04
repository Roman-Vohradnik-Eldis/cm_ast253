#ifndef _RDF_PROJECTION_H
#define _RDF_PROJECTION_H

#include "RDFHeader.hh"
#include "RDFCoordinate.hh"

class RDFProjection
{
	RDFCoorGeographic SourcePoint;
	RDFCoorGeographic DestinationPoint;
	void * SourcePJ;
	void * DestinationPJ;
	bool Recal;

  public:
	RDFProjection ();
	RDFProjection (const RDFProjection & projection);
	~RDFProjection ();

	void SetPjInit (string init);	
	void SetCenter (const RDFCoorGeographic & center);
	
	void Geo2Car (const RDFCoorGeographic & geographic, RDFCoorCartesian & cartesian);
	void Geo2Car (const RDFCoorGeographic * geographic, RDFCoorCartesian * cartesian);
	void Geo2Pol (const RDFCoorGeographic & geographic, RDFCoorPolar & polar);
	void Geo2Pol (const RDFCoorGeographic * geographic, RDFCoorPolar * polar);
	void Car2Geo (const RDFCoorCartesian & cartesian, RDFCoorGeographic & geographic);
	void Car2Geo (const RDFCoorCartesian * cartesian, RDFCoorGeographic * geographic);
	void Car2Pol (const RDFCoorCartesian & cartesian, RDFCoorPolar & polar);
	void Car2Pol (const RDFCoorCartesian * cartesian, RDFCoorPolar * polar);
	void Pol2Geo (const RDFCoorPolar & polar, RDFCoorGeographic & geographic);
	void Pol2Geo (const RDFCoorPolar * polar, RDFCoorGeographic * geographic);
	void Pol2Car (const RDFCoorPolar & polar, RDFCoorCartesian & cartesian);
	void Pol2Car (const RDFCoorPolar * polar, RDFCoorCartesian * cartesian);
	
	void SetSourceDestination (const RDFCoorGeographic & src, const RDFCoorGeographic & dst);
	void RecalTarget (int & x, int & y);
	void RecalTarget (double & x, double & y);

  protected:
	void CountPJ ();
};

class RDFTargetProjection
{
	string pjinit;
	void * source;
	void * destination;
	bool Recal;
	double srcR;             /*  Earth radius at source location  */
	double srcN;             /*  normalised Earth radius (normal distance) at source location  */
	double srcHeight;		 /*  radar height at source location  */
	double dstR;             /*  Earth radius at destination location  */
	double dstN;             /*  normalised Earth radius (normal distance) at destination location  */
	double dstHeight;		 /*  radar height at destination location  */
	double srcS1 [3][3];	 /*  Rotation matrix for local cartesian to geocentric cartesian and vice versa transformation  */
	double srcT1 [3];		 /*  Shift matrix for local cartesian to geocentric cartesian and vice versa transformation  */
	double dstS1 [3][3];	 /*  Rotation matrix for local cartesian to geocentric cartesian and vice versa transformation  */
	double dstT1 [3];		 /*  Shift matrix for local cartesian to geocentric cartesian and vice versa transformation  */
	
  public:
	RDFTargetProjection ();
	~RDFTargetProjection (void);

	void SetPjInit (string init) { pjinit = init; }
	
	void SetCenter (const RDFCoorGeographic & center, double height = 0.);
	void Geo2Car (const RDFCoorGeographic & geographic, RDFCoorCartesian & cartesian);
	void Geo2Pol (const RDFCoorGeographic & geographic, RDFCoorPolar & polar);
	void Car2Geo (const RDFCoorCartesian & cartesian, RDFCoorGeographic & geographic);
	void Car2Pol (const RDFCoorCartesian & cartesian, RDFCoorPolar & polar);
	void Pol2Geo (const RDFCoorPolar & polar, RDFCoorGeographic & geographic);
	void Pol2Car (const RDFCoorPolar & polar, RDFCoorCartesian & cartesian);
	void Rth2Car (double rho, double theta, double height, RDFCoorCartesian & cartesian);
	void Rth2Geo (double rho, double theta, double height, RDFCoorGeographic & geographic);
	void Geo2Rth (RDFCoorGeographic & geographic, double height, double & rho, double & theta);

	void SetSourceDestination (const RDFCoorGeographic & src, const RDFCoorGeographic & dst, double srcheight = 0., double dstheight = 0.);
	void RecalTarget (int & x, int & y);
	void RecalTarget (double & x, double & y);

   private:
	double _ComputeEarthRadius (void * P);
	double _ComputeNormalisedEarthRadius (void * P);
	void _ComputeMatrices (void * P, double N, double *t, double (*s)[3], double height = 0.);
	void _rth2spherical (double rho, double theta, double height, double radar_height, double & psi);
};

#endif /* _RDF_PROJECTION_H */
