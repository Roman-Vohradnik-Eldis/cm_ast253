#include "CoverageMap.hh"
//#include <gdk/gdk.h>

#if (PROJ_VERSION == 4)
#include <projects.h>
#else
#include <proj.h>
#endif

bool	g_scn_run = false;



//********************************************************************************

MAP_CROSS_REF::MAP_CROSS_REF()
{
	entry_num		= 0;
	num_of_modeS_stations	= 0;
	map_ref			= 0;
	radar_par_set_ref	= 0;
}


//********************************************************************************

CLUSTER_HEADER :: CLUSTER_HEADER ()
{
	MagicCluster[0] = MagicCluster[1] = MagicCluster[2] = 0;
	format_version	= 0;
	check_sum				= 0L;
	main_ver				= 0;
	sub_ver					= 0;
	memset ( cl_modification_date, 0, DATE_LEN );
	memset ( local_modification_date, 0, DATE_LEN );
	comment_length	= 0;

	comment	= NULL;
}


CLUSTER_HEADER ::	~CLUSTER_HEADER ()
{
	if ( comment )
		delete [] comment;
}


//********************************************************************************

GROUND_STATION :: GROUND_STATION ()
{
	name_length			= 0;
	memset ( name, 0, GS_NAME_LEN );

	node_ref_num		= 0;
	net_addr_length	= 0;

	SIC = SAC = 0;
	height		= 0;

	lat = lon = 0.0;

//	cms_client	= NULL;
//	cms_server	= NULL;

	ip_client		= NULL;
//	ip_server		= NULL;

	m_timer_node_alive = NULL;
	m_state	= NODE_INITIAL;

	remote_time	 = 0;
}


GROUND_STATION :: ~GROUND_STATION ()
{
/*
	if ( cms_client )
		delete cms_client;

	if ( cms_server )
		delete cms_server;
*/

	if ( ip_client )
		delete ip_client;
/*
	if ( ip_server )
		delete ip_server;
*/
	if ( m_timer_node_alive )
		g_timer_destroy ( m_timer_node_alive );
}



//********************************************************************************


bool	DekodujNetAddr ( char *bafr, NET_ADDR *addr )
{
	char *p = strrchr ( bafr, ':' );
	addr->port = atoi ( p + 1 );
	*p = 0x0;

	p	= strchr ( bafr, ':' );
	//g_print ( "ip=%s\n" , addr->ip.c_str() );
	addr->ip = p + 1;

	return true;
}


void	CoverageMap :: Init ()
{
	m_num_of_node_desc	= 0;

	m_min_band	=	0xFFFF;
	m_max_band	= 0;

	m_num_of_solution_entries	= 0;

	m_maps.clear ();
	m_solution.clear ();

	m_map_gs.clear ();
	m_map_gs_id.clear ();
	m_map_state_num.clear ();

	m_SIC = m_SAC = 0;
	m_maps_dir_name[0] = 0x0;

	m_gs_me		= NULL;

	m_path_to_cluster.clear ();
	m_path_to_cluster_maps.clear ();
//	m_path_to_one_map.clear();
//	m_direct_addr	 = false;
}



CoverageMap :: CoverageMap ( const char *path_to_project )
{
	Init ();

	m_path_to_cluster	= path_to_project;
}



CoverageMap :: CoverageMap ( uint SIC, uint SAC, const char *path_to_project, bool direct_addr )
{
	Init ();

	m_path_to_cluster	= path_to_project;
//	m_direct_addr			= direct_addr;

	SetSS ( SIC, SAC );
}



CoverageMap :: CoverageMap ( std::string *path_to_project, bool direct_addr )
{
	Init ();

	m_path_to_cluster	= *path_to_project;
//	m_direct_addr			= direct_addr;
}



