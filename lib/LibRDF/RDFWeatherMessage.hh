#ifndef _RDF_WEATHER_MESSAGE_H
#define _RDF_WEATHER_MESSAGE_H

#include "RDFHeader.hh"
#include "RDFTargetStruct.hh"

/*makro pro odsazovani *out_stru*/
#ifndef NEW_LINE
#define NEW_LINE(st,cnt,c) \
   if((void *)&st != NULL) {\
      if(one_line == false) { \
         st << "\n";          \
         int i = 0;           \
         while(i++ < (int)cnt)\
            st << (c);        \
                              \
       } else {               \
         st << ';';           \
       }\
   }
#endif

enum WEATHER_MESSAGE_TYPE
{
	WEATHER_MESSAGE_TYPE_UNKNOWN,
	WEATHER_MESSAGE_TYPE_START_OF_PICTURE,
	WEATHER_MESSAGE_TYPE_END_OF_PICTURE,
	WEATHER_MESSAGE_TYPE_INTERMEDIATE_UPDATE_STEP,
	WEATHER_MESSAGE_TYPE_POLAR_VECTOR,
	WEATHER_MESSAGE_TYPE_CARTESIAN_VECTOR_1,
	WEATHER_MESSAGE_TYPE_CARTESIAN_VECTOR_2,
	WEATHER_MESSAGE_TYPE_CONTOUR_RECORD,
};

enum WEATHER_CONTOUR_TYPE
{
	WEATHER_CONTOUR_TYPE_UNKNOWN,
	WEATHER_CONTOUR_TYPE_INTERMEDIATE,
	WEATHER_CONTOUR_TYPE_LAST,
	WEATHER_CONTOUR_TYPE_FIRST,
	WEATHER_CONTOUR_TYPE_FIRST_AND_ONLY,
};


struct RDFWeatherPolarVector
{
	double StartRange;		// vzdalenost zacatku vektoru (m)
	double EndRange;		// vzdalenost konce vektoru (m)
	double Azimuth;			// uhel (rad)

	RDFWeatherPolarVector ();
	~RDFWeatherPolarVector ();
   void to_stringstream(std::stringstream &out, bool one_line = false);


};

struct RDFWeatherCartesianVector
{
	double X, Y;			// souradnice zacatku vektoru (m)
	double Length;			// delka vektoru (m)
	double Azimuth;			// uhel (rad)

	RDFWeatherCartesianVector ();
	~RDFWeatherCartesianVector ();
   void to_stringstream(std::stringstream &out, bool one_line = false);

};

struct RDFWeatherWeatherVector
{
	double X1, Y1;			// souradnice zacatku vektoru
	double X2, Y2;			// souradnice konce vektoru
	RDFWeatherWeatherVector ();
	~RDFWeatherWeatherVector ();
   void to_stringstream(std::stringstream &out, bool one_line = false);

};

struct RDFWeatherContourPoint {
	double X, Y;			// souradnice bodu

	RDFWeatherContourPoint ();
	~RDFWeatherContourPoint ();
   void to_stringstream(std::stringstream &out, bool one_line = false);
};

struct RDFWeatherMessage
{
	RDFTime Time;

	uint8_t SAC;					// identifikace zdroje
	uint8_t SIC;					// identifikace zdroje

	bool VectorSystemCoordinates : 1;
	bool ContourSystemCoordinates : 1;
	bool TestVector : 1;

	enum WEATHER_MESSAGE_TYPE MessageType;		// typ zpravy
	enum WEATHER_CONTOUR_TYPE ContourType;
	uint8_t VectorIntensity;
	uint8_t ContourIntensity;
	unsigned TotalNumberOfItems;
	uint8_t ContourSN;
	uint8_t StepSN;

	deque<RDFWeatherPolarVector> PolarVector;
	deque<RDFWeatherCartesianVector> CartesianVector;
	deque<RDFWeatherWeatherVector> WeatherVector;
	deque<RDFWeatherContourPoint> ContourPoint;
	
	RDFWeatherMessage ();
	~RDFWeatherMessage ();
 	RDFWeatherMessage (const RDFWeatherMessage & weather);
	const RDFWeatherMessage & operator = (const RDFWeatherMessage & weather);
   void to_stringstream(std::stringstream &out, bool one_line = false);
  protected:
	void init ();
	void erase ();
	void assign (const RDFWeatherMessage & weather);
};

#endif /* _RDF_WEATHER_MESSAGE_H */
