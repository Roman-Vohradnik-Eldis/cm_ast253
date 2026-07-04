#include "CMSDebug.hh"
#include "ClusterMap.hh"
#include <stdio.h>
#include <string.h>
#include <glib.h>



////////	CELL_DEF	////////////////////////////////////////////////////////////

CELL_DEF :: CELL_DEF ()
{
	num_of_bands	= 0;
	alt_bands.clear ();
}


CELL_DEF :: ~CELL_DEF ()
{
	for ( uint i=0; i<alt_bands.size (); i++ )
		delete alt_bands[i];

	alt_bands.clear ();
}



////////	CLUSTER_MAP	////////////////////////////////////////////////////////////
//
CLUSTER_MAP :: CLUSTER_MAP ()
{
	comment	= NULL;
	num_of_deltas	= 0;

	cells.clear ();
}


CLUSTER_MAP :: ~CLUSTER_MAP ()
{
	if ( comment )
		delete [] comment;

	for ( uint i=0; i<cells.size (); i++ )
		delete cells[i];

	cells.clear ();
}


////////	ClusterMap	////////////////////////////////////////////////////////////

ClusterMap :: ClusterMap ( const char *filename, unsigned char id, uint color_index )
{
	m_map_id		= id;
	m_show			= false;
	m_map_bands	= NULL;
	m_color_index	= color_index;

	m_min_band	=	0xFFFF;
	m_max_band	= 0;

	m_clu_map.lat_max	= m_clu_map.lon_max	= 0;


	FILE *fgs	= fopen ( filename, "rb" );
	if ( fgs )
	{
		//unsigned	char bafr[100];
		int32_t		pom32, precteno;
		//int16_t		pom16;

		precteno = fread ( &m_clu_map, 1, 39, fgs );

		if ( m_clu_map.comment_length )
		{
			m_clu_map.comment	= new char [ m_clu_map.comment_length + 1 ];
			memset ( m_clu_map.comment, 0x0, m_clu_map.comment_length+1 );
			precteno = fread ( m_clu_map.comment, 1, m_clu_map.comment_length, fgs );
		}

		precteno = fread ( &m_clu_map.map_ref, 1, 1, fgs );

		precteno = fread ( &pom32, 1, 4, fgs );
		m_clu_map.lat		= GUINT32_FROM_BE ( pom32 ) / 360000.0;

		precteno = fread ( &pom32, 1, 4, fgs );
		m_clu_map.lon		= GUINT32_FROM_BE ( pom32 ) / 360000.0;

		precteno = fread ( &pom32, 1, 4, fgs );
		m_clu_map.lat_d		= GUINT32_FROM_BE ( pom32 ) / 360000.0;

		precteno = fread ( &pom32, 1, 4, fgs );
		m_clu_map.lon_d		= GUINT32_FROM_BE ( pom32 ) / 360000.0;

		precteno	= fread ( &m_clu_map.num_of_lat_rows, 1, 4, fgs );
		if ( precteno )
		{
			m_clu_map.num_of_deltas	= GUINT16_FROM_BE ( m_clu_map.num_of_deltas );
			//g_print ( "\n\n\n\nNUM DELTAS = %u\n", m_clu_map.num_of_deltas );
			for ( uint cell=0; cell<m_clu_map.num_of_deltas; cell++ )
			{
				CELL_DEF	*cd = new CELL_DEF;
				precteno	= fread ( &cd->lat_slice_num, 1, 3, fgs );
				//g_print ( "\tNUM BANDS = %u\n", cd->num_of_bands );
				if ( precteno )
				{

					for ( uint band=0; band<cd->num_of_bands; band++ )
					{
						ALT_BAND	*ab	= new ALT_BAND;
						precteno	= fread ( &ab->ground_station, 1, 4, fgs );
						if ( precteno )
							cd->alt_bands.push_back ( ab );
						else
						{
							cms_ns_error("librdf","fread ERR !!! 2" );
							break;
						}
					}
					m_clu_map.cells.push_back ( cd );

					//g_print ( "CELL %u - pos=%li\n", cell, ftell ( fgs ) );

				}
				else
				{
					cms_ns_error("librdf","fread ERR !!! 2" );
					break;
				}

			}		//for ( uint cell=0; cell<m_clu_map.num_of_deltas; cell++ )

			m_clu_map.lat_max	= m_clu_map.lat + m_clu_map.num_of_lat_rows * m_clu_map.lat_d;
			m_clu_map.lon_max	= m_clu_map.lon + m_clu_map.num_of_lon_cols * m_clu_map.lon_d;

		}
		else
			cms_ns_error("librdf","fread ERR !!! 1" );

		unsigned char neco[20];
		size_t	len	= fread ( &neco, 1, 20, fgs );
		if ( len > 0 )
			cms_ns_info("librdf","Last map id=%u fread = %zu", id, len );

		InitMap ();

		fclose ( fgs );
	}		//if ( fgs )

}


