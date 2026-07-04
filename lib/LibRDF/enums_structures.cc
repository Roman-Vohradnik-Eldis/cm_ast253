#include "enums_structures.hh"
#include <string.h>


/*
void	ANY_DATA_LIST :: init ( uint len )
{
	init ();
	if ( len > 0 && len < 256 )
	{
		Data = new 
	}
};
*/

void	ANY_DATA_LIST :: assign ( const ANY_DATA_LIST &to_copy )
{
	erase ();

	if ( to_copy.Present > 0 )
	{
		Present = to_copy.Present;
		Data = new unsigned char [ Present ];
		memcpy ( Data, to_copy.Data, Present );
	}
}

