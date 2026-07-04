#ifndef _RDF_PAR_TARGET_H
#define _RDF_PAR_TARGET_H

#include "RDFHeader.hh"
#include "RDFCoordinate.hh"
#include "RDFTargetStruct.hh"

enum PAR_TARGET_MESSAGE_TYPE
{
	PAR_TARGET_MESSAGE_TYPE_UNKNOWN,
	PAR_TARGET_MESSAGE_TYPE_AZIMUTHAL_TARGET,
	PAR_TARGET_MESSAGE_TYPE_ELEVATION_TARGET,
};

enum PAR_TARGET_LOP
{
	PAR_TARGET_LOP_UNDETERMINED,
	PAR_TARGET_LOP_START,
	PAR_TARGET_LOP_STOP
};

enum PAR_TARGET_TOT
{
	PAR_TARGET_TOT_UNDETERMINED,
	PAR_TARGET_TOT_AIRCRAFT
};

struct RDFParTarget
{
	RDFTime Time;
	RDFTrackNumber TrackNumber;
	
	unsigned char SAC;
	unsigned char SIC;

	unsigned char PamAzimuthPresent : 1;
	unsigned char PamElevationPresent : 1;
	unsigned char PrlAzimuthPresent : 1;
	unsigned char PrlElevationPresent : 1;
	unsigned char MeasuredAzimuthalPositionPresent : 1;
	unsigned char MeasuredElevationPositionPresent : 1;
	unsigned char CalculatedAzimuthalPositionPresent : 1;
	unsigned char CalculatedElevationPositionPresent : 1;
	unsigned char AzimuthalVelocityPresent : 1;
	unsigned char ElevationVelocityPresent : 1;
	
	unsigned char Simulated : 1;
	unsigned char Tested : 1;
	unsigned char ParDetection : 1;

	unsigned char Tentative : 1;
	unsigned char Cancel : 1;
	unsigned char Ghost : 1;
	
	unsigned char MessageType;
	unsigned char LOP;
	unsigned char TOT;
	char PamAzimuth;
	char PamElevation;
	unsigned char PrlAzimuth;
	unsigned char PrlElevation;
	
	RDFCoorPolar MeasuredAzimuthalPosition;
	RDFCoorPolar MeasuredElevationPosition;
	RDFCoorPolar CalculatedAzimuthalPosition;
	RDFCoorPolar CalculatedElevationPosition;

	RDFTargetGroundSpeed AzimuthalVelocity;
	RDFTargetGroundSpeed ElevationVelocity;
	
	vector<unsigned char> WarningError;

	string UserText;

  public:
	RDFParTarget ();
	~RDFParTarget ();
	RDFParTarget (const RDFParTarget & target);
	const RDFParTarget & operator = (const RDFParTarget & target);

	static vector<string> getkeys ();
	string get (string key);
  protected:
	void init ();
	void assign (const RDFParTarget & target);
};

#endif /* _RDF_TARGET_PLOT_H */
