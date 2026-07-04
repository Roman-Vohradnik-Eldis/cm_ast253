#ifndef _RDF_RADAR_SERVICE_H
#define _RDF_RADAR_SERVICE_H

#include "CMSDebug.hh"			//INTERNAL_ERROR
#include "RDFHeader.hh"
#include "RDFTargetTrack.hh"
#include "RDFCoordinate.hh"		//rad2dms
#include "RDFTargetStruct.hh"	//RDFPostion3D

enum RADAR_SERVICE_TYPE
{
	RADAR_SERVICE_TYPE_UNKNOWN = 0,
	RADAR_SERVICE_TYPE_NORTH,
	RADAR_SERVICE_TYPE_SECTOR,
	RADAR_SERVICE_GEO_FILTER,
	RADAR_SERVICE_JAMMING_STROBE,
};

struct RDFSystemConfigurationAndStatus
{
	bool COMPresent;
	char COM;
	bool PSRPresent;
	char PSR;
	bool SSRPresent;
	char SSR;
	bool MDSPresent;
   /**
    * Polozka je dvoubajtova, v MDS_1 se nachazi druhy Bajt
    */
	char MDS;
	char MDS_1;

	  RDFSystemConfigurationAndStatus ();
	 ~RDFSystemConfigurationAndStatus ();
	  RDFSystemConfigurationAndStatus (const RDFSystemConfigurationAndStatus * status);
};

/**
 * Data Item 1034/060
 */
struct RDFSystemProcessingMode
{
	bool COMPresent;
	char COM;
	bool PSRPresent;
	char PSR;
	bool SSRPresent;
	char SSR;
	bool MDSPresent;
	char MDS;

	  RDFSystemProcessingMode ():COMPresent (false), COM (0), PSRPresent (false),
		PSR (0), SSRPresent (false), SSR (0), MDSPresent (false), MDS (0)
	{
	};
	;
	~RDFSystemProcessingMode ()
	{
	};

	RDFSystemProcessingMode (const RDFSystemProcessingMode * mode)
	{

		COMPresent = mode->COMPresent;
		COM = mode->COM;
		PSRPresent = mode->PSRPresent;
		PSR = mode->PSR;
		SSRPresent = mode->SSRPresent;
		SSR = mode->SSR;
		MDSPresent = mode->MDSPresent;
		MDS = mode->MDS;
	}
};

struct RDFCountValues
{
	//TODO: predelat na pole definovat pristupova makra
	uint8_t Rep;				// Number of counters  

	uint16_t NoDetection;		// number of misses
	uint16_t PSR;				// single PSR targets
	uint16_t SSR;				// single SSR targets (Non-Mode S)
	uint16_t SSR_PSR;			// SSR+PSR targets (Non-Mode S)
	uint16_t AllCall;			// single All-Call targets (Mode S)
	uint16_t RollCall;			// single Roll-Call targets (Mode S)
	uint16_t AllCall_PSR;		// All-Call+PSR (Mode S) targets
	uint16_t RollCall_PSR;		// Roll-Call+PSR (Mode S) targets

	//filters
	uint16_t FilterWeather;		// Filter for weather data
	uint16_t FilterJamming;		// Filter for jamming strobe
	uint16_t FilterPSR;			// Filter for PSR data
	uint16_t FilterSSR;			// Filter for SSR data
	uint16_t FilterSSR_PSR;		// FIlter for SSR + PSR data
	uint16_t FilterEnh;			// Filter for enhanced surviealance data
	uint16_t FilterEnhPSR;		// Filter for PSR enhanced data
	uint16_t FilterNotAPI;		// Filter for enhanced PSR SSR data no in area of prome interest
	uint16_t FilterEnhPSR_SSR;	//

	  RDFCountValues ();
	 ~RDFCountValues ();
	  RDFCountValues (const RDFCountValues * values);
};

struct RDFCollimationError
{
	int Range;					// averaged difference in range for PSR target position with respect to SSR target position
	int Azimuth;				// averaged difference in azimuth for PSR target position with respect to SSR target position

	  RDFCollimationError ();
	 ~RDFCollimationError ();
	  RDFCollimationError (const RDFCollimationError * error);
};

struct RDFAntennaRotation
{
	double Speed;				// [s] period between two consecutive north crossings or averaged value

	  RDFAntennaRotation ();
	 ~RDFAntennaRotation ();
	  RDFAntennaRotation (const RDFAntennaRotation * rotation);
};

struct RDFRadarService
{
   /**
    * 31 bitu cas, 1 bit actual
    */
	RDFTime Time;

	uint8_t SAC;
	uint8_t SIC;
   /**
    * Source identificator
    */
	enum RADAR_SERVICE_TYPE Type;
	float Sector;

	uint8_t DataFilter;			/*1 */

	RDFSystemProcessingMode *SystemProcessingMode;	/*1 */
	RDFAntennaRotation *AntennaRotation;
	RDFPosition3D *Position3D;
	RDFSystemConfigurationAndStatus *SystemConfigurationAndStatus;
	RDFCountValues *CountValues;	/*1 */
	RDFCollimationError *CollimationError;	/*1 */

	std::string UserText;

   /**
    *  @func Metoda prevede radar service na string.
    *  @param vystup vystupni znakovy stream do ktereho se vepisou
    *                jednotlive datove pole
    *  @param one_line pokud je nastaveno na 1 nebude se do znakoveho
    *                  proudu vkladat `\n', v tom pripade budou polozky
    *                  oddeleny oddelovacem ';'
    */
	void to_stringstream (std::stringstream & vystup, bool one_line);
	  RDFRadarService ();		//DT filter
	 ~RDFRadarService ();
	  RDFRadarService (const RDFRadarService & service);
	const RDFRadarService & operator = (const RDFRadarService & service);

  protected:
	void init ();
	void erase ();
	void assign (const RDFRadarService & service);
};

#endif /* _RDF_SERVICE_H */
