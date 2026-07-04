#ifndef _RDF_WARNING_ALERT_H
#define _RDF_WARNING_ALERT_H

#include "RDFHeader.hh"
#include "RDFTargetStruct.hh"

enum ALERT_TYPE
{
	ALERT_TYPE_NONE,			// no alert
	ALERT_TYPE_ALIVE,			// Alive Message
	ALERT_TYPE_RAMLD,			// Route Adherence Monitor Longitudinal Deviation
	ALERT_TYPE_RAMHD,			// Route Adherence Monitor Heading Deviation
	ALERT_TYPE_MSAW,			// Minimum Safe Altitude Warning
	ALERT_TYPE_APW,				// Area Proximity Warning
	ALERT_TYPE_CLAM,			// Clearance Level Adherence Monitor
	ALERT_TYPE_STCA,			// Short Term Conflict Alert
	ALERT_TYPE_AFDA,			// Approach Funnel Deviation Alert
	ALERT_TYPE_ALM,				// RIMCAS Arrival / Landing Monitor
	ALERT_TYPE_WRA,				// RIMCAS Arrival / Departure Wrong Runway Alert
	ALERT_TYPE_OTA,				// RIMCAS Arrival / Departure Oposite Traffic Alert
	ALERT_TYPE_RDM,				// RIMCAS Departure Monitor
	ALERT_TYPE_RCM,				// RIMCAS Runway / Taxiway Crossing Monitor
	ALERT_TYPE_TSM,				// RIMCAS Taxiway Separation Monitor
	ALERT_TYPE_UTMM,			// RIMCAS Unauthorized Taxiway Movement Monitor
	ALERT_TYPE_SBOA,			// RIMCAS Stop Bar Overrun Alert
	ALERT_TYPE_CONFLICT_END,	// End Of Conflict
};

#define ALERT_TYPE_DAIW ALERT_TYPE_APW
#define ALERT_TYPES ALERT_TYPE_CONFLICT_END

struct RDFAlertSeparation
{
	bool TimeToConflictPresent;
	unsigned TimeToConflict;
	bool TimeToClosestApproachPresent;
	unsigned TimeToClosestApproach;
	bool CurrentHSeparationPresent;
	unsigned CurrentHSeparation;
	bool EstimatedHSeparationPresent;
	unsigned EstimatedHSeparation;
	bool CurrentVSeparationPresent;
	unsigned CurrentVSeparation;
	bool EstimatedVSeparationPresent;
	unsigned EstimatedVSeparation;

	RDFAlertSeparation ();
	~RDFAlertSeparation ();
	RDFAlertSeparation (const RDFAlertSeparation * separation);
};

struct RDFAlertArea
{
	string Name;

	RDFAlertArea ();
	~RDFAlertArea ();
	RDFAlertArea (const RDFAlertArea * area);
};

struct RDFConflictClassification
{
	unsigned TableId : 4;
	unsigned ConflictProperties : 3;
	unsigned ConflictSeverity : 1;

	RDFConflictClassification();
};

struct RDFConflictCharacteristics
{
	bool ConflictClassificationPresent;
	RDFConflictClassification ConflictClassification;

	bool DurationPresent;
	unsigned Duration;

	RDFConflictCharacteristics ();
	~RDFConflictCharacteristics ();
	RDFConflictCharacteristics (const RDFConflictCharacteristics * characteristics);
};
struct RDFSystemFuncAndStatus
{
	uint16_t present : 1;
	uint16_t MRVA : 1;
	uint16_t RAMLD : 1;
	uint16_t RAMHD : 1;
	uint16_t MSAW : 1;
	uint16_t APW : 1;
	uint16_t CLAM : 1;
	uint16_t STCA : 1;

	uint16_t AFDA : 1;
	uint16_t RIMCA : 1;
	uint16_t overflow : 1;
	uint16_t overload : 1;

	RDFSystemFuncAndStatus ();

};
struct RDFWarningAlert
{
	RDFTime Time;

	unsigned char SAC;
	unsigned char SIC;

	enum ALERT_TYPE AlertType;
	unsigned AlertID;
	unsigned AlertStatus;

	RDFTrackNumber TrackNumber;
	RDFTrackNumber TrackNumber2;

	RDFSystemFuncAndStatus SystemFuncAndStatus;
	RDFAlertSeparation * Separation;
	RDFAlertArea * Area;
	RDFConflictCharacteristics * Conflict;

	bool VerticalDeviationPresent;
	int VerticalDeviation;
	
	RDFWarningAlert ();
	~RDFWarningAlert ();
	RDFWarningAlert (const RDFWarningAlert & alert);
	const RDFWarningAlert & operator = (const RDFWarningAlert & alert);

  protected:
	void init ();
	void erase ();
	void assign (const RDFWarningAlert & alert);
};

#endif /* _RDF_ALERT_H */
