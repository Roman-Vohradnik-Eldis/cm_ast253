#ifndef		CLASS_CLUSTER_COVERAGE_MAP
#define		CLASS_CLUSTER_COVERAGE_MAP


#include "RDFProjection.hh"
//#include "CIP.hh"
#include "ClusterMap.hh"
#include "enums_structures.hh"
#include "CMSIP.hh"
#include "RDFAsterix.hh"



#define		CLUSTER_MAIN_FILE							"cluster.dat"

#define		DATE_LEN											14
#define		NUM_OF_MODE_S_STATIONS				6
#define		GS_NAME_LEN										17

//#define		CC_NUM										6
//#define		NUM_OF_GROUND_STATIONS		( CC_NUM+1 )
#define		NUM_OF_GROUND_STATIONS				6
#define		MAX_NUM_OF_NODES							7


#define		NET_ADR_LEN										40
#define		NET_ADR_LEN1									( NET_ADR_LEN+1 )
#define		PROJ_STEREA										"+proj=sterea +lat_0=%f +lon_0=%f"

#define		WRONG_II_SI										255

#define		CLUSTER_ERR_NO_VALID_MAP			-1
#define		CLUSTER_ERR_NO_VALID_GS				-2
#define		CLUSTER_ERR_NO_CONNECTION			-3

//************* od ROVO *****************************************
#include <stdint.h>
//#include <glib.h>


#define STRATEGY_MODES_OFF									0
#define STRATEGY_MODES_ON										1
#define STRATEGY_MODES_INTERMITTENT					2
#define STRATEGY_MODES_UNDEFINED						127
#define STRATEGY_MODES_REQUIRED_ALTITUDE		126
#define STRATEGY_MODES_MAP									128

// meters
#define UNKNOWN_ALTITUDE_DOUBLE							-1000000.
//************* od ROVO *****************************************



#pragma	pack ( push, 1 )


struct	MODE_S_STATION
{
	unsigned	char	node_ref_num;
	unsigned	char	II_SI_code;

	MODE_S_STATION () { node_ref_num = 0; II_SI_code = WRONG_II_SI; };

};


struct	MAP_CROSS_REF
{
	unsigned	char	entry_num;
	unsigned	char	num_of_modeS_stations;
	MODE_S_STATION	mode_S_stations[NUM_OF_MODE_S_STATIONS];
	unsigned	char	map_ref;
	unsigned	char	radar_par_set_ref;

	MAP_CROSS_REF ();
};


struct	CLUSTER_HEADER
{
	char	MagicCluster[3];
	unsigned	char	format_version;
	uint32_t	check_sum;
	unsigned	char	main_ver;
	unsigned	char	sub_ver;
	char	cl_modification_date[DATE_LEN];
	char	local_modification_date[DATE_LEN];

	unsigned	char	comment_length;				//sum = 39
	char	*comment;

	MAP_CROSS_REF		maps[128];
	

	CLUSTER_HEADER ();
	~CLUSTER_HEADER ();

};




enum	NODE_STATES
{
	NODE_ME						= 0,
	NODE_INITIAL,
	NODE_LIVING,
	NODE_UNSTABLE,
	NODE_DISCONNECT,
	NODE_REMOTE_STATES_NUM
};


struct	NET_ADDR
{
	std::string	ip;
	uint				port;

	NET_ADDR () { ip.clear(); port = 0; };
};


struct	GROUND_STATION
{
	//stanice
	unsigned	char	name_length;
	char	name[GS_NAME_LEN];
	unsigned	char	node_ref_num;
//	unsigned	char	IISI_code;
//	unsigned	char	radar_param_set_ref;
	unsigned	char	SAC;
	unsigned	char	SIC;

	unsigned	char	net_addr_length;			//sum = 21
	NET_ADDR	net_addr[4];
	int32_t	height;

	double	lat, lon;
	RDFCoorGeographic	geo_centre;

//pba promenne

	void * proj_centre;
	RDFProjection	proj_centre_rdf;

//	CMSClient	*cms_client;
//	CMSServer	*cms_server;

//	CMSIP	*ip_server;
	CMSIP	*ip_client;

	GTimer	*m_timer_node_alive;
	NODE_STATES	m_state;
//	bool	gs_list[NUM_OF_GROUND_STATIONS];
	bool	show;
	int		remote_time;

