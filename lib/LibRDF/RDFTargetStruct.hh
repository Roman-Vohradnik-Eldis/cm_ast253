#ifndef _RDF_TARGET_H
#define _RDF_TARGET_H

#include "RDFHeader.hh"

enum WARNING_ERROR
{
	WARNING_ERROR_NOT_DEFINED,
	WARNING_ERROR_MULTIPATH_REPLAY,
	WARNING_ERROR_REPLAY_DUE_TO_SIDELOBE_INTERROGATION_RECEPTION,
	WARNING_ERROR_SPLIT_PLOT,
	WARNING_ERROR_SECOND_TIME_AROUND_REPLAY,
	WARNING_ERROR_ANGEL,
	WARNING_ERROR_SLOW_MOVING_TARGET_CORRELATED_WITH_ROAD_INFRASTRUCTURE,
	WARNING_ERROR_FIXED_PSR_PLOT,
	WARNING_ERROR_SLOW_PSR_TARGET,
	WARNING_ERROR_LOW_QUALITY_PSR_PLOT,
	WARNING_ERROR_PHANTOM_SSR_PLOT,
	WARNING_ERROR_NON_MATCHING_MODE_3_A_CODE,
	WARNING_ERROR_MODE_C_S_ALTITUDE_CODE_ABNORMAL_VALUE_COMPARED_TO_THE_TRACK,
	WARNING_ERROR_TARGET_IN_CLUTTER_AREA,
	WARNING_ERROR_MAXIMUM_DOPPLER_RESPONSE_IN_ZERO_FILTER,
	WARNING_ERROR_TRANSPONDER_ANOMALLY_DETECTED,
	WARNING_ERROR_DUPLICATED_OR_ILLEGAL_MODE_S_AIRCRAFT_ADDRESS,
	WARNING_ERROR_MODE_S_ERROR_CORRECTION_APPLIED,
	WARNING_ERROR_UNDECODABLE_MODE_C_S_ALTITUDE_CODE,
	WARNING_ERROR_BIRDS,
	WARNING_ERROR_FLOCK_OF_BIRDS,
	WARNING_ERROR_MODE_1_WAS_PRESENT_IN_ORIGINAL_REPLY,
	WARNING_ERROR_MODE_2_WAS_PRESENT_IN_ORIGINAL_REPLY,
	WARNING_ERROR_PLOT_POTENTIALLY_CAUSED_BY_WIND_TURBINE,
};

struct RDFUvdAltitude
{
	bool Present:1;
	uint32_t Code;
	bool get_altitude_relative ();
	int32_t get_altitude ();
	uint32_t get_fuel ();
	bool get_emergency ();
};

struct RDFUvdSpeed
{
	bool Present:1;
	uint32_t Code;
	float get_speed ();
	float get_heading ();
};

struct RDFUvdCode
{
	bool Present:1;
	uint32_t Code;
	uint32_t get_identification ();
};

struct RDFUvdFriendStatus
{
	bool Present:1;
	uint8_t Code;
};

struct RDFTime
{
	unsigned Time:31;			// milisec
	unsigned Present:1;

	  RDFTime ();
	void init ();
};

unsigned RDF_time_to_asterix_time(unsigned long long time_msec);

struct RDFPosition3D
{
	double Height;				// vyska v metrech
	double Latitude;			// zemepisna sirka v radianech
	double Longitude;			// zemepisna delka v radianech

	RDFPosition3D ();
	~RDFPosition3D ();
	RDFPosition3D (const RDFPosition3D & position);
	RDFPosition3D (const RDFPosition3D * position);
	void init ();
	bool equal (const RDFPosition3D & position);
	bool operator == (const RDFPosition3D & position)
	{
		return equal (position);
	}
	bool operator != (const RDFPosition3D & position)
	{
		return !equal (position);
	}

};

struct RDFAircraftAddress
{
	unsigned Address:31;
	unsigned Present:1;

	RDFAircraftAddress ();
	void init ();
};

struct RDFTargetMode
{
	unsigned short Code:12;
	unsigned short unused:1;
	unsigned short Tracked:1;
	unsigned short Garbled:1;
	unsigned short Valid:1;
	unsigned short ConfidenceCode:12;
	unsigned short ConfidencePresent:1;
	unsigned Age;				// milisec

	RDFTargetMode ();
	~RDFTargetMode ();
	RDFTargetMode (RDFTargetMode * mode);
};

struct RDFTargetFlightLevel
{
	bool Valid:1;
	bool Garbled:1;
	unsigned Age;				// milisec
	double Height;

	RDFTargetFlightLevel ();
	~RDFTargetFlightLevel ();
	RDFTargetFlightLevel (RDFTargetFlightLevel * level);
};

struct RDFTargetHeight
{
	bool QualityPresent:1;
	bool QNH:1;
	unsigned Quality;
	unsigned Age;				// milisec
	double Height;

	RDFTargetHeight ();
	~RDFTargetHeight ();
	RDFTargetHeight (double height);
	RDFTargetHeight (RDFTargetHeight * height);
};

struct RDFTrackNumber
{
	unsigned Number:30;
	unsigned Temporary:1;
	unsigned Present:1;

	RDFTrackNumber ();
	void init ();
};

struct RDFTargetGroundSpeed
{
	double Speed;				// meter/sec
	double Heading;				// rad

	RDFTargetGroundSpeed ();
	~RDFTargetGroundSpeed ();
	RDFTargetGroundSpeed (double speed, double heading);
	RDFTargetGroundSpeed (RDFTargetGroundSpeed * speed);
	void init ();
};

struct RDFExpectedBDS
{
	uint8_t BDS_1;
	uint8_t BDS_2;

	bool present;

	RDFExpectedBDS ();
	void init ();
};

struct RDFDatalinkControl
{
	uint8_t CommATms;
	uint8_t DownElmCtrl;
	uint8_t UpElmCtrl;
	uint8_t CommBCtrl;

	bool present;

	RDFDatalinkControl ();
	void init ();
};

enum TRANS_ACCEL
{
	TRANS_ACCEL_CONSTANT_COURSE,
	TRANS_ACCEL_RIGHT_TURN,
	TRANS_ACCEL_LEFT_TURN,
	TRANS_ACCEL_UNDETERMINED
};

enum LONG_ACCEL
{
	LONG_ACCEL_CONSTANT_GROUNDSPEED,
	LONG_ACCEL_INCREASING_GROUNDSPEED,
	LONG_ACCEL_DECREASING_GROUNDSPEED,
	LONG_ACCEL_UNDETERMINED
};

enum TRACK_CLIMB
{
	TRACK_CLIMB_UNKNOWN,
	TRACK_CLIMB_MAINTAINING,
	TRACK_CLIMB_CLIMBING,
	TRACK_CLIMB_DESCENDING,
};

struct RDFTargetClimbSpeed
{
	double Speed;

	RDFTargetClimbSpeed ();
	~RDFTargetClimbSpeed ();
	RDFTargetClimbSpeed (double speed);
	RDFTargetClimbSpeed (RDFTargetClimbSpeed * speed);
};

#endif /* _RDF_TARGET_H */
