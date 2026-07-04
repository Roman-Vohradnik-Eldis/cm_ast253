#include "RDFSurvCoordFunction.hh"



void	TRANSPONDER_CAP :: assign ( const TRANSPONDER_CAP &tc )
{
	Present	= tc.Present;
	Cap			= tc.Cap;
	Spare		= 0;
}


void	STATUS_TRACK_INNER :: assign ( const STATUS_TRACK_INNER &sti )
{
	CST		= sti.CST;
	FLT		= sti.FLT;
	Spare	= 0;
}


void	STATUS_TRACK :: assign ( const STATUS_TRACK &st )
{
	Present	= st.Present;
	StatusTrack.assign ( st.StatusTrack );
}



void	STATION_LIST :: assign ( const STATION_LIST &to_copy )
{
	erase ();
	if ( to_copy.Present > 0 )
	{
		Present = to_copy.Present;
		Data = to_copy.Data;
	}
}


void	STATION_LIST :: add_ss ( unsigned char SIC, unsigned char SAC )
{
	RADAR_ID	rid;
	rid.SIC	= SIC;
	rid.SAC	= SAC;

	add_ss ( rid );
}


void	STATION_LIST :: add_ss ( RADAR_ID rid )
{
	Data.push_back ( rid );

	Present = Data.size ();
}


void	STATION_LIST :: ins_ss ( RADAR_ID rid )
{
	Data.push_back ( rid );
}


/*
void	STATION_LIST :: assign ( const STATION_LIST &to_copy )
{
	erase ();

	if ( to_copy.Present > 0 )
	{
		Present = to_copy.Present;
		Data = new unsigned char [ 2 * Present ];
		memcpy ( Data, to_copy.Data, 2 * Present );
	}
}


void	STATION_LIST :: add_ii ( unsigned char SIC, unsigned char SAC )
{
	if ( !Data && !Present )
		Data = new unsigned char [50];

	Data [ 2 * Present ] = SAC;
	Data [ 2 * Present + 1 ] = SIC;
	Present++;
}
*/




RDFSurvCoordFunction :: RDFSurvCoordFunction ()
{
	init ();
}


RDFSurvCoordFunction ::	~RDFSurvCoordFunction ()
{
	erase ();
}


RDFSurvCoordFunction::RDFSurvCoordFunction ( const RDFTargetTrack & track )
{
	init ();
	assign ( track );
}

RDFSurvCoordFunction::RDFSurvCoordFunction ( const RDFTargetTrack * track )
{
	init ();
	if ( track ) assign ( *track );
}


RDFSurvCoordFunction::RDFSurvCoordFunction ( const RDFSurvCoordFunction & scf )
{
	init ();
	assign ( scf );
}


RDFSurvCoordFunction::RDFSurvCoordFunction ( const RDFSurvCoordFunction * scf )
{
	init ();
	if ( scf ) assign ( *scf );
}


const	RDFSurvCoordFunction & RDFSurvCoordFunction :: operator = ( const RDFSurvCoordFunction & scf )
{
//	RDFTargetTrack::erase ();
//	RDFTargetTrack::assign (track);
	erase ();
	assign ( scf );
	return scf;
}


void	RDFSurvCoordFunction :: init ()
{
	Source.init ();
	Destination.init ();
	CAT	= 17;
	Message_type	= NETWORK_INFORMATION;
	Station_list.init ();

#if		ZDEDENO_Z_TARGET_TRACK
	RDFTargetTrack::init ();
#else
	AircraftAddress.Present = false;
	AircraftAddress.Address = 0;

	DuplAddrRefNum.Present	= false;
	DuplAddrRefNum.Number		= 0;

	Time.Present = false;
	Time.Time = 0;

	Geographic	= NULL;
	Mode3A			= NULL;
	FlightLevel	= NULL;
	GroundSpeed	= NULL;	//speed and heading
#endif

	TransponderCap.init ();
	TrackStatus.init ();
	ModeS_addr_list.init ();
	CC_State.init ();
}