	GROUND_STATION ();
	~GROUND_STATION ();

};

#pragma	pack ( pop )

typedef	std::map	< uint, GROUND_STATION* > MAP_GS;
typedef	std::map	< uint, uint > MAP_GS_INT;





class	CoverageMap
{


		unsigned	char	m_num_of_node_desc;
		unsigned	char	m_num_of_solution_entries;

		//*****************************************************

		int		LoadClusterProject ();
		void	LoadClusterMaps ();

	protected :

		struct	CLUSTER_HEADER	m_clu_header;

		std::string		m_path_to_cluster;
		std::string		m_path_to_cluster_maps;
//		std::string		m_path_to_one_map;
//		bool	m_direct_addr;

		unsigned	char	m_SIC, m_SAC;
		char	m_maps_dir_name[50];


		GROUND_STATION	m_gs[MAX_NUM_OF_NODES];
		GROUND_STATION	*m_gs_me;

		VECTOR_CLUSTER_MAP	m_maps;
		MAP_STATE_TO_SOLUTION		m_solution;

		MAP_GS			m_map_gs;
		MAP_GS_INT	m_map_gs_id;
		MAP_GS_INT	m_map_state_num;

		uint	m_min_band;
		uint	m_max_band;

		void	Init ();

		void	SetRefNum_me ();
		void	SetSS ( unsigned char SIC, unsigned char SAC );


	public :

//		CoverageMap () { Init(); };
		CoverageMap ( const char *path );
		CoverageMap ( uint SIC, uint SAC, const char *path, bool direct_addr = false );
		CoverageMap ( std::string *path, bool direct_addr = false );
		~CoverageMap ();


		int	LoadCoverageMaps ();		// bool direct_addr = false );
//		int	LoadCoverageMaps ( const char *path );
//		int	LoadCoverageMaps ( std::string *path );

		const	VECTOR_CLUSTER_MAP	*GetMaps () { return &m_maps; };

		GROUND_STATION	*GetGS () { return m_gs; };
		GROUND_STATION	*GetGSme () { return m_gs_me; };
		const	GROUND_STATION	*GetGSfromID ( int id );
		const	GROUND_STATION	*GetGSfromSS ( int sic_sac );
		const	uint	GetIDfromSS ( int sic_sac );
		const	uint	GetIDfromSS ( unsigned char sic, unsigned char sac );

		const	bool	UsingCC ( uint &cc_id );
		const	bool	IsCCfromSS ( int sic_sac );

		const	unsigned char	GetNumOfNode () { return m_num_of_node_desc; };
		const	unsigned char	GetNumOfSolution () { return m_num_of_solution_entries; };

		ClusterMap	*GetSolution ( uint state ) { return m_solution [ state ]; };
};



class	CoverageMapFS : public CoverageMap
{
//pro ROVO *****************************************************************************

		std::string m_scn_ip_adr;
		GThread	*g_scn_thread;
		CIP	*m_scn_ip;
		uint	m_state;
		ClusterMap	*m_actual_map;

		void	Init ( std::string *scn_ip );
		void	SetNewState ( uint new_state );

	public :

		CoverageMapFS ( uint SIC, uint SAC, std::string *path, std::string *scn_ip, bool direct_addr = false );
		CoverageMapFS ( std::string *path, std::string *scn_ip );		//, bool direct_addr = false );
		virtual ~CoverageMapFS ();

		int		GetII ();
		bool	IsConnected () { return m_scn_ip->IsConnected (); };

//		uint	GetClusterFlags ( double rho, double theta, int height );
		int	GetClusterFlags ( double rho, double theta, int height, uint8_t &Surv, uint8_t &Lock, uint8_t &Data );
		int	GetClusterFlags ( double rho, double theta, uint8_t &Surv, uint8_t &Lock, uint8_t &Data );

		virtual		void	NewSCFMsg ( RDFSurvCoordFunction *scf ) {}
		virtual		void	NewState ( uint state ) {}

		virtual		uint	GetState () { return m_state; };
//		void	SetState ( unsigned char state ) { m_state = state; };

		friend		gpointer scn_thread ( gpointer arg );
};



uint	GetSS ( RADAR_ID *rid );
uint	GetSS ( RADAR_ID rid );
uint	GetSS ( GROUND_STATION *gs );


#endif		//CLASS_CLUSTER_COVERAGE_MAP
