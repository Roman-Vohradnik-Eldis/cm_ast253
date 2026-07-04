#include "RDFDatalinkFunction.hh"
#include "CMSDebug.hh"
#include "RDFAsterix.hh"



void	II_CODE :: assign ( int new_II_code )
{
	if ( is_former )
	{
		Former		= Current;
		Current		= new_II_code;
	}
	else
	{
		is_former	= true;
		Former = Current = new_II_code;
	}
}


DL_COMMAND_INNER :: DL_COMMAND_INNER ()
{
	ULM = DLM = IGNORED;
	ULC = DLC = STOPPED;
	Spare	= 0;
};



DL_STATUS :: DL_STATUS ()
{
	UDS = DDS = UCS = DCS = STOPPED;
	EI = IC = 0;
};


RDFTargetSpeed::RDFTargetSpeed (RDFTargetSpeed *speed)
{
	if (!speed)
	{
		INTERNAL_ERROR ("null pointer");
	}

	Speed = speed->Speed;
}

RDFTargetSpeed::RDFTargetSpeed (const uint8_t *field)
{
	if (!field)
	{
		INTERNAL_ERROR ("null pointer");
	}

	Speed = (double) field2unsigned (const_cast<uint8_t *>(field), 16) * 1852 / (1<<14);
}

RDFTargetHeading::RDFTargetHeading (RDFTargetHeading *heading)
{
	if (!heading)
	{
		INTERNAL_ERROR ("null pointer");
	}

	Heading = heading->Heading;
}

RDFTargetHeading::RDFTargetHeading (const uint8_t *field)
{
	if (!field)
	{
		INTERNAL_ERROR ("null pointer");
	}

	Heading = (double) field2unsigned (const_cast<uint8_t *>(field + 2), 16) * (2 * M_PI) / (1<<16);
};

RDFDatalinkFunction::RDFDatalinkFunction ()		// : RDFTargetPlot ()
{
	init ();
}


RDFDatalinkFunction::~RDFDatalinkFunction ()
{
	erase ();
}



RDFDatalinkFunction::RDFDatalinkFunction ( const RDFTargetPlot & plot )
{
	init ();
}

RDFDatalinkFunction::RDFDatalinkFunction ( const RDFTargetPlot * plot )
{
	init ();
}

RDFDatalinkFunction::RDFDatalinkFunction (const RDFTargetTrack &target)
{
	init ();

	Message_type = AIRCRAFT_REPORT;
	AircraftAddress = target.AircraftAddress;
	CQF_Method.Present = true;
	CQF.Present = true;

	if (target.CommCapabilityAndFlightStatus)
	{
		D_Comm.Present = true;
		D_Comm.ComCap = target.CommCapabilityAndFlightStatus->CommCapability;
		if ((target.CommCapabilityAndFlightStatus->FlightStatus == FLIGHT_STATUS_ON_GROUND) ||
			(target.CommCapabilityAndFlightStatus->FlightStatus == FLIGHT_STATUS_ON_GROUND_ALERT))
			CQF.FlightStatus = GROUND;
	}

	CapReport = new RDFTargetModeS (target.datalink_capability ());

	if (target.Polar)
		Polar_Pos = new RDFCoorPolar (target.Polar);

	if (target.Cartesian)
		Cartesian_Pos = new RDFCoorCartesian (target.Cartesian);

	if (!target.AircraftIdentification.empty ())
		AircraftIdentity.assign (target.AircraftIdentification);

	if (target.Mode3A)
		Mode3A = new RDFTargetMode (target.Mode3A);

	if (target.FlightLevel)
		FlightLevel = new RDFTargetFlightLevel (target.FlightLevel);

	if (target.GroundSpeed)
	{
		Speed = new  RDFTargetSpeed (target.GroundSpeed->Speed);
		Heading = new RDFTargetHeading (target.GroundSpeed->Heading);
	}
}

RDFDatalinkFunction::RDFDatalinkFunction ( const RDFDatalinkFunction & dlf )
{
	init ();
	assign ( dlf );
}


RDFDatalinkFunction::RDFDatalinkFunction ( const RDFDatalinkFunction * dlf )
{
	init ();
	if ( dlf ) assign ( *dlf );
}


const RDFDatalinkFunction & RDFDatalinkFunction::operator = ( const RDFDatalinkFunction & dlf )
{
	erase ();
	assign ( dlf );

	return dlf;
}


void RDFDatalinkFunction::init ()
{
	CAT	= 18;

	Message_type	= NO_MESSAGE;

	GICB_Extracted	= NULL;
	CapReport = NULL;
	Polar_Pos				= NULL;
	Cartesian_Pos		= NULL;
	Broadcast		= NULL;
	Mode3A			= NULL;
	FlightLevel	= NULL;
	Speed = NULL;
	Heading = NULL;
}


