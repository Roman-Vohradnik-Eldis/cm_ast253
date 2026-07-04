#include "RDFAsterix.hh"
#include "RDFTargetAdsb.hh"
#include "CMSDebug.hh"

RDFTargetAirSpeed::RDFTargetAirSpeed (const double &speed, const bool &mach,
		const bool &re)
{
	Speed = speed;
	Mach = mach;
	RE = re;
}

RDFTargetAirSpeed::~RDFTargetAirSpeed ()
{
}

RDFTargetAirSpeed::RDFTargetAirSpeed (RDFTargetAirSpeed * speed)
{
	if (!speed)
	{
		INTERNAL_ERROR ("null pointer");
	}

	Speed = speed->Speed;
	Mach = speed->Mach;
	RE = speed->RE;
}	

RDFTargetAdsb::RDFTargetAdsb ()
{
	init ();
}

RDFTargetAdsb::~RDFTargetAdsb ()
{
	erase ();
}

RDFTargetAdsb::RDFTargetAdsb (const RDFTargetAdsb & adsb)
{
	init ();
	assign (adsb);
}

RDFTargetAdsb::RDFTargetAdsb (const RDFTargetAdsb * adsb)
{
	init ();
	if (adsb)
	{
		assign (*adsb);
	}
}

const RDFTargetAdsb & RDFTargetAdsb::operator = (const RDFTargetAdsb & adsb)
{
	erase ();
	assign (adsb);
	return adsb;
}


void RDFTargetAdsb::init ()
{
	SAC = 0;
	SIC = 0;

	AircraftAddress.Present = false;
	AircraftAddress.Address = 0;

	TrackNumber.Present = false;
	TrackNumber.Number = 0;

	AddressType = ADDRESS_TYPE_ICAO;
	AltitudeCapability = ALTITUDE_CAPABILITY_25FT;
	ConfidenceLevel = CONFIDENCE_LEVEL_VALID;

	HRPosition = false;

	Cancel = false;
	Tentative = false;
	CST = false;
	Simulated = false;
	Tested = false;
	Fixed = false;
	SPI = false;
	RC = false;
	DCR = false;
	GBS = false;
	SAA = false;
	IPC = false;
	NOGO = false;
	CPR = false;
	LDPJ = false;
	RCF = false;

	Time.init ();
	TOAp.init ();
	TOMRp.init ();
	TOAv.init ();
	TOMRv.init ();

	NUCr_NACv = 0;
	NUCp_NIC = 0;
	NICbaro = 0;
	SIL = 0;
	NACp = 0;
	SILs = 0;
	SDA = 0;
	GVA = 0;
	PIC = 0;
	CDTI = false;

	ICF = false;
	LNAV = false;
	PriorityStatus = PRIORITY_STATUS_NO_EMERGENCY;
	SurveillanceStatus = SURVEILLANCE_STATUS_NO_CONDITION;

	VNS = false;
	VersionNumber = 0;
	LinkTechnology = LINK_TYPE_OTHER;

	ModeS.clear ();
	Mode1 = NULL;
	Mode2 = NULL;
	Mode3A = NULL;
	ModeC = NULL;
	FlightLevel = NULL;
	MeasuredHeight = NULL;
	Geographic = NULL;
	Cartesian = NULL;
	GeometricHeight = NULL;
	GroundSpeed = NULL;
	TrackClimb = TRACK_CLIMB_UNKNOWN;

	AirSpeed = NULL;
	TrueAirspeed = NULL;

	WarningError.clear ();
		
	SP_data.clear ();
	RE_data.clear ();
	UserText.erase ();
}

