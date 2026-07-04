
#include "RDFCommandTarget.hh"

void RDFCommandFormat::erase ()
{
    CommandNumber = 0;
    Azimuth = 0;
    Range = 0;
    TTGMS = 0;
    Datagram5 = 0;
    Datagram6 = 0;
    Datagram7 = 0;
    Datagram8 = 0;
    Datagram9 = 0;
    Datagram10 = 0;
    Datagram11 = 0;
}

int TargetTrack_to_CommandFormat (deque<RDFTargetTrack> &track_list, short CommandNumber, 
	RDFCommandFormat * data)
{

    
   RDFCommandFormat RDFCommand;
   RDFCommand.erase(); 

	// seznam je prazdny, neni co zpracovavat
	if (track_list.size () == 0) return 0;
	
	if (track_list.size () > 0)
	{
		RDFTargetTrack track = track_list.front ();
				
		// aircraft address - jen kdyz je s-mode cislo
		if (track.AircraftAddress.Present)
		{

             RDFCommand.Datagram8 = ((track.AircraftAddress.Address <<8) & 0xff00);
             RDFCommand.Datagram7 = ((track.AircraftAddress.Address >>8) & 0xffff);    
		   if (track.Polar)
		  {
			unsigned short rho = (int)  (track.Polar->Rho * 256 / 1852);
			unsigned short theta = (int) (track.Polar->Theta * 0x10000 / (2 * 3.14159265));
			RDFCommand.Azimuth = theta;
			RDFCommand.Range = rho;
		  }
		   else printf("Polar coordinates not present");
		 }  

		// command number
		{
			RDFCommand.CommandNumber = CommandNumber<<4;
		}
		
		memcpy (data, &RDFCommand, sizeof(RDFCommand));
		 // odstranime track z fronty
		track_list.pop_front ();
	}

	return 1;
}


int TargetTrack_to_Command (RDFTargetTrack track, short CommandNumber, RDFCommandFormat * data)
{
    
   RDFCommandFormat RDFCommand;
   RDFCommand.erase(); 
		// aircraft address - jen kdyz je s-mode cislo
		if (track.AircraftAddress.Present)
		{
             RDFCommand.Datagram8 = ((track.AircraftAddress.Address <<8) & 0xff00);
             RDFCommand.Datagram7 = ((track.AircraftAddress.Address >>8) & 0xffff);    
		   if (track.Polar)
		  {
			unsigned short rho = (int)  (track.Polar->Rho * 256 / 1852);
			unsigned short theta = (int) (track.Polar->Theta * 0x10000 / (2 * 3.14159265));
			RDFCommand.Azimuth = theta;
			RDFCommand.Range = rho;
		  }
		  else {printf("Polar coordinates not present");
		         return 0; 
		        } 
		 } 
		 else { printf("Aircraft Address not present");
		        return 0;
		      }    

		// command number
		{
			RDFCommand.CommandNumber = CommandNumber<<4;
		}
		
		memcpy (data, &RDFCommand, sizeof(RDFCommand));

	return 1;
}

