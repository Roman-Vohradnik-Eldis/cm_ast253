#ifndef _RDF_TARGET_PLOT_H
#define _RDF_TARGET_PLOT_H

#include "RDFHeader.hh"
#include "RDFCoordinate.hh"
#include "RDFTargetDetections.hh"
#include "RDFTargetModeS.hh"
#include "RDFTargetStruct.hh"

#define TARGET_TYPES 8

enum TARGET_TYPE
{
	TARGET_TYPE_UNKNOWN,
	TARGET_TYPE_PRIMARY,
	TARGET_TYPE_SECONDARY,
	TARGET_TYPE_COMBINED,
	TARGET_TYPE_MODES_ALL_CALL,
	TARGET_TYPE_MODES_ROLL_CALL,
	TARGET_TYPE_MODES_ALL_CALL_PSR,
	TARGET_TYPE_MODES_ROLL_CALL_PSR,
	TARGET_TYPE_ADSB,
};

enum TARGET_FOEFRI
{
	TARGET_FOEFRI_NO_MODE4,
	TARGET_FOEFRI_FRIENDLY_TARGET,
	TARGET_FOEFRI_UNKNOWN_TARGET,
	TARGET_FOEFRI_NO_REPLY,
};

struct RDFTargetCharacteristics
{
	bool SsrRunlengthPresent:1;
	bool SsrRepliesPresent:1;
	bool SsrAmplitudePresent:1;
	bool PsrRunlengthPresent:1;
	bool PsrAmplitudePresent:1;
	bool PsrSsrRangeDiffPresent:1;
	bool PsrSsrAzimuthDiffPresent:1;
	unsigned char SsrRunlength;
	unsigned char SsrReplies;
	char SsrAmplitude;
	unsigned char PsrRunlength;
	char PsrAmplitude;
	char PsrSsrRangeDiff;
	char PsrSsrAzimuthDiff;

	RDFTargetCharacteristics ();
	~RDFTargetCharacteristics ();
	RDFTargetCharacteristics (RDFTargetCharacteristics * characteristics);
};

enum COMM_CAPABILITY
{
	COMM_CAPABILITY_NONE,		// 0 - no communication capability (surveillance only)
	COMM_CAPABILITY_A_B,		// 1 - Comm. A and Comm. B capabilty
	COMM_CAPABILITY_A_B_ELMU,	// 2 - Comm. A, Comm. B and Uplink ELM capabilty
	COMM_CAPABILITY_A_B_ELMU_ELMD,	// 3 - Comm. A, Comm. B, Uplink ELM and Downlink ELM capabilty
	COMM_CAPABILITY_LEVEL_5,	// 4 - Level 5 Transponder capabilty
};

enum FLIGHT_STATUS
{
	FLIGHT_STATUS_AIRBORNE,		// 0 - No alert, no SPI, aircraft airborne
	FLIGHT_STATUS_ON_GROUND,	// 1 - No alert, no SPI, aircraft on ground
	FLIGHT_STATUS_AIRBORNE_ALERT,	// 2 - Alert, no SPI, aircraft airborne
	FLIGHT_STATUS_ON_GROUND_ALERT,	// 3 - Alert, no SPI, aircraft on ground
	FLIGHT_STATUS_ALERT_SPI,	// 4 - Alert, SPI, aircraft airborne or on ground
	FLIGHT_STATUS_SPI,			// 5 - No alert, SPI, aircraft airborne or on ground
	FLIGHT_STATUS_GENERAL_EMERGENCY,	// 6 - General Emergency
	FLIGHT_STATUS_LIFEGUARD,	// 7 - Lifeguard / medical
	FLIGHT_STATUS_MINIMUM_FUEL,	// 8 - Minimum fuel
	FLIGHT_STATUS_NO_COMMUNICATIONS,	// 9 - No Communications
	FLIGHT_STATUS_UNLAWFUL_INTERFERENCE,	// 10 - Unlawful interference
};

struct RDFCommCapabilityAndFlightStatus
{
	enum COMM_CAPABILITY CommCapability;
	enum FLIGHT_STATUS FlightStatus;
	bool TransponderCapability;
	bool ModeSSpecificService;
	bool AltitudeCapability;	// 0 ... 100 ft resolution, 1 ... 25 ft resolution
	bool IdentificationCapability;
	bool B1A;					// BDS 1,0 bit 16
	unsigned char B1B;			// BDS 1,0 bits 37-40
	bool ACASOperational;
	bool MultipleAids;			// Multiple Navigational Aids Operating
	bool DifferentialCorrection;

	RDFCommCapabilityAndFlightStatus ();
	~RDFCommCapabilityAndFlightStatus ();
	RDFCommCapabilityAndFlightStatus (RDFCommCapabilityAndFlightStatus * status);
};

struct RDFResolutionAdvisory
{
	bool Present;
	unsigned char Data[7];

	RDFResolutionAdvisory ()
	{
		init ();
	}
	void init ();
};

struct RDFDopplerSpeed
{
	bool RawPresent:1;
	bool CalculatedPresent:1;
	bool CalculatedValid:1;
	unsigned char RepetitonFactor;
	unsigned short DopplerSpeed;
	unsigned short AmbiguityRange;
	unsigned short TransmitterFrequency;
	unsigned short CalculatedSpeed;

	RDFDopplerSpeed ()
	{
		init ();
	}
	void init ();
};

enum CODE_SOURCE
{
	CODE_SOURCE_NONE,
	CODE_SOURCE_SIF,
	CODE_SOURCE_ROLL_CALL,
	CODE_SOURCE_ADS,
};

