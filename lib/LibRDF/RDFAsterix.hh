#ifndef _RDF_ASTERIX_H
#define _RDF_ASTERIX_H

#include "RDFHeader.hh"

#include "RDFParService.hh"
#include "RDFParTarget.hh"
#include "RDFRadarService.hh"
#include "RDFTargetPlot.hh"
#include "RDFTargetTrack.hh"
#include "RDFTargetAdsb.hh"
#include "RDFWarningAlert.hh"
#include "RDFWeatherMessage.hh"
#include "RDFSurvCoordFunction.hh"
#include "RDFDatalinkFunction.hh"
#include "RDFVersionNumber.hh"

#include <sstream>
#include <iomanip>

int             field2signed (const unsigned char *const field, unsigned bits);
unsigned        field2unsigned (const unsigned char *const field, unsigned bits);

unsigned        asterix_decode_mode123 (unsigned code);
unsigned        asterix_encode_mode123 (unsigned value);
unsigned        asterix_decode_modec (unsigned code);
unsigned        asterix_encode_modec (unsigned value);
/**
 * Funkce vepise do stringstreamu latitude a longitude ve stupnich
 * minutach - lat, lon jsou v radianech
 */
void            geo_to_sstr (const double &lat, const double &lon, std::stringstream & out);

char            asterix_decode_char (unsigned char code);
unsigned char   asterix_encode_char (char ch);
void            asterix_decode_callsign (const unsigned char *code, string & callsign);
void            asterix_encode_callsign (string callsign, unsigned char *code);

const RDFCoorGeographic WeatherCenter009 (51. / 180 * M_PI, 8. / 180. * M_PI);  // stred zpracovani mulitradarove meteoinformace, cat 009

/*
 * ASTERIX obecne funkce
 */
bool            AsterixGetSACSIC (const void *data, unsigned size, unsigned char cat, unsigned char &sac, unsigned char &sic);

/*
 * ASTERIX 001
 */
bool            Asterix001_to_TargetTrack (const void *data, int size, deque < RDFTargetTrack > &track_list, int scaling_factor, std::stringstream * outstr =
                                           NULL, bool one_line = false);
bool            Asterix001_to_TargetPlot (const void *data, int size, deque < RDFTargetPlot > &plot_list, std::stringstream * outstr = NULL, bool one_line = false);
int             TargetTrack_to_Asterix001 (deque < RDFTargetTrack > &track_list, void *data, int maxsize, int scaling_factor);
int             TargetPlot_to_Asterix001 (deque < RDFTargetPlot > &plot_list, void *data, int maxsize);

/*
 * ASTERIX 002
 */
bool            Asterix002_to_RadarService (const void *data, int size, deque < RDFRadarService > &service_list);
int             RadarService_to_Asterix002 (deque < RDFRadarService > &service_list, void *data, int maxsize);

/*
 * ASTERIX 003
 */
bool            Asterix003_to_TargetTrack (const void *data, int size, deque < RDFTargetTrack > &track_list);
int             TargetTrack_to_Asterix003 (deque < RDFTargetTrack > &target_list, void *data, int maxsize);

/*
 * ASTERIX 004
 */
bool            Asterix004_to_WarningAlert (const void *data, int size, deque < RDFWarningAlert > &alert_list);
int             WarningAlert_to_Asterix004 (deque < RDFWarningAlert > &alert_list, void *data, int maxsize);

/*
 * ASTERIX 008
 */

bool            Asterix008_to_WeatherMessage (const void *data, int size,
                                              deque < RDFWeatherMessage > &weather_list, int &scaling_factor,
                                              std::stringstream * out_str = NULL, bool one_line = false);
int             WeatherMessage_to_Asterix008 (deque < RDFWeatherMessage > &weather_list,
                                              void *data, int maxsize, int scaling_factor, unsigned &WrittenNumberOfItems);

/*
 * ASTERIX 009
 */
bool            Asterix009_to_WeatherMessage (const void *data, int size,
                                              deque < RDFWeatherMessage > &weather_list, int &scaling_factor,
                                              std::stringstream * out_str = NULL, bool one_line = false);

int             WeatherMessage_to_Asterix009 (deque < RDFWeatherMessage > &weather_list,
                                              void *data, int maxsize, int scaling_factor, unsigned &WrittenNumberOfItems);

/*
 * ASTERIX 010
 */
bool            Asterix010_to_TargetTrack (const void *data, int size, deque < RDFTargetTrack > &track_list);
bool            Asterix010_to_TargetPlot (const void *data, int size, deque < RDFTargetPlot > &plot_list);

/*
 * ASTERIX 011
 */
bool            Asterix011_to_TargetTrack (const void *data, int size, deque < RDFTargetTrack > &track_list);

/*
 * ASTERIX 012
 */
bool            Asterix012_to_ParTarget (const void *data, int size, deque < RDFParTarget > &target_list, std::stringstream * out_str = NULL, bool one_line = false);
int             ParTarget_to_Asterix012 (deque < RDFParTarget > &target_list, void *data, int maxsize);

