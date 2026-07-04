#ifndef _RDF_ASTERIX_250_PRIVATE_H
#define _RDF_ASTERIX_250_PRIVATE_H

// Time Of Day
void asterix_250_read_201 (RDFSifDetections & sif, unsigned char *&field);
void asterix_250_write_201 (RDFSifDetections & sif, unsigned char *&field);
void asterix_250_tobuffer_201 (std::stringstream & out, unsigned char *&field,
	bool one_line = false);

// Reply Report Descriptor
void asterix_250_read_202 (RDFSifDetections & sif, unsigned char *&field);
void asterix_250_write_202 (RDFSifDetections & sif, unsigned char *&field);
void asterix_250_tobuffer_202 (std::stringstream & out, unsigned char *&field,
	bool one_line = false);

// Interrogation Mode
void asterix_250_read_203 (RDFSifDetections & sif, unsigned char *&field);
void asterix_250_write_203 (RDFSifDetections & sif, unsigned char *&field);
void asterix_250_tobuffer_203 (std::stringstream & out, unsigned char *&field,
	bool one_line = false);

// Sweep Number
void asterix_250_read_204 (RDFSifDetections & sif, unsigned char *&field);
void asterix_250_write_204 (RDFSifDetections & sif, unsigned char *&field);
void asterix_250_tobuffer_204 (std::stringstream & out, unsigned char *&field,
	bool one_line = false);

// Code
void asterix_250_read_205 (RDFSifDetections & sif, unsigned char *&field);
void asterix_250_write_205 (RDFSifDetections & sif, unsigned char *&field);
void asterix_250_tobuffer_205 (std::stringstream & out, unsigned char *&field,
	bool one_line = false);

// Ref Amplitude
void asterix_250_read_206 (RDFSifDetections & sif, unsigned char *&field);
void asterix_250_write_206 (RDFSifDetections & sif, unsigned char *&field);
void asterix_250_tobuffer_206 (std::stringstream & out, unsigned char *field,
	bool one_line = false);

// Measured Position in Slant Range Coordinates
void asterix_250_read_207 (RDFSifDetections & sif, unsigned char *&field);
void asterix_250_write_207 (RDFSifDetections & sif, unsigned char *&field);
void asterix_250_tobuffer_207 (std::stringstream & out, unsigned char *field,
	bool one_line = false);

// SUM Pulse
void asterix_250_read_208 (RDFSifDetections & sif, unsigned char *&field);
void asterix_250_write_208 (RDFSifDetections & sif, unsigned char *&field);
void asterix_250_tobuffer_208 (std::stringstream & out, unsigned char *field,
	bool one_line = false);
void
asterix_250_to_vect_208 (std::vector<std::string> & out, unsigned char *field, bool one_line = false);

// DIFF Pulse
void asterix_250_read_209 (RDFSifDetections & sif, unsigned char *&field);
void asterix_250_write_209 (RDFSifDetections & sif, unsigned char *&field);
void
asterix_250_to_vect_209 (std::vector<std::string> & out, unsigned char *field, bool one_line = false);

// SUM + DIFF Pulse
void asterix_250_to_buffer_208_209(std::stringstream &out, const std::vector<std::string> sum_diff[2], bool one_line = false);

// Sample Diff
void asterix_250_read_210(RDFSifDetections & sif, unsigned char *&field);
void asterix_250_write_210 (RDFSifDetections & sif, unsigned char *&field);
void asterix_250_tobuffer_210 (std::stringstream & out, unsigned char *field,
	bool one_line = false);
// Sample DIff
void asterix_250_read_211 (RDFSifDetections & sif, unsigned char *&field);
void asterix_250_write_211 (RDFSifDetections & sif, unsigned char *&field);
void asterix_250_tobuffer_211 (std::stringstream & out, unsigned char *field,
	bool one_line = false);
// Sample Phase
void asterix_250_read_212 (RDFSifDetections & sif, unsigned char *&field);
void asterix_250_write_212 (RDFSifDetections & sif, unsigned char *&field);
void asterix_250_tobuffer_212 (std::stringstream & out, unsigned char *field,
	bool one_line = false);

// Time Of Day
void asterix_250_read_301 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_write_301 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_tobuffer_301 (std::stringstream & out, unsigned char *field,
	bool one_line = false);

// Reply Report Descriptor
void asterix_250_read_302 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_write_302 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_tobuffer_302 (std::stringstream & out, unsigned char *field,
	bool one_line = false);

// DF Raw
void asterix_250_read_303 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_write_303 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_tobuffer_303 (std::stringstream & out, unsigned char *field,
	bool one_line = false);

// DF Confidence
void asterix_250_read_304 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_write_304 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_tobuffer_304 (std::stringstream & out, unsigned char *field,
	bool one_line = false);

// DF Corrected
void asterix_250_read_305 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_write_305 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_tobuffer_305 (std::stringstream & out, unsigned char *field,
	bool one_line = false);

// Measured Position in Slant Polar Coordinates
void asterix_250_read_308 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_write_308 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_tobuffer_308 (std::stringstream & out, unsigned char *field,
	bool one_line = false);

// Measured Amplitude
void asterix_250_read_309 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_write_309 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_tobuffer_309 (std::stringstream & out, unsigned char *field,
	bool one_line = false);

// Interrogation Mode
void asterix_250_read_310 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_write_310 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_tobuffer_310 (std::stringstream & out, unsigned char *field,
	bool one_line = false);

// Sweep Number
void asterix_250_read_311 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_write_311 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_tobuffer_311 (std::stringstream & out, unsigned char *field,
	bool one_line = false);
