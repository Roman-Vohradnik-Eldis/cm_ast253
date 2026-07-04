#ifndef _RDF_DATALINK_FUNCTION_H
#define _RDF_DATALINK_FUNCTION_H


#include "RDFHeader.hh"
#include "RDFTargetTrack.hh"
#include "enums_structures.hh"


#define			LEN4		4
#define			LEN6		6
#define			LEN7		7



enum	ENUM_MESSAGE_TYPE_18
{
	ASSOCIATE_REQ						=	0x00,		//Connection establishment
	ASSOCIATE_RESP					= 0x01,		//Connection establishment
	RELEASE_REQ							= 0x02,		//Connection disabling
	RELEASE_RESP 						= 0x03,		//Connection disabling
	ABORT_REQ								= 0x04,		//Connection disabling
	KEEP_ALIVE							= 0x05,		//Keep alive procedure

	AIRCRAFT_REPORT					= 0x10,		//Routing information
	AIRCRAFT_COMMAND				= 0x11,		//Routing information
	II_CODE_CHANGE					= 0x12,		//Routing information

	UPLINK_PACKET						= 0x20,		//Manage uplink data flow
	CANCEL_UPLINK_PACKET		= 0x21,		//Manage uplink data flow
	UPLINK_PACKET_ACK				= 0x22,		//Manage uplink data flow
	DOWNLINK_PACKET					= 0x23,		//Manage downlink data flow

	DATA_XON								= 0x26,		//Flow control
	DATA_XOFF								= 0x27,		//Flow control

	UPLINK_BROADCAST				= 0x30,		//Manage broadcast service
	CANCEL_UPLINK_BROADCAST	= 0x31,		//Manage broadcast service
	UPLINK_BROADCAST_ACK		= 0x32,		//Manage broadcast service
	DOWNLINK_BROADCAST			= 0x34,		//Manage broadcast service

	GICB_EXTRACTION					= 0x40,		//Manage GICB service
	CANCEL_GICB_EXTRACTION	= 0x41,		//Manage GICB service
	GICB_EXTRACTION_ACK			= 0x42,		//Manage GICB service
	GICB_RESPONSE 					= 0x43,		//Manage GICB service

	NO_MESSAGE							= 0x255
};



enum	ENUM_RESULT_CAUSE
{
	RESULT_CAUSE_ACCEPTED		=	0x0,
	RESULT_CAUSE_REJECTED		= 0x1,
	RESULT_CAUSE_CANCELLED	= 0x2,
	RESULT_CAUSE_FINISHED		= 0x3,
	RESULT_CAUSE_DELAYED		= 0x4,
	RESULT_CAUSE_INPROGRESS	= 0x5,
	RESULT_CAUSE_INVALID		= 0x6
};


enum	ENUM_RESULT_DIAG
{
	RESULT_DIAG_NO_DIAG_AVAL				= 0,
	RESULT_DIAG_AIRCRAFT_EXIT				= 1,
	RESULT_DIAG_INCORRECT_ADDR			= 2,
	RESULT_DIAG_IMPOSS_PROCESS_MESS	= 3,
	RESULT_DIAG_INSUFF_DL_CAP				= 4,
	RESULT_DIAG_INVALID_LV_FIELD		= 5,
	RESULT_DIAG_DUPL_REQUEST_NUM		= 6,
	RESULT_DIAG_UNKN_REQUEST_NUM		= 7,
	RESULT_DIAG_TIMER_T3_EXP				= 8,
	RESULT_DIAG_EXP_IR_TIMER				= 9,
	RESULT_DIAG_UF_DISABLE_BY_UPC		= 10				//0x10		//23.9.2012 ... dle bezdi
};


struct RDFTargetSpeed
{
	double Speed;

	RDFTargetSpeed (const double &speed = 0.0) : Speed (speed) {};
	RDFTargetSpeed (RDFTargetSpeed *speed);
	RDFTargetSpeed (const uint8_t *field);
};

struct RDFTargetHeading
{
	double Heading;

	RDFTargetHeading (const double &heading = 0.0) : Heading (heading) {};
	RDFTargetHeading (RDFTargetHeading *heading);
	RDFTargetHeading (const uint8_t *field);
};


