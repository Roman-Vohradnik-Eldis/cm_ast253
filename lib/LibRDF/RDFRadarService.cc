#include "RDFRadarService.hh"
#include "CMSDebug.hh"

RDFSystemConfigurationAndStatus::RDFSystemConfigurationAndStatus ()
{
    COMPresent = false;
    COM = 0;
    PSRPresent = false;
    PSR = 0;
    SSRPresent = false;
    SSR = 0;
    MDSPresent = false;
    MDS = 0;
   MDS_1 = 0;
}

RDFSystemConfigurationAndStatus::~RDFSystemConfigurationAndStatus ()
{
}

RDFSystemConfigurationAndStatus::
RDFSystemConfigurationAndStatus (const RDFSystemConfigurationAndStatus * status)
{
    if (!status)
    {
        INTERNAL_ERROR ("null pointer");
    }
    COMPresent = status->COMPresent;
    COM = status->COM;
    PSRPresent = status->PSRPresent;
    PSR = status->PSR;
    SSRPresent = status->SSRPresent;
    SSR = status->SSR;
    MDSPresent = status->MDSPresent;
    MDS = status->MDS;
   MDS_1 = status->MDS_1;
}

RDFCountValues::RDFCountValues ()
{
    NoDetection = 0;
    PSR = 0;
    SSR = 0;
    SSR_PSR = 0;
    AllCall = 0;
    RollCall = 0;
    AllCall_PSR = 0;
    RollCall_PSR = 0;
}

RDFCountValues::~RDFCountValues ()
{
}

RDFCountValues::RDFCountValues (const RDFCountValues * values)
{
    if (!values)
    {
        INTERNAL_ERROR ("null pointer");
    }
    NoDetection = values->NoDetection;
    PSR = values->PSR;
    SSR = values->SSR;
    SSR_PSR = values->SSR_PSR;
    AllCall = values->AllCall;
    RollCall = values->AllCall_PSR;
    AllCall_PSR = values->AllCall_PSR;
    RollCall_PSR = values->RollCall_PSR;
}

RDFCollimationError::RDFCollimationError ()
{
    Range = 0;
    Azimuth = 0;
}

RDFCollimationError::~RDFCollimationError ()
{
}

RDFCollimationError::RDFCollimationError (const RDFCollimationError * error)
{
    if (!error)
    {
        INTERNAL_ERROR ("null pointer");
    }
    Range = error->Range;
    Azimuth = error->Azimuth;
}

RDFAntennaRotation::RDFAntennaRotation ()
{
    Speed = 0;
}

RDFAntennaRotation::~RDFAntennaRotation ()
{
}

RDFAntennaRotation::RDFAntennaRotation (const RDFAntennaRotation * rotation)
{
    if (!rotation)
    {
        INTERNAL_ERROR ("null pointer");
    }
    Speed = rotation->Speed;
}

RDFRadarService::RDFRadarService ()
{
    init ();
}

RDFRadarService::~RDFRadarService ()
{
    erase ();
}

RDFRadarService::RDFRadarService (const RDFRadarService & service)
{
    init ();
    assign (service);
}

const RDFRadarService & RDFRadarService::operator = (const RDFRadarService & service)
{
    erase ();
    assign (service);
    return service;
}

void RDFRadarService::init ()
{
    Time.init ();

    SAC = 0;
    SIC = 0;
   DataFilter = 0;

    Type = RADAR_SERVICE_TYPE_UNKNOWN;
    Sector = -1.0;

    SystemProcessingMode = NULL;
    AntennaRotation = NULL;
    Position3D = NULL;
    SystemConfigurationAndStatus = NULL;
    CountValues = NULL;
    CollimationError = NULL;
}

void RDFRadarService::erase ()
{
    Time.init ();

    SAC = 0;
    SIC = 0;
   DataFilter = 0;

    Type = RADAR_SERVICE_TYPE_UNKNOWN;
    Sector = -1.0;

    if (AntennaRotation)
    {
        delete AntennaRotation;
        AntennaRotation = NULL;
    }
    if (Position3D)
    {
        delete Position3D;
        Position3D = NULL;
    }
    if (SystemConfigurationAndStatus)
    {
        delete SystemConfigurationAndStatus;
        SystemConfigurationAndStatus = NULL;
    }
    if (SystemProcessingMode)
    {
        delete SystemProcessingMode;
        SystemProcessingMode = NULL;
    }
    if (CountValues)
    {
        delete CountValues;
        CountValues = NULL;
    }
    if (CollimationError)
    {
        delete CollimationError;
        CollimationError = NULL;
    }
    UserText.erase ();
}