/*
 * ASTERIX 013
 */
bool            Asterix013_to_ParService (const void *data, int size, deque < RDFParService > &service_list, std::stringstream * out_str = NULL, bool one_line =
                                          false);
int             ParService_to_Asterix013 (deque < RDFParService > &service_list, void *data, int maxsize);

/*
 * ASTERIX 017
 */
bool            Asterix017_to_SurvCoordFunction (const unsigned char *data, int size, std::deque < RDFSurvCoordFunction > &scf_list);
int             SurvCoordFunction_to_Asterix017 (std::deque < RDFSurvCoordFunction > &scf_list, unsigned char *data, int maxsize);

/*
 * ASTERIX 018
 */
bool            Asterix018_to_DLFunction (const unsigned char *data, int size, deque < RDFDatalinkFunction > &track_list);
int             DLFunction_to_Asterix018 (deque < RDFDatalinkFunction > &track_list, unsigned char *data, int maxsize);

/*
 * ASTERIX 019
 */
bool            Asterix019_to_unknown (const void *data, int size);

// ASTERIX 020
bool            Asterix020_to_TargetAdsb (const void *data, int size, deque < RDFTargetAdsb > &adsb_list);
int             TargetAdsb_to_Asterix020 (deque < RDFTargetAdsb > &adsb_list, void *data, int maxsize);

/*
 * ASTERIX 021
 */
bool            Asterix021_to_TargetAdsb (const void *data, int size, deque < RDFTargetAdsb > &adsb_list, std::stringstream * out_str = NULL, bool one_line = false);

int             TargetAdsb_to_Asterix021 (deque < RDFTargetAdsb > &adsb_list, void *data, int maxsize);

/*
 * ASTERIX 030
 */
bool            Asterix030_to_TargetTrack (const void *data, int size, deque < RDFTargetTrack > &track_list);
int             TargetAdsb_to_Asterix020 (deque < RDFTargetAdsb > &adsb_list, void *data, int maxsize);

/*
 * ASTERIX 034
 */
bool            Asterix034_to_RadarService (const void *data, int size, deque < RDFRadarService > &service_list);
int             RadarService_to_Asterix034 (deque < RDFRadarService > &service_list, void *data, int maxsize);

/*
 * ASTERIX 048
 */
bool            Asterix048_to_Target (const void *data, int size, deque < RDFTargetTrack > &track_list, std::stringstream * outstr = NULL, bool one_line = false);
bool            Asterix048_to_Target (const void *data, int size, deque < RDFTargetTrack > &track_list, bool one_line);
bool            Asterix048_to_TargetTrack (const void *data, int size, deque < RDFTargetTrack > &track_list);
bool            Asterix048_to_TargetPlot (const void *data, int size, deque < RDFTargetPlot > &plot_list);
int             Target_to_Asterix048 (deque < RDFTargetTrack > &track_list, void *data, int maxsize);
int             TargetTrack_to_Asterix048 (deque < RDFTargetTrack > &track_list, void *data, int maxsize);
int             TargetPlot_to_Asterix048 (deque < RDFTargetPlot > &plot_list, void *data, int maxsize);

/*
 * ASTERIX 062
 */
bool            Asterix062_to_TargetTrack (const void *data, int size, deque < RDFTargetTrack > &track_list, std::stringstream * out_str = NULL, bool one_line =
                                           false);
int             TargetTrack_to_Asterix062 (deque < RDFTargetTrack > &track_list, void *data, int size);

/*
 * ASTERIX 065
 */
bool            Asterix065_to_unknown (const void *data, int size);

/*
 * ASTERIX 245
 */
bool            Asterix245_to_WarningAlert (const void *data, int size, deque < RDFWarningAlert > &alert_list);

/*
 * ASTERIX 247
 */
bool           Asterix247_to_VersionNumber (const void *data, int size, deque < RDFVersionNumber > &version_list);
int             VersionNumber_to_Asterix247 (deque < RDFVersionNumber > &version_list, void *data, int maxsize);

/*
 * ASTERIX 250
 */
bool            Asterix250_to_Target (const void *data, int size, deque < RDFTargetTrack > &target_list, std::stringstream * out_str = NULL, bool one_line = false);
bool            Asterix250_to_TargetTrack (const void *data, int size, deque < RDFTargetTrack > &track_list);
bool            Asterix250_to_TargetPlot (const void *data, int size, deque < RDFTargetPlot > &plot_list);
int             Target_to_Asterix250 (deque < RDFTargetTrack > &target_list, void *data, int maxsize);
int             TargetTrack_to_Asterix250 (deque < RDFTargetTrack > &track_list, void *data, int maxsize);
int             TargetPlot_to_Asterix250 (deque < RDFTargetPlot > &plot_list, void *data, int maxsize);

/*
 * ASTERIX 252
 */
bool            Asterix252_to_unknown (const void *data, int size);

#endif /* _RDF_ASTERIX_H */
