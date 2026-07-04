#ifndef _RDF_PAR_SERVICE_H
#define _RDF_PAR_SERVICE_H

#include "RDFHeader.hh"
#include "RDFTargetStruct.hh"

enum PAR_SERVICE_MESSAGE_TYPE
{
	PAR_SERVICE_MESSAGE_TYPE_UNKNOWN = 0,
	PAR_SERVICE_MESSAGE_TYPE_AZIMUTHAL_SECTOR = 1,
	PAR_SERVICE_MESSAGE_TYPE_ELEVATION_SECTOR = 2,
	PAR_SERVICE_MESSAGE_TYPE_PERIODIC_STATUS = 3,
};

struct RDFParElevationAntennaTilt
{
	float X;
	float Y;

	RDFParElevationAntennaTilt ();
	void init ();
};

struct RDFParRadarBaseTilt
{
  float X;
  float Y;

  RDFParRadarBaseTilt ();
  void init ();
};

struct RDFParRunwayInUse
{
	uint8_t IDPresent : 1;
	uint8_t RWDPresent : 1;
	uint8_t TDDPresent : 1;
	uint8_t LDRPresent : 1;
	uint8_t TDEPresent : 1;
	uint8_t RWNPresent : 1;

	std::string ID;
	float RWD;
	float TDD;
	float LDR;
	float TDE;
	uint8_t RWN;

	RDFParRunwayInUse ();
	void init ();
};

struct RDFParSystemConfigurationStatusCOM
{
	uint8_t NOGO : 1;
	uint8_t RDPC : 1;
	uint8_t RDPR : 1;
	uint8_t OVLRDP : 1;
	uint8_t OVLXMT : 1;
	uint8_t MSC : 1;
	uint8_t TSV : 1;

	RDFParSystemConfigurationStatusCOM ();
	void init ();
};

struct RDFParSystemConfigurationStatusWND
{
	float SPEED_10s;
	float DIRECTION_10s;
	float SPEED_1min;
	float DIRECTION_1min;

	RDFParSystemConfigurationStatusWND ();
	void init ();
};

struct RDFParSystemConfigurationStatusPAR
{
	uint8_t ANT : 1;
	uint8_t CHAB : 2;
	uint8_t OVL : 1;
	uint8_t MRC : 1;
	uint8_t ARR : 1;
	uint8_t OPR : 1;
	uint8_t CAL : 1;

	RDFParSystemConfigurationStatusPAR ();
	void init ();
};

struct RDFParSystemConfigurationStatusCTR
{
	uint8_t RNG : 1;
	uint8_t FREQ : 1;
  uint8_t PSDM : 1;

	RDFParSystemConfigurationStatusCTR ();
	void init ();
};

struct RDFParSystemConfigurationStatusPOL
{
	uint8_t CRA : 1;
  uint8_t CLA : 1;
	uint8_t CRE : 1;
  uint8_t CLE : 1;
	uint8_t LVA : 1;
	uint8_t LHA : 1;
	uint8_t LVE : 1;
	uint8_t LHE : 1;

	RDFParSystemConfigurationStatusPOL ();
	void init ();
};

struct RDFParSystemConfigurationStatusPSV
{
  float AngleElevation;
	float AngleAzimuthal;
	float AspectRatioElevation;
  float AspectRatioAzimuthal;

	RDFParSystemConfigurationStatusPSV ();
	void init ();
};

struct RDFParSystemConfigurationStatus
{
	uint8_t COMPresent : 1;
	uint8_t WNDPresent : 1;
	uint8_t PARPresent : 1;
	uint8_t CTRPresent : 1;
	uint8_t POLPresent : 1;
	uint8_t PSVPresent : 1;

	RDFParSystemConfigurationStatusCOM COM;
	RDFParSystemConfigurationStatusWND WND;
	RDFParSystemConfigurationStatusPAR PAR;
	RDFParSystemConfigurationStatusCTR CTR;
	RDFParSystemConfigurationStatusPOL POL;
	RDFParSystemConfigurationStatusPSV PSV;

	RDFParSystemConfigurationStatus ();
	void init ();
};

struct RDFParService
{
	uint8_t SectorAnglePresent : 1;
	uint8_t AzimuthalElevationAnglePresent : 1;
	uint8_t AntennaScanningPeriodPresent : 1;
	uint8_t AntennaPedestalDirectionPresent : 1;
	uint8_t AzimuthalAntennaTiltPresent : 1;
	uint8_t ElevationAntennaTiltPresent : 1;
	uint8_t RadarBaseTiltPresent : 1;
	uint8_t ElevationCorrectionPresent : 1;
	uint8_t RunwayInUsePresent : 1;
	uint8_t ConfigurationStatusPresent : 1;
	uint8_t Position3DPresent : 1;

	enum PAR_SERVICE_MESSAGE_TYPE MessageType;
	uint8_t SAC;
	uint8_t SIC;
	float SectorAngle;
	RDFTime Time;
	float AzimuthalElevationAngle;
	uint32_t AntennaScanningPeriod;
	float AntennaPedestalDirection;
	float AzimuthalAntennaTilt;
	struct RDFParElevationAntennaTilt ElevationAntennaTilt;
	struct RDFParRadarBaseTilt RadarBaseTilt;
	float ElevationCorrection;
	struct RDFParRunwayInUse RunwayInUse;
	struct RDFParSystemConfigurationStatus ConfigurationStatus;
	struct RDFPosition3D Position3D;

  public:	
	RDFParService ();
	~RDFParService ();
 	RDFParService (const RDFParService & service);
	const RDFParService & operator = (const RDFParService & service);

  protected:
	void init ();
	void assign (const RDFParService & service);
};

#endif /* _RDF_SERVICE_H */