CoverageMap :: ~CoverageMap ()
{
	g_scn_run	= false;

	for ( uint i=0; i < m_num_of_node_desc; i++ )
	{
		if ( m_gs[i].proj_centre )
#if (PROJ_VERSION == 4)
			pj_free ((projPJ)m_gs[i].proj_centre );
#else
			proj_destroy ((PJ *)m_gs[i].proj_centre );
#endif
	}

	for ( uint i=0; i < m_maps.size(); i++ )
	{
		ClusterMap	*cm = m_maps[i];
		if ( cm )
			delete cm;
	}
	m_maps.clear ();

}


void	CoverageMap :: SetSS ( unsigned char SIC, unsigned char SAC )
{
	m_SIC = SIC;
	m_SAC = SAC;

//	if ( !m_direct_addr )
		sprintf ( m_maps_dir_name, "M%02x%02x001.001", m_SAC, m_SIC );
}


int	CoverageMap :: LoadClusterProject ()		// const char *filename, const char *path_to_project, uint node )
{
/*
	for ( uint i=0; i<MAX_NUM_OF_NODES; i++ )
		m_client[i] = NULL;
*/

//	bool ret = -1;

	for ( uint poc_pok=0; poc_pok<2; poc_pok++ )
	{

		std::string	path = m_path_to_cluster.c_str();
		if ( '/' != m_path_to_cluster [ strlen ( m_path_to_cluster.c_str() ) - 1 ] )
			path.append ( "/" );

		if ( 0 == poc_pok )
		{
			if ( strlen ( m_maps_dir_name ) )	//( && !m_direct_addr )
			{
				path.append ( m_maps_dir_name );
				path.append ( "/" );
			}
		}

		m_path_to_cluster_maps	=	path;
		path.append ( CLUSTER_MAIN_FILE );

		cms_ns_info( "librdf", "CoverageMap: CESTA %u !!! %s", poc_pok, path.c_str() );

		uint	precteno;

		FILE *cp	= fopen ( path.c_str(), "rb" );
		if ( cp )
		{
	//		fread ( &clu_header, sizeof ( clu_header ), 1, cp );
			precteno = fread ( &m_clu_header, 1, 39, cp );

			//g_print ( "%u\n", clu_header.comment_length );
			if ( m_clu_header.comment_length )
			{
				m_clu_header.comment	= new char [ m_clu_header.comment_length + 1 ];
				memset ( m_clu_header.comment, 0, m_clu_header.comment_length + 1 );
				precteno = fread ( m_clu_header.comment, 1, m_clu_header.comment_length, cp );
			}

			precteno = fread ( &m_num_of_node_desc, 1, 1, cp );

			for ( uint i=0; i < m_num_of_node_desc; i++ )
			{
				//stanice
				//memset ( m_gs[i].name, 0, GS_NAME_LEN );
				precteno = fread ( &m_gs[i].name_length, 1, GS_NAME_LEN, cp );
				uint	j = GS_NAME_LEN-1;
				while ( m_gs[i].name[--j] == 32 )
				{
					m_gs[i].name[j] = 0x0;
				}


				precteno = fread ( &m_gs[i].node_ref_num, 1, 4, cp );

				uint	ss = GetSS ( &m_gs[i] );
				m_map_gs.insert ( make_pair ( ss, &m_gs[i] ) );
				m_map_gs_id.insert ( make_pair ( ss, i ) );

				if ( m_gs[i].net_addr_length )
				{
					/*
					if ( m_clu_header.m_gs[i].net_addr_length > 3*NET_ADR_LEN )
					{
						m_clu_header.m_gs[i].net_addr[3]	= new char [ NET_ADR_LEN1 ];
						memset ( m_clu_header.m_gs[i].net_addr[3], 0x0, NET_ADR_LEN1 );
					}
					if ( m_clu_header.m_gs[i].net_addr_length > 2*NET_ADR_LEN )
					{
						m_clu_header.m_gs[i].net_addr[2]	= new char [ NET_ADR_LEN1 ];
						memset ( m_clu_header.m_gs[i].net_addr[2], 0x0, NET_ADR_LEN1 );
					}
					if ( m_clu_header.m_gs[i].net_addr_length > NET_ADR_LEN )
					{
						m_clu_header.m_gs[i].net_addr[1]	= new char [ NET_ADR_LEN1 ];
						memset ( m_clu_header.m_gs[i].net_addr[1], 0x0, NET_ADR_LEN1 );
					}

					m_clu_header.m_gs[i].net_addr[0]	= new char [ NET_ADR_LEN1 ];
					memset ( m_clu_header.m_gs[i].net_addr[0], 0x0, NET_ADR_LEN1 );
					*/

					char	bafr[NET_ADR_LEN1];
					memset ( bafr, 0x0, NET_ADR_LEN1 );
					precteno = fread ( bafr, 1, NET_ADR_LEN, cp );
					DekodujNetAddr ( bafr, &m_gs[i].net_addr[0] );

					if ( m_gs[i].net_addr_length > 40 )
					{
						memset ( bafr, 0x0, NET_ADR_LEN1 );
						precteno = fread ( bafr, 1, NET_ADR_LEN, cp );
						DekodujNetAddr ( bafr, &m_gs[i].net_addr[1] );
					}

					if ( m_gs[i].net_addr_length > 80 )
					{
						memset ( bafr, 0x0, NET_ADR_LEN1 );
						precteno = fread ( bafr, 1, NET_ADR_LEN, cp );
						DekodujNetAddr ( bafr, &m_gs[i].net_addr[2] );
					}

					if ( m_gs[i].net_addr_length > 120 )
					{
						memset ( bafr, 0x0, NET_ADR_LEN1 );
						precteno = fread ( bafr, 1, NET_ADR_LEN, cp );
						DekodujNetAddr ( bafr, &m_gs[i].net_addr[3] );
					}

					m_gs[i].net_addr_length /= NET_ADR_LEN;

				}

				int32_t		pom;
				precteno = fread ( &pom, 1, 4, cp );
				m_gs[i].lat	= GUINT32_FROM_BE ( pom ) / 360000.0;
				m_gs[i].geo_centre.Latitude		= m_gs[i].lat * DEG_TO_RAD;

				precteno = fread ( &pom, 1, 4, cp );
				m_gs[i].lon	= GUINT32_FROM_BE ( pom ) / 360000.0;
				m_gs[i].geo_centre.Longitude	= m_gs[i].lon * DEG_TO_RAD;

	/*
				m_clu_header.m_gs[i].lat		= GUINT32_FROM_BE ( m_clu_header.m_gs[i].lat ) / 360000;
				m_clu_header.m_gs[i].lon		= GUINT32_FROM_BE ( m_clu_header.m_gs[i].lon );
	*/
				precteno = fread ( &pom, 1, 4, cp );
				m_gs[i].height	= GUINT32_FROM_BE ( pom );
			}


			precteno = fread ( &m_num_of_solution_entries, 1, 1, cp );
			for ( uint i=0; i < m_num_of_solution_entries; i++ )
			{
				MAP_CROSS_REF	*mcr = &m_clu_header.maps[i];
				precteno = fread ( &mcr->entry_num, 1, 2, cp );
				for ( uint s=0; s<mcr->num_of_modeS_stations; s++ )
				{
					precteno = fread ( &mcr->mode_S_stations[s], 1, 2, cp );
				}
				
				precteno = fread ( &mcr->map_ref, 1, 2, cp );
			}

			unsigned char neco[20];
			precteno = fread ( &neco, 1, 20, cp );
			if ( precteno > 0 )
				cms_ns_info ( "librdf", "CoverageMap: Last cluster fread = %u", precteno );

			fclose ( cp );


			char	text[100];
			for ( uint i=0; i < m_num_of_node_desc; i++ )
			{
				sprintf ( text, PROJ_STEREA, m_gs[i].lat, m_gs[i].lon );
				//g_print ( "%s\n", text );
#if (PROJ_VERSION == 4)
				m_gs[i].proj_centre = pj_init_plus ( text );
#else
				m_gs[i].proj_centre = proj_create (PJ_DEFAULT_CTX, text );
#endif

				if ( !m_gs[i].proj_centre )
#if (PROJ_VERSION == 4)
					cms_ns_error ( "librdf", "CoverageMap: Projection ERR GS:%u\tpj_errno : %i - %s", i, 
						pj_errno, pj_strerrno ( pj_errno ) );
#else
					cms_ns_error ( "librdf", "CoverageMap: Projection ERR GS:%u\tpj_errno : %i - %s", i, 
						proj_errno ((PJ *)m_gs[i].proj_centre),
						proj_errno_string (proj_errno ((PJ *)m_gs[i].proj_centre)));
#endif
				m_gs[i].proj_centre_rdf.SetCenter ( m_gs[i].geo_centre );

			}

			return 0;

		}		//if ( cp )

	}		//for ( uint poc_pok=0; poc_pok<2; poc_pok++ )

//	else
	{
		//spatna cesta, soubor neexistuje
		return -1;
	}

}


