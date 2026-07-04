#ifndef _RDF_TARGET_ADSB_H
#define _RDF_TARGET_ADSB_H

#include "RDFHeader.hh"
#include "RDFCoordinate.hh"
#include "RDFTargetModeS.hh"
#include "RDFTargetStruct.hh"

enum ADDRESS_TYPE
{
	ADDRESS_TYPE_ICAO,
	ADDRESS_TYPE_DUPLICATE,
	ADDRESS_TYPE_SURFACE,
	ADDRESS_TYPE_ANONYMOUS,
	ADDRESS_TYPE_RESERVED4,
	ADDRESS_TYPE_RESERVED5,
	ADDRESS_TYPE_RESERVED6,
	ADDRESS_TYPE_RESERVED7
};

enum ALTITUDE_CAPABILITY
{
	ALTITUDE_CAPABILITY_25FT,
	ALTITUDE_CAPABILITY_100FT,
	ALTITUDE_CAPABILITY_UNKNOWN,
	ALTITUDE_CAPABILITY_INVALID
};

enum CONFIDENCE_LEVEL
{
	CONFIDENCE_LEVEL_VALID,
	CONFIDENCE_LEVEL_SUSPECT,
	CONFIDENCE_LEVEL_NOINFO,
	CONFIDENCE_LEVEL_RESERVED
};

enum PRIORITY_STATUS
{
	PRIORITY_STATUS_NO_EMERGENCY,
	PRIORITY_STATUS_GENERAL_EMERGENCY,
	PRIORITY_STATUS_LIFEGUARD_EMERGENCY,
	PRIORITY_STATUS_MINIMUM_FUEL,
	PRIORITY_STATUS_NO_COMMUNICATIONS,
	PRIORITY_STATUS_UNLAWFUL_NTERFERENCE,
	PRIORITY_STATUS_DOWNED_AIRCRAFT
};

enum SURVEILLANCE_STATUS
{
	SURVEILLANCE_STATUS_NO_CONDITION,
	SURVEILLANCE_STATUS_PERMANENT_ALERT,
	SURVEILLANCE_STATUS_TEMPORARY_ALERT,
	SURVEILLANCE_STATUS_SPI_SET
};

enum TECHNOLOGY_TYPE
{
	LINK_TYPE_OTHER,
	LINK_TYPE_UAT,
	LINK_TYPE_ES,
	LINK_TYPE_VDL4,
	LINK_TYPE_RESERVED4,
	LINK_TYPE_RESERVED5,
	LINK_TYPE_RESERVED6,
	LINK_TYPE_RESERVED7
};

struct RDFTargetAirSpeed
{
	double Speed;
	bool Mach;
	bool RE;

	RDFTargetAirSpeed (const double &speed = 0, const bool &mach = false,
		const bool &re = false);
	~RDFTargetAirSpeed ();
	RDFTargetAirSpeed (RDFTargetAirSpeed * speed);
};

struct RDFTargetAdsb
{
	unsigned char SAC;
	unsigned char SIC;

	RDFAircraftAddress AircraftAddress;
	std::string AircraftIdentification;

	RDFTrackNumber TrackNumber;
	enum ADDRESS_TYPE AddressType : 3;
	enum ALTITUDE_CAPABILITY AltitudeCapability : 2;
	enum CONFIDENCE_LEVEL ConfidenceLevel : 2;

	bool HRPosition : 1;

	bool Cancel : 1;
	bool Tentative : 1;
	bool CST : 1;
	bool Simulated : 1;
	bool Tested : 1;
	bool Fixed;
	bool SPI;
	/* range check passed, validation pending (1) */
	bool RC : 1;
	bool DCR : 1;
	bool GBS : 1;
	bool SAA : 1;
	bool IPC : 1;
	bool NOGO : 1;
	/* CPR validation correct */
	bool CPR : 1;
	bool LDPJ : 1;
	/* range check failed */
	bool RCF : 1;

	/* time of message transmission */
	RDFTime Time;

	/* time of applicability for position */
	RDFTime TOAp;

	/* time of message reception for position */
	RDFTime TOMRp;

	/* time of applicability for velocity */
	RDFTime TOAv;

	/* time of message reception for velocity */
	RDFTime TOMRv;

	/* Quality indicators / figure of merit */
	unsigned NUCr_NACv : 3;
	unsigned NUCp_NIC : 4;
	unsigned NICbaro : 1;
	unsigned SIL : 2;
	unsigned NACp : 4;
	unsigned SILs : 1;
	unsigned SDA : 2;
	unsigned GVA : 2;
	unsigned PIC : 4;
	unsigned CDTI : 1;

	/* target status */
	bool ICF : 1;
	bool LNAV : 1;
	enum PRIORITY_STATUS PriorityStatus : 3;
	enum SURVEILLANCE_STATUS SurveillanceStatus : 2;

	/* MOPS version / link technology indicator */
	bool VNS : 1;
	unsigned VersionNumber : 3;
	enum TECHNOLOGY_TYPE LinkTechnology : 3;

	std::vector<RDFTargetModeS> ModeS;
	RDFTargetMode * Mode1;
	RDFTargetMode * Mode2;
	RDFTargetMode * Mode3A;
	RDFTargetMode * ModeC;
	RDFTargetFlightLevel * FlightLevel;
	RDFTargetHeight * MeasuredHeight;
	RDFCoorGeographic * Geographic;
	RDFCoorCartesian * Cartesian;
	RDFTargetHeight * GeometricHeight;
	RDFTargetGroundSpeed * GroundSpeed;
	enum TRACK_CLIMB TrackClimb;

	/* true airspeed, airspeed */
	RDFTargetAirSpeed * TrueAirspeed;
	RDFTargetAirSpeed * AirSpeed;

	vector<unsigned char> WarningError;

	std::vector<unsigned char> SP_data;
	std::vector<unsigned char> RE_data;

	std::string UserText;

	RDFTargetAdsb ();
	~RDFTargetAdsb ();
	RDFTargetAdsb (const RDFTargetAdsb & adsb);
	RDFTargetAdsb (const RDFTargetAdsb * adsb);
	const RDFTargetAdsb & operator = (const RDFTargetAdsb & adsb);

	static std::vector<std::string> getkeys ();
	std::string get (std::string key);

  protected:
  	void init ();
	void erase ();
	void assign (const RDFTargetAdsb & adsb);

};

#endif /* _RDF_TARGET_ADSB_H */