ClusterMap :: ~ClusterMap ()
{
	for ( uint c=0; c<m_clu_map.cells.size(); c++ )
	{
		CELL_DEF	*cd = m_clu_map.cells[c];
		if ( cd )
			delete cd;
	}
	m_clu_map.cells.clear ();

	if ( m_map_bands )
		delete [] m_map_bands;
}


void	ClusterMap :: InitMap ()
{
	uint	map_size	= m_clu_map.num_of_lat_rows * m_clu_map.num_of_lon_cols;

	m_map_bands	= new VECTOR_BANDS [ map_size ];

	if ( m_map_bands )
	{
		for ( uint i=0; i<map_size; i++ )
			m_map_bands[i].clear ();

		VECTOR_BANDS	new_bands;

		for ( uint i=0; i<m_clu_map.cells.size(); i++ )
		{

/*
			if ( 88 == i )
			{
				g_print ( "TADYYYYYYY!!!!\n" );
			}
			if ( i+1 == m_clu_map.cells.size() )
			{
				g_print ( "TADYYYYYYY!!!!\n" );
			}
*/
			CELL_DEF	*cell1 = m_clu_map.cells[i];
			VECTOR_BANDS	bands = cell1->alt_bands;

			for ( uint b=0; b<bands.size(); b++ )
			{
				ALT_BAND	*band = bands[b];

				//ruzne vysky = aktivni = PRIDAVAM nebo MENIM !!!
				if ( band->low_alt != band->high_alt )
				{
					bool	nalezeno = false;
					//for ( uint nb=0; nb<new_bands.size(); nb++ )
                                        for (VECTOR_BANDS_ITER nb = new_bands.begin(); nb != new_bands.end();nb++)
					{
						ALT_BAND	*new_band = (*nb);//new_bands[nb];
						if ( new_band->ground_station == band->ground_station )
						{
							if ( new_band->map_type == band->map_type )
							{
								//NALEZENO !! zmenime vysky
								nalezeno	= true;
								new_band->low_alt	= band->low_alt;
								new_band->high_alt	= band->high_alt;
								break;
							}else if((new_band->map_type==4 && band->map_type==8)
                                                                ||(new_band->map_type==8 && band->map_type==4)){
                                                            
                                                                new_bands.erase(nb);
                                                                break;
                                                        }
						}
					}

					if ( !nalezeno )
						new_bands.push_back ( band );

					//g_print ( "new band add %u\n", new_bands.size () );
				}
				else
				//stejne vysky = deaktivace = RUSIM
				//if ( band->low_alt == band->high_alt )
				{

/*
					VECTOR_BANDS_ITER nb = new_bands.begin();
					VECTOR_BANDS_ITER iter_end = new_bands.end();
					while ( nb != iter_end )
*/
					for ( VECTOR_BANDS_ITER nb = new_bands.begin(); nb != new_bands.end(); )
					//for ( uint nb=0; nb<new_bands.size(); nb++ )
					{
						ALT_BAND	*band_to_del = (*nb);
						if ( band_to_del )
						{
							if ( band_to_del->ground_station == band->ground_station )
							{
								if ( ( band_to_del->map_type == band->map_type ) 
                                                                        || ( 8 == band_to_del->map_type && 4 == band->map_type )
                                                                        || ( 4 == band_to_del->map_type && 8 == band->map_type ) )
								{
									nb = new_bands.erase ( nb );
									//iter_end = new_bands.end();
									//break;

									//g_print ( "new band del %u\n", new_bands.size () );
								}
								else
									++nb;
							}
							else
								++nb;
						}	//if ( del_band )
						else
							break;
					}	//for ( VECTOR_BANDS_ITER nb = .... )

				}	//if ( band->low_alt == band->high_alt )

			}	//for ( uint b=0; b<bands.size(); b++ )

			uint index1 = cell1->lat_slice_num * m_clu_map.num_of_lon_cols + cell1->lon_cell_num;
			uint index2	= 0;

			if ( i+1 == m_clu_map.cells.size() )
			{
				//posledni (nejvrchnejsi) radek
					if ( new_bands.size() )
					{
						//muze byt aktivni opravdu az do konce
						//g_print ( "Aktivni band na ruznych radcich !! rows=%u,%u\n", cell1->lat_slice_num, cell2->lat_slice_num );

						index2 = cell1->lat_slice_num * m_clu_map.num_of_lon_cols;
						
						for ( uint i=index1; i<index2; i++ )
							m_map_bands[i] = new_bands;

						new_bands.clear ();
					}
				
			}
			else
			{
				CELL_DEF	*cell2 = m_clu_map.cells[i+1];
				index2 = cell2->lat_slice_num * m_clu_map.num_of_lon_cols + cell2->lon_cell_num;

				if ( cell1->lat_slice_num == cell2->lat_slice_num )
				{
				/*
					//muze byt "snulovan" samozrejme behem radku, a i opakovane
					if ( !new_bands.size() )
						g_print ( "Prazdny band na stejnem radku !! row=%u\n", cell1->lat_slice_num );
				*/
					for ( uint i=index1; i<index2; i++ )
						m_map_bands[i] = new_bands;

				}
				else
				{
					if ( new_bands.size() )
					{
						//muze byt aktivni opravdu az do konce
						//g_print ( "Aktivni band na ruznych radcich !! rows=%u,%u\n", cell1->lat_slice_num, cell2->lat_slice_num );
						//PrintBands ( &new_bands );

						index2 = cell2->lat_slice_num * m_clu_map.num_of_lon_cols;
						
						for ( uint i=index1; i<index2; i++ )
							m_map_bands[i] = new_bands;

						new_bands.clear ();
					}

				}	//if ( cell1->lat_slice_num == cell2->lat_slice_num )

			}	//if ( i+1 == m_clu_map.cells.size() )

		}	//for ( uint i=0; i<m_clu_map.cells.size()-1; i++ )


		for ( uint i=0; i<map_size; i++ )
		{
			VECTOR_BANDS	bands = m_map_bands[i];
			//tady rozpitvat jednotlivy bands
			if ( bands.size () )
			{
				if ( i < m_min_band )
					m_min_band	= i;
				if ( i > m_max_band )
					m_max_band	= i;
			}
		}

		//g_print ( "min=%u\tmax=%u\n", m_min_band, m_max_band );
	}
}


