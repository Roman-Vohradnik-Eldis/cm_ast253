#ifndef _RDF_COMMAND_TARGET_H
#define _RDF_COMMAND_TARGET_H

#include "RDFHeader.hh"
#include "RDFTargetPlot.hh"
#include "RDFTargetTrack.hh"

struct RDFCommandFormat
{
    unsigned short CommandNumber;
    unsigned short Azimuth;
    unsigned short Range;
    short TTGMS;
    short Datagram5;
    short Datagram6;
    short Datagram7;
    short Datagram8;
    short Datagram9;
    short Datagram10;
    short Datagram11;
    short TrackNumber;
    
	RDFCommandFormat () { erase (); }
	void erase ();
};


int TargetTrack_to_CommandFormat (deque<RDFTargetTrack> &track_list, short CommandNumber, RDFCommandFormat * data);
int TargetTrack_to_Command (RDFTargetTrack track, short CommandNumber, RDFCommandFormat * data);

#endif
