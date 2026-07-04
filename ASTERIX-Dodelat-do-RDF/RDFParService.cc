#include "RDFParService.hh"
#include "CMSDebug.hh"

RDFParElevationAntennaTilt::RDFParElevationAntennaTilt ()
{
	init ();
}

void RDFParElevationAntennaTilt::init ()
{
	X = 0;
	Y = 0;
}

RDFParRadarBaseTilt::RDFParRadarBaseTilt ()
{
	init ();
}

void RDFParRadarBaseTilt::init ()
{
	X = 0;
	Y = 0;
}

RDFParRunwayInUse::RDFParRunwayInUse ()
{
	init ();
}

void RDFParRunwayInUse::init ()
{
	IDPresent = 0;
	RWDPresent = 0;
	TDDPresent = 0;
	LDRPresent = 0;
	TDEPresent = 0;
	RWNPresent = 0;
	
	ID.clear ();
	RWD = 0;
	TDD = 0;
	LDR = 0;
	TDE = 0;
	RWN = 0;
}

RDFParSystemConfigurationStatusCOM::RDFParSystemConfigurationStatusCOM ()
{
	init ();
}

void RDFParSystemConfigurationStatusCOM::init ()
{
	NOGO = 0;
	RDPC = 0;
	RDPR = 0;
	OVLRDP = 0;
	OVLXMT = 0;
	MSC = 0;
	TSV = 0;
}

RDFParSystemConfigurationStatusWND::RDFParSystemConfigurationStatusWND ()
{
	init ();
}

void RDFParSystemConfigurationStatusWND::init ()
{
	SPEED_10s = 0;
	DIRECTION_10s = 0;
	SPEED_1min = 0;
	DIRECTION_1min = 0;
}

RDFParSystemConfigurationStatusPAR::RDFParSystemConfigurationStatusPAR ()
{
	init ();
}

void RDFParSystemConfigurationStatusPAR::init ()
{
	ANT = 0;
	CHAB = 0;
	OVL = 0;
	MRC = 0;
	ARR = 0;
	OPR = 0;
	CAL = 0;
}

RDFParSystemConfigurationStatusCTR::RDFParSystemConfigurationStatusCTR ()
{
	init ();
}

void RDFParSystemConfigurationStatusCTR::init ()
{
	RNG = 0;
	FREQ = 0;
  PSDM = 0;
}

RDFParSystemConfigurationStatusPOL::RDFParSystemConfigurationStatusPOL ()
{
	init ();
}

void RDFParSystemConfigurationStatusPOL::init ()
{
	CRA = 0;
  CLA = 0;
	CRE = 0;
  CLE = 0;
	LVA = 0;
	LHA = 0;
	LVE = 0;
	LHE = 0;
}

RDFParSystemConfigurationStatusPSV::RDFParSystemConfigurationStatusPSV ()
{
	init ();
}

void RDFParSystemConfigurationStatusPSV::init ()
{
	AngleElevation = 0;
  AngleAzimuthal = 0;
  AspectRatioElevation = 0;
  AspectRatioAzimuthal = 0;
}

RDFParSystemConfigurationStatus::RDFParSystemConfigurationStatus ()
{
	init ();
}

void RDFParSystemConfigurationStatus::init ()
{
	COMPresent = false;
	WNDPresent = false;
	PARPresent = false;
	CTRPresent = false;
	POLPresent = false;
	PSVPresent = false;
	COM.init ();
	WND.init ();
	PAR.init ();
	CTR.init ();
	POL.init ();
	PSV.init ();
}

RDFParService::RDFParService ()
{
	init ();
}

RDFParService::~RDFParService ()
{
}

RDFParService::RDFParService (const RDFParService & service)
{
	assign (service);
}

const RDFParService & RDFParService::operator = (const RDFParService & service)
{
	assign (service);
	return service;
}

void RDFParService::init ()
{
	SectorAnglePresent = 0;
	AzimuthalElevationAnglePresent = 0;
	AntennaScanningPeriodPresent = 0;
	AntennaPedestalDirectionPresent = 0;
	AzimuthalAntennaTiltPresent = 0;
	ElevationAntennaTiltPresent = 0;
	RadarBaseTiltPresent = 0;
	ElevationCorrectionPresent = 0;
	RunwayInUsePresent = 0;
	ConfigurationStatusPresent = 0;
	Position3DPresent = 0;

	MessageType = PAR_SERVICE_MESSAGE_TYPE_UNKNOWN;
	SAC = 0;
	SIC = 0;
	SectorAngle = 0;
	Time.init ();
	AzimuthalElevationAngle = 0;
	AntennaScanningPeriod = 0;
	AntennaPedestalDirection = 0;
	AzimuthalAntennaTilt = 0;
	ElevationAntennaTilt.init ();
	ElevationCorrection = 0;
	RunwayInUse.init ();
	ConfigurationStatus.init ();
	Position3D.init ();
}

void RDFParService::assign (const RDFParService & service)
{
	SectorAnglePresent = service.SectorAnglePresent;
	AzimuthalElevationAnglePresent = service.AzimuthalElevationAnglePresent;
	AntennaScanningPeriodPresent = service.AntennaScanningPeriodPresent;
	AntennaPedestalDirectionPresent = service.AntennaPedestalDirectionPresent;
	AzimuthalAntennaTiltPresent = service.AzimuthalAntennaTiltPresent;
	ElevationAntennaTiltPresent = service.ElevationAntennaTiltPresent;
	RadarBaseTiltPresent = service.RadarBaseTiltPresent;
	ElevationCorrectionPresent = service.ElevationCorrectionPresent;
	RunwayInUsePresent = service.RunwayInUsePresent;
	ConfigurationStatusPresent = service.ConfigurationStatusPresent;
	Position3DPresent = service.Position3DPresent;

	MessageType = service.MessageType;
	SAC = service.SAC;
	SIC = service.SIC;
	SectorAngle = service.SectorAngle;
	Time = service.Time;
	AzimuthalElevationAngle = service.AzimuthalElevationAngle;
	AntennaScanningPeriod = service.AntennaScanningPeriod;
	AntennaPedestalDirection = service.AntennaPedestalDirection;
	AzimuthalAntennaTilt = service.AzimuthalAntennaTilt;
	ElevationAntennaTilt = service.ElevationAntennaTilt;
	ElevationCorrection = service.ElevationCorrection;
	RunwayInUse = service.RunwayInUse;
	ConfigurationStatus = service.ConfigurationStatus;
	Position3D = service.Position3D;
}

