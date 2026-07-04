#include "RDFWarningAlert.hh"
#include "CMSDebug.hh"

RDFSystemFuncAndStatus::RDFSystemFuncAndStatus()
{
	present=false;
	MRVA=false;
	RAMHD=false;
	RAMLD=false;
	MSAW=false;
	APW=false;
	CLAM=false;
	STCA=false;
	AFDA=false;
	RIMCA=false;
	overflow=false;
	overload=false;
}

RDFAlertSeparation::RDFAlertSeparation ()
{
	TimeToConflictPresent = false;
	TimeToClosestApproachPresent = false;
	CurrentHSeparationPresent = false;
	EstimatedHSeparationPresent = false;
	CurrentVSeparationPresent = false;
	EstimatedVSeparationPresent = false;
	TimeToConflict = 0;
	TimeToClosestApproach = 0;
	CurrentHSeparation = 0;
	EstimatedHSeparation = 0;
	CurrentVSeparation = 0;
	EstimatedVSeparation = 0;
}

RDFAlertSeparation::~RDFAlertSeparation ()
{
}

RDFAlertSeparation::RDFAlertSeparation (const RDFAlertSeparation * separation)
{
	if (!separation)
	{
		INTERNAL_ERROR ("null pointer");
	}
	TimeToConflictPresent = separation->TimeToConflictPresent;
	TimeToClosestApproachPresent = separation->TimeToClosestApproachPresent;
	CurrentHSeparationPresent = separation->CurrentHSeparationPresent;
	EstimatedHSeparationPresent = separation->EstimatedHSeparationPresent;
	CurrentVSeparationPresent = separation->CurrentVSeparationPresent;
	EstimatedVSeparationPresent = separation->EstimatedVSeparationPresent;
	TimeToConflict = separation->TimeToConflict;
	TimeToClosestApproach = separation->TimeToClosestApproach;
	CurrentHSeparation = separation->CurrentHSeparation;
	EstimatedHSeparation = separation->EstimatedHSeparation;
	CurrentVSeparation = separation->CurrentVSeparation;
	EstimatedVSeparation = separation->EstimatedVSeparation;
}

RDFAlertArea::RDFAlertArea ()
{
}

RDFAlertArea::~RDFAlertArea ()
{
}

RDFAlertArea::RDFAlertArea (const RDFAlertArea * area)
{
	if (!area)
	{
		INTERNAL_ERROR ("null pointer");
	}
	Name = area->Name;
}

RDFConflictClassification::RDFConflictClassification ()
{
 	TableId = 0;
	ConflictProperties = 0;
	ConflictSeverity = 0;
}

RDFConflictCharacteristics::RDFConflictCharacteristics ()
{
	ConflictClassificationPresent = false;

	DurationPresent = false;
	Duration = 0;
}

RDFConflictCharacteristics::~RDFConflictCharacteristics ()
{
}

RDFConflictCharacteristics::RDFConflictCharacteristics (const RDFConflictCharacteristics * characteristics)
{
	if (!characteristics)
	{
		INTERNAL_ERROR ("null pointer");
	}
	ConflictClassificationPresent = characteristics->ConflictClassificationPresent;
	ConflictClassification = characteristics->ConflictClassification;

	DurationPresent = characteristics->DurationPresent;
	Duration = characteristics->Duration;
}

RDFWarningAlert::RDFWarningAlert ()
{
	init ();
}

RDFWarningAlert::RDFWarningAlert (const RDFWarningAlert & alert)
{
	init ();
	assign (alert);
}

RDFWarningAlert::~RDFWarningAlert ()
{
	erase ();
}

const RDFWarningAlert & RDFWarningAlert::operator = (const RDFWarningAlert & alert)
{
	erase ();
	assign (alert);
	return alert;
}

void RDFWarningAlert::init ()
{	
	Time.init ();

	SAC = 0;
	SIC = 0;

	AlertType = ALERT_TYPE_NONE;
	AlertID = 0;
	AlertStatus = 0;

	TrackNumber.Present = false;
	TrackNumber.Number = 0;
	TrackNumber2.Present = false;
	TrackNumber2.Number = 0;

	Separation = NULL;
	Area = NULL;
	Conflict = NULL;

	VerticalDeviationPresent = false;
	VerticalDeviation = 0;
}

void RDFWarningAlert::erase ()
{	
	if (Separation)
	{
		delete Separation;
		Separation = NULL;
	}
	if (Area)
	{
		delete Area;
		Area = NULL;
	}
	if (Conflict)
	{
		delete Conflict;
		Conflict = NULL;
	}
}

void RDFWarningAlert::assign (const RDFWarningAlert & alert)
{
	Time = alert.Time;

	SAC = alert.SAC;
	SIC = alert.SIC;

	AlertType = alert.AlertType;
	AlertID = alert.AlertID;
	AlertStatus = alert.AlertStatus;

	TrackNumber.Present = alert.TrackNumber.Present;	
	TrackNumber.Number = alert.TrackNumber.Number;
	TrackNumber2.Present = alert.TrackNumber2.Present;
	TrackNumber2.Number = alert.TrackNumber2.Number;

	if (alert.Separation)
	{
		Separation = new RDFAlertSeparation (alert.Separation);
	}
	if (alert.Area)
	{
		Area = new RDFAlertArea (alert.Area);
	}
	if (alert.Conflict)
	{
		Conflict = new RDFConflictCharacteristics (alert.Conflict);
	}

	VerticalDeviationPresent = alert.VerticalDeviationPresent;
	VerticalDeviation = alert.VerticalDeviation;
}