void	CoverageMap :: LoadClusterMaps ()
{
/*
		std::string	path = m_path_to_cluster.c_str();
		if ( '/' != m_path_to_cluster [ strlen ( m_path_to_cluster.c_str() ) - 1 ] )
			path.append ( "/" );

		if ( strlen ( m_maps_dir_name ) )	//&& !m_direct_addr )
		{
			path.append ( m_maps_dir_name );
			path.append ( "/" );
		}
*/

		for ( uint i=0,c=0; i < m_num_of_solution_entries; i++,c++ )
		{
			char filename[300]	= {0};

			//cislo stanice
			sprintf ( filename, "%ssysmp%03u.dat", m_path_to_cluster_maps.c_str(), m_clu_header.maps[i].map_ref );
			cms_ns_info ( "librdf", "CoverageMap: filename is : \"%s\"\n", filename );

			//path.append ( filename );

			FILE *f = fopen ( filename, "rb" );
			if ( f )
				fclose ( f );
			else
				continue;

			ClusterMap	*map = new ClusterMap ( filename, m_clu_header.maps[i].map_ref, c );
			if ( map )
			{
				m_maps.push_back ( map );
				m_solution.insert ( make_pair ( m_clu_header.maps[i].map_ref, map ) );
				m_map_state_num.insert ( make_pair ( m_clu_header.maps[i].map_ref, i ) );
				//g_print ( "Vkladam %u map_ref %u\n", i, m_clu_header.maps[i].map_ref );

				uint	min, max;
				map->GetMinMax ( &min, &max );

				if ( min < m_min_band )
					m_min_band	= min;
				if ( max > m_max_band )
					m_max_band	= max;

			}	//if ( map )

		}	//for ( uint i=0,c=0; i<m_clu_header.num_of_solution_entries; i++,c++ )

}


