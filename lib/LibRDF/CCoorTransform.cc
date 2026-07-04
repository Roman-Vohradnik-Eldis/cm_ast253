/*******************************************************************************
 * Info je v CCoorTransform.hh
 *******************************************************************************/


#include "CCoorTransform.hh"

//#include "debug.h"

#if defined(__GNUC__)
#define COMPILER_STRING "gcc " __VERSION__
#else
#define COMPILER_STRING "a non-gcc compiler"
#endif

/*******************************************************************************
 *  Constants
 *******************************************************************************/
static const string c_CCoorTransformVersion = CCOORTRANSFORM_VERSION; // version of the CCoorTransform module

// constanty z Annex A: ASTERIX Category 17 Co-ordinate Transformations
// const double CCoorTransform::WGS84_a = 6378137.000;           // semi-major axis
// const double CCoorTransform::WGS84_f = 1.0/298.2572235630;    // flattening of WGS84 ellipsoid
// const double CCoorTransform::WGS84_b = 6356752.314;           // semi-minor axis
// const double CCoorTransform::WGS84_e2 = 0.00669437999013;     // (first eccentricity)^2

// konstanty presne podle definice WGS84 elipsoidu (definice: a, f - zbytek se pocita)
const double CCoorTransform::WGS84_a = 6378137.000;           // semi-major axis
const double CCoorTransform::WGS84_f = 1. / 298.2572235630;   // flattening of WGS84 ellipsoid
const double CCoorTransform::WGS84_b = (1. - WGS84_f) * WGS84_a; // semi-minor axis
const double CCoorTransform::WGS84_e2 = 1. - pow2(WGS84_b) / pow2(WGS84_a);   // (first eccentricity)^2
/*******************************************************************************
 *  Constants end
 *******************************************************************************/


CCoorTransform::CCoorTransform()
{
  // ctor
  NorthAlignmentError = 0.;

  TTranslation.resize(3, 1); // translation factors
  RRotation.resize(3, 3);    // rotation matrix
  RRotation_T.resize(3, 3);  // transposed rotation matrix
}


CCoorTransform::CCoorTransform(const SLLH &RadarPosGlobLLH)
{
  // ctor
  NorthAlignmentError = 0.;

  TTranslation.resize(3, 1); // translation factors
  RRotation.resize(3, 3);    // rotation matrix
  RRotation_T.resize(3, 3);  // transposed rotation matrix

  SetSensorPosGlobal(RadarPosGlobLLH);
}


CCoorTransform::CCoorTransform(const SLLH &RadarPosGlobLLH, const double NorthAlignmentError = 0.)
{
  // ctor
  CCoorTransform::NorthAlignmentError = NorthAlignmentError;

  TTranslation.resize(3, 1); // translation factors
  RRotation.resize(3, 3);    // rotation matrix
  RRotation_T.resize(3, 3);  // transposed rotation matrix

  SetSensorPosGlobal(RadarPosGlobLLH);
}


CCoorTransform::~CCoorTransform()
{
  // dtor
}


/*******************************************************************************
 * Name                 : CCoorTransform::SetSensorPosGlobal
 * Description          : Nastavi globalni polohu radaru
 * Parameters           : in  : llh - globalni poloha radaru (Latitude [rad], Longitude [rad], Height [m])
 *                      : out : -
 * Return               : void
 * Critical/explanation :
 *******************************************************************************/
