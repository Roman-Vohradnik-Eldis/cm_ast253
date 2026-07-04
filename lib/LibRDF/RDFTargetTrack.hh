#ifndef _RDF_TARGET_TRACK_H
#define _RDF_TARGET_TRACK_H

#include "RDFHeader.hh"
#include "RDFTargetPlot.hh"

#include <sstream>//stringstream

enum FLIGHT_RULES
{
	FLIGHT_RULES_UNKNOWN,
	FLIGHT_RULES_VFR,
	FLIGHT_RULES_IFR,
};

struct RDFTargetCartesianCovariance
{
	double Cov;

	RDFTargetCartesianCovariance ();
	~RDFTargetCartesianCovariance ();
	RDFTargetCartesianCovariance (double cov);
	RDFTargetCartesianCovariance (RDFTargetCartesianCovariance * cov);
};

struct RDFTargetAccuracy
{
	RDFCoorCartesian * Cartesian;
	RDFCoorGeographic * Geographic;
	RDFCoorPolar * PolarSpeed;
	RDFCoorCartesian * CartesianSpeed;
	RDFTargetClimbSpeed * ClimbSpeed;
	RDFCoorCartesian * Acceleration;
	RDFTargetHeight * GeometricHeight;
	RDFTargetHeight * BarometricHeight;
	RDFTargetCartesianCovariance * CovXY;

	RDFTargetAccuracy();
	~RDFTargetAccuracy ();
	RDFTargetAccuracy(RDFTargetAccuracy * accuracy);
};


struct RDFTargetFlightPlan
{
	bool ClearedFlightLevelPresent : 1;
	bool CurrentControlPresent : 1;
	unsigned char CurrentControlCenter;
	unsigned char CurrentControlPosition;
	char WakeTurbulenceCategory;
	enum FLIGHT_RULES FlightRules;
	double ClearedFlightLevel;
	string Callsign;
	string TypeOfAircraft;
	string DepartureAirport;
	string DestinationAirport;
	string Route;

	RDFTargetFlightPlan ();
	~RDFTargetFlightPlan ();
	RDFTargetFlightPlan (RDFTargetFlightPlan * plan);
};

struct RDFTargetTrack : public RDFTargetPlot
{
	RDFTrackNumber TrackNumber;

	bool Init : 1;
	bool Cancel : 1;
	bool Tentative : 1;
	bool Correlated : 1;
	bool CST : 1;
	bool AFF : 1;
	bool Monosensor : 1;
	bool Manoeuvre : 1;

	bool DOU : 1;
	bool MAH : 1;
	bool SUP : 1;
	
	bool PSR : 1;
	bool SSR : 1;
	bool MDS : 1;
	bool ADSB : 1;

	RDFTime Age;
	RDFTime AgePSR;
	RDFTime AgeSSR;
	RDFTime AgeMDS;
	RDFTime AgeADSB;

	RDFCoorCartesian * Cartesian;
	RDFCoorGeographic * Geographic;

	RDFTargetMode * SmoothedMode3A;
	RDFTargetHeight * GeometricHeight;
	RDFTargetHeight * BarometricHeight;
	
	RDFTargetGroundSpeed * GroundSpeed;
	RDFCoorCartesian * Acceleration;
	RDFTargetClimbSpeed * ClimbSpeed;

	enum TRANS_ACCEL TransversalAcceleration;
	enum LONG_ACCEL LongitudinalAcceleration;
	enum TRACK_CLIMB TrackClimb;

	RDFTargetAccuracy * Accuracy;
	RDFTargetFlightPlan * FlightPlan;

	RDFTargetTrack ();
	~RDFTargetTrack ();
	RDFTargetTrack (const RDFTargetPlot & plot);
	RDFTargetTrack (const RDFTargetPlot * plot);
	RDFTargetTrack (const RDFTargetTrack & track);
	RDFTargetTrack (const RDFTargetTrack * track);
	const RDFTargetTrack & operator = (const RDFTargetTrack & track);

	static vector<string> getkeys ();
	string get (string key);

  protected:
  	void init ();
	void erase ();
	void assign (const RDFTargetTrack & track);

};

#endif /* _RDF_TARGET_TRACK_H */
