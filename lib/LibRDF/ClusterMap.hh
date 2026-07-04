#ifndef			HEADER_CLASS_CLUSTER_MAP
#define			HEADER_CLASS_CLUSTER_MAP


#include <vector>
#include <map>
#include <stdint.h>
#include <stdlib.h>


#define		MAP_TYPE_SR					0b00000001
#define		MAP_TYPE_DL					0b00000010
#define		MAP_TYPE_LO					0b00001100
#define		MAP_SHIFT_LO				2


#define		HIGH_UNIT						200
#define		HIGH_MAX						( 255 * HIGH_UNIT )



#pragma	pack ( push, 1 )

struct	ALT_BAND
{
	unsigned	char	ground_station;			//node_ref_num;
	unsigned	char	map_type;
	unsigned	char	low_alt;
	unsigned	char	high_alt;
};

typedef		std::vector < ALT_BAND* >	VECTOR_BANDS;
typedef		std::vector < ALT_BAND* >	:: iterator	VECTOR_BANDS_ITER;



struct	CELL_DEF
{
	unsigned	char	lat_slice_num;
	unsigned	char	lon_cell_num;
	unsigned	char	num_of_bands;

	VECTOR_BANDS	alt_bands;

	CELL_DEF ();
	~CELL_DEF ();
};



struct	CLUSTER_MAP
{
	char	MagicMap[3];
	unsigned	char	format_version;
	uint32_t	check_sum;
	unsigned	char	main_ver;
	unsigned	char	sub_ver;
	char	map_modification_date[14];
	char	local_modification_date[14];

	unsigned	char	comment_length;				//sum = 39
	char	*comment;

	unsigned	char	map_ref;

	unsigned	char	num_of_lat_rows;
	unsigned	char	num_of_lon_cols;

	uint16_t	num_of_deltas;

	double	lat, lon, lat_d, lon_d;
	double	lat_max, lon_max;

	std::vector < CELL_DEF* >	cells;


	CLUSTER_MAP ();
	~CLUSTER_MAP ();
};

#pragma	pack ( pop )



class		ClusterMap
{
		CLUSTER_MAP		m_clu_map;
		unsigned	char	m_map_id;

		VECTOR_BANDS	*m_map_bands;
		uint	m_min_band;
		uint	m_max_band;

		bool	m_show;
		uint	m_color_index;

		void	InitMap ();

	public:

		ClusterMap ( const char *filename, unsigned char id, uint color_index = 0 );
		~ClusterMap ();

		const	unsigned char	GetId () { return m_map_id; };
		void	SetShow ( bool show ) { m_show = show; };	// g_print ( "Map %u : %u\n", m_clu_map.map_ref, show ); };
		const	bool	GetShow () { return m_show; };

		const	CLUSTER_MAP		*GetMapDesc () { return &m_clu_map; };
		const	VECTOR_BANDS	*GetMapBands () { return m_map_bands; };

		const	VECTOR_BANDS	*GetBands ( uint index );
		const	VECTOR_BANDS	*GetBands ( double lat, double lon );
		const	VECTOR_BANDS	*GetBands ( double azim, double range, double height );

		const	void	GetMinMax ( uint *min, uint *max );

//		VECTOR_BANDS	*GetBands ( double rho, double theta );
//		const	GdkColor	*GetColor () { return m_color; };
//		const	uint GetColorIndex () { return m_color; };
};


typedef		std::vector < ClusterMap* > VECTOR_CLUSTER_MAP;
typedef		std::vector < ClusterMap* > :: iterator VECTOR_CLUSTER_MAP_ITERATOR;

typedef		std::map < uint , ClusterMap* > MAP_STATE_TO_SOLUTION;


#endif			//HEADER_CLASS_CLUSTER_MAP