void CCoorTransform::SetSensorPosGlobal(const SLLH &llh)
{
  SensorPosGlobLLH.Latitude = llh.Latitude;
  SensorPosGlobLLH.Longitude = llh.Longitude;
  SensorPosGlobLLH.Height = llh.Height;

  // muze se pocitat dopredu pro danou polohu radaru
  sin_lat = sin(SensorPosGlobLLH.Latitude);
  cos_lat = cos(SensorPosGlobLLH.Latitude);
  sin_lon = sin(SensorPosGlobLLH.Longitude);
  cos_lon = cos(SensorPosGlobLLH.Longitude);

  // (3-5)
  nu = WGS84_a / sqrt(1. - WGS84_e2 * pow2(sin_lat));

  // (3-4) position of the radar system is also the co-ordinate origin for the local co-ordinate system
  TTranslation << (nu + SensorPosGlobLLH.Height)*cos_lat*cos_lon
               << (nu + SensorPosGlobLLH.Height)*cos_lat*sin_lon
               << ((nu * (1. - WGS84_e2)) + SensorPosGlobLLH.Height)*sin_lat;

  // (3-8) rotation matrix
  //  CMatrix RRotation(3,3);
  RRotation << -sin_lon         <<  cos_lon         << 0.
            << -sin_lat*cos_lon << -sin_lat*sin_lon << cos_lat
            <<  cos_lat*cos_lon <<  cos_lat*sin_lon << sin_lat;
  RRotation_T = RRotation.t();
}


/*******************************************************************************
 * Name                 : CCoorTransform::GetTargetPosGlobalFromLocal
 * Description          : Vrati globalni polohu letadla z lokalni
 * Parameters           : in  : TargetPosLocRAF - lokalni poloha letadla (Range [m], Azimuth [rad], FlightLevel [m])
 *                      : out : TargetPosGlobLLH - globalni poloha letadla (Latitude [rad], Longitude [rad], Height [m])
 * Return               : void
 * Global               : in  : CCoorTransform::SensorPosGlobLLH - globalni poloha radaru (Latitude [rad], Longitude [rad], Height [m])
 *                      : in  : CCoorTransform::NorthAlignmentError - chyba nastaveni geografickeho severu radaru
 * Critical/explanation : FlightLevel = vyska nad WGS84 elipsoidem
 *                      : Return.Latitude  -> <-pi/2,pi/2>
 *                      : Return.Longitude -> <-pi,pi>
 *******************************************************************************/
void CCoorTransform::GetTargetPosGlobalFromLocal(const SRAF &TargetPosLocRAF, SLLH &TargetPosGlobLLH)
{
  // local polar co-ordinates (ρ, θ, H) of the aircraft -> local spherical co-ordinates (ρ, θ, ψ).

  // (2-1) earth radius at a given geodetic latitude
  double R_Earth = WGS84_a * (1. - WGS84_e2) / sqrt(pow(1. - WGS84_e2 * pow2(sin_lat), 3));

  // (2-3) elevation angle (max. error 0.2° (2-4)
  double ElevationLoc = asin((2.*R_Earth * (TargetPosLocRAF.FlightLevel - SensorPosGlobLLH.Height) + pow2(TargetPosLocRAF.FlightLevel) - pow2(SensorPosGlobLLH.Height) - pow2(TargetPosLocRAF.Range))
                             / (2.*TargetPosLocRAF.Range * (R_Earth + SensorPosGlobLLH.Height))); // elevation - psi

  // local spherical co-ordinates (ρ, θ, ψ) -> local Cartesian co-ordinates (Xl, Yl, Zl)
  CMatrix XYZLoc(3, 1);

  // (2-5) local cartesian co-ordinate set
  // Note: NorthAlignmentError represents the error in the alignment to the geographical north of the radar
  double cos_elev = cos(ElevationLoc);
  XYZLoc << TargetPosLocRAF.Range*cos_elev*sin(TargetPosLocRAF.Azimuth + NorthAlignmentError)
         << TargetPosLocRAF.Range*cos_elev*cos(TargetPosLocRAF.Azimuth + NorthAlignmentError)
         << TargetPosLocRAF.Range*sin(ElevationLoc);

  // local Cartesian co-ordinates -> geocentric Cartesian co-ordinates
  CMatrix XYZGlob(3, 1);

  // (3-7) geocentric cartesian co-ordinate set
  XYZGlob = TTranslation + RRotation_T * XYZLoc;

  // global geocentric -> geodesic co-ordinates
  SXYZ XYZg;
  XYZg.X = XYZGlob(0, 0);
  XYZg.Y = XYZGlob(1, 0);
  XYZg.Z = XYZGlob(2, 0);

  // (4-1)
  TargetPosGlobLLH.Longitude = atan2(XYZg.Y, XYZg.X);
  // (4-6)
  // The value of the normalised earth radius at the aircraft latitude η (nu) can only be determined accurately once the latitude is known.
  // -> nu shall be approximated by using the normalised earth radius at the radar position, the resulting error will be smaller than 6e-8. Note that the error will be less than 1 meter.
  TargetPosGlobLLH.Latitude = atan2((1. + TargetPosLocRAF.FlightLevel / nu) * XYZg.Z, ((1. - WGS84_e2) + TargetPosLocRAF.FlightLevel / nu) * (sqrt(pow2(XYZg.X) + pow2(XYZg.Y))));
  // the altitude of the aircraft is already known, from the on-board measurement of H
  TargetPosGlobLLH.Height = TargetPosLocRAF.FlightLevel;
}


