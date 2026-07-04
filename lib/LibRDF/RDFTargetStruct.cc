#include "RDFTargetStruct.hh"
#include "CMSDebug.hh"

RDFTime::RDFTime ()
{
	init ();
}

void RDFTime::init ()
{
	Present = false;
	Time = 0;
}

// Pozor, nevejde se do uint32
unsigned RDF_time_to_asterix_time(unsigned long long time_msec)
{
	return ((time_msec * 128 + 500) / 1000) % (128 * 24 * 3600);
}

RDFPosition3D::RDFPosition3D ()
{
	init ();
}

RDFPosition3D::~RDFPosition3D ()
{
}

RDFPosition3D::RDFPosition3D (const RDFPosition3D * position)
{
	if (!position)
		return;
	Height = position->Height;
	Latitude = position->Latitude;
	Longitude = position->Longitude;
}

RDFPosition3D::RDFPosition3D (const RDFPosition3D & position)
{
	Height = position.Height;
	Latitude = position.Latitude;
	Longitude = position.Longitude;
}

void RDFPosition3D::init ()
{
	Height = 0;
	Latitude = 0;
	Longitude = 0;
}

bool RDFPosition3D::equal (const RDFPosition3D & position)
{
	if (Height != position.Height)
	{
		return false;
	}
	if (Latitude != position.Latitude)
	{
		return false;
	}
	if (Longitude != position.Longitude)
	{
		return false;
	}
	return true;
}


RDFAircraftAddress::RDFAircraftAddress ()
{
	init ();
}

void RDFAircraftAddress::init ()
{
	Present = false;
	Address = 0;
}

RDFTargetMode::RDFTargetMode ()
{
	Valid = false;
	Garbled = false;
	Tracked = false;
	ConfidencePresent = false;
	Code = 0;
	ConfidenceCode = 0;
	Age = 0;
}

RDFTargetMode::~RDFTargetMode ()
{
}

RDFTargetMode::RDFTargetMode (RDFTargetMode * mode)
{
	if (!mode)
		return;
	Valid = mode->Valid;
	Garbled = mode->Garbled;
	Tracked = mode->Tracked;
	ConfidencePresent = mode->ConfidencePresent;
	Code = mode->Code;
	ConfidenceCode = mode->ConfidenceCode;
	Age = mode->Age;
}

RDFTargetFlightLevel::RDFTargetFlightLevel ()
{
	Valid = false;
	Garbled = false;
	Height = 0;
	Age = 0;
}

RDFTargetFlightLevel::~RDFTargetFlightLevel ()
{
}

RDFTargetFlightLevel::RDFTargetFlightLevel (RDFTargetFlightLevel * level)
{
	if (!level)
		return;
	Valid = level->Valid;
	Garbled = level->Garbled;
	Height = level->Height;
	Age = level->Age;
}

RDFTargetHeight::RDFTargetHeight ()
{
	QualityPresent = false;
	QNH = false;
	Height = 0;
	Quality = 0;
	Age = 0;
}

RDFTargetHeight::~RDFTargetHeight ()
{
}

RDFTargetHeight::RDFTargetHeight (double height)
{
	QualityPresent = false;
	QNH = false;
	Height = height;
	Quality = 0;
	Age = 0;
}

RDFTargetHeight::RDFTargetHeight (RDFTargetHeight * height)
{
	if (!height)
		return;
	QualityPresent = height->QualityPresent;
	QNH = height->QNH;
	Height = height->Height;
	Quality = height->Quality;
	Age = height->Age;
}

RDFTrackNumber::RDFTrackNumber ()
{
	init ();
}

void RDFTrackNumber::init ()
{
	Present = false;
	Temporary = false;
	Number = 0;
}

RDFTargetGroundSpeed::RDFTargetGroundSpeed ()
{
	init ();
}

RDFTargetGroundSpeed::~RDFTargetGroundSpeed ()
{
}

RDFTargetGroundSpeed::RDFTargetGroundSpeed (double speed, double heading)
{
	Speed = speed;
	Heading = heading;
}

