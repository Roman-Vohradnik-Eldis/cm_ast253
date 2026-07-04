#include "RDFTargetTrack.hh"
#include "CMSDebug.hh"

RDFTargetCartesianCovariance::RDFTargetCartesianCovariance ()
{
	Cov = DBL_MAX;
}

RDFTargetCartesianCovariance::~RDFTargetCartesianCovariance ()
{
}

RDFTargetCartesianCovariance::RDFTargetCartesianCovariance (double cov)
{
	Cov = cov;
}

RDFTargetCartesianCovariance::RDFTargetCartesianCovariance (RDFTargetCartesianCovariance * cov)
{
	if (!cov)
	{
		INTERNAL_ERROR ("null pointer");
	}
	
	Cov = cov->Cov;
}

RDFTargetAccuracy::RDFTargetAccuracy ()
{
	Cartesian = NULL;
	Geographic = NULL;
	PolarSpeed = NULL;
	CartesianSpeed = NULL;
	ClimbSpeed = NULL;
	Acceleration = NULL;
	GeometricHeight = NULL;
	BarometricHeight = NULL;
	CovXY = NULL;
}

RDFTargetAccuracy::~RDFTargetAccuracy ()
{
	if (Cartesian)
	{
		delete Cartesian;
	}
	if (Geographic)
	{
		delete Geographic;
	}
	if (PolarSpeed)
	{
		delete PolarSpeed;
	}
	if (CartesianSpeed)
	{
		delete CartesianSpeed;
	}
	if (ClimbSpeed)
	{
		delete ClimbSpeed;
	}
	if (Acceleration)
	{
		delete Acceleration;
	}
	if (GeometricHeight)
	{
		delete GeometricHeight;
	}
	if (BarometricHeight)
	{
		delete BarometricHeight;
	}
	if (CovXY)
	{
		delete CovXY;
	}
}

RDFTargetAccuracy::RDFTargetAccuracy (RDFTargetAccuracy * accuracy)
{
	if (!accuracy)
	{
		INTERNAL_ERROR ("null pointer");
	}
	
	Cartesian = accuracy->Cartesian ? new RDFCoorCartesian (accuracy->Cartesian) : NULL;
	Geographic = accuracy->Geographic ? new RDFCoorGeographic (accuracy->Geographic) : NULL;
	PolarSpeed = accuracy->PolarSpeed ? new RDFCoorPolar (accuracy->PolarSpeed) : NULL;
	CartesianSpeed = accuracy->CartesianSpeed ? new RDFCoorCartesian (accuracy->CartesianSpeed) : NULL;
	ClimbSpeed = accuracy->ClimbSpeed ? new RDFTargetClimbSpeed (accuracy->ClimbSpeed) : NULL;
	Acceleration = accuracy->Acceleration ? new RDFCoorCartesian (accuracy->Acceleration) : NULL;
	GeometricHeight = accuracy->GeometricHeight ? new RDFTargetHeight (accuracy->GeometricHeight) : NULL;
	BarometricHeight = accuracy->BarometricHeight ? new RDFTargetHeight (accuracy->BarometricHeight) : NULL;
	CovXY = accuracy->CovXY ? new RDFTargetCartesianCovariance (accuracy->CovXY) : NULL;
}

RDFTargetFlightPlan::RDFTargetFlightPlan ()
{
	ClearedFlightLevelPresent = false;
	CurrentControlPresent = false;
	CurrentControlCenter = 0;
	CurrentControlPosition = 0;
	WakeTurbulenceCategory = 0;
	FlightRules = FLIGHT_RULES_UNKNOWN;
	ClearedFlightLevel = 0;
	Route.erase ();
}

RDFTargetFlightPlan::~RDFTargetFlightPlan ()
{
}

