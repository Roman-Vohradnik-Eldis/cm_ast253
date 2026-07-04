#ifndef _RDF_TARGET_CLUSTER_H
#define _RDF_TARGET_CLUSTER_H


//#include "RDFHeader.hh"
//#include "RDFTargetTrack.hh"
#include "stdlib.h"


//#define		FEET		0.3048
const	float	FEET = 0.3048;



struct	RADAR_ID
{
	unsigned char SAC;
	unsigned char SIC;

	void	init () { SAC = SIC = 0; };
	RADAR_ID () { init (); };
	bool operator == (const RADAR_ID &ID) const { return ((ID.SAC == SAC) && (ID.SIC == SIC)); };
};


struct	STRUCT_UCHAR
{
	bool	Present;
	unsigned	char	Value;

	void	init () { Present = false; Value = 0; };
	STRUCT_UCHAR () { init (); };
};


struct	ANY_DATA_LIST
{
	unsigned char	Present;	//pocitadlo dat
	unsigned char *Data;

	ANY_DATA_LIST () { init (); };
	~ANY_DATA_LIST () { erase (); };

	void	init () { Present = 0; Data = NULL; };
//	void	init ( uint len = 0 ); { Present = 0; Data = NULL; };
	void	erase () { if ( Data ) delete [] Data; init (); };
	void	assign ( const ANY_DATA_LIST &to_copy );

	void	add_ss ( unsigned char SIC, unsigned char SAC );
};


#endif
