#include "UF_Subfields.hh"

void sd_di0::to_stringstream(std::stringstream &out, bool one_line) const
{
    out << "IIS=" << static_cast<int>(IIS);
    out << " OVC=" << OVC;
}

void sd_di1 :: to_stringstream(std::stringstream &out, bool one_line)  const
{
    out << "IIS=" << static_cast<int>(IIS) << " ";

    if(MBS_to_String.end() != MBS_to_String.find(MBS))
        out << "MBS=" << MBS << ((MBS_to_String.find(MBS)->second).c_str()) << " ";
    else
        out << "MBS=" << MBS << "not_defined "; 

    if(MES_to_String.end() != MES_to_String.find(MES))
        out << "MES=" << MES << ((MES_to_String.find(MES)->second).c_str()) << " ";
    else
        out << "MES=" << MES << "not_defined "; 

    out << "LOS=" << LOS << " ";

    if(RSS_to_String.end() != RSS_to_String.find(RSS))
        out << "RSS=" << RSS <<  ((RSS_to_String.find(RSS)->second).c_str()) << " ";
    else
        out << "RSS=" << RSS << "not_defined ";

    out << "TMS=" << TMS << " ";
}

void sd_di2 :: to_stringstream(std::stringstream &out, bool one_line) const
{

    if(TCS_to_String.end() != TCS_to_String.find(TCS))
        out << "TCS=" << TCS << ((TCS_to_String.find(TCS)->second).c_str()) << " ";
    else
        out << "TCS=" << TCS << "not_defined ";

    if(RCS_to_String.end() != RCS_to_String.find(RCS))
        out << "RCS=" << RCS << ((RCS_to_String.find(RCS)->second).c_str()) << " ";
    else
        out << "RCS=" << RCS << "not_defined " ;

    if(SAS_to_String.end() != SAS_to_String.find(SAS))
        out << "SAS=" << SAS << ((SAS_to_String.find(SAS)->second).c_str()) << " ";
    else
        out << "SAS=" << SAS <<"not_defined ";
}

void sd_di3 :: to_stringstream(std::stringstream &out, bool one_line) const
{

    out << std::dec << "SIS=" << SIS;          //SI Code
    out << " LSS=" << LSS;          //multisite lockout
    out << " RRS=" << RRS;          //BDS2 code
    out << " OVC=" << OVC << " ";

}

void sd_di7 :: to_stringstream(std::stringstream &out, bool one_line) const {
    out << "IIS=" << static_cast<int>(IIS);


    out << " RRS_hex=" << std::hex << 
       static_cast<int>(RRS) << std::dec;

    out << " LOS=" << static_cast<int>(LOS);
    out << " TMS=" << static_cast<int>(TMS);
    out << " OVC=" << static_cast<int>(OVC) << " ";
}

