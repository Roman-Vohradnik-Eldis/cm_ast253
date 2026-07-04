#include "RDFAsterix.hh"
#include "RDFTargetPlot.hh"
#include "CMSDebug.hh"

RDFTargetCharacteristics::RDFTargetCharacteristics ()
{
	PsrRunlengthPresent = false;
	PsrRunlength = 0;
	PsrAmplitudePresent = false;
	PsrAmplitude = 0;
	SsrRunlengthPresent = false;
	SsrRunlength = 0;
	SsrAmplitudePresent = false;
	SsrAmplitude = 0;
	SsrRepliesPresent = false;
	SsrReplies = 0;
	PsrSsrRangeDiffPresent = false;
	PsrSsrRangeDiff = 0;
	PsrSsrAzimuthDiffPresent = false;
	PsrSsrAzimuthDiff = 0;
}

RDFTargetCharacteristics::~RDFTargetCharacteristics ()
{
}

RDFTargetCharacteristics::RDFTargetCharacteristics (RDFTargetCharacteristics *
	characteristics)
{
	if (!characteristics)
	{
		INTERNAL_ERROR ("null pointer");
	}

	PsrAmplitudePresent = characteristics->PsrAmplitudePresent;
	PsrAmplitude = characteristics->PsrAmplitude;
	PsrRunlengthPresent = characteristics->PsrRunlengthPresent;
	PsrRunlength = characteristics->PsrRunlength;
	SsrAmplitudePresent = characteristics->SsrAmplitudePresent;
	SsrAmplitude = characteristics->SsrAmplitude;
	SsrRunlengthPresent = characteristics->SsrRunlengthPresent;
	SsrRunlength = characteristics->SsrRunlength;
	SsrRepliesPresent = characteristics->SsrRepliesPresent;
	SsrReplies = characteristics->SsrReplies;
	PsrSsrRangeDiffPresent = characteristics->PsrSsrRangeDiffPresent;
	PsrSsrRangeDiff = characteristics->PsrSsrRangeDiff;
	PsrSsrAzimuthDiffPresent = characteristics->PsrSsrAzimuthDiffPresent;
	PsrSsrAzimuthDiff = characteristics->PsrSsrAzimuthDiff;
}

RDFCommCapabilityAndFlightStatus::RDFCommCapabilityAndFlightStatus ()
{
	CommCapability = COMM_CAPABILITY_NONE;
	FlightStatus = FLIGHT_STATUS_AIRBORNE;
	TransponderCapability = true;
	ModeSSpecificService = false;
	AltitudeCapability = false;
	IdentificationCapability = false;
	B1A = false;
	B1B = 0;
	ACASOperational = false;
	MultipleAids = false;
	DifferentialCorrection = false;
}

RDFCommCapabilityAndFlightStatus::~RDFCommCapabilityAndFlightStatus ()
{
}

RDFCommCapabilityAndFlightStatus::
RDFCommCapabilityAndFlightStatus (RDFCommCapabilityAndFlightStatus * status)
{
	if (!status)
	{
		INTERNAL_ERROR ("null pointer");
	}

	CommCapability = status->CommCapability;
	FlightStatus = status->FlightStatus;
	TransponderCapability = status->TransponderCapability;
	ModeSSpecificService = status->ModeSSpecificService;
	AltitudeCapability = status->AltitudeCapability;
	IdentificationCapability = status->IdentificationCapability;
	B1A = status->B1A;
	B1B = status->B1B;
	ACASOperational = status->ACASOperational;
	MultipleAids = status->MultipleAids;
	DifferentialCorrection = status->DifferentialCorrection;
}

void RDFResolutionAdvisory::init ()
{
	Present = false;
	memset (Data, 0, sizeof (Data));
}

void RDFDopplerSpeed::init ()
{
	RawPresent = false;
	CalculatedPresent = false;
	CalculatedValid = false;
	DopplerSpeed = 0;
	AmbiguityRange = 0;
	TransmitterFrequency = 0;
	CalculatedSpeed = 0;
}

void RDFTargetComposition::init ()
{
	Mode3A = CODE_SOURCE_NONE;
	ModeC = CODE_SOURCE_NONE;
	Position = POSITION_SOURCE_NONE;
	Detection = DETECTION_SOURCE_NONE;
}

void RDFTargetDetections::init ()
{
	SifMode3APresent = false;
	SifModeCPresent = false;
	AllCallPresent = false;
	RollCallMode3APresent = false;
	RollCallModeCPresent = false;
	AdsPresent = false;

	SifMode3ACount = false;
	SifModeCCount = false;
	AllCallCount = false;
	RollCallMode3ACount = false;
	RollCallModeCCount = false;
	AdsCount = false;

	SifMode3AAge = false;
	SifModeCAge = false;
	AllCallAge = false;
	RollCallMode3AAge = false;
	RollCallModeCAge = false;
	AdsAge = false;
}

RDFTargetPlot::RDFTargetPlot ()
{
	init ();
}

RDFTargetPlot::~RDFTargetPlot ()
{
	erase ();
}

RDFTargetPlot::RDFTargetPlot (const RDFTargetPlot & plot)
{
	init ();


	assign (plot);
}

RDFTargetPlot::RDFTargetPlot (const RDFTargetPlot * plot)
{
	init ();
	if (plot)
	{
		assign (*plot);
	}
}