/*
int	CoverageMap :: LoadCoverageMaps ( const char *path_to_project )
{
	m_path_to_cluster	= path_to_project;

	return LoadCoverageMaps ();
}


int	CoverageMap :: LoadCoverageMaps ( std::string *path_to_project )
{
	m_path_to_cluster	= *path_to_project;

	return LoadCoverageMaps ();
}
*/


//int	CoverageMap :: LoadCoverageMaps ( bool direct_addr )
int	CoverageMap :: LoadCoverageMaps ()
{
//	m_direct_addr	= direct_addr;

	int	ret = LoadClusterProject ();
	if ( !ret )
	{
		LoadClusterMaps ();

		SetRefNum_me ();
		return 0;
	}
	else
		return ret;
}


const	GROUND_STATION	*CoverageMap :: GetGSfromID ( int id )
{
	if ( id < 0 )
		return m_gs;
	else
	if ( id < m_num_of_node_desc )
		return &m_gs[id];
	else
		return NULL;
};



const	GROUND_STATION	*CoverageMap :: GetGSfromSS ( int sic_sac )
{
	if ( sic_sac < 0 )
		return m_gs;
	else
	{
		return m_map_gs[sic_sac];
	}
};


const	uint	CoverageMap :: GetIDfromSS ( int sic_sac )
{
	return	m_map_gs_id[sic_sac];
}


