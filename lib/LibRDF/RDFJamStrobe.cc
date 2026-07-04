#include "RDFJamStrobe.hh"

RDFJamStrobe::RDFJamStrobe ()
{
	init ();
}

RDFJamStrobe::~RDFJamStrobe ()
{
	erase ();
}

RDFJamStrobe::RDFJamStrobe (const RDFJamStrobe & jam)
{
	init ();
	assign (jam);
}

const RDFJamStrobe & RDFJamStrobe::operator = (RDFJamStrobe & jam)
{
	erase ();
	assign (jam);
	return jam;
}

void RDFJamStrobe::init ()
{
	StrobeLines = 0;
	X = 0;
	Y = 0;
	Azimuth = 0;
	Angle = 0;
	Amplitude = 0;
}
	
void RDFJamStrobe::erase ()
{
}

void RDFJamStrobe::assign (const RDFJamStrobe & jam)
{
	StrobeLines = jam.StrobeLines;
	X = jam.X;
	Y = jam.Y;
	Azimuth = jam.Azimuth;
	Angle = jam.Angle;
	Amplitude = jam.Amplitude;	
}