void RDFDatalinkFunction::erase ()
{
	if ( GICB_Extracted )
	{
		delete GICB_Extracted;
		GICB_Extracted = NULL;
	}

	if ( CapReport )
	{
		delete CapReport;
		CapReport = NULL;
	}

	if ( Polar_Pos )
	{
		delete Polar_Pos;
		Polar_Pos	= NULL;
	}

	if ( Cartesian_Pos )
	{
		delete Cartesian_Pos;
		Cartesian_Pos = NULL;
	}

	if ( Broadcast )
	{
		delete Broadcast;
		Broadcast = NULL;
	}

	if ( Mode3A )
	{
		delete Mode3A;
		Mode3A = NULL;
	}

	if ( FlightLevel )
	{
		delete FlightLevel;
		FlightLevel = NULL;
	}

	if ( Speed )
	{
		delete Speed;
		Speed = NULL;
	}

	if ( Heading )
	{
		delete Heading;
		Heading = NULL;
	}

	init ();

}

void RDFDatalinkFunction::assign ( const RDFDatalinkFunction & dlf )
{
	Source = dlf.Source;
	Destination = dlf.Destination;
	Message_type = dlf.Message_type;
	Result = dlf.Result;
	AircraftAddress = dlf.AircraftAddress;
	Packet_Num = dlf.Packet_Num;
	Packet_List = dlf.Packet_List;

	Packet_Prop = dlf.Packet_Prop;
	ModeS_Packet = dlf.ModeS_Packet;
	GICB_Periodicity = dlf.GICB_Periodicity;
	GICB_Prop = dlf.GICB_Prop;
	GICB_Num = dlf.GICB_Num;
	BDS_Code = dlf.BDS_Code;
	if ( dlf.GICB_Extracted ) GICB_Extracted = new RDFTargetModeS ( *dlf.GICB_Extracted );

	Time = dlf.Time;
	ModeS_addr_list = dlf.ModeS_addr_list;
	DL_Command = dlf.DL_Command;
	DL_Status = dlf.DL_Status;
	DL_Report_Req = dlf.DL_Report_Req;
	D_Comm = dlf.D_Comm;
	if ( dlf.CapReport ) CapReport = new RDFTargetModeS ( *dlf.CapReport );

	if ( dlf.Polar_Pos ) Polar_Pos = new RDFCoorPolar ( dlf.Polar_Pos );
	if ( dlf.Cartesian_Pos ) Cartesian_Pos = new RDFCoorCartesian ( dlf.Cartesian_Pos );
	Broadcast_Num = dlf.Broadcast_Num;
	Broadcast_Prop = dlf.Broadcast_Prop;
	Broadcast_Prefix = dlf.Broadcast_Prefix;
	if ( dlf.Broadcast ) Broadcast = new RDFTargetModeS ( *dlf.Broadcast );
	II_Code	= dlf.II_Code;

	AircraftIdentity = dlf.AircraftIdentity;
	if ( dlf.Mode3A ) Mode3A = new RDFTargetMode ( dlf.Mode3A );
	if ( dlf.FlightLevel ) FlightLevel = new RDFTargetFlightLevel ( dlf.FlightLevel );
	if ( dlf.Speed ) Speed = new RDFTargetSpeed ( dlf.Speed );
	if ( dlf.Heading ) Heading = new RDFTargetHeading ( dlf.Heading );
	CQF = dlf.CQF;
	CQF_Method = dlf.CQF_Method;
}

#define addstring(s) { if (find (keys.begin(), keys.end(), s) == keys.end()) keys.push_back(s); }

vector<string> RDFDatalinkFunction::getkeys ()
{
	vector<string> keys = RDFTargetPlot::getkeys ();

	addstring ("TRACK_NUMBER");
	addstring ("GROUND_SPEED 3X KPH NOINV");
	addstring ("GROUND_HEADING");
	addstring ("CLIMBING");
	addstring ("CLIMB_SPEED");
	addstring ("GEOGRAPHIC");
	addstring ("AST_CALLSIGN");
	addstring ("AST_WTC");
	addstring ("AST_CFL");

	return keys;
}


string RDFDatalinkFunction::get ( string key )
{
//	char text[32];
//	char text2[32];


	key.push_back (' ');

/*
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
				if (inv && (speed < 0 || speed >= 10000)) return string ("INV");
				sprintf (text, "%04d", speed);
			}
			else if (key.find (" 3X ") != string::npos)
			{
				int speed = (int) rint (GroundSpeed->Speed * 3.6 / 1.852);
				if (inv && (speed < 0 || speed >= 1000)) return string ("INV");
				sprintf (text, "%03d", speed);
			}
			else
			{
				int speed = (int) rint (GroundSpeed->Speed * 3.6 / 1.852 / 10);
				if (inv && (speed < 0 || speed >= 100)) return string ("IN");
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
					if (ClimbSpeed->Speed > 0) return string ("/");
					if (ClimbSpeed->Speed < 0) return string ("\\");
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
*/
	{
//		return RDFTargetPlot::get ( key );
	}

	return string ();
}