void RDFTargetAdsb::erase ()
{
	SAC = 0;
	SIC = 0;

	AircraftAddress.Present = false;
	AircraftAddress.Address = 0;
	AircraftIdentification.erase ();

	TrackNumber.Present = false;
	TrackNumber.Number = 0;

	AddressType = ADDRESS_TYPE_ICAO;
	AltitudeCapability = ALTITUDE_CAPABILITY_25FT;
	ConfidenceLevel = CONFIDENCE_LEVEL_VALID;

	HRPosition = false;

	Cancel = false;
	Tentative = false;
	CST = false;
	Simulated = false;
	Tested = false;
	Fixed = false;
	SPI = false;
	RC = false;
	DCR = false;
	GBS = false;
	SAA = false;
	IPC = false;
	NOGO = false;
	CPR = false;
	LDPJ = false;
	RCF = false;

	Time.init ();
	TOAp.init ();
	TOMRp.init ();
	TOAv.init ();
	TOMRv.init ();

	NUCr_NACv = 0;
	NUCp_NIC = 0;
	NICbaro = 0;
	SIL = 0;
	NACp = 0;
	SILs = 0;
	SDA = 0;
	GVA = 0;
	PIC = 0;
	CDTI = false;

	ICF = false;
	LNAV = false;
	PriorityStatus = PRIORITY_STATUS_NO_EMERGENCY;
	SurveillanceStatus = SURVEILLANCE_STATUS_NO_CONDITION;

	VNS = false;
	VersionNumber = 0;
	LinkTechnology = LINK_TYPE_OTHER;

	ModeS.clear ();

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
	if (ModeC)
	{
		delete ModeC;
		ModeC = NULL;
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

	if (Geographic)
	{
		delete Geographic;
		Geographic = NULL;
	}
	if (Cartesian)
	{
		delete Cartesian;
		Cartesian = NULL;
	}

	if (GeometricHeight)
	{
		delete GeometricHeight;
		GeometricHeight = NULL;
	}

	if (GroundSpeed)
	{
		delete GroundSpeed;
		GroundSpeed = NULL;
	}

	TrackClimb = TRACK_CLIMB_UNKNOWN;

	if (AirSpeed)
	{
		delete AirSpeed;
		AirSpeed = NULL;
	}

	if (TrueAirspeed)
	{
		delete TrueAirspeed;
		TrueAirspeed = NULL;
	}

	WarningError.clear ();

	SP_data.clear ();
	RE_data.clear ();
	UserText.erase ();
}

void RDFTargetAdsb::assign (const RDFTargetAdsb & adsb)
{
	SAC = adsb.SAC;
	SIC = adsb.SIC;

	AircraftAddress.Present = adsb.AircraftAddress.Present;
	AircraftAddress.Address = adsb.AircraftAddress.Address;
	AircraftIdentification = adsb.AircraftIdentification;

	TrackNumber.Present = adsb.TrackNumber.Present;
	TrackNumber.Number = adsb.TrackNumber.Number;

	AddressType = adsb.AddressType;
	AltitudeCapability = adsb.AltitudeCapability;
	ConfidenceLevel = adsb.ConfidenceLevel;

	HRPosition = adsb.HRPosition;

	Cancel = adsb.Cancel;
	Tentative = adsb.Tentative;
	CST = adsb.CST;
	Simulated = adsb.Simulated;
	Tested = adsb.Tested;
	Fixed = adsb.Fixed;
	SPI = adsb.SPI;
	RC = adsb.RC;
	DCR = adsb.DCR;
	GBS = adsb.GBS;
	SAA = adsb.SAA;
	IPC = adsb.IPC;
	NOGO = adsb.NOGO;
	CPR = adsb.CPR;
	LDPJ = adsb.LDPJ;
	RCF = adsb.RCF;

	Time = adsb.Time;
	TOAp = adsb.TOAp;
	TOMRp = adsb.TOMRp;
	TOAv = adsb.TOAv;
	TOMRv = adsb.TOMRv;

	NUCr_NACv = adsb.NUCr_NACv;
	NUCp_NIC = adsb.NUCp_NIC;
	NICbaro = adsb.NICbaro;
	SIL = adsb.SIL;
	NACp = adsb.NACp;
	SILs = adsb.SILs;
	SDA = adsb.SDA;
	GVA = adsb.GVA;
	PIC = adsb.PIC;
	CDTI = adsb.CDTI;

	ICF = adsb.ICF;
	LNAV = adsb.LNAV;
	PriorityStatus = adsb.PriorityStatus;
	SurveillanceStatus = adsb.SurveillanceStatus;

	VNS = adsb.VNS;
	VersionNumber = adsb.VersionNumber;
	LinkTechnology = adsb.LinkTechnology;

	ModeS = adsb.ModeS;

	if (adsb.Mode1)
	{
		Mode1 = new RDFTargetMode (adsb.Mode1);
	}
	if (adsb.Mode2)
	{
		Mode2 = new RDFTargetMode (adsb.Mode2);
	}
	if (adsb.Mode3A)
	{
		Mode3A = new RDFTargetMode (adsb.Mode3A);
	}
	if (adsb.ModeC)
	{
		ModeC = new RDFTargetMode (adsb.ModeC);
	}

	if (adsb.FlightLevel)
	{
		FlightLevel = new RDFTargetFlightLevel (adsb.FlightLevel);
	}
	if (adsb.MeasuredHeight)
	{
		MeasuredHeight = new RDFTargetHeight (adsb.MeasuredHeight);
	}

	if (adsb.Geographic)
	{
		Geographic = new RDFCoorGeographic (adsb.Geographic);
	}
	if (adsb.Cartesian)
	{
		Cartesian = new RDFCoorCartesian (adsb.Cartesian);
	}
	if (adsb.GeometricHeight)
	{
		GeometricHeight = new RDFTargetHeight (adsb.GeometricHeight);
	}
	if (adsb.GroundSpeed)
	{
		GroundSpeed = new RDFTargetGroundSpeed (adsb.GroundSpeed);
	}

	TrackClimb = adsb.TrackClimb;
	
	if (adsb.AirSpeed)
	{
		AirSpeed = new RDFTargetAirSpeed (adsb.AirSpeed);
	}
	if (adsb.TrueAirspeed)
	{
		TrueAirspeed = new RDFTargetAirSpeed (adsb.TrueAirspeed);
	}

	WarningError = adsb.WarningError;

	SP_data = adsb.SP_data;
	RE_data = adsb.RE_data;
	UserText = adsb.UserText;
}

#define addstring(s) { if (find (keys.begin(), keys.end(), s) == keys.end()) keys.push_back(s); }

std::vector<std::string> RDFTargetAdsb::getkeys ()
{
	std::vector<std::string> keys;

	addstring ("TIME");
	addstring ("TOAP");
	addstring ("TOMRP");
	addstring ("TOAV");
	addstring ("TOMRV");

	addstring ("AIRCRAFT_ADDRESS");
	addstring ("AIRCRAFT_IDENTIFICATION");
	addstring ("MODE_3A IGT");
	addstring ("FLIGHT_LEVEL 25 M IG NOINV");
	addstring ("GEOMETRIC_HEIGHT 25 M NOINV");
	addstring ("INDICATED_AIRSPEED");
	addstring ("MACH_NUMBER");
	addstring ("TRUE_AIRSPEED");
	addstring ("USER_TEXT");

	addstring ("TRACK_NUMBER");
	addstring ("GROUND_SPEED 3X KPH NOINV");
	addstring ("GROUND_HEADING");
	addstring ("GEOGRAPHIC");
	
	return keys;
}

std::string RDFTargetAdsb::get (std::string key)
{
	char text[32];
	char text2[32];
	string str;
	
	key.push_back (' ');
	
	if (!key.compare (0, 5, "TIME "))
	{
		sprintf (text, "%02u:%02u:%02u", Time.Time / 3600000,
			(Time.Time / 60000) % 60, (Time.Time / 1000) % 60);
		return std::string (text);
	}

	else if (!key.compare (0, 5, "TOAP "))
	{
		sprintf (text, "%02u:%02u:%02u", TOAp.Time / 3600000,
			(TOAp.Time / 60000) % 60, (TOAp.Time / 1000) % 60);
		return std::string (text);
	}

	else if (!key.compare (0, 6, "TOMRP "))
	{
		sprintf (text, "%02u:%02u:%02u", TOMRp.Time / 3600000,
			(TOMRp.Time / 60000) % 60, (TOMRp.Time / 1000) % 60);
		return std::string (text);
	}

	else if (!key.compare (0, 5, "TOAV "))
	{
		sprintf (text, "%02u:%02u:%02u", TOAv.Time / 3600000,
			(TOAv.Time / 60000) % 60, (TOAv.Time / 1000) % 60);
		return std::string (text);
	}

	else if (!key.compare (0, 6, "TOMRV "))
	{
		sprintf (text, "%02u:%02u:%02u", TOMRv.Time / 3600000,
			(TOMRv.Time / 60000) % 60, (TOMRv.Time / 1000) % 60);
		return std::string (text);
	}

	else if (!key.compare (0, 24, "AIRCRAFT_IDENTIFICATION "))
	{
		if (AircraftIdentification.size ())
		{
			return AircraftIdentification;
		}
	}

	else if (!key.compare (0, 17, "AIRCRAFT_ADDRESS "))
	{
		if (AircraftAddress.Present)
		{
			sprintf (text, "%06X", AircraftAddress.Address);
			return std::string (text);
		}
	}

	else if (!key.compare (0, 8, "MODE_3A "))
	{
		if (Mode3A)
		{
			sprintf (text, "%04d", asterix_decode_mode123 (Mode3A->Code));

			if (key.find (" IGT ") != std::string::npos)
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
					return std::string ("INV");
				}
			}
			
			return std::string (text);
		}
	}

	else if (!key.compare (0, 13, "FLIGHT_LEVEL "))
	{
		if (FlightLevel)
		{
			bool inv = (key.find (" NOINV ") == std::string::npos);

			if (key.find (" M ") != std::string::npos)
			{
				long m = lrint (FlightLevel->Height);
				if (inv && (m < -9999 || m > 99999))
				{
					return std::string ("INV");
				}
				sprintf (text, "%05ld", m);
			}
			else if (key.find (" 25 ") != std::string::npos)
			{
				if (AltitudeCapability == ALTITUDE_CAPABILITY_25FT)
				{
					int fl = static_cast<int>(rint (FlightLevel->Height / 0.3048));
					if (inv && (fl < -15000 || fl > 150000))
					{
						return std::string ("INV");
					}
					sprintf (text, "%03d.%02u", fl / 100, abs (fl % 100));
				}
				else
				{
					int fl = static_cast<int>(rint (FlightLevel->Height / 30.48));
					if (inv && (fl < -150 || fl > 1500))
					{
						return std::string ("INV");
					}
					sprintf (text, "%03d", fl);
				}
			}
			else
			{
				int fl = static_cast<int>(rint (FlightLevel->Height / 30.48));
				if (inv && (fl <= -150 || fl >= 1500))
				{
					return std::string ("INV");
				}
				sprintf (text, "%03d", fl);
			}

			if (key.find (" IG ") != std::string::npos)
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
					return std::string ("INV");
				}
			}

			return std::string (text);
		}
	}

	else if (!key.compare (0, 17, "GEOMETRIC_HEIGHT "))
	{
		if (GeometricHeight)
		{
			bool inv = (key.find (" NOINV ") == std::string::npos);

			if (key.find (" M ") != std::string::npos)
			{
				long m = lrint (GeometricHeight->Height);
				if (inv && (m < -9999 || m > 99999))
				{
					return std::string ("INV");
				}
				sprintf (text, "%05ld", m);
			}
			else if (key.find (" 25 ") != std::string::npos)
			{
				if (AltitudeCapability == ALTITUDE_CAPABILITY_25FT)
				{
					int fl = static_cast<int>(rint (GeometricHeight->Height / 0.3048));
					if (inv && (fl < -15000 || fl > 150000))
					{
						return std::string ("INV");
					}
					sprintf (text, "%03d.%02u", fl / 100, abs (fl % 100));
				}
				else
				{
					int fl = static_cast<int>(rint (GeometricHeight->Height / 30.48));
					if (inv && (fl < -150 || fl > 1500))
					{
						return std::string ("INV");
					}
					sprintf (text, "%03d", fl);
				}
			}
			else
			{
				int fl = static_cast<int>(rint (GeometricHeight->Height / 30.48));
				if (inv && (fl <= -150 || fl >= 1500))
				{
					return std::string ("INV");
				}
				sprintf (text, "%03d", fl);
			}

			return std::string (text);
		}
	}

	else if (!key.compare (0, 19, "INDICATED_AIRSPEED "))
	{
		if (AirSpeed && !AirSpeed->Mach)
		{
			int speed = static_cast<int>(rint (AirSpeed->Speed * 3.6 / 1.852 / 10));
			if (speed < 0 || speed >= 100)
			{
				return std::string ("IN");
			}
			sprintf (text, "%02d", speed);				
			return std::string (text);
		}
	}

	else if (!key.compare (0, 12, "MACH_NUMBER "))
	{
		if (AirSpeed && AirSpeed->Mach)
		{
			sprintf (text, "M%02d", static_cast<int>(rint (AirSpeed->Speed / 340.3 * 10)));
			return std::string (text);
		}
	}

	else if (!key.compare (0, 14, "TRUE_AIRSPEED "))
	{
		if (TrueAirspeed)
		{
			int speed = static_cast<int>(rint (TrueAirspeed->Speed * 3.6 / 1.852 / 10));
			if (speed < 0 || speed >= 100)
			{
				return std::string ("IN");
			}
			sprintf (text, "%02d", speed);				
			return std::string (text);
		}
	}
	
	else if (!key.compare (0, 10, "USER_TEXT "))
	{
	  	return UserText;
	}

	else if (!key.compare (0, 13, "TRACK_NUMBER "))
	{
		if (TrackNumber.Present)
		{
			sprintf (text, "%d", TrackNumber.Number);
			return std::string (text);
		}
	}

	else if (!key.compare (0, 13, "GROUND_SPEED "))
	{
		if (GroundSpeed)
		{
			bool inv = (key.find (" NOINV ") == std::string::npos);

			if (key.find (" KPH ") != std::string::npos)
			{
				int speed = static_cast<int>(rint (GroundSpeed->Speed * 3.6));
				if (inv && (speed < 0 || speed >= 10000))
				{
					return std::string ("INV");
				}
				sprintf (text, "%04d", speed);
			}
			else if (key.find (" 3X ") != std::string::npos)
			{
				int speed = static_cast<int>(rint (GroundSpeed->Speed * 3.6 / 1.852));
				if (inv && (speed < 0 || speed >= 1000))
				{
					return std::string ("INV");
				}
				sprintf (text, "%03d", speed);
			}
			else
			{
				int speed = static_cast<int>(rint (GroundSpeed->Speed * 3.6 / 1.852 / 10));
				if (inv && (speed < 0 || speed >= 100))
				{
					return std::string ("IN");
				}
				sprintf (text, "%02d", speed);				
			}
			
			return std::string (text);
		}
	}

	else if (!key.compare (0, 15, "GROUND_HEADING "))
	{
		if (GroundSpeed)
		{
			int heading = static_cast<int>(rint (GroundSpeed->Heading * 180 / M_PI));
			sprintf (text, "%03d", heading);
			return std::string (text);
		}
	}

	else if (!key.compare (0, 11, "GEOGRAPHIC "))
	{
		if (Geographic)
		{
			if (key.find (" ATC ") != std::string::npos)
			{
				rdf_geo2strgeoatc (Geographic, text);
				return std::string (text);
			}
			if (key.find (" GEOREF ") != std::string::npos)
			{
				rdf_geo2strgeoref (Geographic, text);
				return std::string (text);
			}
			rdf_geo2strgeo (Geographic->Latitude, Geographic->Longitude, text, text2);
			return std::string (text) + std::string (" ") + std::string (text2);
		}
	}

	else if (!key.compare (0, 4, "BDS "))
	{
		bool bds[256];
		memset (bds, 0, sizeof (bds));

		for (unsigned i = 0; i < ModeS.size (); i++)
		{
			bds[ModeS[i].Address] = true;
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

	else
	{
		cms_warning ("unknown key %s", key.c_str ());
	}
	
	return std::string ();
}

