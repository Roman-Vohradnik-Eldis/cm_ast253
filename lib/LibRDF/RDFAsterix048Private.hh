#ifndef _RDF_ASTERIX_048_PRIVATE_H
#define _RDF_ASTERIX_048_PRIVATE_H

#include <sstream>
#include <cstdio>
#include <cstdlib>

// Data Source Identifier
void asterix_048_read_010 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_010 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_010( const uint8_t * const field,  std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "010"); 
// Target Report Description
bool asterix_048_read_020 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_020 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_020( const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "020");
// Warning/Error Conditions
void asterix_048_read_030 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_030 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_030( const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "030");

// Measured Position in Slant Polar Coordinates
void asterix_048_read_040 (RDFTargetTrack & target, unsigned char * &field);
bool asterix_048_write_040 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_040( const uint8_t * const field, std::stringstream *str_out = NULL,  bool one_line = false, const std::string spec = "040");

// Calculated Position in Cartesian Coordinates
void asterix_048_read_042 (RDFTargetTrack & target, unsigned char * &field);
bool asterix_048_write_042 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_042 ( const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "042");
// Mode-2 Code in Octal Representation
void asterix_048_read_050 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_050 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_050 ( const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "050"); 
// Mode-1 Code in Octal Representation
void asterix_048_read_055 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_055 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_055 ( const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "055"); 
// Mode-2 Code Confidence Indicator
void asterix_048_read_060 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_060 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_060 ( const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "060"); 

// Mode-1 Code Confidence Indicator
void asterix_048_read_065 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_065 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_065 ( const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "065");

// Mode-3/A Code in Octal Representation
void asterix_048_read_070 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_070 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_070 ( const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "070");

// Mode-3/A Code Confidence Indicator
void asterix_048_read_080 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_080 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_080 ( const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "080");

// Flight Level in Binary Representation
void asterix_048_read_090 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_090 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_090 ( const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "090");

// Mode-C Code and Confidence Indicator
void asterix_048_read_100 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_100 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_100 ( const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "100");
// Height Measured by 3D Radar
void asterix_048_read_110 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_110 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_110( const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "110"); 
// Radial Doppler Speed
bool asterix_048_read_120 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_120 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_120 ( const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "120");
// Radar Plot Characteristics
bool asterix_048_read_130 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_130 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_130 ( const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "130");
// Time of Day
void asterix_048_read_140 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_140 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_140( const uint8_t * const field,std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "140");
// Track Number
void asterix_048_read_161 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_161 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_161( const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "161");
// Track Status
void asterix_048_read_170 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_170 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_170 ( const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "170");
// Calculated Track Velocity in Polar Representation
void asterix_048_read_200 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_200 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_200 ( const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "200");
// Track Quality
void asterix_048_read_210 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_210 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_210 ( const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "210");
// Aircraft Address
void asterix_048_read_220 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_220 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_220 ( const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "220");
// Communications / ACAS Capability and Flight Status
void asterix_048_read_230 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_230 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_230 ( const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "230");
// Aircraft Identification
void asterix_048_read_240 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_240 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_240 ( const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "240");
// Mode S MB Data
void asterix_048_read_250 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_250 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_250( const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "250");

// Mode S MB Data with Age
void asterix_048_read_250_with_age (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_250_with_age (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_250_with_age(const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "304");
// ACAS Resolution Advisory Report
void asterix_048_read_260 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_260 (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_260 ( const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "260");

// User Text
void asterix_048_read_user_text (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_user_text (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_user_text(const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "300");

// Calculated Rate of Climb Descent
void asterix_048_read_rate_of_climb (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_rate_of_climb (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_rate_of_climb ( const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "301");

// Calculated Acceleration
void asterix_048_read_acceleration (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_acceleration (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_acceleration(const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "302");

// Mode of Movement
void asterix_048_read_mode_of_movement (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_mode_of_movement (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_mode_of_movement( const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "303");

// Target Composition
void asterix_048_read_target_composition (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_target_composition (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_target_composition ( const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "305");

// Target Detections
void asterix_048_read_target_detections (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_target_detections (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_target_detections(const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "305");

// Transponder Capability (same I017/230)
void asterix_048_read_transponder_capability (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_write_transponder_capability (RDFTargetTrack & target, unsigned char * &field);
void asterix_048_tobuffer_transponder_capability(const uint8_t * const field, std::stringstream *str_out = NULL, bool one_line = false, const std::string spec = "307");

#endif /*_RDF_ASTERIX_048_PRIVATE_H */
