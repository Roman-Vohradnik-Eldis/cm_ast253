#ifndef _C_MARTELLO_H
#define _C_MARTELLO_H

#include "RDFHeader.hh"

#include "RDFTargetPlot.hh"
#include "RDFRadarService.hh"

bool Martello_to_TargetPlot (const void * data, int size, 
	deque<RDFTargetPlot> & plot_list, deque<RDFRadarService> & service_list);
int TargetPlot_to_Martello (RDFTargetPlot & plot, void * data, int maxsize);
int RadarService_to_Martello (RDFRadarService & service, void * data, int maxsize);

#endif /* _C_MARTELLO_H */