RDFTargetGroundSpeed::RDFTargetGroundSpeed (RDFTargetGroundSpeed * speed)
{
	if (!speed)
		return;
	Speed = speed->Speed;
	Heading = speed->Heading;
}

void RDFTargetGroundSpeed::init ()
{
	Speed = 0;
	Heading = 0;
}

RDFTargetClimbSpeed::RDFTargetClimbSpeed ()
{
	Speed = 0;
}

RDFTargetClimbSpeed::~RDFTargetClimbSpeed ()
{
}

RDFTargetClimbSpeed::RDFTargetClimbSpeed (double speed)
{
	Speed = speed;
}

RDFTargetClimbSpeed::RDFTargetClimbSpeed (RDFTargetClimbSpeed * speed)
{
	if (!speed)
		return;
	Speed = speed->Speed;
}

RDFDatalinkControl::RDFDatalinkControl ()
{
	init ();
}

void RDFDatalinkControl::init ()
{
	CommATms = 0;
	DownElmCtrl = 0;
	UpElmCtrl = 0;
	CommBCtrl = 0;

	present = false;
}

RDFExpectedBDS::RDFExpectedBDS ()
{
	init ();
}

void RDFExpectedBDS::init ()
{
	BDS_1 = 0;
	BDS_2 = 0;

	present = false;
}


const uint32_t UVD_ALT_NEG_COMPLEMENT_PREFIX = 0xB00;
const uint32_t UVD_ALT_NEG_DIRECT_PREFIX = 0xE00;
const uint32_t UVD_ALTITUDE_MASK = 0x3FFF;
const uint32_t UVD_ALTITUDE_TYPE_MASK = 0x4000;
const uint32_t UVD_ALTITUDE_EMERGENCY_MASK = 0x8000;
const uint32_t UVD_FUEL_MASK = 0xF0000;
const uint32_t UVD_HEADING_DATA_MASK = 0x1FF;
const uint32_t UVD_HEADING_SIGN_MASK = 0x200;
const uint32_t UVD_HEADING_MASK = UVD_HEADING_DATA_MASK | UVD_HEADING_SIGN_MASK;

bool RDFUvdAltitude::get_altitude_relative ()
{
	if (Code & UVD_ALTITUDE_TYPE_MASK)
		return false;
	return true;
}

int32_t RDFUvdAltitude::get_altitude ()
{
	if ((Code & 0x0F00) == 0xF00)
	{
		return (((Code & 0xf) + (((Code >> 4) & 0x3) * 10)) * -10);
	}
	return (((Code & 0xf) + (((Code >> 4) & 0xf) * 10) + (((Code >> 8) & 0xf) * 100) +
			(((Code >> 12) & 0x3) * 1000)) * 10);
}

uint32_t RDFUvdAltitude::get_fuel ()
{
	if (((Code & UVD_FUEL_MASK) >> 16) <= 10)
		return ((Code & UVD_FUEL_MASK) >> 16) * 5;
	else
		return (((Code & UVD_FUEL_MASK) >> 16) - 5) * 10;
}

bool RDFUvdAltitude::get_emergency ()
{
	if (Code & UVD_ALTITUDE_EMERGENCY_MASK)
		return true;
	else
		return false;
}

float RDFUvdSpeed::get_speed ()
{
	return ((((Code >> 10) & 0xf) * 10) + (((Code >> 14) & 0xf) * 100) +
		(((Code >> 18) & 0x3) * 1000));
}

float RDFUvdSpeed::get_heading ()
{
	return (((Code & 0xf) * 1) + (((Code >> 4) & 0xf) * 10) +
		(((Code >> 8) & 0x3) * 100));
}

uint32_t RDFUvdCode::get_identification ()
{
	return ((Code & 0xf) + (((Code >> 4) & 0xf) * 10) + (((Code >> 8) & 0xf) * 100) +
		(((Code >> 12) & 0xf) * 1000) + (((Code >> 16) & 0xf) * 10000));
}