/*******************************************************************************
 * Name                 : CCoorTransform::GetTargetPosLocalFromGlobal
 * Description          : Vrati lokalni polohu letadla z globalni
 * Parameters           : in  : TargetPosGlobLLH - globalni poloha letadla (Latitude [rad], Longitude [rad], Height [m])
 *                      : out : TargetPosLocRAE - lokalni poloha letadla (Range [rad], Azimuth [rad], Elevation [rad])
 * Return               : void
 * Global               : in  : CCoorTransform::SensorPosGlobLLH - globalni poloha radaru (Latitude [rad], Longitude [rad], Height [m])
 *                      : in  : CCoorTransform::NorthAlignmentError - chyba nastaveni geografickeho severu radaru
 * Critical/explanation : Return.Latitude  -> <-pi/2,pi/2>
 *                      : Return.Longitude -> <-pi,pi>
 *******************************************************************************/
void CCoorTransform::GetTargetPosLocalFromGlobal(const SLLH &TargetPosGlobLLH, SRAE &TargetPosLocRAE)
{
  // poloha cile

  // (3-5)
  // ale pocitame pro polohu cile (ne radaru), proto nevyuzivavame predem urcenou hodnotu
  double nu_target = WGS84_a / sqrt(1. - WGS84_e2 * pow2(sin(TargetPosGlobLLH.Latitude)));

  // (3-4) position of the radar system is also the co-ordinate origin for the local co-ordinate system
  // muze se pocitat dopredu pro danou polohu radaru
  CMatrix TargetGlob(3, 1);
  TargetGlob << (nu_target + TargetPosGlobLLH.Height)*cos(TargetPosGlobLLH.Latitude)*cos(TargetPosGlobLLH.Longitude)
             << (nu_target + TargetPosGlobLLH.Height)*cos(TargetPosGlobLLH.Latitude)*sin(TargetPosGlobLLH.Longitude)
             << ((nu_target * (1. - WGS84_e2)) + TargetPosGlobLLH.Height)*sin(TargetPosGlobLLH.Latitude);


  CMatrix TargetLoc(3, 1);
  TargetLoc = RRotation * (TargetGlob - TTranslation);


  //  SRAE RAE;
  SXYZ TargetLocXYZ; // ({TargetLoc(0), TargetLoc(1), TargetLoc(2)});
  TargetLocXYZ.X = TargetLoc(0);
  TargetLocXYZ.Y = TargetLoc(1);
  TargetLocXYZ.Z = TargetLoc(2);

  convert_XYZ_to_RAE(TargetLocXYZ, TargetPosLocRAE);
  TargetPosLocRAE.Azimuth -= NorthAlignmentError;
}


/*******************************************************************************
 * Name                 : CCoorTransform::version
 * Description          : verze modulu CCoorTransform
 * Parameters           : in  : -
 *                      : out : -
 * Return               : string - verze modulu CCoorTransform
 * Critical/explanation :
 *******************************************************************************/
const std::string CCoorTransform::version(void)
{
  return c_CCoorTransformVersion + " compiled with " + COMPILER_STRING;
}

//#define GET_TARGET_POS_OPTIMIZED


