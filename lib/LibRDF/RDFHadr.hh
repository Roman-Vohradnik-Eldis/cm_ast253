#ifndef _C_HADR_H
#define _C_HADR_H

#include "RDFHeader.hh"
#include "RDFJamStrobe.hh"
#include "RDFTargetPlot.hh"
#include "RDFTargetTrack.hh"

bool Hadr_to_TargetTrack (const void * data, int size, 
	deque<RDFTargetTrack> & track_list);
bool Hadr_to_TargetPlot (const void * data, int size, 
	deque<RDFTargetPlot> & plot_list, deque<RDFJamStrobe> & jam_list);

#endif /* _C_HADR_H */