const RDFTargetPlot & RDFTargetPlot::operator = (const RDFTargetPlot & plot)
{
	erase ();
	assign (plot);
	return plot;
}

void RDFTargetPlot::init ()
{
	Time.init ();

	CAT = 0;
	SAC = 0;
	SIC = 0;

	TargetType = TARGET_TYPE_UNKNOWN;
	TargetFoeFri = TARGET_FOEFRI_NO_MODE4;

	Simulated = false;
	Tested = false;
	SPI = false;
	RAB = false;
	ME = false;
	MI = false;

	Polar = NULL;

	Mode1 = NULL;
	Mode2 = NULL;
	Mode3A = NULL;
	ModeB = NULL;
	ModeC = NULL;
	ModeD = NULL;

	FlightLevel = NULL;
	MeasuredHeight = NULL;

	AircraftAddress.init ();
	AircraftIdentification.clear ();

	ModeS.clear ();
	ModeSAge.clear ();
	ResolutionAdvisory.init ();

	WarningError.clear ();

	TargetCharacteristics = NULL;
	CommCapabilityAndFlightStatus = NULL;

	DopplerSpeed.init ();
	TargetComposition.init ();
	TargetDetections.init ();

	TransponderCapabilityPresent = false;
	TransponderCapabilityNotCapableSI = true;
	TransponderCapabilityCA = 0;

	SifDetections.clear ();
	ModeSDetections.clear ();
	UvdDetections.clear ();

	UserText.clear ();

	UvdFriendStatus.Code = 0;
	UvdFriendStatus.Present = 0;

	UvdCode.Code = 0;
	UvdCode.Present = 0;

	UvdAltitude.Code = 0;
	UvdAltitude.Present = 0;


	UvdSpeed.Code = 0;
	UvdSpeed.Present = 0;

}

void RDFTargetPlot::erase ()
{
	Time.init ();

	CAT = 0;
	SAC = 0;
	SIC = 0;

	TargetType = TARGET_TYPE_UNKNOWN;
	TargetFoeFri = TARGET_FOEFRI_NO_MODE4;

	Simulated = false;
	Tested = false;
	SPI = false;
	RAB = false;
	ME = false;
	MI = false;

	if (Polar)
	{
		delete Polar;
		Polar = NULL;
	}

	if (Mode1)
	{
		delete Mode1;
		Mode1 = NULL;
	}
	if (Mode2)
	{
		delete Mode2;
		Mode2 = NULL;
	}
	if (Mode3A)
	{
		delete Mode3A;
		Mode3A = NULL;
	}
	if (ModeB)
	{
		delete ModeB;
		ModeB = NULL;
	}
	if (ModeC)
	{
		delete ModeC;
		ModeC = NULL;
	}
	if (ModeD)
	{
		delete ModeD;
		ModeD = NULL;
	}

	if (FlightLevel)
	{
		delete FlightLevel;
		FlightLevel = NULL;
	}
	if (MeasuredHeight)
	{
		delete MeasuredHeight;
		MeasuredHeight = NULL;
	}

	AircraftAddress.init ();
	AircraftIdentification.erase ();

	ModeS.clear ();
	ModeSAge.clear ();
	ResolutionAdvisory.init ();

	WarningError.clear ();

	if (TargetCharacteristics)
	{
		delete TargetCharacteristics;
		TargetCharacteristics = NULL;
	}
	if (CommCapabilityAndFlightStatus)
	{
		delete CommCapabilityAndFlightStatus;
		CommCapabilityAndFlightStatus = NULL;
	}

	DopplerSpeed.init ();
	TargetComposition.init ();
	TargetDetections.init ();

	TransponderCapabilityPresent = false;
	TransponderCapabilityNotCapableSI = true;
	TransponderCapabilityCA = 0;

	SifDetections.clear ();
	ModeSDetections.clear ();
	UvdDetections.clear ();

	UserText.erase ();

	UvdFriendStatus.Code = 0;
	UvdFriendStatus.Present = 0;

	UvdCode.Code = 0;
	UvdCode.Present = 0;

	UvdAltitude.Code = 0;
	UvdAltitude.Present = 0;


	UvdSpeed.Code = 0;
	UvdSpeed.Present = 0;

}