const	uint	CoverageMap :: GetIDfromSS ( unsigned char sic, unsigned char sac )

{
	int	sic_sac	= ( sac << 8 ) + sic;

	return	GetIDfromSS ( sic_sac );
}


#define		CC_NUM		6		//NUM_OF_GROUND_STATIONS
const	bool	CoverageMap :: IsCCfromSS ( int sic_sac )
{
	return	( CC_NUM == m_map_gs_id[sic_sac] ) ? true : false;
}


const	bool	CoverageMap :: UsingCC ( uint &cc_id )
{
	for ( uint i=0; i < m_num_of_node_desc; i++ )
	{
		if ( CC_NUM == m_gs[i].node_ref_num )
		{
			cc_id = i;
			return true;
		}
	}

	return false;
}


//************************************************************************************
//

uint	GetSS ( RADAR_ID *rid )
{
	if ( rid )
		return ( rid->SAC << 8 ) + rid->SIC;
	else
		return 0;
}


uint	GetSS ( RADAR_ID rid )
{
	return ( rid.SAC << 8 ) + rid.SIC;
}


uint	GetSS ( GROUND_STATION *gs )
{
	if ( gs )
		return ( gs->SAC << 8 ) + gs->SIC;
	else
		return 0;
}


void	CoverageMap :: SetRefNum_me ()
{
	if ( &m_gs[0] )
		m_gs_me		= &m_gs[0];

	if ( m_SIC && m_SAC )
	{
		for ( uint i=0; i<m_num_of_node_desc; i++ )
		{
			GROUND_STATION	*gs = &m_gs[i];

			if ( m_SIC == gs->SIC && m_SAC == gs->SAC )
			{
				m_gs_me		= gs;
				break;
			}
		}
	}
}



//***************** CoverageMapFS ********************************************
#define		BAFR_LEN		1000

gpointer scn_thread ( gpointer arg )
{
	CoverageMapFS		*cm = (CoverageMapFS*) arg;
//	CIP	*scn_ip	= (CIP*)arg;
	unsigned char	bafr[BAFR_LEN];
	DEQUE_RDF_SCF		msg_scf;

//	g_print ( "CoverageMapFS thread START\n" );


	while ( g_scn_run )
	{

		int	len = cm->m_scn_ip->Recv ( bafr, BAFR_LEN );

		// zamkneme globalni zamek
//		gdk_threads_enter ();

		//g_print ( "CoverageMapFS %u%u recv %i\n", cm->m_SIC, cm->m_SAC, len );

		if ( len > 0 )
		{

			switch ( bafr[0] )
			{

				case	SCF_CAT :

					if ( Asterix017_to_SurvCoordFunction ( bafr, len, msg_scf ) )
					{
						//g_print ( "CoverageMapFS recv AST17 msg count = %u\n", msg_scf.size () );
						for ( uint sl=0; sl < msg_scf.size (); sl++ )
						{
							RDFSurvCoordFunction	scf = msg_scf[sl];

							//g_print ( "CoverageMapFS recv %u%u\n", scf.Destination.SIC, scf.Destination.SAC );

							//if ( cm->m_SIC == scf.Destination.SIC && cm->m_SAC == scf.Destination.SAC )
							{
	//							Zpracuj_SCF_AST17 ( &scf );
								cm->NewSCFMsg ( &scf );

								switch ( scf.Message_type )
								{
									case	MOVE_NODE_TO_NEW_CLUSTER_STATE:
										{
											//g_print ( "CoverageMap SIC:SAC = %u:%u ", cm->m_SIC, cm->m_SAC );

											if ( scf.CC_State.Present )
											{
												cm->SetNewState ( scf.CC_State.Value );
												//g_print ( "SS %u%u new state %u\n", cm->m_SIC, cm->m_SAC, cm->m_state );
												cm->NewState ( cm->m_state );
											}
											else
											{
												//g_print ( "wrong state !!\n" );
											}

										}
										break;

									default:
										break;
								}

							}

						}	//for ( uint sl=0; sl < msg_scf.size (); sl++ )

					}	//if ( Asterix017_to_SurvCoordFunction ( bafr, len, msg_scf ) )
					break;

				default :
					cms_ns_warning ( "librdf", "CoverageMapFS SS:%u%u - Unknown msg !!! type=%u len=%u", cm->m_SIC, cm->m_SAC, bafr[0], len );
					break;

			}	//switch ( bafr[0] )

		}		//	if ( len > 0 )
		else
		{
			//g_print ( "CoverageMapFS WRONG msg len !!!\n" );
		}

			// odemkneme globalni zamek
	//		gdk_threads_leave ();

	//		usleep ( 10000 );
	}

	return NULL;
}


