#ifndef _RDF_SURVEILLANCE_COORDINATION_FUNCTION_HEADER
#define _RDF_SURVEILLANCE_COORDINATION_FUNCTION_HEADER


#include "enums_structures.hh"
//#include "RDFAsterix.hh"
#include "RDFTargetTrack.hh"
#include <vector>


#define		ZDEDENO_Z_TARGET_TRACK		0



enum	ENUM_MESSAGE_TYPE_17
{
	NETWORK_INFORMATION									= 00,
	TRACK_DATA													= 10,

	TRACK_DATA_REQUEST									= 20,
	TRACK_DATA_STOP											= 21,
	CANCEL_TRACK_DATA_REQUEST						= 22,
	TRACK_DATA_STOP_ACK									= 23,

	NEW_NODE_INITIAL										= 30,
	NEW_NODE_FINAL											= 31,
	NEW_NODE_INITIAL_REPLY							= 32,
	NEW_NODE_FINAL_REPLY								= 33,

	MOVE_NODE_TO_NEW_CLUSTER_STATE			= 110,
	MOVE_NODE_TO_NEW_CLUSTER_STATE_ACK	= 111,
		//ELDIS
	CLUSTER_STATE_REQ	= 200
};

/*
struct	DUPL_ADDR_REF_NUM
{
	bool	Present;
	unsigned short	Num;

	void	init () { Present = false; Num = 0; };
	DUPL_ADDR_REF_NUM () { init (); };
};
*/

enum	ENUM_COMM_CAP
{
	COMM_CAP_NO_COMM_CAP17	= 0,
	COMM_CAP_AB_7_GROUND		= 4,
	COMM_CAP_AB_7_AIRBORNE	= 5,
	COMM_CAP_AB_7_BOTH			= 6,
	COMM_CAP_DR							= 7
};


struct	TRANSPONDER_CAP
{
	ENUM_COMM_CAP	Cap				: 3;
	unsigned			Spare			: 5;

	bool	Present;

	void	init () { Present = false; Spare = 0; Cap = COMM_CAP_NO_COMM_CAP17; };
	void	assign ( const TRANSPONDER_CAP &tc );
	TRANSPONDER_CAP () { init(); };
};


struct	STATUS_TRACK_INNER
{
	uint8_t	CST		: 1;
	uint8_t	FLT		: 1;
	uint8_t	Spare	: 6;

	void	init () { CST = FLT = Spare = 0; };
	void	assign ( const STATUS_TRACK_INNER &sti );
	STATUS_TRACK_INNER () { init(); };
};


struct	STATUS_TRACK
{
	bool	Present;
	STATUS_TRACK_INNER	StatusTrack;	

 void	init () { Present = false; StatusTrack.init(); };
	void	assign ( const STATUS_TRACK &st );
	STATUS_TRACK () { init(); };
};


struct	STATION_LIST
{
	unsigned char	Present;	//pocitadlo dat
	std::vector < RADAR_ID > Data;

	STATION_LIST () { init (); };
	~STATION_LIST () { erase (); };

	void	init () { Present = 0; Data.clear (); };
//	void	init ( uint len = 0 ); { Present = 0; Data = NULL; };
	void	erase () { init (); };
	void	assign ( const STATION_LIST &to_copy );

	void	add_ss ( unsigned char SIC, unsigned char SAC );
	void	add_ss ( RADAR_ID rid );
	void	ins_ss ( RADAR_ID rid );
};

/*
struct	STATION_LIST
{
	unsigned char	Present;	//pocitadlo dat
	unsigned char *Data;

	STATION_LIST () { init (); };
	~STATION_LIST () { erase (); };

	void	init () { Present = 0; Data = NULL; };
//	void	init ( uint len = 0 ); { Present = 0; Data = NULL; };
	void	erase () { if ( Data ) delete [] Data; init (); };
	void	assign ( const STATION_LIST &to_copy );

	void	add_ii ( unsigned char SIC, unsigned char SAC );
};
*/






#if	ZDEDENO_Z_TARGET_TRACK
struct	RDFSurvCoordFunction : public RDFTargetTrack
#else
struct	RDFSurvCoordFunction
#endif
{
	unsigned char CAT;

	RADAR_ID							Source;
	RADAR_ID							Destination;
	ENUM_MESSAGE_TYPE_17	Message_type;
	STATION_LIST					Station_list;

#if		!ZDEDENO_Z_TARGET_TRACK
	RDFAircraftAddress		AircraftAddress;
//	DUPL_ADDR_REF_NUM			DuplAddrRefNum;
	RDFTrackNumber				DuplAddrRefNum;
	RDFTime								Time;

	RDFCoorGeographic			*Geographic;
	RDFTargetMode					*Mode3A;
	RDFTargetFlightLevel	*FlightLevel;
	RDFTargetGroundSpeed	*GroundSpeed; //speed and heading
#endif

	TRANSPONDER_CAP				TransponderCap;
	STATUS_TRACK					TrackStatus;
	ANY_DATA_LIST					ModeS_addr_list;
	STRUCT_UCHAR					CC_State;

//***********************************************

///	RDFCoorPolar	*Polar;
///	enum TRACK_CLIMB TrackClimb;


	RDFSurvCoordFunction ();
	~RDFSurvCoordFunction ();

	RDFSurvCoordFunction ( const RDFTargetTrack & track );
	RDFSurvCoordFunction ( const RDFTargetTrack * track );

	RDFSurvCoordFunction ( const RDFSurvCoordFunction & scf );
	RDFSurvCoordFunction ( const RDFSurvCoordFunction * scf );

	const RDFSurvCoordFunction & operator = ( const RDFSurvCoordFunction & scf );

	static vector<string> getkeys ();
	string get (string key);

		void assign ( const RDFTargetTrack & track );

//	protected:
		void init ();
		void erase ();
		void assign ( const RDFSurvCoordFunction & scf );

};


typedef	std::deque < RDFSurvCoordFunction > DEQUE_RDF_SCF;

//typedef	std::map < uint, RDFSurvCoordFunction > MAP_TRACKS;
//typedef	std::map < uint, RDFSurvCoordFunction > :: iterator	MAP_TRACKS_ITER;


#endif	//_RDF_SURVEILLANCE_COORDINATION_FUNCTION_HEADER