RDFTargetFlightPlan::RDFTargetFlightPlan (RDFTargetFlightPlan * plan)
{
	if (!plan)
	{
		INTERNAL_ERROR ("null pointer");
	}
	
	ClearedFlightLevelPresent = plan->ClearedFlightLevelPresent;
	CurrentControlPresent = plan->CurrentControlPresent;
	CurrentControlCenter = plan->CurrentControlCenter;
	CurrentControlPosition = plan->CurrentControlPosition;
	WakeTurbulenceCategory = plan->WakeTurbulenceCategory;
	FlightRules = plan->FlightRules;
	ClearedFlightLevel = plan->ClearedFlightLevel;
	Callsign = plan->Callsign;
	TypeOfAircraft = plan->TypeOfAircraft;
	DepartureAirport = plan->DepartureAirport;
	DestinationAirport = plan->DestinationAirport;
	Route = plan->Route;
}

RDFTargetTrack::RDFTargetTrack () : RDFTargetPlot ()
{
	init ();
}

RDFTargetTrack::~RDFTargetTrack ()
{
	erase ();
}

RDFTargetTrack::RDFTargetTrack (const RDFTargetPlot & plot) : RDFTargetPlot (plot)
{
	init ();
}

RDFTargetTrack::RDFTargetTrack (const RDFTargetPlot * plot) : RDFTargetPlot (plot)
{
	init ();
}

RDFTargetTrack::RDFTargetTrack (const RDFTargetTrack & track) : RDFTargetPlot (track)
{
	init ();
	assign (track);
}

RDFTargetTrack::RDFTargetTrack (const RDFTargetTrack * track) : RDFTargetPlot (track)
{
	init ();
	if (track)
	{
		assign (*track);
	}
}

const RDFTargetTrack & RDFTargetTrack::operator = (const RDFTargetTrack & track)
{
	RDFTargetPlot::erase ();
	RDFTargetPlot::assign (track);
	erase ();
	assign (track);
	return track;
}


void RDFTargetTrack::init ()
{
	TrackNumber.init ();

	Init = false;
	Cancel = false;
	Tentative = false;
	Correlated = false;
	CST = false;
	AFF = false;
	Monosensor = true;	
	Manoeuvre = false;
	DOU = false;
	MAH = false;
	SUP = false;
	
	PSR = false;
	SSR = false;
	MDS = false;
	ADSB = false;

	Age.init ();
	AgePSR.init ();
	AgeSSR.init ();
	AgeMDS.init ();
	AgeADSB.init ();

	Cartesian = NULL;
	Geographic = NULL;

	SmoothedMode3A = NULL;
	GeometricHeight = NULL;
	BarometricHeight = NULL;

	GroundSpeed = NULL;
	Acceleration = NULL;
	ClimbSpeed = NULL;

	TransversalAcceleration = TRANS_ACCEL_UNDETERMINED;
	LongitudinalAcceleration = LONG_ACCEL_UNDETERMINED;
	TrackClimb = TRACK_CLIMB_UNKNOWN;

	Accuracy = NULL;
	FlightPlan = NULL;
}

void RDFTargetTrack::erase ()
{
	TrackNumber.init ();

	Init = false;
	Cancel = false;
	Tentative = false;
	Correlated = false;
	CST = false;
	AFF = false;
	Monosensor = true;
	Manoeuvre = false;
	DOU = false;
	MAH = false;
	SUP = false;
	
	PSR = false;
	SSR = false;
	MDS = false;
	ADSB = false;

	Age.init ();
	AgePSR.init ();
	AgeSSR.init ();
	AgeMDS.init ();
	AgeADSB.init ();

	if (Cartesian)
	{
		delete Cartesian;
		Cartesian = NULL;
	}
	if (Geographic)
	{
		delete Geographic;
		Geographic = NULL;
	}

	if (SmoothedMode3A)
	{
		delete SmoothedMode3A;
		SmoothedMode3A = NULL;
	}
	if (GeometricHeight)
	{
		delete GeometricHeight;
		GeometricHeight = NULL;
	}
	if (BarometricHeight)
	{
		delete BarometricHeight;
		BarometricHeight = NULL;
	}

	if (GroundSpeed)
	{
		delete GroundSpeed;
		GroundSpeed = NULL;
	}
	if (Acceleration)
	{
		delete Acceleration;
		Acceleration = NULL;
	}
	if (ClimbSpeed)
	{
		delete ClimbSpeed;
		ClimbSpeed = NULL;
	}
	
	TransversalAcceleration = TRANS_ACCEL_UNDETERMINED;
	LongitudinalAcceleration = LONG_ACCEL_UNDETERMINED;
	TrackClimb = TRACK_CLIMB_UNKNOWN;

	if (Accuracy)
	{
		delete Accuracy;
		Accuracy = NULL;
	}
	if (FlightPlan)
	{
		delete FlightPlan;
		FlightPlan = NULL;
	}
}