/*******************************************************************************
 * Name                 : GetTargetGlobalPositionFromLocal
 * Description          : Vrati globalni polohu letadla z lokalni
 * Parameters           : in  : SensorPosGlobLLH - globalni poloha radaru (Latitude [rad], Longitude [rad], Height [m])
 *                      : in  : TargetLocRAF - lokalni poloha letadla (Range [m], Azimuth [rad], FlightLevel [m])
 *                      : in  : NorthAlignmentError - chyba nastaveni geografickeho severu radaru
 *                      : out : TargetGlobLLH - globalni poloha letadla (Latitude [rad], Longitude [rad], Height [m])
 * Return               : void
 * Global               : -
 * Critical/explanation : FlightLevel = vyska nad WGS84 elipsoidem
 *                      : TargetGlob.Latitude  -> <-pi/2,pi/2>
 *                      : TargetGlob.Longitude -> <-pi,pi>
 *******************************************************************************/
void GetTargetGlobalPositionFromLocal(const SLLH &SensorPosGlobLLH, const SRAF &TargetLocRAF, SLLH &TargetGlobLLH, const double NorthAlignmentError = 0.)
{
  const double WGS84_a  = CCoorTransform::WGS84_a;
  const double WGS84_e2 = CCoorTransform::WGS84_e2;

#ifdef GET_TARGET_POS_OPTIMIZED
  const double sin_lat = sin(SensorPosGlobLLH.Latitude);     // muze se pocitat dopredu pro danou polohu radaru
  const double cos_lat = cos(SensorPosGlobLLH.Latitude);     // muze se pocitat dopredu pro danou polohu radaru
  const double sin_lon = sin(SensorPosGlobLLH.Longitude);    // muze se pocitat dopredu pro danou polohu radaru
  const double cos_lon = cos(SensorPosGlobLLH.Longitude);    // muze se pocitat dopredu pro danou polohu radaru

  // local polar co-ordinates (ρ, θ, H) of the aircraft -> local spherical co-ordinates (ρ, θ, ψ).

  // (2-1) earth radius at a given geodetic latitude
  double R_Earth = WGS84_a * (1. - WGS84_e2) / sqrt(pow(1. - WGS84_e2 * pow2(sin_lat), 3));

  // (2-3) elevation angle (max. error 0.2° (2-4)
  double ElevationLoc = asin((2.*R_Earth * (TargetLocRAF.FlightLevel - SensorPosGlobLLH.Height) + pow2(TargetLocRAF.FlightLevel) - pow2(SensorPosGlobLLH.Height) - pow2(TargetLocRAF.Range))
                             / (2.*TargetLocRAF.Range * (R_Earth + SensorPosGlobLLH.Height))); // elevation - psi

  // local spherical co-ordinates (ρ, θ, ψ) -> local Cartesian co-ordinates (Xl, Yl, Zl)
  CMatrix XYZLoc(3, 1);

  // (2-5) local cartesian co-ordinate set
  // Note: NorthAlignmentError represents the error in the alignment to the geographical north of the radar.
  XYZLoc << TargetLocRAF.Range*cos(ElevationLoc)*sin(TargetLocRAF.Azimuth + NorthAlignmentError)
         << TargetLocRAF.Range*cos(ElevationLoc)*cos(TargetLocRAF.Azimuth + NorthAlignmentError)
         << TargetLocRAF.Range*sin(ElevationLoc);

  // local Cartesian co-ordinates -> geocentric Cartesian co-ordinates
  CMatrix Ref(3, 1);
  CMatrix R(3, 3);
  CMatrix XYZGlob(3, 1);

  // (3-5)
  // muze se pocitat dopredu pro danou polohu radaru
  double nu = WGS84_a / sqrt(1. - WGS84_e2 * pow2(sin_lat));

  // (3-4) position of the radar system is also the co-ordinate origin for the local co-ordinate system
  // muze se pocitat dopredu pro danou polohu radaru
  Ref << (nu + SensorPosGlobLLH.Height)*cos_lat*cos_lon
      << (nu + SensorPosGlobLLH.Height)*cos_lat*sin_lon
      << ((nu * (1. - WGS84_e2)) + SensorPosGlobLLH.Height)*sin_lat;

  // (3-8) rotation matrix    // muze se pocitat dopredu pro danou polohu radaru
  R << -sin_lon         <<  cos_lon         << 0.
    << -sin_lat*cos_lon << -sin_lat*sin_lon << cos_lat
    <<  cos_lat*cos_lon <<  cos_lat*sin_lon << sin_lat;

  // (3-7) geocentric cartesian co-ordinate set
  XYZGlob = Ref + R.t() * XYZLoc;

  // global geocentric -> geodesic co-ordinates
  SXYZ XYZg;
  XYZg.X = XYZGlob(0, 0);
  XYZg.Y = XYZGlob(1, 0);
  XYZg.Z = XYZGlob(2, 0);

  // (4-1)
  TargetGlobLLH.Longitude = atan2(XYZg.Y, XYZg.X);
  // (4-6)
  // The value of the normalised earth radius at the aircraft latitude η (nu) can only be determined accurately once the latitude is known.
  // -> nu shall be approximated by using the normalised earth radius at the radar position, the resulting error will be smaller than 6e-8. Note that the error will be less than 1 meter.
  TargetGlobLLH.Latitude = atan2((1. + TargetLocRAF.FlightLevel / nu) * XYZg.Z, ((1. - WGS84_e2) + TargetLocRAF.FlightLevel / nu) * (sqrt(pow2(XYZg.X) + pow2(XYZg.Y))));
  // the altitude of the aircraft is already known, from the on-board measurement of H
  TargetGlobLLH.Height = TargetLocRAF.FlightLevel;

#else // GET_TARGET_POS_OPTIMIZED

  // local polar co-ordinates (ρ, θ, H) of the aircraft -> local spherical co-ordinates (ρ, θ, ψ).

  // (2-1) earth radius at a given geodetic latitude
  double R_Earth = WGS84_a * (1. - WGS84_e2) / sqrt(pow(1. - WGS84_e2 * pow2(sin(SensorPosGlobLLH.Latitude)), 3));

  // (2-3) elevation angle (max. error 0.2° (2-4)
  double ElevationLoc = asin((2.*R_Earth * (TargetLocRAF.FlightLevel - SensorPosGlobLLH.Height) + pow2(TargetLocRAF.FlightLevel) - pow2(SensorPosGlobLLH.Height) - pow2(TargetLocRAF.Range))
                             / (2.*TargetLocRAF.Range * (R_Earth + SensorPosGlobLLH.Height))); // elevation - psi
  //double elev = asin((2*R1*(H-h1) + pow2(H) - pow2(h1) - pow2(r)) / (2*r*(R1+h1)));   // elevation - psi

  // local spherical co-ordinates (ρ, θ, ψ) -> local Cartesian co-ordinates (Xl, Yl, Zl)
  CMatrix XYZLoc(3, 1);

  // (2-5) local cartesian co-ordinate set
  // Note: NorthAlignmentError represents the error in the alignment to the geographical north of the radar.
  XYZLoc << TargetLocRAF.Range*cos(ElevationLoc)*sin(TargetLocRAF.Azimuth + NorthAlignmentError)
         << TargetLocRAF.Range*cos(ElevationLoc)*cos(TargetLocRAF.Azimuth + NorthAlignmentError)
         << TargetLocRAF.Range*sin(ElevationLoc);

  // local Cartesian co-ordinates -> geocentric Cartesian co-ordinates
  CMatrix Ref(3, 1);
  CMatrix R(3, 3);
  CMatrix XYZGlob(3, 1);

  // (3-5)
  // muze se pocitat dopredu pro danou polohu radaru
  double nu = WGS84_a / sqrt(1. - WGS84_e2 * pow2(sin(SensorPosGlobLLH.Latitude)));

  // (3-4) position of the radar system is also the co-ordinate origin for the local co-ordinate system
  // muze se pocitat dopredu pro danou polohu radaru
  Ref << (nu + SensorPosGlobLLH.Height)*cos(SensorPosGlobLLH.Latitude)*cos(SensorPosGlobLLH.Longitude)
      << (nu + SensorPosGlobLLH.Height)*cos(SensorPosGlobLLH.Latitude)*sin(SensorPosGlobLLH.Longitude)
      << ((nu * (1. - WGS84_e2)) + SensorPosGlobLLH.Height)*sin(SensorPosGlobLLH.Latitude);

  // (3-8) rotation matrix    // muze se pocitat dopredu pro danou polohu radaru
  R << -sin(SensorPosGlobLLH.Longitude)                            <<  cos(SensorPosGlobLLH.Longitude)                            << 0.
    << -sin(SensorPosGlobLLH.Latitude)*cos(SensorPosGlobLLH.Longitude) << -sin(SensorPosGlobLLH.Latitude)*sin(SensorPosGlobLLH.Longitude) << cos(SensorPosGlobLLH.Latitude)
    <<  cos(SensorPosGlobLLH.Latitude)*cos(SensorPosGlobLLH.Longitude) <<  cos(SensorPosGlobLLH.Latitude)*sin(SensorPosGlobLLH.Longitude) << sin(SensorPosGlobLLH.Latitude);

  // (3-7) geocentric cartesian co-ordinate set
  XYZGlob = Ref + R.t() * XYZLoc;

  // global geocentric -> geodesic co-ordinates
  SXYZ XYZg;
  XYZg.X = XYZGlob(0, 0);
  XYZg.Y = XYZGlob(1, 0);
  XYZg.Z = XYZGlob(2, 0);

  // (4-1)
  TargetGlobLLH.Longitude = atan2(XYZg.Y, XYZg.X);
  // (4-6)
  // The value of the normalised earth radius at the aircraft latitude η (nu) can only be determined accurately once the latitude is known.
  // -> nu shall be approximated by using the normalised earth radius at the radar position, the resulting error will be smaller than 6e-8. Note that the error will be less than 1 meter.
  TargetGlobLLH.Latitude = atan2((1. + TargetLocRAF.FlightLevel / nu) * XYZg.Z, ((1. - WGS84_e2) + TargetLocRAF.FlightLevel / nu) * (sqrt(pow2(XYZg.X) + pow2(XYZg.Y))));
  // the altitude of the aircraft is already known, from the on-board measurement of H
  TargetGlobLLH.Height = TargetLocRAF.FlightLevel;

#endif // GET_TARGET_POS_OPTIMIZED
}