struct	RDFResult
{
	ENUM_RESULT_CAUSE	Cause	: 4;
	ENUM_RESULT_DIAG	Diag	: 4;

	bool	Present;
	RDFResult () { Cause = RESULT_CAUSE_INVALID; Diag = RESULT_DIAG_NO_DIAG_AVAL; Present = false; };
};


struct	II_CODE
{
	unsigned char	Current	: 4;
	unsigned char	Former	: 4;

	bool	is_former;
	bool	Present;

	II_CODE () { Current = Former = 0; is_former = false; Present = false; };
	void	assign ( int new_code );
};


enum	ENUM_LINK_MASK
{
	IGNORED	= 0,
	VALID
};


enum	ENUM_LINK_COMMAND
{
	ENABLED	= 0,
	STOPPED
};


struct	DL_COMMAND_INNER
{
	ENUM_LINK_MASK			ULM	: 1;
	ENUM_LINK_MASK			DLM	: 1;
	ENUM_LINK_COMMAND		ULC	: 1;
	ENUM_LINK_COMMAND		DLC	: 1;
	unsigned Spare					: 4;

	DL_COMMAND_INNER ();
};


struct	DL_COMMAND
{
	bool	Present;
	DL_COMMAND_INNER	Command;

	DL_COMMAND () { Present = false; };
};


struct	DL_STATUS
{
	bool Present;
	ENUM_LINK_COMMAND	UDS	: 1;
	ENUM_LINK_COMMAND	DDS	: 1;
	ENUM_LINK_COMMAND	UCS	: 1;
	ENUM_LINK_COMMAND	DCS	: 1;
	unsigned	EI			: 1;
	unsigned	IC			: 1;

	DL_STATUS ();
};


struct	DL_REPORT_REQUEST
{
	bool Present;
	unsigned	DL_Status			: 1;
	unsigned	D_COM					: 1;
	unsigned	D_ECA					: 1;
	unsigned	D_CQF					: 1;
	unsigned	D_CQF_Method	: 1;
	unsigned	D_Polar_Pos		: 1;
	unsigned	D_Cart_Pos		: 1;
	/* 1st extent */
	unsigned	ID						: 1;
	unsigned	Mode_A				: 1;
	unsigned	Speed					: 1;
	unsigned	Height				: 1;
	unsigned	Heading				: 1;

	DL_REPORT_REQUEST () { DL_Status = D_COM = D_ECA = D_CQF = D_CQF_Method = D_Polar_Pos = D_Cart_Pos = ID = Mode_A = Speed = Height = Heading = 0; };
};


struct	D_COM
{
	unsigned Spare			: 5;
	COMM_CAPABILITY ComCap	: 3;

	bool	Present;
	D_COM () { Present = false; Spare = 0; ComCap = COMM_CAPABILITY_NONE; };
};


enum	AIRCRAFT_CQF
{
	AIRBORNE	= 0,
	GROUND
};


struct	D_CQF
{
	AIRCRAFT_CQF	FlightStatus	: 1;
	unsigned			CQF						: 7;

	bool	Present;
	D_CQF () { Present = false; FlightStatus = AIRBORNE; CQF = 0; };
};


enum	ENUM_PACKET_TYPE
{
	SVC_PACKETS		= 0,
	MSP_PACKETS		= 1,
	ROUTE_PACKETS	= 2
};


struct	D_PACKET_PROPERTIES
{
	unsigned	Spare					: 1;
	unsigned	Priority			: 5;
	ENUM_PACKET_TYPE	Type	: 2;

	bool	Present;
	D_PACKET_PROPERTIES () { Present = false; Spare = Priority = 0; Type = SVC_PACKETS; };
};


struct	BROADCAST_PROPERTIES_INNER
{
	unsigned	Priority	: 4;
	unsigned	Power			: 4;
	unsigned	char	Duration;
	unsigned	Coverage;

	BROADCAST_PROPERTIES_INNER () { Priority = Power = Duration = 0; Coverage = 0L; };
};

struct	BROADCAST_PROPERTIES
{
	bool	Present;
	BROADCAST_PROPERTIES_INNER	Prop;

	BROADCAST_PROPERTIES () { Present = false; };
};