void RDFTargetTrack::assign (const RDFTargetTrack & track)
{
	TrackNumber = track.TrackNumber;

	Init = track.Init;
	Cancel = track.Cancel;
	Tentative = track.Tentative;
	Correlated = track.Correlated;
	CST = track.CST;
	AFF = track.AFF;
	Monosensor = track.Monosensor;
	Manoeuvre = track.Manoeuvre;
	DOU = track.DOU;
	MAH = track.MAH;
	SUP = track.SUP;

	PSR = track.PSR;
	SSR = track.SSR;
	MDS = track.MDS;
	ADSB = track.ADSB;

	Age = track.Age;
	AgePSR = track.AgePSR;
	AgeSSR = track.AgeSSR;
	AgeMDS = track.AgeMDS;
	AgeADSB = track.AgeADSB;

	if (track.Cartesian)
	{
		Cartesian = new RDFCoorCartesian (track.Cartesian);
	}
	if (track.Geographic)
	{
		Geographic = new RDFCoorGeographic (track.Geographic);
	}

	if (track.SmoothedMode3A)
	{
		SmoothedMode3A = new RDFTargetMode (track.SmoothedMode3A);
	}
	if (track.GeometricHeight)
	{
		GeometricHeight = new RDFTargetHeight (track.GeometricHeight);
	}
	if (track.BarometricHeight)
	{
		BarometricHeight = new RDFTargetHeight (track.BarometricHeight);
	}

	if (track.GroundSpeed)
	{
		GroundSpeed = new RDFTargetGroundSpeed (track.GroundSpeed);
	}
	if (track.Acceleration)
	{
		Acceleration = new RDFCoorCartesian (track.Acceleration);
	}
	if (track.ClimbSpeed)
	{
		ClimbSpeed = new RDFTargetClimbSpeed (track.ClimbSpeed);
	}
	
	TransversalAcceleration = track.TransversalAcceleration;
	LongitudinalAcceleration = track.LongitudinalAcceleration;
	TrackClimb = track.TrackClimb;

	if (track.Accuracy)
	{
		Accuracy = new RDFTargetAccuracy(track.Accuracy);
	}
	if (track.FlightPlan)
	{
		FlightPlan = new RDFTargetFlightPlan (track.FlightPlan);
	}
}

vector<string> RDFTargetTrack::getkeys ()
{
	vector<string> keys = RDFTargetPlot::getkeys ();

	keys.push_back ("TRACK_NUMBER");
	keys.push_back ("GROUND_SPEED 3X KPH NOINV");
	keys.push_back ("GROUND_HEADING");
	keys.push_back ("CLIMBING");
	keys.push_back ("CLIMB_SPEED");
	keys.push_back ("GEOGRAPHIC ATC GEOREF");
	keys.push_back ("AST_CALLSIGN");
	keys.push_back ("AST_WTC");
	keys.push_back ("AST_CFL");

	return keys;
}