enum POSITION_SOURCE
{
	POSITION_SOURCE_NONE,
	POSITION_SOURCE_SIF,
	POSITION_SOURCE_SIF_FALSE,
	POSITION_SOURCE_ALL_CALL,
	POSITION_SOURCE_ALL_CALL_FALSE,
	POSITION_SOURCE_ROLL_CALL,
	POSITION_SOURCE_ROLL_CALL_FALSE,
	POSITION_SOURCE_ALL_CALL_ROLL_CALL,
	POSITION_SOURCE_SIF_ALL_CALL,
	POSITION_SOURCE_SIF_ROLL_CALL,
	POSITION_SOURCE_SIF_ALL_CALL_ROLL_CALL,
	POSITION_SOURCE_ADS,
};

enum DETECTION_SOURCE
{
	DETECTION_SOURCE_NONE,
	DETECTION_SOURCE_SIF,
	DETECTION_SOURCE_ALL_CALL,
	DETECTION_SOURCE_ROLL_CALL,
	DETECTION_SOURCE_ALL_CALL_ROLL_CALL,
	DETECTION_SOURCE_ADS,
	DETECTION_SOURCE_SIF_ALL_CALL,
	DETECTION_SOURCE_SIF_ROLL_CALL,
	DETECTION_SOURCE_SIF_ALL_CALL_ROLL_CALL,
	DETECTION_SOURCE_SIF_ALL_CALL_ADS,
	DETECTION_SOURCE_SIF_ROLL_CALL_ADS,
	DETECTION_SOURCE_SIF_ADS,
	DETECTION_SOURCE_ALL_CALL_ADS,
	DETECTION_SOURCE_ROLL_CALL_ADS,
	DETECTION_SOURCE_ALL_CALL_ROLL_CALL_ADS,
	DETECTION_SOURCE_SIF_ALL_CALL_ROLL_CALL_ADS,
};

struct RDFTargetComposition
{
	unsigned char Mode3A;
	unsigned char ModeC;
	unsigned char Position;
	unsigned char Detection;

	RDFTargetComposition ()
	{
		init ();
	}
	void init ();
};

struct RDFTargetDetections
{
	bool SifMode3APresent:1;
	bool SifModeCPresent:1;
	bool AllCallPresent:1;
	bool RollCallMode3APresent:1;
	bool RollCallModeCPresent:1;
	bool AdsPresent:1;

	unsigned char SifMode3ACount;
	unsigned char SifModeCCount;
	unsigned char AllCallCount;
	unsigned char RollCallMode3ACount;
	unsigned char RollCallModeCCount;
	unsigned char AdsCount;

	unsigned char SifMode3AAge;
	unsigned char SifModeCAge;
	unsigned char AllCallAge;
	unsigned char RollCallMode3AAge;
	unsigned char RollCallModeCAge;
	unsigned char AdsAge;

	RDFTargetDetections ()
	{
		init ();
	}
	void init ();
};

struct RDFTargetPlot
{
	RDFTime Time;

	unsigned char CAT;
	unsigned char SAC;
	unsigned char SIC;

	enum TARGET_TYPE TargetType;
	enum TARGET_FOEFRI TargetFoeFri;

	bool Simulated:1;
	bool Tested:1;
	bool SPI:1;
	bool RAB:1;
	bool ME:1;
	bool MI:1;

	RDFCoorPolar *Polar;

	RDFTargetMode *Mode1;
	RDFTargetMode *Mode2;
	RDFTargetMode *Mode3A;
	RDFTargetMode *ModeB;
	RDFTargetMode *ModeC;
	RDFTargetMode *ModeD;

	RDFTargetFlightLevel *FlightLevel;
	RDFTargetHeight *MeasuredHeight;

	RDFUvdFriendStatus UvdFriendStatus;
	RDFUvdCode UvdCode;
	RDFUvdAltitude UvdAltitude;
	RDFUvdSpeed UvdSpeed;

	RDFAircraftAddress AircraftAddress;
	string AircraftIdentification;

	vector < RDFTargetModeS > ModeS;
	vector < RDFTargetModeS > ModeSAge;
	RDFResolutionAdvisory ResolutionAdvisory;

	vector < unsigned char >WarningError;

	RDFTargetCharacteristics *TargetCharacteristics;
	RDFCommCapabilityAndFlightStatus *CommCapabilityAndFlightStatus;

	RDFDopplerSpeed DopplerSpeed;
	RDFTargetComposition TargetComposition;
	RDFTargetDetections TargetDetections;

	vector < RDFSifDetections > SifDetections;
	vector < RDFModeSDetections > ModeSDetections;
	vector < RDFUvdDetections > UvdDetections;

	string UserText;

	bool TransponderCapabilityPresent:1;
	bool TransponderCapabilityNotCapableSI:1;
	unsigned char TransponderCapabilityCA:3;

	RDFTargetPlot ();
	~RDFTargetPlot ();
	RDFTargetPlot (const RDFTargetPlot & plot);
	RDFTargetPlot (const RDFTargetPlot * plot);
	const RDFTargetPlot & operator = (const RDFTargetPlot & plot);

	bool datalink_capability_present (void) const;
	RDFTargetModeS datalink_capability (void) const;
	void datalink_capability (const RDFTargetModeS & modes);
	unsigned char modes_subnetwork (void) const;

	static vector < string > getkeys ();
	string get (string key);

protected:
	void init ();
	void erase ();
	void assign (const RDFTargetPlot & plot);
};

#endif /* _RDF_TARGET_PLOT_H */