struct	BROADCAST_PREFIX_INNER
{
	unsigned	Spare			: 5;
	unsigned	Prefix		: 27;

	BROADCAST_PREFIX_INNER () { Spare = Prefix = 0; };
};


struct	BROADCAST_PREFIX
{
	bool	Present;
	BROADCAST_PREFIX_INNER	Prefix;
	
	BROADCAST_PREFIX () { Present = false; };
};


struct	STRUCT_UINT
{
	bool	Present;
	unsigned	Value;

	STRUCT_UINT () { Present = false; Value = 0L; };
};

/*
struct	PACKET_LIST
{
	unsigned char	Present;
	unsigned char *Data;

	PACKET_LIST () { init (); };
	~PACKET_LIST () { erase (); };

	void	init () { Present = 0; Data = NULL; };
	void	erase () { if ( Data ) delete [] Data; init (); };
	void	assign ( const PACKET_LIST &to_copy );
	
};
*/
/*
struct	GICB_PERIODICITY
{
	bool	Present;
	unsigned	Second;

	GICB_PERIODICITY () { Present = false; Second = 0L; };
};
*/

struct	GICB_PROP_INNER
{
	unsigned	Priority	: 5;
	unsigned	Spare			: 3;
	unsigned	PC				: 1;
	unsigned	AU				: 1;
	unsigned	NE				: 1;
	unsigned	RD				: 2;
	unsigned	Spare2		: 3;

	GICB_PROP_INNER () { Priority = Spare = PC = AU = NE = RD = Spare2 = 0; };
};


struct	D_GICB_PROPERTIES
{
	GICB_PROP_INNER	Prop;

	bool	Present;
	D_GICB_PROPERTIES () { Present = false; };
};


struct RDFDatalinkFunction
{

	RADAR_ID Source;
	RADAR_ID Destination;
	ENUM_MESSAGE_TYPE_18 Message_type;
	RDFResult Result;
	RDFAircraftAddress AircraftAddress;
	STRUCT_UINT Packet_Num;
	std::vector<uint32_t> Packet_List;

//**********************************************

	D_PACKET_PROPERTIES Packet_Prop;
	std::vector<uint8_t> ModeS_Packet;
	STRUCT_UINT GICB_Periodicity;
	D_GICB_PROPERTIES GICB_Prop;
	STRUCT_UINT GICB_Num;
	STRUCT_UCHAR BDS_Code;
	RDFTargetModeS *GICB_Extracted;

//**********************************************

	RDFTime Time;
	std::vector<uint32_t> ModeS_addr_list;
	DL_COMMAND DL_Command;
	DL_STATUS DL_Status;
	DL_REPORT_REQUEST DL_Report_Req;
	D_COM D_Comm;
	RDFTargetModeS *CapReport;

//**********************************************

	RDFCoorPolar *Polar_Pos;
	RDFCoorCartesian *Cartesian_Pos;
	STRUCT_UINT Broadcast_Num;
	BROADCAST_PROPERTIES Broadcast_Prop;
	BROADCAST_PREFIX Broadcast_Prefix;
	RDFTargetModeS *Broadcast;
	II_CODE II_Code;

//**********************************************

	std::string AircraftIdentity;
	RDFTargetMode *Mode3A;
	RDFTargetFlightLevel *FlightLevel;
	RDFTargetSpeed *Speed;
	RDFTargetHeading *Heading;
	D_CQF CQF;
	STRUCT_UCHAR CQF_Method;

	unsigned char CAT;

	RDFDatalinkFunction ();
	~RDFDatalinkFunction ();

	RDFDatalinkFunction (const RDFTargetPlot & plot);
	RDFDatalinkFunction (const RDFTargetPlot * plot);
	RDFDatalinkFunction (const RDFTargetTrack &target);
	RDFDatalinkFunction (const RDFDatalinkFunction & track);
	RDFDatalinkFunction (const RDFDatalinkFunction * track);

	const RDFDatalinkFunction & operator = (const RDFDatalinkFunction & track);

	static vector<string> getkeys ();
	string get (string key);

	protected:
		void init ();
		void erase ();
		void assign ( const RDFDatalinkFunction & track );

};

#endif //	_RDF_DATALINK_FUNCTION_H