/*
const	VECTOR_BANDS*	ClusterMap :: GetBands ( double azim, double range, double height )
{
	double lat, lon;

	lat = lon = 0.0;
	return GetBands ( lat, lon );
}
*/

const	VECTOR_BANDS*	ClusterMap :: GetBands ( double lat, double lon )
{
	if ( lat > m_clu_map.lat && lon > m_clu_map.lon && lat < m_clu_map.lat_max && lon < m_clu_map.lon_max )
	{
		uint index = (uint)( ( lat - m_clu_map.lat ) / m_clu_map.lat_d ) * m_clu_map.num_of_lon_cols +
					(uint)( ( lon - m_clu_map.lon ) / m_clu_map.lon_d );
		return GetBands ( index );
	} else return NULL;
//	g_print ( "track index = %u\n", index );
}

/*
VECTOR_BANDS*	ClusterMap :: GetBands ( double rho, double theta )
{

	return NULL;
}
*/

const	VECTOR_BANDS	*ClusterMap :: GetBands ( uint index )
{
	if ( index >= m_min_band && index <= m_max_band )
	{
		//g_print ( "track index = %u\n", index );
		return &m_map_bands[ index ];
	}
	else
		return NULL;
}


const	void	ClusterMap :: GetMinMax ( uint *min, uint *max )
{
	*min	= m_min_band;
	*max	= m_max_band;
}