CoverageMapFS :: CoverageMapFS ( uint SIC, uint SAC, std::string *path_to_project, std::string *scn_ip, bool direct_addr )
: CoverageMap ( SIC, SAC, path_to_project->c_str(), direct_addr )
{
	Init ( scn_ip );
}


CoverageMapFS :: CoverageMapFS ( std::string *path_to_project, std::string *scn_ip )			//, bool direct_addr )
: CoverageMap ( path_to_project )
{
	Init ( scn_ip );
}


CoverageMapFS :: ~CoverageMapFS ()
{
	//zrusit vlakno SCN
	g_scn_run			= false;
	if ( m_scn_ip ) m_scn_ip->ShutDown ();
	g_thread_join (g_scn_thread);
	if ( m_scn_ip ) delete m_scn_ip;
}


void	CoverageMapFS :: Init ( std::string *scn_ip )
{
//	m_path_to_cluster	= *path_to_project;
	m_scn_ip_adr	= *scn_ip;

	g_scn_thread	= NULL;
	m_scn_ip			= NULL;
	g_scn_run			= false;
	m_state				= 0;
	m_actual_map	= NULL;

	LoadCoverageMaps ();		// m_direct_addr );

	//rozjet vlakno na cteni AST 17 info o stavu
	m_scn_ip	= new CIP ( CMS_FLAG_RECV, m_scn_ip_adr );

	g_scn_run	= true;

#if GLIB_CHECK_VERSION (2, 32, 0)
	g_scn_thread	= g_thread_new ("scn_thread", scn_thread, this);
#else
	g_scn_thread	= g_thread_create (scn_thread, this, TRUE, NULL );
#endif
}


int	CoverageMapFS :: GetII ()
{
	int	pom = WRONG_II_SI;

	if ( m_gs_me && m_actual_map )
	{
		//uint state = ( m_state > 2 ) ? 1 : 0;
		//g_print ( "m_gs_me->node_ref_num = %u\n", m_gs_me->node_ref_num );
		pom = m_clu_header.maps [ m_map_state_num [ m_state ] ].mode_S_stations[ m_gs_me->node_ref_num ].II_SI_code;

//		if ( pom > 15 && pom != WRONG_II_SI ) pom -= 16;
	}
	else
	{
		if ( !m_actual_map ) return CLUSTER_ERR_NO_VALID_MAP;
		if ( !m_gs_me ) return CLUSTER_ERR_NO_VALID_GS;
	}

	if ( !m_scn_ip->IsConnected () ) return CLUSTER_ERR_NO_CONNECTION;

	return	pom;
}


int	CoverageMapFS :: GetClusterFlags ( double rho, double theta, uint8_t &Surv, uint8_t &Lock, uint8_t &Data )
{
	return	GetClusterFlags ( rho, theta, UNKNOWN_ALTITUDE_DOUBLE, Surv, Lock, Data );
}