// Command Number
void asterix_250_read_312 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_write_312 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_tobuffer_312(std::stringstream & out, unsigned char *field,
	bool one_line = false);

// Track Number
void asterix_250_read_313 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_write_313 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_tobuffer_313(std::stringstream & out, unsigned char *field,
	bool one_line = false);

// BDS
void asterix_250_read_314 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_write_314 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_tobuffer_314(std::stringstream & out, unsigned char *field,
	bool one_line = false);

// Uplink Data
void asterix_250_read_315 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_write_315 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_tobuffer_315(std::stringstream & out, unsigned char *field,
	bool one_line = false);

// Datalink Control
void asterix_250_read_316 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_write_316 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_tobuffer_316(std::stringstream & out, unsigned char *field,
	bool one_line = false);

//Sweep Length
void asterix_250_read_317 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_write_317 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_tobuffer_317(std::stringstream & out, unsigned char *field,
	bool one_line = false);

//Interrogation Delay
void asterix_250_read_318 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_write_318 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_tobuffer_318(std::stringstream & out, unsigned char *field,
	bool one_line = false);

//Sample Sum
void asterix_250_read_319(RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_write_319 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_tobuffer_319(std::stringstream & out, unsigned char *field,
	bool one_line = false);

//Sample Diff
void asterix_250_read_320(RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_write_320 (RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_tobuffer_320(std::stringstream & out, unsigned char *field,
	bool one_line = false);

//Sample Phase
void asterix_250_read_321(RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_write_321(RDFModeSDetections & modes, unsigned char *&field);
void asterix_250_tobuffer_321(std::stringstream & out, unsigned char *field,
	bool one_line = false);

void asterix_250_read_136(RDFTargetPlot & plot, unsigned char *&field);
void asterix_250_write_136(RDFTargetPlot & plot, unsigned char *&field);
void asterix_250_tobuffer_136(std::stringstream & out, unsigned char *field,
	bool one_line = false);

void asterix_250_read_137(RDFTargetPlot & plot, unsigned char *&field);
void asterix_250_write_137(RDFTargetPlot & plot, unsigned char *&field);
void asterix_250_tobuffer_137(std::stringstream & out, unsigned char *field,
	bool one_line = false);

void asterix_250_read_138(RDFTargetPlot & plot, unsigned char *&field);
void asterix_250_write_138(RDFTargetPlot & plot, unsigned char *&field);
void asterix_250_tobuffer_138(std::stringstream & out, unsigned char *field,
	bool one_line = false);

void asterix_250_read_139(RDFTargetPlot & plot, unsigned char *&field);
void asterix_250_write_139(RDFTargetPlot & plot, unsigned char *&field);
void asterix_250_tobuffer_139(std::stringstream & out, unsigned char *field,
	bool one_line = false);

void asterix_250_read_501(RDFUvdDetections & uvd, unsigned char *&field);
void asterix_250_tobuffer_501(std::stringstream &out, unsigned char *field, bool one_line = false);
void asterix_250_write_501(RDFUvdDetections & uvd, unsigned char *&field);

void asterix_250_read_502(RDFUvdDetections & uvd, unsigned char *&field);
void asterix_250_tobuffer_502(std::stringstream &out, unsigned char *field, bool one_line = false);
void asterix_250_write_502(RDFUvdDetections & uvd, unsigned char *&field);

void asterix_250_read_503(RDFUvdDetections & uvd, unsigned char *&field);
void asterix_250_tobuffer_503(std::stringstream &out, unsigned char *field, bool one_line = false);
void asterix_250_write_503(RDFUvdDetections & uvd, unsigned char *&field);

void asterix_250_read_504(RDFUvdDetections & uvd, unsigned char *&field);
void asterix_250_tobuffer_504(std::stringstream &out, unsigned char *field, bool one_line = false);
void asterix_250_write_504(RDFUvdDetections & uvd, unsigned char *&field);

void asterix_250_read_505(RDFUvdDetections & uvd, unsigned char *&field);
void asterix_250_tobuffer_505(std::stringstream &out, unsigned char *field, bool one_line = false);
void asterix_250_write_505(RDFUvdDetections & uvd, unsigned char *&field);

void asterix_250_read_506(RDFUvdDetections & uvd, unsigned char *&field);
void asterix_250_tobuffer_506(std::stringstream &out, unsigned char *field, bool one_line = false);
void asterix_250_write_506(RDFUvdDetections & uvd, unsigned char *&field);

void asterix_250_read_507(RDFUvdDetections & uvd, unsigned char *&field);
void asterix_250_tobuffer_507(std::stringstream &out, unsigned char *field, bool one_line = false);
void asterix_250_write_507(RDFUvdDetections & uvd, unsigned char *&field);

void asterix_250_read_508(RDFUvdDetections & uvd, unsigned char *&field);
void asterix_250_write_508(RDFUvdDetections & uvd, unsigned char *&field);

void asterix_250_read_509(RDFUvdDetections & uvd, unsigned char *&field);
void asterix_250_write_509(RDFUvdDetections & uvd, unsigned char *&field);

void asterix_250_read_510(RDFUvdDetections & uvd, unsigned char *&field);
void asterix_250_write_510(RDFUvdDetections & uvd, unsigned char *&field);

void asterix_250_read_511(RDFUvdDetections & uvd, unsigned char *&field);
void asterix_250_write_511(RDFUvdDetections & uvd, unsigned char *&field);

void asterix_250_read_512(RDFUvdDetections & uvd, unsigned char *&field);
void asterix_250_write_512(RDFUvdDetections & uvd, unsigned char *&field);

#endif /* _RDF_ASTERIX_250_PRIVATE_H */
