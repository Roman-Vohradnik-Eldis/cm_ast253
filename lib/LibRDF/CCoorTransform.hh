/*******************************************************************************
 * Module      : CCoorTransform
 * File        : CCoorTransform.hh
 * Description : Transformace souradnic ve WGS-84
 * Version     : 0.1.0
 * Author      : Jiri Tihelka, j.tihelka@gmail.cz
 * Date        : 20.08.2013
 * Reference   :
 *   Annex A: ASTERIX Category 17 Co-ordinate Transformations
 *                               (Co-ordinate transformation algorithms for the
 *                                hand-over of targets between POEMS interrogators)
 *******************************************************************************/

 /*******************************************************************************
 * Change history:
 * 22.03.2013 - Jiri Tihelka
 *   Nova verze.
 * 20.08.2013 - Jiri Tihelka
 *   Oprava v komentari (Reference).
 * 17.11.2013 - Jiri Tihelka
 *   Kompletne prepsano. Nyni zalozeno na nasledujicich metodach:
 *    - SetSensorPosGlobal
 *    - GetTargetPosGlobalFromLocal
 *    - GetTargetPosLocalFromGlobal
 *******************************************************************************/


#ifndef _CCOOR_TRANSFORMATIONS_H
#define _CCOOR_TRANSFORMATIONS_H

#include <math.h>
#include <string>
#include "CMatrix.hh"

#define CCOORTRANSFORM_VERSION "0.1.0.20131117"




#ifndef MULTITRACK_COORD
// local
typedef struct {
  double X;
  double Y;
  double Z;
} SXYZ;

typedef struct {
  double Range;     // [m]
  double Azimuth;   // [rad]
  double Elevation; // [rad]
} SRAE;

typedef struct {
  double Range;     // [m]
  double Azimuth;   // [rad]
  double Height;    // [m]
} SRAH;

typedef struct {
  double Range;         // [m]
  double Azimuth;       // [rad]
  double FlightLevel;   // [m]
} SRAF;

// global
typedef struct {
  double Latitude;  // [rad]
  double Longitude; // [rad]
  double Height;    // [m]
} SLLH;


inline double atan2_0_2pi(double y, double x)
{
  double z = atan2(y,x);

  return (z < 0 ? z +(2*M_PI) : z);
}

#else
#include "CCoord.hh"
#include "CCoordGlobal.hh"


#endif // MULTITRACK_COORD


inline double pow2(double x)
{
  return ((x)*(x));
}



class CCoorTransform {
  public:
    // Definice parametru elipsoidu WGS84
    static const double WGS84_a;           // semi-major axis
    static const double WGS84_f;           // flattening of WGS84 ellipsoid
    static const double WGS84_b;           // semi-minor axis
    static const double WGS84_e2;          // (first eccentricity)^2

    CCoorTransform();
    CCoorTransform(const SLLH &RadarPosGlobLLH);
    CCoorTransform(const SLLH &RadarPosGlobLLH, const double NorthAlignmentError);
//    CCoorTransform(const SLLH &radar_glob, const SRAF &target_loc);
    virtual ~CCoorTransform();

/*******************************************************************************
 * Name                 : CCoorTransform::SetSensorPosGlobal
 * Description          : Nastavi globalni polohu radaru
 * Parameters           : in  : llh - globalni poloha radaru (Latitude [rad], Longitude [rad], Height [m])
 *                      : out : -
 * Return               : void
 * Critical/explanation :
 *******************************************************************************/
    void SetSensorPosGlobal(const SLLH &llh);


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
    void GetTargetPosGlobalFromLocal(const SRAF &TargetPosLocRAE, SLLH &TargetPosGlobLLH);


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
    void GetTargetPosLocalFromGlobal(const SLLH &TargetPosGlobLLH, SRAE &TargetPosLocRAE);


/*******************************************************************************
 * Name                 : CCoorTransform::SetNorthAlignmentError
 * Description          : Nastavi chybu geografickeho severu radaru
 * Parameters           : in  : error
 *                      : out : -
 * Return               : void
 * Critical/explanation :
 *******************************************************************************/
    void SetNorthAlignmentError(const double error) {
      NorthAlignmentError = error;
    };


/*******************************************************************************
 * Name                 : CCoorTransform::version
 * Description          : verze modulu CCoorTransform
 * Parameters           : in  : -
 *                      : out : -
 * Return               : string - verze modulu CCoorTransform
 * Critical/explanation :
 *******************************************************************************/
    const std::string version(void);  // version

  protected:

  private:

    SLLH SensorPosGlobLLH;                // rad, rad, m
    double NorthAlignmentError;    // rad

    // pro rychlejsi vypocet, vse co je mozne napocitat po zadani radaru
    double sin_lat;
    double cos_lat;
    double sin_lon;
    double cos_lon;
    double nu;
    CMatrix TTranslation;  // translation factors
    CMatrix RRotation;     // rotation matrix
    CMatrix RRotation_T;   // transposed rotation matrix
};


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
void GetTargetGlobalPositionFromLocal(const SLLH &SensorPosGlobLLH, const SRAF &TargetLocRAF, SLLH &TargetGlob, const double NorthAlignmentError);


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
void GetTargetLocalPositionFromGlobal(const SLLH &SensorPosGlobLLH, const SLLH &TargetGlob, SRAE &TargetLocRAE, const double NorthAlignmentError);

/* Konverze lokalnich souradnic */
void convert_XYZ_to_RAE(const SXYZ &xyz, SRAE &rae);
void convert_XYZ_to_RAH(const SXYZ &xyz, SRAH &rah);
void convert_RAE_to_XYZ(const SRAE &rae, SRAE &xyz);
void convert_RAH_to_XYZ(const SRAH &rah, SXYZ &xyz);
void convert_RAE_to_RAH(const SRAE &rae, SRAH &rah);
void convert_RAH_to_RAE(const SRAH &rah, SRAE &rae);


#endif // _CCOOR_TRANSFORMATIONS_H
