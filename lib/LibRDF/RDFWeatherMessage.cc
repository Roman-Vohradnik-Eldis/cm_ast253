#include "RDFWeatherMessage.hh"

#define M_TO_NM(m) (m) * 0.000539956

RDFWeatherPolarVector::RDFWeatherPolarVector ()
{
   StartRange = EndRange = 0;
   Azimuth = 0;
}

RDFWeatherPolarVector::~RDFWeatherPolarVector ()
{
}
void 
RDFWeatherPolarVector::to_stringstream(std::stringstream &out, bool one_line) {
   out << std::setprecision(2) << std::fixed << "[start=" 
      << M_TO_NM(StartRange)  << " NM end=" << M_TO_NM(EndRange)
      << " NM azimuth=" << Azimuth * RAD_TO_DEG <<  "deg]";

}

RDFWeatherCartesianVector::RDFWeatherCartesianVector ()
{
   X = Y = 0;
   Length = 0;
   Azimuth = 0;
}

RDFWeatherCartesianVector::~RDFWeatherCartesianVector ()
{
}

void
RDFWeatherCartesianVector::to_stringstream(std::stringstream &out, bool one_line) {

   out << std::setprecision(2) << std::fixed << "[x=" 
       << M_TO_NM(X) << " NM, y=" << M_TO_NM(Y)  << " NM, length=" 
       << M_TO_NM(Length) << " NM"
//       << "]"
      << " azimuth=" << Azimuth * RAD_TO_DEG <<  "deg"
      << "]"
       ;

}


RDFWeatherWeatherVector::RDFWeatherWeatherVector ()
{
   X1 = Y1 = X2 = Y2 = 0;
}

RDFWeatherWeatherVector::~RDFWeatherWeatherVector ()
{
}
void
RDFWeatherWeatherVector::to_stringstream(std::stringstream &out, bool one_line) {
   out << std::setprecision(2) << std::fixed << "[x_1=" 
       << M_TO_NM(X1) << " NM, y_1=" << M_TO_NM(Y1) << " NM, x_2=" 
       << M_TO_NM(X2) << " NM, y_2=" << M_TO_NM(Y2) << " NM]";

}

RDFWeatherContourPoint::RDFWeatherContourPoint ()
{
   X = Y = 0;
}

RDFWeatherContourPoint::~RDFWeatherContourPoint ()
{
}

void
RDFWeatherContourPoint::to_stringstream(std::stringstream &out, bool one_line){
   out << std::setprecision(2) << std::fixed << "[x=" 
       << M_TO_NM(X) << " NM, y=" << M_TO_NM(Y) << " NM]";
}

RDFWeatherMessage::RDFWeatherMessage ()
{
   init ();
}

RDFWeatherMessage::~RDFWeatherMessage ()
{
   erase ();
}

RDFWeatherMessage::RDFWeatherMessage (const RDFWeatherMessage & weather)
{
   init ();
   assign (weather);
}

const RDFWeatherMessage & RDFWeatherMessage::operator = (const RDFWeatherMessage & weather)
{
   erase ();
   assign (weather);
   return weather;
}

void RDFWeatherMessage::init ()
{
   Time.init ();

   SAC = 0;
   SIC = 0;

   VectorSystemCoordinates = false;
   ContourSystemCoordinates = false;
   TestVector = false;

   MessageType = WEATHER_MESSAGE_TYPE_UNKNOWN;
   ContourType = WEATHER_CONTOUR_TYPE_UNKNOWN;
   VectorIntensity = 0;
   ContourIntensity = 0;
   TotalNumberOfItems = 0;
   ContourSN = 0;
   StepSN = 0;
}

void RDFWeatherMessage::erase ()
{
   Time.init ();

   SAC = 0;
   SIC = 0;

   VectorSystemCoordinates = false;
   ContourSystemCoordinates = false;
   TestVector = false;

   MessageType = WEATHER_MESSAGE_TYPE_UNKNOWN;
   ContourType = WEATHER_CONTOUR_TYPE_UNKNOWN;
   VectorIntensity = 0;
   ContourIntensity = 0;
   TotalNumberOfItems = 0;
   ContourSN = 0;
   StepSN = 0;

   PolarVector.clear ();
   CartesianVector.clear ();
   WeatherVector.clear ();
   ContourPoint.clear ();
}

void RDFWeatherMessage::assign (const RDFWeatherMessage & weather)
{
   Time = weather.Time;

   SIC = weather.SIC;
   SAC = weather.SAC;

   VectorSystemCoordinates = weather.VectorSystemCoordinates;
   ContourSystemCoordinates = weather.ContourSystemCoordinates;
   TestVector = weather.TestVector;

   MessageType = weather.MessageType;
   ContourType = weather.ContourType;
   VectorIntensity = weather.VectorIntensity;
   ContourIntensity = weather.ContourIntensity;
   TotalNumberOfItems = weather.TotalNumberOfItems;
   ContourSN = weather.ContourSN;
   StepSN = weather.StepSN;

   PolarVector = weather.PolarVector;
   CartesianVector = weather.CartesianVector;
   WeatherVector = weather.WeatherVector;
   ContourPoint = weather.ContourPoint;
}