int CoverageMapFS :: GetClusterFlags ( double rho, double theta, int height, uint8_t &Surv, uint8_t &Lock, uint8_t &Data )
//uint	CoverageMapFS :: GetClusterFlags ( double rho, double theta, int height )
{
	int ret = CLUSTER_ERR_NO_VALID_MAP;
	uint8_t	surv, dl, lock;
	surv = dl = lock = 0;

//	ClusterMap	*map = GetSolution ( m_state );
	if ( m_actual_map )
	{
		RDFCoorPolar Polar ( rho, DEG_TO_RAD * theta );
		RDFCoorGeographic	geo;

		//g_print ( "pointer m_gs_me = %p\n", m_gs_me );
		if ( !m_gs_me ) return CLUSTER_ERR_NO_VALID_GS;

		m_gs_me->proj_centre_rdf.Pol2Geo ( Polar, geo );
//		g_print ( "lat=%f lon=%f\n", RAD_TO_DEG * geo.Latitude, RAD_TO_DEG * geo.Longitude );


		const	VECTOR_BANDS	*bands = m_actual_map->GetBands ( RAD_TO_DEG * geo.Latitude, RAD_TO_DEG * geo.Longitude );
		if ( bands )
		{

			for ( uint b=0; b < bands->size(); b++ )
			{
				const	ALT_BAND	*band = (*bands)[b];

				if ( band->ground_station == m_gs_me->node_ref_num )
				{

					if ( (int)UNKNOWN_ALTITUDE_DOUBLE == height )
					{
						//g_print ( "Map type : %u\n", band->map_type );

						if ( band->map_type )
						{
							if ( MAP_TYPE_SR & band->map_type )
								surv++;

							if ( MAP_TYPE_DL & band->map_type )
								dl = 1;

							if ( MAP_TYPE_LO & band->map_type )
								lock = ( MAP_TYPE_LO & band->map_type ) >> MAP_SHIFT_LO;
						}
					}
					else
					{
						if ( height < 0 ) height = 0;
						if ( height > HIGH_MAX ) height = HIGH_MAX;

						//vyska byla zadana
						uint hi = height / HIGH_UNIT;

//						if ( ( band->low_alt != band->high_alt ) && ... )
						if ( band->low_alt <= hi && hi <= band->high_alt )
						{
							//g_print ( "Map type : %u\n", band->map_type );

							if ( MAP_TYPE_SR & band->map_type )
								surv++;

							if ( MAP_TYPE_DL & band->map_type )
								dl = 1;

							if ( MAP_TYPE_LO & band->map_type )
								lock = ( MAP_TYPE_LO & band->map_type ) >> MAP_SHIFT_LO;
						}
					}

				}	//	if ( band->ground_station == m_gs_me->node_ref_num )

			}	//	for ( uint b=0; b < bands->size(); b++ )

		}

		ret = 0;
	}

	Surv = STRATEGY_MODES_OFF;
	if ( 1 == surv )	Surv = STRATEGY_MODES_ON;
	if ( surv > 1 )		Surv = STRATEGY_MODES_REQUIRED_ALTITUDE;

	Lock = STRATEGY_MODES_OFF;
	if ( lock == 1 ) Lock = STRATEGY_MODES_ON;
	if ( lock == 2 ) Lock = STRATEGY_MODES_INTERMITTENT;

	Data = STRATEGY_MODES_OFF;
	if ( dl ) Data = STRATEGY_MODES_ON;

	if ( !m_scn_ip->IsConnected () ) return CLUSTER_ERR_NO_CONNECTION;

	return ret;
}


void	CoverageMapFS :: SetNewState ( uint new_state )
{
//	if (new_state < m_solution.size())
	{
		m_state	= new_state;

		m_actual_map = m_solution [ m_state ];

		cms_ns_info ( "librdf", "CoverageMap: New state %u - map = %p int %u", m_state, m_actual_map, m_map_gs_id [ m_state ] );
	}
/*
	else
	{
		cms_ns_error ( "librdf", "CoverageMap: New state %u .. but we have only %u solutions", new_state, (unsigned) m_solution.size());
	}
*/
}