/*******************************************************************************
 * Name                 : GetTargetLocalPositionFromGlobal
 * Description          : Vrati lokalni polohu letadla z globalni
 * Parameters           : in  : SensorPosGlobLLH - globalni poloha radaru (Latitude [rad], Longitude [rad], Height [m])
 *                      : in  : TargetPosGlobLLH - globalni poloha letadla (Latitude [rad], Longitude [rad], Height [m])
 *                      : in  : NorthAlignmentError - chyba nastaveni geografickeho severu radaru
 *                      : out : TargetPosLocRAE - lokalni poloha letadla (Range [rad], Azimuth [rad], Elevation [rad])
 * Return               : void
 * Global               : -
 * Critical/explanation : Return.Latitude  -> <-pi/2,pi/2>
 *                      : Return.Longitude -> <-pi,pi>
 *******************************************************************************/
void GetTargetLocalPositionFromGlobal(const SLLH &SensorPosGlobLLH, const SLLH &TargetPosGlobLLH, SRAE &TargetPosLocRAE, const double NorthAlignmentError = 0.)
{
  const double WGS84_a  = CCoorTransform::WGS84_a;
  const double WGS84_e2 = CCoorTransform::WGS84_e2;

#ifdef GET_TARGET_POS_OPTIMIZED
  // poloha radaru

  const double sin_lat = sin(SensorPosGlobLLH.Latitude);     // muze se pocitat dopredu pro danou polohu radaru
  const double cos_lat = cos(SensorPosGlobLLH.Latitude);     // muze se pocitat dopredu pro danou polohu radaru
  const double sin_lon = sin(SensorPosGlobLLH.Longitude);    // muze se pocitat dopredu pro danou polohu radaru
  const double cos_lon = cos(SensorPosGlobLLH.Longitude);    // muze se pocitat dopredu pro danou polohu radaru

  // (3-5)
  double nu = WGS84_a / sqrt(1. - WGS84_e2 * pow2(sin_lat));

  // (3-4) position of the radar system is also the co-ordinate origin for the local co-ordinate system
  CMatrix TTranslation(3, 1);
  //  CMatrix R(3,3);
  //  CMatrix XYZGlob(3,1);

  TTranslation << (nu + SensorPosGlobLLH.Height)*cos_lat*cos_lon
               << (nu + SensorPosGlobLLH.Height)*cos_lat*sin_lon
               << ((nu * (1. - WGS84_e2)) + SensorPosGlobLLH.Height)*sin_lat;

  // (3-8) rotation matrix
  CMatrix RRotation(3, 3);
  RRotation << -sin_lon         <<  cos_lon         << 0.
            << -sin_lat*cos_lon << -sin_lat*sin_lon << cos_lat
            <<  cos_lat*cos_lon <<  cos_lat*sin_lon << sin_lat;
  //  RRotation_T = RRotation.t();


  // poloha cile

  // (3-5)
  // muze se pocitat dopredu pro danou polohu radaru
  double nu_target = WGS84_a / sqrt(1. - WGS84_e2 * pow2(sin(TargetPosGlobLLH.Latitude)));

  // (3-4) position of the radar system is also the co-ordinate origin for the local co-ordinate system
  // muze se pocitat dopredu pro danou polohu radaru
  CMatrix TargetGlob(3, 1);
  TargetGlob << (nu_target + TargetPosGlobLLH.Height)*cos(TargetPosGlobLLH.Latitude)*cos(TargetPosGlobLLH.Longitude)
             << (nu_target + TargetPosGlobLLH.Height)*cos(TargetPosGlobLLH.Latitude)*sin(TargetPosGlobLLH.Longitude)
             << ((nu_target * (1. - WGS84_e2)) + TargetPosGlobLLH.Height)*sin(TargetPosGlobLLH.Latitude);




  //  CCoordLocal Loc(S);
  //  Loc.Rotate(pos_ref.GetLongitude(), pos_ref.GetLatitude());

  CMatrix TargetLoc(3, 1);
  TargetLoc = RRotation * (TargetGlob - TTranslation);

  SXYZ TargetLocXYZ; // ({TargetLoc(0), TargetLoc(1), TargetLoc(2)});
  TargetLocXYZ.X = TargetLoc(0);
  TargetLocXYZ.Y = TargetLoc(1);
  TargetLocXYZ.Z = TargetLoc(2);

  convert_XYZ_to_RAE(TargetLocXYZ, TargetPosLocRAE);
  TargetPosLocRAE.Azimuth -= NorthAlignmentError;

#else // GET_TARGET_POS_OPTIMIZED
  // poloha radaru

  // local Cartesian co-ordinates -> geocentric Cartesian co-ordinates
  CMatrix Ref(3, 1);
  CMatrix R(3, 3);
  CMatrix XYZGlob(3, 1);

  // (3-5)
  // muze se pocitat dopredu pro danou polohu radaru
  double nu = WGS84_a / sqrt(1. - WGS84_e2 * pow2(sin(SensorPosGlobLLH.Latitude)));

  // (3-4) position of the radar system is also the co-ordinate origin for the local co-ordinate system
  // muze se pocitat dopredu pro danou polohu radaru
  Ref << (nu + SensorPosGlobLLH.Height)*cos(SensorPosGlobLLH.Latitude)*cos(SensorPosGlobLLH.Longitude)
      << (nu + SensorPosGlobLLH.Height)*cos(SensorPosGlobLLH.Latitude)*sin(SensorPosGlobLLH.Longitude)
      << ((nu * (1. - WGS84_e2)) + SensorPosGlobLLH.Height)*sin(SensorPosGlobLLH.Latitude);

  // (3-8) rotation matrix    // muze se pocitat dopredu pro danou polohu radaru
  R << -sin(SensorPosGlobLLH.Longitude)          <<  cos(SensorPosGlobLLH.Longitude)          << 0.
    << -sin(SensorPosGlobLLH.Latitude)*cos(SensorPosGlobLLH.Longitude) << -sin(SensorPosGlobLLH.Latitude)*sin(SensorPosGlobLLH.Longitude) << cos(SensorPosGlobLLH.Latitude)
    <<  cos(SensorPosGlobLLH.Latitude)*cos(SensorPosGlobLLH.Longitude) <<  cos(SensorPosGlobLLH.Latitude)*sin(SensorPosGlobLLH.Longitude) << sin(SensorPosGlobLLH.Latitude);

  // poloha cile

  // (3-5)
  // muze se pocitat dopredu pro danou polohu radaru
  double nu_target = WGS84_a / sqrt(1. - WGS84_e2 * pow2(sin(TargetPosGlobLLH.Latitude)));

  // (3-4) position of the radar system is also the co-ordinate origin for the local co-ordinate system
  // muze se pocitat dopredu pro danou polohu radaru
  CMatrix TargetGlob(3, 1);
  TargetGlob << (nu_target + TargetPosGlobLLH.Height)*cos(TargetPosGlobLLH.Latitude)*cos(TargetPosGlobLLH.Longitude)
             << (nu_target + TargetPosGlobLLH.Height)*cos(TargetPosGlobLLH.Latitude)*sin(TargetPosGlobLLH.Longitude)
             << ((nu_target * (1. - WGS84_e2)) + TargetPosGlobLLH.Height)*sin(TargetPosGlobLLH.Latitude);


  CMatrix TargetLoc(3, 1);
  TargetLoc = R * (TargetGlob - Ref);

  SXYZ TargetLocXYZ; // ({TargetLoc(0), TargetLoc(1), TargetLoc(2)});
  TargetLocXYZ.X = TargetLoc(0);
  TargetLocXYZ.Y = TargetLoc(1);
  TargetLocXYZ.Z = TargetLoc(2);

  convert_XYZ_to_RAE(TargetLocXYZ, TargetPosLocRAE);
  TargetPosLocRAE.Azimuth -= NorthAlignmentError;

#endif // GET_TARGET_POS_OPTIMIZED

}