void RDFRadarService::assign (const RDFRadarService & service)
{
    Time = service.Time;
   DataFilter = service.DataFilter;

    SAC = service.SAC;
    SIC = service.SIC;

    Type = service.Type;
    Sector = service.Sector;
    if (service.SystemProcessingMode)
    {
        SystemProcessingMode =
            new RDFSystemProcessingMode (service.SystemProcessingMode);
    }
    if (service.AntennaRotation)
    {
        AntennaRotation = new RDFAntennaRotation (service.AntennaRotation);
    }
    if (service.Position3D)
    {
        Position3D = new RDFPosition3D (service.Position3D);
    }
    if (service.SystemConfigurationAndStatus)
    {
        SystemConfigurationAndStatus =
            new RDFSystemConfigurationAndStatus (service.SystemConfigurationAndStatus);
    }
    if (service.CountValues)
    {
        CountValues = new RDFCountValues (service.CountValues);
    }
    if (service.CollimationError)
    {
        CollimationError = new RDFCollimationError (service.CollimationError);
    }

    UserText = service.UserText;
}

   /**
    *  @func Metoda prevede radar service na string.
    *  @param vystup vystupni znakovy stream do ktereho se vepisou
    *                jednotlive datove pole
    *  @param one_line pokud je nastaveno na 1 nebude se do znakoveho
    *                  proudu vkladat `\n', v tom pripade budou polozky
    *                  oddeleny oddelovacem ';'
    */
