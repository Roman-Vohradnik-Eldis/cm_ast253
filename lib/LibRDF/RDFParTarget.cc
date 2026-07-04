#include "RDFAsterix.hh"
#include "RDFParTarget.hh"
#include "CMSDebug.hh"

RDFParTarget::RDFParTarget ()
{
	init ();
}

RDFParTarget::~RDFParTarget ()
{
}

RDFParTarget::RDFParTarget (const RDFParTarget & target)
{
	assign (target);
}

const RDFParTarget & RDFParTarget::operator = (const RDFParTarget & target)
{
	assign (target);
	return target;
}


void RDFParTarget::init ()
{
	Time.init ();
	TrackNumber.init ();
	
	SAC = 0;
	SIC = 0;

	PamAzimuthPresent = false;
	PamElevationPresent = false;
	PrlAzimuthPresent = false;
	PrlElevationPresent = false;
	MeasuredAzimuthalPositionPresent = false;
	MeasuredElevationPositionPresent = false;
	CalculatedAzimuthalPositionPresent = false;
	CalculatedElevationPositionPresent = false;
	AzimuthalVelocityPresent = false;
	ElevationVelocityPresent = false;
	
	Simulated = false;
	Tested = false;
	ParDetection = false;

	Tentative = false;
	Cancel = false;
	Ghost = false;
	
	MessageType = PAR_TARGET_MESSAGE_TYPE_UNKNOWN;
	LOP = 0;
	TOT = 0;
	PamAzimuth = 0;
	PamElevation = 0;
	PrlAzimuth = 0;
	PrlElevation = 0;
	
	MeasuredAzimuthalPosition.init ();
	MeasuredElevationPosition.init ();
	CalculatedAzimuthalPosition.init ();
	CalculatedElevationPosition.init ();

	AzimuthalVelocity.init ();
	ElevationVelocity.init ();
	
	WarningError.clear ();

	UserText.clear ();
}

void RDFParTarget::assign (const RDFParTarget & target)
{
	Time = target.Time;
	TrackNumber = target.TrackNumber;
	
	SAC = target.SAC;
	SIC = target.SIC;

	PamAzimuthPresent = target.PamAzimuthPresent;
	PamElevationPresent = target.PamElevationPresent;
	PrlAzimuthPresent = target.PrlAzimuthPresent;
	PrlElevationPresent = target.PrlElevationPresent;
	MeasuredAzimuthalPositionPresent = target.MeasuredAzimuthalPositionPresent;
	MeasuredElevationPositionPresent = target.MeasuredElevationPositionPresent;
	CalculatedAzimuthalPositionPresent = target.CalculatedAzimuthalPositionPresent;
	CalculatedElevationPositionPresent = target.CalculatedElevationPositionPresent;
	AzimuthalVelocityPresent = target.AzimuthalVelocityPresent;
	ElevationVelocityPresent = target.ElevationVelocityPresent;
	
	Simulated = target.Simulated;
	Tested = target.Tested;
	ParDetection = target.ParDetection;

	Tentative = target.Tentative;
	Cancel = target.Cancel;
	Ghost = target.Ghost;
	
	MessageType = target.MessageType;
	LOP = target.LOP;
	TOT = target.TOT;
	PamAzimuth = target.PamAzimuth;
	PamElevation = target.PamElevation;
	PrlAzimuth = target.PrlAzimuth;
	PrlElevation = target.PrlElevation;

	MeasuredAzimuthalPosition = target.MeasuredAzimuthalPosition;
	MeasuredElevationPosition = target.MeasuredElevationPosition;
	CalculatedAzimuthalPosition = target.CalculatedAzimuthalPosition;
	CalculatedElevationPosition = target.CalculatedElevationPosition;

	AzimuthalVelocity = target.AzimuthalVelocity;
	ElevationVelocity = target.ElevationVelocity;
	
	WarningError = target.WarningError;

	UserText = target.UserText;
}

#define addstring(s) { if (find (keys.begin(), keys.end(), s) == keys.end()) keys.push_back(s); }

vector<string> RDFParTarget::getkeys ()
{
	vector<string> keys;

	addstring ("TIME");
	addstring ("USER_TEXT");

	return keys;
}

string RDFParTarget::get (string key)
{
	char text[1024];
	
	key.push_back (' ');

	if (!key.compare (0, 5, "TIME "))
	{
		sprintf (text, "%02u:%02u:%02u", Time.Time / 3600000,
			(Time.Time / 60000) % 60, (Time.Time / 1000) % 60);
		return string (text);
	}

	else if (!key.compare (0, 10, "USER_TEXT "))
	{
	  	return UserText;
	}

	else
	{
		cms_warning ("unknown key %s", key.c_str ());
	}
	
	return string ();
}