void RDFTargetPlot::assign (const RDFTargetPlot & plot)
{
	Time = plot.Time;

	CAT = plot.CAT;
	SAC = plot.SAC;
	SIC = plot.SIC;

	TargetType = plot.TargetType;
	TargetFoeFri = plot.TargetFoeFri;

	Simulated = plot.Simulated;
	Tested = plot.Tested;
	SPI = plot.SPI;
	RAB = plot.RAB;
	ME = plot.ME;
	MI = plot.MI;

	if (plot.Polar)
	{
		Polar = new RDFCoorPolar (plot.Polar);
	}

	if (plot.Mode1)
	{
		Mode1 = new RDFTargetMode (plot.Mode1);
	}
	if (plot.Mode2)
	{
		Mode2 = new RDFTargetMode (plot.Mode2);
	}
	if (plot.Mode3A)
	{
		Mode3A = new RDFTargetMode (plot.Mode3A);
	}
	if (plot.ModeB)
	{
		ModeB = new RDFTargetMode (plot.ModeB);
	}
	if (plot.ModeC)
	{
		ModeC = new RDFTargetMode (plot.ModeC);
	}
	if (plot.ModeD)
	{
		ModeD = new RDFTargetMode (plot.ModeD);
	}

	if (plot.FlightLevel)
	{
		FlightLevel = new RDFTargetFlightLevel (plot.FlightLevel);
	}
	if (plot.MeasuredHeight)
	{
		MeasuredHeight = new RDFTargetHeight (plot.MeasuredHeight);
	}

	AircraftAddress = plot.AircraftAddress;
	AircraftIdentification = plot.AircraftIdentification;

	ModeS = plot.ModeS;
	ModeSAge = plot.ModeSAge;
	ResolutionAdvisory = plot.ResolutionAdvisory;

	WarningError = plot.WarningError;

	if (plot.TargetCharacteristics)
	{
		TargetCharacteristics =
			new RDFTargetCharacteristics (plot.TargetCharacteristics);
	}
	if (plot.CommCapabilityAndFlightStatus)
	{
		CommCapabilityAndFlightStatus =
			new RDFCommCapabilityAndFlightStatus (plot.CommCapabilityAndFlightStatus);
	}

	DopplerSpeed = plot.DopplerSpeed;
	TargetComposition = plot.TargetComposition;
	TargetDetections = plot.TargetDetections;

	TransponderCapabilityPresent = plot.TransponderCapabilityPresent;
	TransponderCapabilityNotCapableSI = plot.TransponderCapabilityNotCapableSI;
	TransponderCapabilityCA = plot.TransponderCapabilityCA;


	SifDetections = plot.SifDetections;
	ModeSDetections = plot.ModeSDetections;
	UvdDetections = plot.UvdDetections;

	UserText = plot.UserText;

	UvdFriendStatus = plot.UvdFriendStatus;
	UvdCode = plot.UvdCode;
	UvdAltitude = plot.UvdAltitude;
	UvdSpeed = plot.UvdSpeed;
}

bool RDFTargetPlot::datalink_capability_present (void) const 
{
	return ((std::find_if (ModeSAge.begin (), ModeSAge.end (),
				is_datalink_capability) != ModeSAge.end ()) ||
		(std::find_if (ModeS.begin (), ModeS.end (),
				is_datalink_capability) != ModeS.end ()));
}

RDFTargetModeS RDFTargetPlot::datalink_capability (void) const 
{
	std::vector < RDFTargetModeS >::const_iterator i;
	i = std::find_if (ModeSAge.begin (), ModeSAge.end (), is_datalink_capability);

	if (i != ModeSAge.end ())
		return *i;

	/* in case of SP removal */
	i = std::find_if (ModeS.begin (), ModeS.end (), is_datalink_capability);

	if (i != ModeS.end ())
		return *i;

	/* fallback */
	RDFTargetModeS res;
	res.MessageData[0] = res.Address = 0x10;
	return res;
}

void RDFTargetPlot::datalink_capability (const RDFTargetModeS & modes)
{
	if (datalink_capability_present ())
		return;

	ModeSAge.push_back (modes);
}

unsigned char RDFTargetPlot::modes_subnetwork (void) const 
{
	std::vector < RDFTargetModeS >::const_iterator i;
	i = std::find_if (ModeSAge.begin (), ModeSAge.end (), is_datalink_capability);
	if (i != ModeSAge.end ())
		return i->MessageData[2] >> 1;

	i = std::find_if (ModeS.begin (), ModeS.end (), is_datalink_capability);
	if (i != ModeS.end ())
		return i->MessageData[2] >> 1;

	return 0;
}

vector < string > RDFTargetPlot::getkeys ()
{
	vector < string > keys;

	keys.push_back ("TIME");
	keys.push_back ("TARGET_TYPE");
	keys.push_back ("AIRCRAFT_ADDRESS");
	keys.push_back ("AIRCRAFT_IDENTIFICATION");
	keys.push_back ("MODE_1 IGT");
	keys.push_back ("MODE_2 IGT");
	keys.push_back ("MODE_3A IGT");
	keys.push_back ("MODE_4");
	keys.push_back ("FLIGHT_LEVEL 25 M IG NOINV");
	keys.push_back ("MEASURED_HEIGHT M NOINV");
	keys.push_back ("MAGNETIC_HEADING");
	keys.push_back ("TRUE_TRACK_ANGLE");
	keys.push_back ("INDICATED_AIRSPEED");
	keys.push_back ("MACH_NUMBER");
	keys.push_back ("TRUE_AIRSPEED");
	keys.push_back ("MCP_FCU_SELECTED_ALTITUDE");
	keys.push_back ("USER_TEXT");
	keys.push_back ("SRL");
	keys.push_back ("SRR");
	keys.push_back ("SAM");
	keys.push_back ("PRL");
	keys.push_back ("PAM");
	keys.push_back ("RPD");
	keys.push_back ("APD");
	keys.push_back ("WARNING_ERROR");
	keys.push_back ("BDS");
	keys.push_back ("RHO KM .3X");
	keys.push_back ("THETA");
	keys.push_back ("COMPOSITION_MODE_3A");
	keys.push_back ("COMPOSITION_MODE_C");
	keys.push_back ("COMPOSITION_POSITION");
	keys.push_back ("COMPOSITION_DETECTION");
	keys.push_back ("SSR_AMPLITUDE");
	keys.push_back ("UVD_IDENTIFICATION");
	keys.push_back ("UVD_ALTITUDE");
	keys.push_back ("UVD_SPEED");
	keys.push_back ("UVD_HEADING");
	keys.push_back ("UVD_EMERGENCY");
	keys.push_back ("UVD_FUEL");
	keys.push_back ("UVD_FRIEND");

	return keys;
}

