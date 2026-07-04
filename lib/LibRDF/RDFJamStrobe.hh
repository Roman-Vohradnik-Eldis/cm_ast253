#ifndef _RDF_JAM_STROBE_H
#define _RDF_JAM_STROBE_H

#include "RDFHeader.hh"

struct RDFJamStrobe
{
	int StrobeLines;
	int X;
	int Y;
	double Azimuth;
	int Angle;
	int Amplitude;

	RDFJamStrobe ();
	~RDFJamStrobe ();
	RDFJamStrobe (const RDFJamStrobe & jam);
	const RDFJamStrobe & operator = (RDFJamStrobe & jam);

  protected:
	void init ();
	void erase ();
	void assign (const RDFJamStrobe & jam);
};

#endif /* _RDF_JAM_STROBE_H */