void RDFRadarService::to_stringstream (std::stringstream & vystup, bool one_line) {

#ifndef NEW_LINE
#define NEW_LINE(st,cnt,c) \
   if((void *)&st != NULL) {\
      if(one_line == false) { \
         st << "\n";          \
         int i = 0;           \
         while(i++ < (int)cnt)\
            st << (c);        \
                              \
       } else {               \
         st << ';';           \
       }\
   }
#endif

    vystup << std::fixed << std::setprecision (3);

    /*SIC a SAC */
    vystup << "010 [sac=" << static_cast < int >(this->SAC)
        << " sic=" << static_cast < int >(this->SIC) << "]";
    NEW_LINE(vystup, 0, '\0')

        /*message type */
   switch (this->Type)
    {
        case RADAR_SERVICE_TYPE_NORTH:
            vystup << "000 [type=north_marker]";
            break;
        case RADAR_SERVICE_TYPE_SECTOR:
            vystup << "000 [type=sector_crossing]";
            break;
        case RADAR_SERVICE_GEO_FILTER:
            vystup << "000 [type=geographical_filtering]";
            break;
        case RADAR_SERVICE_JAMMING_STROBE:
            vystup << "000 [type=jamming_strobe]";
            break;
        default:
            vystup << "000 [unknown type=" << this->Type << "]";
            break;
    }

    NEW_LINE(vystup, 0, '\0')

   uint64_t time = Time.Time;

   time = floor(((double)time / 1000.0) * 128) + 1;

   vystup << "030 [time=" << time << " ";

   uint64_t mod = time % 128;
   time = time / 128;


   vystup << std::setw(2)  << std::setfill('0')  << time / 3600 //h
      << ":" << std::setw(2)  << std::setfill('0')
      << ((time  % 3600) / 60) << ":" << std::setw(2) //m
      << std::setfill('0') << ((time % 3600) % 60) //s
      << "." << std::setw(3) << int(1000 * mod * (1.0/128.0))//ms
      << "]";

    NEW_LINE(vystup, 0, '\0')

        /*Sector number */
      if(Type != RADAR_SERVICE_TYPE_NORTH) {
         double sec_nod = (Sector * 256.0) / (2 * M_PI);
         int sec_no = static_cast<int>(round(sec_nod));
         sec_no = sec_no >> 3 & 0x1f;
         vystup.precision(2);
         vystup << "020 [sector " << sec_no << " (azimuth "
            << std::fixed << this->Sector * RAD_TO_DEG << " deg)]";
        NEW_LINE(vystup, 0, '\0')
      }
        /*Antenna Rotation Speed */
        if (this->AntennaRotation)
    {
        vystup << "041 [antenna rp="
         << " " << this->AntennaRotation->Speed << " s]";
    NEW_LINE(vystup, 0, '\0')
}

    /*System configuration and statusi - begin */
    if (this->SystemConfigurationAndStatus)
    {
        vystup << "050 [present:";
        if (this->SystemConfigurationAndStatus->COMPresent)
            vystup << " COM";
        if (this->SystemConfigurationAndStatus->PSRPresent)
            vystup << " PSR";
        if (this->SystemConfigurationAndStatus->SSRPresent)
            vystup << " SSR";
        if (this->SystemConfigurationAndStatus->MDSPresent)
            vystup << " MDS";
        vystup << "]";
        NEW_LINE(vystup, 0, '\0')

 if (this->SystemConfigurationAndStatus->COMPresent)
        {
            const unsigned char COM = this->SystemConfigurationAndStatus->COM;

            if (one_line == false)
                vystup << "    [COM: ";
            else
                vystup << "[COM: ";

            if (COM & 0x80)
                vystup << "NOGO";
            else
                vystup << "-";

         if(COM & 0x40)
         {
            vystup << " RDPC-2";
         } else {
            vystup << " RDPC-1";
         }

            if (COM & 0x20)
                vystup << " RDPR";
            else
            vystup << " -";

            if (COM & 0x10)
                vystup << " RDP_OVL";
            else
                vystup << " -";

            if (COM & 0x08)
                vystup << " XMT_OVL";
            else
                vystup << " -";

            if (COM & 0x04)
                vystup << " MSC_DISC";
            else
                vystup << " MSC_CONN";

            if (COM & 0x02)
                vystup << " TSV_INV]";
            else
                vystup << " TSV_VAL]";

        NEW_LINE(vystup, 0, '\0')
}
         /*PSR*/ if (this->SystemConfigurationAndStatus->PSRPresent)
        {
            const unsigned char PSR = this->SystemConfigurationAndStatus->PSR;

            if (one_line == false)
            {
                vystup << "    [PSR: ANT_" << (PSR >> 7) + 1;
            } else {
                vystup << "[PSR: ANT_" << (PSR >> 7) + 1;
            }

            switch (PSR & 0x60)
            {
                case 0:
                    vystup << " CH_NONE";
                    break;
                case 0x20:
                    vystup << " CH_A";
                    break;
                case 0x40:
                    vystup << " CH_B";
                    break;
                case 0x60:
                    vystup << " CH_DIV";
                    break;
            }

            if (PSR & 0x10)
            {
                vystup << " OVL";
            } else {
                vystup << " -";
            }

            if (PSR & 0x8)
            {
                vystup << " MSC_DISC]";
            } else {
                vystup << " MSC_CONN]";
            }

        NEW_LINE(vystup, 0, '\0')
}
         /*SSR*/ if (this->SystemConfigurationAndStatus->SSRPresent)
        {
            const unsigned char SSR = this->SystemConfigurationAndStatus->SSR;
         if(one_line == false)
         {
            vystup << "    [SSR: ANT_" << (SSR >> 7) + 1;
         } else {
            vystup << "[SSR: ANT_" << (SSR >> 7) + 1;
         }

            switch (SSR & 0x60)
            {
                case 0:
                    vystup << " CH_NONE";
                    break;
                case 0x20:
                    vystup << " CH_A";
                    break;
                case 0x40:
                    vystup << " CH_B";
                    break;
                case 0x60:
                    vystup << " CH_INV";
                    break;
            }

            if (SSR & 0x10)
            {
                vystup << " OVL";
            } else {
                vystup << " -";
            }

            if (SSR & 0x8)
            {
                vystup << " MSC_DISC]";
            } else {
                vystup << " MSC_CONN]";
            }

        NEW_LINE(vystup, 0, '\0')
}
         /*MDS*/
      if (this->SystemConfigurationAndStatus->MDSPresent)
        {
            const unsigned char MDS = this->SystemConfigurationAndStatus->MDS;
            const unsigned char MDS_1 = this->SystemConfigurationAndStatus->MDS_1;

         if(one_line == false)
         {
            vystup << "    [MDS: ANT_" << (MDS >> 7) + 1;
         } else {
            vystup << "[MDS: ANT_" << (MDS >> 7) + 1;
         }

            switch (MDS & 0x60)
            {
                case 0:
                    vystup << " CH_NONE";
                    break;
                case 0x20:
                    vystup << " CH_A";
                    break;
                case 0x40:
                    vystup << " CH_B";
                    break;
                case 0x60:
                    vystup << " CH_INV";
                    break;
            }

            if (MDS & 0x10)
            {
                vystup << " OVL";
            } else {
                vystup << " -";
            }

            if (MDS & 0x8)
            {
                vystup << " MSC_DISC";
            } else {
                vystup << " MSC_CONN";
            }

            if (MDS & 0x4)
            {
                vystup << " SCF_B";
            } else {
                vystup << " SCF_A";
            }

            if (MDS & 0x2)
            {
                vystup << " DLF_B";
            } else {
                vystup << " DLF_A";
            }

            if (MDS & 0x1)
            {
                vystup << " OVL_SCF";
            } else {
                vystup << " -";
            }

            if (MDS_1 & 0x80)
            {
                vystup << " OVL_DLF";
            } else {
                vystup << " -";
            }

         if(MDS_1 & 0x1)
         {
            vystup << " CH_A]";
         } else {
            vystup << " CH_B]";
         }

        NEW_LINE(vystup, 0, '\0')
}
    }
    /*System configuration and statusi - end */

    /*System processing mode - begin */
    if (this->SystemProcessingMode)
    {

        vystup << "060 [present:";
        if (this->SystemProcessingMode->COM)
            vystup << " COM";
        if (this->SystemProcessingMode->PSR)
            vystup << " PSR";
        if (this->SystemProcessingMode->SSR)
            vystup << " SSR";
        if (this->SystemProcessingMode->MDS)
            vystup << " MDS";
        vystup << "]";

        NEW_LINE(vystup, 0, '\0')

            /* system processing mode COM */
            if (this->SystemProcessingMode->COMPresent)
        {

            const unsigned char COM = this->SystemProcessingMode->COM;

            if (one_line == false)
                vystup << "    [COM: RED-RDP=" << ((COM >> 4) & 0x7)
                    << " RED-XMT=" << ((COM >> 1) & 0x7) << "]";
            else
                vystup << "[COM: RED-RDP=" << ((COM >> 4) & 0x7)
                    << " RED-XMT=" << ((COM >> 1) & 0x7) << "]";
        }
        /* system processing mode PSR */
        if (this->SystemProcessingMode->PSRPresent)
        {
            const unsigned char PSR = this->SystemProcessingMode->PSR;

            if (one_line == false)
                vystup << "    [PSR: ";
            else
                vystup << "[PSR: ";

            if (PSR & 0x80)
                vystup << "CIR";
            else
                vystup << "LIN";

            vystup << " RED-RAD=" << ((PSR >> 4) & 0x7)
                << " STC-MAP-" << (((PSR >> 2) & 0x3) + 1) << "]";

        NEW_LINE(vystup, 0, '\0')
}
        /* system processing mode SSR */
        if (this->SystemProcessingMode->SSRPresent)
        {
            const unsigned char SSR = this->SystemProcessingMode->SSR;

            if (one_line == false)
                vystup << "    [SSR: RED-RAD=" << ((SSR >> 5) & 0x7) << "]\n";
            else
                vystup << "[SSR: RED-RAD=" << ((SSR >> 5) & 0x7) << "]";
        }
        /* system processing mode MDS */
        if (this->SystemProcessingMode->MDSPresent)
        {
            const unsigned char MDS = this->SystemProcessingMode->MDS;
            if (one_line == false)
                vystup << "    [SSR: RED-RAD=" << ((MDS >> 5) & 0x7);
            else
                vystup << "[SSR: RED-RAD=" << ((MDS >> 5) & 0x7);

            if (MDS & 0x10)
                vystup << " NOT-AUTONOMOUS]";
            else
                vystup << " AUTONOMOUS]";

        NEW_LINE(vystup, 0, '\0');
}
    }
    /*system proccesing mode - end */

    /* message count values - begin */
    if (this->CountValues)
    {
        vystup << "070 [rep" << CountValues->Rep << "]";
        NEW_LINE (vystup, 4, ' ')
            vystup << "[NoDetection=" << CountValues->NoDetection << "]";
        NEW_LINE (vystup, 4, ' ') vystup << "[PSR=" << CountValues->PSR << "]";
        NEW_LINE (vystup, 4, ' ') vystup << "[SSR=" << CountValues->SSR << "]";
        NEW_LINE (vystup, 4, ' ') vystup << "[SSR_PSR=" << CountValues->SSR_PSR << "]";
        NEW_LINE (vystup, 4, ' ') vystup << "[AllCall=" << CountValues->AllCall << "]";
        NEW_LINE (vystup, 4, ' ')
            vystup << "[RollCall=" << CountValues->RollCall << "]";
        NEW_LINE (vystup, 4, ' ')
            vystup << "[AllCall_PSR=" << CountValues->AllCall_PSR << "]";
        NEW_LINE (vystup, 4, ' ')
            vystup << "[RollCall_PSR=" << CountValues->RollCall_PSR << "]";
        NEW_LINE (vystup, 4, ' ')
            vystup << "[filter_Weather=" << CountValues->FilterWeather << "]";
        NEW_LINE (vystup, 4, ' ')
            vystup << "filter_Jamming=" << CountValues->FilterJamming << "]";
        NEW_LINE (vystup, 4, ' ')
            vystup << "[filter_PSR=" << CountValues->FilterPSR << "]";
        NEW_LINE (vystup, 4, ' ')
            vystup << "[filter_SSR=" << CountValues->FilterSSR << "]";
        NEW_LINE (vystup, 4, ' ')
            vystup << "[filter_SSR_PSR=" << CountValues->FilterSSR_PSR << "]";
        NEW_LINE (vystup, 4, ' ')
            vystup << "[filter_Enh=" << CountValues->FilterEnh << "]";
        NEW_LINE (vystup, 4, ' ')
            vystup << "[filter_PSR_Enh=" << CountValues->FilterEnhPSR << "]";
        NEW_LINE (vystup, 4, ' ')
            vystup << "[filter_not_API=" << CountValues->FilterNotAPI << "]";
        NEW_LINE (vystup, 4, ' ')
            vystup << "[filter_PSR_Enh_SSR=" << CountValues->FilterEnhPSR_SSR << "]";
    NEW_LINE (vystup, 4, ' ')}
    /*system proccesing mode - end */

    /*generic polar window */

    /*data filter */
    if (DataFilter)
    {
        switch (DataFilter)
        {
         case 0://not set
            break;
            case 1:
                vystup << "110 [filter for weather data]";
                break;
            case 2:
                vystup << "110 [filter for jamming strobe]";
                break;
            case 3:
                vystup << "110 [filter for PSR data]";
                break;
            case 4:
                vystup << "110 [filter for SSR/Mode-S data]";
                break;
            case 5:
                vystup << "110 [filter for SSR/Mode-S+PSR data]";
                break;
            case 6:
                vystup << "110 [enhanced surveillance data]";
                break;
            case 7:
                vystup << "110 [filter for PSR+ESD]";
                break;
            case 8:
                vystup << "110 [filter for PSR+ESD+SSR/Mode-S data not in API]";
                break;
            case 9:
                vystup << "110 [filter for PSR+ESD+all SSR/Mode-S data]";
                break;
            default:
                vystup << "110 [invalid value " << static_cast<int>(DataFilter) << "]";
                break;
        }
    NEW_LINE(vystup, 0, '\0')
}
    /*data filter - end */

    /*3D-position of data source */
    if (Position3D)
    {
        int deg, min, sec, way;
        vystup << "120 [height=" << static_cast<int>(Position3D->Height)
         << "m (lat=";
        char flag = '\0';

        rdf_rad2dms (Position3D->Latitude, way, deg, min, sec);

        if (way == 1)
            flag = 'N';
        else
            flag = 'S';

        vystup << setfill ('0') << std::setw (3) << deg << " "
            << std::setw (2) << min << " " << std::setw (2)
            << (int) trunc (sec) << "." << std::setw (2)
            << (int) trunc (sec * 100) % 100 << " " << flag << " lon=";

        rdf_rad2dms (Position3D->Longitude, way, deg, min, sec);

        if (way == 1)
            flag = 'E';
        else
            flag = 'W';

        vystup << setfill ('0') << std::setw (3) << deg << " "
            << std::setw (2) << min << " " << std::setw (2)
            << (int) trunc (sec) << "." << std::setw (2)
            << (int) trunc (sec * 100) % 100 << " " << flag;

        vystup << ")]";
    NEW_LINE(vystup, 0, '\0')
}
    /*3D position data source - end */
NEW_LINE(vystup, 0, '\0')
}