string RDFTargetPlot::get (string key)
{
	char text[32];
	string str;
	double roll_angle;
	double true_track_angle;
	double ground_speed;
	double track_angle_rate;
	double true_airspeed;
	double magnetic_heading;
	double indicated_airspeed;
	double mach_number;
	double altitude_rate;
	double vertical_velocity;
	unsigned short int mcp_altitude;
	unsigned short int fms_altitude;
	double barometric_pressure_setting;
	bool havemodes;
	bool vnav_mode;
	bool alt_hold_mode;
	bool approach_mode;
	bool altitude_source;
	unsigned char tgt_alt_src;

	key.push_back (' ');

	// pokud nebudeme mit roll-call, vratime prazdnou hodnotu
	if (key.find (" ROLLCALL ") != string::npos
		&& TargetType != TARGET_TYPE_MODES_ROLL_CALL
		&& TargetType != TARGET_TYPE_MODES_ROLL_CALL_PSR)
	{
		return string ();
	}

	if (!key.compare (0, 5, "TIME "))
	{
		sprintf (text, "%02u:%02u:%02u", Time.Time / 3600000,
			(Time.Time / 60000) % 60, (Time.Time / 1000) % 60);
		return string (text);
	}

	else if (!key.compare (0, 12, "TARGET_TYPE "))
	{
		switch (TargetType)
		{
			case TARGET_TYPE_UNKNOWN:
			{
				return string ();
			}
			case TARGET_TYPE_PRIMARY:
			{
				return string ("PSR");
			}
			case TARGET_TYPE_SECONDARY:
			{
				return string ("SSR");
			}
			case TARGET_TYPE_COMBINED:
			{
				return string ("PSR SSR");
			}
			case TARGET_TYPE_MODES_ALL_CALL:
			{
				return string ("ALL");
			}
			case TARGET_TYPE_MODES_ROLL_CALL:
			{
				return string ("ROLL");
			}
			case TARGET_TYPE_MODES_ALL_CALL_PSR:
			{
				return string ("PSR ALL");
			}
			case TARGET_TYPE_MODES_ROLL_CALL_PSR:
			{
				return string ("PSR ROLL");
			}
			case TARGET_TYPE_ADSB:
			{
				return string ("ADSB");
			}
		}
		return string ();
	}

	else if (!key.compare (0, 17, "AIRCRAFT_ADDRESS "))
	{
		if (AircraftAddress.Present)
		{
			sprintf (text, "%06X", AircraftAddress.Address);
			return string (text);
		}
	}

	else if (!key.compare (0, 24, "AIRCRAFT_IDENTIFICATION "))
	{
		if (AircraftIdentification.size ())
		{
			return AircraftIdentification;
		}
	}

	else if (!key.compare (0, 7, "MODE_1 "))
	{
		if (Mode1)
		{
			sprintf (text, "%02d", asterix_decode_mode123 (Mode1->Code));

			if (key.find (" IGT ") != string::npos)
			{
				if (!Mode1->Valid)
				{
					strcat (text, "i");
				}
				if (Mode1->Garbled)
				{
					strcat (text, "g");
				}
				if (Mode1->Tracked)
				{
					strcat (text, "t");
				}
			}
			else
			{
				if (!Mode1->Valid)
				{
					return string ("INV");
				}
			}

			return string (text);
		}
	}

	else if (!key.compare (0, 7, "MODE_2 "))
	{
		if (Mode2)
		{
			sprintf (text, "%04d", asterix_decode_mode123 (Mode2->Code));

			if (key.find (" IGT ") != string::npos)
			{
				if (!Mode2->Valid)
				{
					strcat (text, "i");
				}
				if (Mode2->Garbled)
				{
					strcat (text, "g");
				}
				if (Mode2->Tracked)
				{
					strcat (text, "t");
				}
			}
			else
			{
				if (!Mode2->Valid)
				{
					return string ("INV");
				}
			}

			return string (text);
		}
	}

	else if (!key.compare (0, 8, "MODE_3A "))
	{
		if (Mode3A)
		{
			sprintf (text, "%04d", asterix_decode_mode123 (Mode3A->Code));

			if (key.find (" IGT ") != string::npos)
			{
				if (!Mode3A->Valid)
				{
					strcat (text, "i");
				}
				if (Mode3A->Garbled)
				{
					strcat (text, "g");
				}
				if (Mode3A->Tracked)
				{
					strcat (text, "t");
				}
			}
			else
			{
				if (!Mode3A->Valid)
				{
					return string ("INV");
				}
			}

			return string (text);
		}
	}

	else if (!key.compare (0, 7, "MODE_4 "))
	{
		if (TargetFoeFri == TARGET_FOEFRI_FRIENDLY_TARGET)
		{
			return string ("FRI");
		}
		if (TargetFoeFri == TARGET_FOEFRI_UNKNOWN_TARGET)
		{
			return string ("UKN");
		}
	}

	else if (!key.compare (0, 13, "FLIGHT_LEVEL "))
	{
		if (FlightLevel)
		{
			bool inv = (key.find (" NOINV ") == string::npos);

			if (key.find (" M ") != string::npos)
			{
				long m = lrint (FlightLevel->Height);
				if (inv && (m < -9999 || m > 99999))
				{
					return string ("INV");
				}
				sprintf (text, "%05ld", m);
			}
			else if (key.find (" 25 ") != string::npos)
			{
				if (CommCapabilityAndFlightStatus
					&& CommCapabilityAndFlightStatus->AltitudeCapability)
				{
					int fl = (int) rint (FlightLevel->Height / 0.3048);
					if (inv && (fl <= -10000 || fl >= 100000))
					{
						return string ("INV");
					}
					sprintf (text, "%03d.%02u", fl / 100, abs (fl % 100));
				}
				else
				{
					int fl = (int) rint (FlightLevel->Height / 30.48);
					if (inv && (fl <= -100 || fl >= 1000))
					{
						return string ("INV");
					}
					sprintf (text, "%03d", fl);
				}
			}
			else
			{
				int fl = (int) rint (FlightLevel->Height / 30.48);
				if (inv && (fl <= -100 || fl >= 1000))
				{
					return string ("INV");
				}
				sprintf (text, "%03d", fl);
			}

			if (key.find (" IG ") != string::npos)
			{
				if (!FlightLevel->Valid)
				{
					strcat (text, "i");
				}
				if (FlightLevel->Garbled)
				{
					strcat (text, "g");
				}
			}
			else
			{
				if (!FlightLevel->Valid)
				{
					return string ("INV");
				}
			}

			return string (text);
		}
	}

	else if (!key.compare (0, 16, "MEASURED_HEIGHT "))
	{
		if (MeasuredHeight)
		{
			bool inv = (key.find (" NOINV ") == string::npos);

			if (key.find (" M ") != string::npos)
			{
				long m = lrint (MeasuredHeight->Height);
				if (inv && (m < -9999 || m > 99999))
				{
					return string ("INV");
				}
				sprintf (text, "%05ld", m);
			}
			else
			{
				int fl = (int) rint (MeasuredHeight->Height / 30.48);
				if (inv && (fl <= -100 || fl >= 1000))
				{
					return string ("INV");
				}
				sprintf (text, "%03d", fl);
			}

			return string (text);

		}
	}

	else if (!key.compare (0, 17, "MAGNETIC_HEADING "))
	{
		for (unsigned i = 0; i < ModeS.size (); i++)
		{
			if (ModeS[i].Address == 0x53)
			{
				ModeS[i].Get53 (magnetic_heading, indicated_airspeed,
					mach_number, true_airspeed, altitude_rate);
				sprintf (text, "%03d", (int) rint (magnetic_heading * 180 / M_PI));
				return string (text);
			}
			else if (ModeS[i].Address == 0x60)
			{
				ModeS[i].Get60 (magnetic_heading, indicated_airspeed,
					mach_number, altitude_rate, vertical_velocity);
				sprintf (text, "%03d", (int) rint (magnetic_heading * 180 / M_PI));
				return string (text);
			}
		}
		for (unsigned i = 0; i < ModeSAge.size (); i++)
		{
			if (ModeSAge[i].Address == 0x53)
			{
				ModeSAge[i].Get53 (magnetic_heading, indicated_airspeed,
					mach_number, true_airspeed, altitude_rate);
				sprintf (text, "%03d", (int) rint (magnetic_heading * 180 / M_PI));
				return string (text);
			}
			else if (ModeSAge[i].Address == 0x60)
			{
				ModeSAge[i].Get60 (magnetic_heading, indicated_airspeed,
					mach_number, altitude_rate, vertical_velocity);
				sprintf (text, "%03d", (int) rint (magnetic_heading * 180 / M_PI));
				return string (text);
			}
		}
	}

	else if (!key.compare (0, 17, "TRUE_TRACK_ANGLE "))
	{
		for (unsigned i = 0; i < ModeS.size (); i++)
		{
			if (ModeS[i].Address == 0x50)
			{
				ModeS[i].Get50 (roll_angle, true_track_angle,
					ground_speed, track_angle_rate, true_airspeed);
				sprintf (text, "%03d", (int) rint (true_track_angle * 180 / M_PI));
				return string (text);
			}
		}
		for (unsigned i = 0; i < ModeSAge.size (); i++)
		{
			if (ModeSAge[i].Address == 0x50)
			{
				ModeSAge[i].Get50 (roll_angle, true_track_angle,
					ground_speed, track_angle_rate, true_airspeed);
				sprintf (text, "%03d", (int) rint (true_track_angle * 180 / M_PI));
				return string (text);
			}
		}
	}

	else if (!key.compare (0, 19, "INDICATED_AIRSPEED "))
	{
		for (unsigned i = 0; i < ModeS.size (); i++)
		{
			if (ModeS[i].Address == 0x53)
			{
				ModeS[i].Get53 (magnetic_heading, indicated_airspeed,
					mach_number, true_airspeed, altitude_rate);
				int speed = (int) rint (indicated_airspeed * 3.6 / 1.852 / 10);
				if (speed < 0 || speed >= 100)
				{
					return string ("IN");
				}
				sprintf (text, "%02d", speed);
				return string (text);
			}
			else if (ModeS[i].Address == 0x60)
			{
				ModeS[i].Get60 (magnetic_heading, indicated_airspeed,
					mach_number, altitude_rate, vertical_velocity);
				int speed = (int) rint (indicated_airspeed * 3.6 / 1.852 / 10);
				if (speed < 0 || speed >= 100)
				{
					return string ("IN");
				}
				sprintf (text, "%02d", speed);
				return string (text);
			}
		}
		for (unsigned i = 0; i < ModeSAge.size (); i++)
		{
			if (ModeSAge[i].Address == 0x53)
			{
				ModeSAge[i].Get53 (magnetic_heading, indicated_airspeed,
					mach_number, true_airspeed, altitude_rate);
				int speed = (int) rint (indicated_airspeed * 3.6 / 1.852 / 10);
				if (speed < 0 || speed >= 100)
				{
					return string ("IN");
				}
				sprintf (text, "%02d", speed);
				return string (text);
			}
			else if (ModeSAge[i].Address == 0x60)
			{
				ModeSAge[i].Get60 (magnetic_heading, indicated_airspeed,
					mach_number, altitude_rate, vertical_velocity);
				int speed = (int) rint (indicated_airspeed * 3.6 / 1.852 / 10);
				if (speed < 0 || speed >= 100)
				{
					return string ("IN");
				}
				sprintf (text, "%02d", speed);
				return string (text);
			}
		}
	}

	else if (!key.compare (0, 12, "MACH_NUMBER "))
	{
		for (unsigned i = 0; i < ModeS.size (); i++)
		{
			if (ModeS[i].Address == 0x53)
			{
				ModeS[i].Get53 (magnetic_heading, indicated_airspeed,
					mach_number, true_airspeed, altitude_rate);
				sprintf (text, "M%02d", (int) rint (mach_number * 10));
				return string (text);
			}
			else if (ModeS[i].Address == 0x60)
			{
				ModeS[i].Get60 (magnetic_heading, indicated_airspeed,
					mach_number, altitude_rate, vertical_velocity);
				sprintf (text, "M%02d", (int) rint (mach_number * 10));
				return string (text);
			}
		}
		for (unsigned i = 0; i < ModeSAge.size (); i++)
		{
			if (ModeSAge[i].Address == 0x53)
			{
				ModeSAge[i].Get53 (magnetic_heading, indicated_airspeed,
					mach_number, true_airspeed, altitude_rate);
				sprintf (text, "M%02d", (int) rint (mach_number * 10));
				return string (text);
			}
			else if (ModeSAge[i].Address == 0x60)
			{
				ModeSAge[i].Get60 (magnetic_heading, indicated_airspeed,
					mach_number, altitude_rate, vertical_velocity);
				sprintf (text, "M%02d", (int) rint (mach_number * 10));
				return string (text);
			}
		}
	}

	else if (!key.compare (0, 14, "TRUE_AIRSPEED "))
	{
		for (unsigned i = 0; i < ModeS.size (); i++)
		{
			if (ModeS[i].Address == 0x50)
			{
				ModeS[i].Get50 (roll_angle, true_track_angle,
					ground_speed, track_angle_rate, true_airspeed);
				int speed = (int) rint (true_airspeed * 3.6 / 1.852 / 10);
				if (speed < 0 || speed >= 100)
				{
					return string ("IN");
				}
				sprintf (text, "%02d", speed);
				return string (text);
			}
			else if (ModeS[i].Address == 0x53)
			{
				ModeS[i].Get53 (magnetic_heading, indicated_airspeed,
					mach_number, true_airspeed, altitude_rate);
				int speed = (int) rint (true_airspeed * 3.6 / 1.852 / 10);
				if (speed < 0 || speed >= 100)
				{
					return string ("IN");
				}
				sprintf (text, "%02d", speed);
				return string (text);
			}
		}
		for (unsigned i = 0; i < ModeSAge.size (); i++)
		{
			if (ModeSAge[i].Address == 0x50)
			{
				ModeSAge[i].Get50 (roll_angle, true_track_angle,
					ground_speed, track_angle_rate, true_airspeed);
				int speed = (int) rint (true_airspeed * 3.6 / 1.852 / 10);
				if (speed < 0 || speed >= 100)
				{
					return string ("IN");
				}
				sprintf (text, "%02d", speed);
				return string (text);
			}
			else if (ModeSAge[i].Address == 0x53)
			{
				ModeSAge[i].Get53 (magnetic_heading, indicated_airspeed,
					mach_number, true_airspeed, altitude_rate);
				int speed = (int) rint (true_airspeed * 3.6 / 1.852 / 10);
				if (speed < 0 || speed >= 100)
				{
					return string ("IN");
				}
				sprintf (text, "%02d", speed);
				return string (text);
			}
		}
	}

	else if (!key.compare (0, 26, "MCP_FCU_SELECTED_ALTITUDE "))
	{
		for (unsigned i = 0; i < ModeS.size (); i++)
		{
			if (ModeS[i].Address == 0x40)
			{
				ModeS[i].Get40 (mcp_altitude, fms_altitude, 
					barometric_pressure_setting, havemodes,
					vnav_mode, alt_hold_mode, approach_mode, 
					altitude_source, tgt_alt_src);
				sprintf (text, "CL%03ld", lrint (mcp_altitude / 100));
				return string (text);
			}
		}
		for (unsigned i = 0; i < ModeSAge.size (); i++)
		{
			if (ModeSAge[i].Address == 0x40)
			{
				ModeSAge[i].Get40 (mcp_altitude, fms_altitude, 
					barometric_pressure_setting, havemodes,
					vnav_mode, alt_hold_mode, approach_mode, 
					altitude_source, tgt_alt_src);
				sprintf (text, "CL%03ld", lrint (mcp_altitude / 100));
				return string (text);
			}
		}
	}

	else if (!key.compare (0, 10, "USER_TEXT "))
	{
		return UserText;
	}

	else if (!key.compare (0, 4, "SRL "))
	{
		if (TargetCharacteristics && TargetCharacteristics->SsrRunlengthPresent)
		{
			sprintf (text, "%.03lf", TargetCharacteristics->SsrRunlength
				* 360.0 / (1 << 13));
			return string (text);
		}
	}

	else if (!key.compare (0, 4, "SRR "))
	{
		if (TargetCharacteristics && TargetCharacteristics->SsrRepliesPresent)
		{
			sprintf (text, "%u", TargetCharacteristics->SsrReplies);
			return string (text);
		}
	}

	else if (!key.compare (0, 4, "SAM "))
	{
		if (TargetCharacteristics && TargetCharacteristics->SsrAmplitudePresent)
		{
			sprintf (text, "%d", TargetCharacteristics->SsrAmplitude);
			return string (text);
		}
	}

	else if (!key.compare (0, 4, "PRL "))
	{
		if (TargetCharacteristics && TargetCharacteristics->PsrRunlengthPresent)
		{
			sprintf (text, "%.03lf", TargetCharacteristics->PsrRunlength
				* 360.0 / (1 << 13));
			return string (text);
		}
	}

	else if (!key.compare (0, 4, "PAM "))
	{
		if (TargetCharacteristics && TargetCharacteristics->PsrAmplitudePresent)
		{
			sprintf (text, "%d", TargetCharacteristics->PsrAmplitude);
			return string (text);
		}
	}

	else if (!key.compare (0, 4, "RPD "))
	{
		if (TargetCharacteristics && TargetCharacteristics->PsrSsrRangeDiffPresent)
		{
			sprintf (text, "%d", TargetCharacteristics->PsrSsrRangeDiff);
			return string (text);
		}
	}

	else if (!key.compare (0, 4, "APD "))
	{
		if (TargetCharacteristics && TargetCharacteristics->PsrSsrAzimuthDiffPresent)
		{
			sprintf (text, "%.03lf", TargetCharacteristics->PsrSsrAzimuthDiff
				* 360.0 / (1 << 14));
			return string (text);
		}
	}

	else if (!key.compare (0, 14, "WARNING_ERROR "))
	{
		for (unsigned i = 0; i < WarningError.size (); i++)
		{
			if (str.size ())
			{
				str.push_back (32);
			}

			switch (WarningError[i])
			{
				case WARNING_ERROR_NOT_DEFINED:
				{
					str.append ("TRU");
					break;
				}
				case WARNING_ERROR_MULTIPATH_REPLAY:
				{
					str.append ("MUL");
					break;
				}
				case WARNING_ERROR_REPLAY_DUE_TO_SIDELOBE_INTERROGATION_RECEPTION:
				{
					str.append ("SLB");
					break;
				}
				case WARNING_ERROR_SPLIT_PLOT:
				{
					str.append ("SPL");
					break;
				}
				case WARNING_ERROR_SECOND_TIME_AROUND_REPLAY:
				{
					str.append ("TAR");
					break;
				}
				case WARNING_ERROR_ANGEL:
				{
					str.append ("ANG");
					break;
				}
				case WARNING_ERROR_SLOW_MOVING_TARGET_CORRELATED_WITH_ROAD_INFRASTRUCTURE:
				{
					str.append ("VEH");
					break;
				}
				case WARNING_ERROR_FIXED_PSR_PLOT:
				{
					str.append ("FIX");
					break;
				}
				case WARNING_ERROR_SLOW_PSR_TARGET:
				{
					str.append ("SLW");
					break;
				}
				case WARNING_ERROR_LOW_QUALITY_PSR_PLOT:
				{
					str.append ("LQA");
					break;
				}
				case WARNING_ERROR_PHANTOM_SSR_PLOT:
				{
					str.append ("PHA");
					break;
				}
				case WARNING_ERROR_NON_MATCHING_MODE_3_A_CODE:
				{
					str.append ("NM3");
					break;
				}
				case WARNING_ERROR_MODE_C_S_ALTITUDE_CODE_ABNORMAL_VALUE_COMPARED_TO_THE_TRACK:
				{
					str.append ("ABA");
					break;
				}
				case WARNING_ERROR_TARGET_IN_CLUTTER_AREA:
				{
					str.append ("CLT");
					break;
				}
				case WARNING_ERROR_MAXIMUM_DOPPLER_RESPONSE_IN_ZERO_FILTER:
				{
					str.append ("ZDP");
					break;
				}
				case WARNING_ERROR_TRANSPONDER_ANOMALLY_DETECTED:
				{
					str.append ("TAN");
					break;
				}
				case WARNING_ERROR_DUPLICATED_OR_ILLEGAL_MODE_S_AIRCRAFT_ADDRESS:
				{
					str.append ("DUP");
					break;
				}
				case WARNING_ERROR_MODE_S_ERROR_CORRECTION_APPLIED:
				{
					str.append ("SER");
					break;
				}
				case WARNING_ERROR_UNDECODABLE_MODE_C_S_ALTITUDE_CODE:
				{
					str.append ("MCU");
					break;
				}
				case WARNING_ERROR_BIRDS:
				{
					str.append ("BIR");
					break;
				}
				case WARNING_ERROR_FLOCK_OF_BIRDS:
				{
					str.append ("FBR");
					break;
				}
				case WARNING_ERROR_MODE_1_WAS_PRESENT_IN_ORIGINAL_REPLY:
				{
					str.append ("MD1");
					break;
				}
				case WARNING_ERROR_MODE_2_WAS_PRESENT_IN_ORIGINAL_REPLY:
				{
					str.append ("MD2");
					break;
				}
				case WARNING_ERROR_PLOT_POTENTIALLY_CAUSED_BY_WIND_TURBINE:
				{
					str.append ("WTU");
					break;
				}
				default:
				{
					sprintf (text, "#%u", WarningError[i]);
					str.append (text);
				}
			}
		}

		return str;
	}

	else if (!key.compare (0, 4, "BDS "))
	{
		bool bds[256];
		memset (bds, 0, sizeof (bds));

		for (unsigned i = 0; i < ModeS.size (); i++)
		{
			bds[ModeS[i].Address] = true;
		}
		for (unsigned i = 0; i < ModeSAge.size (); i++)
		{
			if (ModeSAge[i].Age < 2)
			{
				bds[ModeSAge[i].Address] = true;
			}
		}

		for (unsigned i = 0; i < 256; i++)
		{
			if (bds[i])
			{
				sprintf (text, "%02X", i);
				if (str.size ())
				{
					str.push_back (32);
				}
				str.append (text);
			}
		}

		return str;
	}

	else if (!key.compare (0, 4, "RHO "))
	{
		if (Polar)
		{
			if (key.find (" KM ") != string::npos)
			{
				if (key.find (" .3X ") != string::npos)
				{
					sprintf (text, "%.03lf", Polar->Rho / 1000);
				}
				else
				{
					sprintf (text, "%.00lf", Polar->Rho / 1000);
				}
			}
			else
			{
				if (key.find (" .3X ") != string::npos)
				{
					sprintf (text, "%.03lf", Polar->Rho / 1852);
				}
				else
				{
					sprintf (text, "%.00lf", Polar->Rho / 1852);
				}
			}
			return string (text);
		}
	}

	else if (!key.compare (0, 6, "THETA "))
	{
		if (Polar)
		{
			sprintf (text, "%03.00lf", Polar->Theta * 180 / M_PI);
			return string (text);
		}
	}

	else if (!key.compare (0, 20, "COMPOSITION_MODE_3A "))
	{
		sprintf (text, "%u", TargetComposition.Mode3A);
		return string (text);
	}

	else if (!key.compare (0, 19, "COMPOSITION_MODE_C "))
	{
		sprintf (text, "%u", TargetComposition.ModeC);
		return string (text);
	}

	else if (!key.compare (0, 21, "COMPOSITION_POSITION "))
	{
		sprintf (text, "%u", TargetComposition.Detection);
		return string (text);
	}

	else if (!key.compare (0, 22, "COMPOSITION_DETECTION "))
	{
		sprintf (text, "%u", TargetComposition.Detection);
		return string (text);
	}

	else if (!key.compare (0, 14, "SSR_AMPLITUDE "))
	{
		unsigned MaxValue = 0;
		if ((SifDetections.size ()) || (ModeSDetections.size ()))
		{
			for (unsigned i = 0; i < SifDetections.size (); i++)
			{
				if (MaxValue < SifDetections[i].SumValue)
					MaxValue = SifDetections[i].SumValue;
			}
			for (unsigned i = 0; i < ModeSDetections.size (); i++)
			{
				if (MaxValue < ModeSDetections[i].SumValue)
					MaxValue = ModeSDetections[i].SumValue;
			}
			sprintf (text, "%02.02lf", (MaxValue / 128.) - 100.);
			return string (text);
		}
		else
		{
			return string ("-1.0");
		}
	}
	else if (!key.compare (0, 19, "UVD_IDENTIFICATION "))
	{
		if (UvdCode.Present)
		{
			sprintf (text, "U%u", UvdCode.get_identification ());
			return string (text);
		}
	}
	else if (!key.compare (0, 13, "UVD_ALTITUDE "))
	{
		if (UvdAltitude.Present)
		{
			sprintf (text, "U%c%d", 
				UvdAltitude.get_altitude_relative () ? 'R' : 'A', 
				UvdAltitude.get_altitude ());
			return string (text);
		}
	}
	else if (!key.compare (0, 10, "UVD_SPEED "))
	{
		if (UvdSpeed.Present)
		{
			sprintf (text, "U%d", (int) rint (UvdSpeed.get_speed ()));
			return string (text);
		}
	}
	else if (!key.compare (0, 12, "UVD_HEADING "))
	{
		if (UvdSpeed.Present)
		{
			sprintf (text, "U%d", (int) rint (UvdSpeed.get_heading ()));
			return string (text);
		}
	}
	else if (!key.compare (0, 14, "UVD_EMERGENCY "))
	{
		if (UvdAltitude.Present && UvdAltitude.get_emergency ())
		{
			return string ("UEMG");
		}
	}
	else if (!key.compare (0, 9, "UVD_FUEL "))
	{
		if (UvdAltitude.Present)
		{
			sprintf (text, "U%u", UvdAltitude.get_fuel ());
			return string (text);
		}
	}
	else if (!key.compare (0, 11, "UVD_FRIEND "))
	{
		if (UvdFriendStatus.Present)
		{
			sprintf (text, "U%d", UvdFriendStatus.Code);
			return string (text);
		}
	}
	else
	{
		cms_warning ("unknown key %s", key.c_str ());
	}

	return string ();
}