string RDFTargetTrack::get (string key)
{
	char text[32];
	char text2[32];

	key.push_back (' ');

	// pokud nebudeme mit roll-call, vratime prazdnou hodnotu
	if (key.find (" ROLLCALL ") != string::npos
		&& TargetType != TARGET_TYPE_MODES_ROLL_CALL
		&& TargetType != TARGET_TYPE_MODES_ROLL_CALL_PSR)
	{
		return string ();
	}

	if (!key.compare (0, 13, "TRACK_NUMBER "))
	{
		if (TrackNumber.Present)
		{
			sprintf (text, "%d", TrackNumber.Number);
			return string (text);
		}
	}

	else if (!key.compare (0, 13, "GROUND_SPEED "))
	{
		if (GroundSpeed)
		{
			bool inv = (key.find (" NOINV ") == string::npos);

			if (key.find (" KPH ") != string::npos)
			{
				int speed = (int) rint (GroundSpeed->Speed * 3.6);
				if (inv && (speed < 0 || speed >= 10000))
				{
					return string ("INV");
				}
				sprintf (text, "%04d", speed);
			}
			else if (key.find (" 3X ") != string::npos)
			{
				int speed = (int) rint (GroundSpeed->Speed * 3.6 / 1.852);
				if (inv && (speed < 0 || speed >= 1000))
				{
					return string ("INV");
				}
				sprintf (text, "%03d", speed);
			}
			else
			{
				int speed = (int) rint (GroundSpeed->Speed * 3.6 / 1.852 / 10);
				if (inv && (speed < 0 || speed >= 100))
				{
					return string ("IN");
				}
				sprintf (text, "%02d", speed);				
			}
			
			return string (text);
		}
	}

	else if (!key.compare (0, 15, "GROUND_HEADING "))
	{
		if (GroundSpeed)
		{
			int heading = (int) rint (GroundSpeed->Heading * 180 / M_PI);
			sprintf (text, "%03d", heading);
			return string (text);
		}
	}

	else if (!key.compare (0, 9, "CLIMBING "))
	{
		switch (TrackClimb)
		{
			case TRACK_CLIMB_UNKNOWN:
			{
				if (ClimbSpeed)
				{
					if (ClimbSpeed->Speed > 0)
					{
						return string ("/");
					}
					if (ClimbSpeed->Speed < 0)
					{
						return string ("\\");
					}
					else return string ("=");
				}
				return string ();
			}
			case TRACK_CLIMB_CLIMBING:
			{
				return string ("/");
			}
			case TRACK_CLIMB_DESCENDING:
			{
				return string ("\\");
			}
			case TRACK_CLIMB_MAINTAINING:
			{
				return string ("=");
			}
		}
	}

	else if (!key.compare (0, 12, "CLIMB_SPEED "))
	{
		if (ClimbSpeed)
		{
			sprintf (text, "%d", abs ((int) rint (ClimbSpeed->Speed / 0.3048 * 60)));
			return string (text);
		}
	}

	else if (!key.compare (0, 11, "GEOGRAPHIC "))
	{
		if (Geographic)
		{
			if (key.find (" ATC ") != string::npos)
			{
				rdf_geo2strgeoatc (Geographic, text);
				return string (text);
			}
			if (key.find (" GEOREF ") != string::npos)
			{
				rdf_geo2strgeoref (Geographic, text);
				return string (text);
			}
			rdf_geo2strgeo (Geographic->Latitude, Geographic->Longitude, text, text2);
			return string (text) + string (" ") + string (text2);
		}
	}

	else if (!key.compare (0, 4, "AST_"))
	{
		if (FlightPlan)
		{
			if (!key.compare (4, 9, "CALLSIGN "))
			{
				return FlightPlan->Callsign;
			}

			else if (!key.compare (4, 4, "WTC "))
			{
				return string (1, FlightPlan->WakeTurbulenceCategory);
			}

			else if (!key.compare (4, 4, "CFL "))
			{
				if (FlightPlan->ClearedFlightLevelPresent)
				{
					sprintf (text, "%03d", (int) rint (FlightPlan->ClearedFlightLevel / 30.48));
					return string (text);
				}
			}

			else
			{
				cms_warning ("unknown key %s", key.c_str ());
			}
		}
	}

	else
	{
		return RDFTargetPlot::get (key);
	}

	return string ();
}