void	RDFSurvCoordFunction :: erase ()
{
	Station_list.erase ();
	ModeS_addr_list.erase ();

#if		ZDEDENO_Z_TARGET_TRACK
	RDFTargetTrack::erase ();
#else
	if ( Geographic )
		delete Geographic;

	if ( Mode3A )
		delete Mode3A;

	if ( FlightLevel )
		delete FlightLevel;

	if ( GroundSpeed )
		delete GroundSpeed;
#endif
}


void	RDFSurvCoordFunction :: assign ( const RDFSurvCoordFunction & to_copy )
{
	Source.SIC	= to_copy.Source.SIC;
	Source.SAC	= to_copy.Source.SAC;
	Destination.SIC	= to_copy.Destination.SIC;
	Destination.SAC	= to_copy.Destination.SAC;

	Message_type	= to_copy.Message_type;
	Station_list.assign ( to_copy.Station_list );

#if		ZDEDENO_Z_TARGET_TRACK
	RDFTargetTrack::assign ( to_copy );
#else
	AircraftAddress.Present = to_copy.AircraftAddress.Present;
	AircraftAddress.Address = to_copy.AircraftAddress.Address;

	DuplAddrRefNum.Present	= to_copy.DuplAddrRefNum.Present;
	DuplAddrRefNum.Number		= to_copy.DuplAddrRefNum.Number;

	Time.Present	= to_copy.Time.Present;
	Time.Time			= to_copy.Time.Time;

	if ( to_copy.Geographic ) Geographic		= new RDFCoorGeographic ( to_copy.Geographic );
	if ( to_copy.Mode3A ) Mode3A = new RDFTargetMode ( to_copy.Mode3A );
	if ( to_copy.FlightLevel ) FlightLevel	= new RDFTargetFlightLevel ( to_copy.FlightLevel );
	if ( to_copy.GroundSpeed ) GroundSpeed	= new RDFTargetGroundSpeed ( to_copy.GroundSpeed );
#endif

	TransponderCap.assign ( to_copy.TransponderCap );
	TrackStatus.assign ( to_copy.TrackStatus );
	ModeS_addr_list.assign ( to_copy.ModeS_addr_list );
	CC_State.Present	= to_copy.CC_State.Present;
	CC_State.Value		= to_copy.CC_State.Value;
}


void	RDFSurvCoordFunction :: assign ( const RDFTargetTrack & track )
{
/*
	Source.SIC	= track.Source.SIC;
	Source.SAC	= track.Source.SAC;
	Destination.SIC	= track.Destination.SIC;
	Destination.SAC	= track.Destination.SAC;

	Message_type	= track.Message_type;
	Station_list.assign ( track.Station_list );
*/

#if		ZDEDENO_Z_TARGET_TRACK
	RDFTargetTrack::assign ( track );
#else
	AircraftAddress.Present = track.AircraftAddress.Present;
	AircraftAddress.Address = track.AircraftAddress.Address;

	DuplAddrRefNum.Present	= track.TrackNumber.Present;
	DuplAddrRefNum.Number		= track.TrackNumber.Number;

	Time.Present	= track.Time.Present;
	Time.Time			= track.Time.Time;

	if ( track.Geographic ) Geographic		= new RDFCoorGeographic ( track.Geographic );
	if ( track.Mode3A ) Mode3A = new RDFTargetMode ( track.Mode3A );
	if ( track.FlightLevel ) FlightLevel	= new RDFTargetFlightLevel ( track.FlightLevel );
	if ( track.GroundSpeed ) GroundSpeed	= new RDFTargetGroundSpeed ( track.GroundSpeed );
#endif
/*
	TransponderCap.assign ( track.TransponderCap );
	TrackStatus.assign ( track.TrackStatus );
	ModeS_addr_list.assign ( track.ModeS_addr_list );
	CC_State.Present	= CC_State.Present;
	CC_State.Value		= CC_State.Value;
*/
}