void convert_XYZ_to_RAE(const SXYZ &XYZ, SRAE &RAE)
{
  RAE.Range = sqrt(pow2(XYZ.X) + pow2(XYZ.Y) + pow2(XYZ.Z));
  double Rxy = sqrt(pow2(XYZ.X) + pow2(XYZ.Y));
  RAE.Azimuth = atan2_0_2pi(XYZ.X, XYZ.Y);
  RAE.Elevation = acos(Rxy / RAE.Range);
}


void convert_XYZ_to_RAH(const SXYZ &XYZ, SRAH &RAH)
{
  RAH.Range = sqrt(pow2(XYZ.X) + pow2(XYZ.Y) + pow2(XYZ.Z));
  RAH.Azimuth = atan2_0_2pi(XYZ.X, XYZ.Y);
  RAH.Height = XYZ.Z;
}


void convert_RAE_to_XYZ(const SRAE &RAE, SXYZ &XYZ)
{
  double Rxy = RAE.Range * cos(RAE.Elevation);
  XYZ.X = Rxy * sin(RAE.Azimuth);
  XYZ.Y = Rxy * cos(RAE.Azimuth);
  XYZ.Z = RAE.Range * sin(RAE.Elevation);
}


void convert_RAH_to_XYZ(const SRAH &RAH, SXYZ &XYZ)
{
  double Rxy   = sqrt(pow2(RAH.Range) - pow2(RAH.Height));
  XYZ.X = Rxy * sin(RAH.Azimuth);
  XYZ.Y = Rxy * cos(RAH.Azimuth);
  XYZ.Z = RAH.Height;
}


void convert_RAE_to_RAH(const SRAE &RAE, SRAH &RAH)
{
  RAH.Range = RAE.Range;
  RAH.Azimuth = RAE.Azimuth;
  RAH.Height = RAE.Range * sin(RAE.Elevation);
}


void convert_RAH_to_RAE(const SRAH &RAH, SRAE &RAE)
{
  RAE.Range = RAH.Range;
  double Rxy   = sqrt(pow2(RAH.Range) - pow2(RAH.Height));
  RAE.Azimuth = RAH.Azimuth;
  RAE.Elevation = acos(Rxy / RAE.Range);
}
