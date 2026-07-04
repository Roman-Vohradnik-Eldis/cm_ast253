#include "RDFTargetDetections.hh"
#include "DF_Items.hh"
#include "UF_Items.hh"

void RDFSumPulse::init ()
{
	OtherUsed = false;
	OtherMatched = false;
	AmplitudeNotMatched = false;
	Garbled = false;
	Amplitude = 0;
}

void RDFDiffPulse::init ()
{
	Signum = false;
	RSLS = false;
	Amplitude = 0;
}

void RDFSifDetections::init ()
{
	Time.init ();
	PeriodMode = 0;
	SweepNumber = 0;

	RawCodeX = false;
	RawCodeSPI = false;
	RawCodeValue = 0;

	SumSignum = false;
	SumValue = 0;
	DiffSignum = false;
	DiffValue = 0;
   PhaseValue = 0;

   SumSamples.clear();
   DiffSamples.clear();
   PhaseSamples.clear();

	Rho = 0;
	Theta = 0;
	ThetaEncoder = 0;

	for (unsigned i = 0; i < PULSES; i++)
	{
		SumPulse[i].init ();
		DiffPulse[i].init ();
	}
}

void RDFModeSDetections::init ()
{
	Time.init ();

	RollCallNotReceived = false;
	ReplyCorrected = false;
	AzimuthValid =  false;
	AzimuthRSLS = false;
    AzimuthInBeam = false;
    AzimuthDiffOk = false;

	LockoutLockedOut = false;
	InvalidBDS = false;
	TargetBDS = 0;

	SweepLengthPresent = false;
	InterrogationDelayPresent = false;

	Uplink.Data.clear ();
	DfRaw.Data.clear ();
	DfCorrected.Data.clear ();
	DfConfidence.clear ();

   SumSamples.clear();
   DiffSamples.clear();
   PhaseSamples.clear();

	Rho = 0;
	Theta = 0;
	ThetaEncoder = 0;
    AzimuthQuality = 0;

	SumSignum = false;
	DiffSignum = false;
	SumValue = 0;
	DiffValue = 0;
    PhaseValue = 0;
    DiffRSLS = false;

	PeriodType = 0;
	SweepNumber = 0;
	CommandNumber = 0;

	TrackNumber.init ();

   ExpectedBDS.init();

   DatalinkControl.init();
}

// funkce na prevod detekceS do stareho formatu
// funkce neni vyzkousena!!!!
bool RDFModeSDetections :: DF_to_Message(std::vector<uint16_t> &Message)
{
//uint8_t temp;
const float pi=3.14159265;
const uint8_t reserv_req = 1;
const uint8_t reserv_stat_req = 2;
const uint8_t reserv_closeout = 3;

df_items DFItems;
uf_items UFItems;

uint8_t dwnELMctrl_temp=0;
uint8_t upELMctrl_temp=0;
uint8_t CommBctrl_temp=0;
uint8_t CommA_TMS_temp = 0x0F;

uint16_t Datalink_control;


    Message.clear();
    Message.resize(14);
//    if (!Get_Service_Flag(temp)) return false;
//    if (temp)
  bool UsableAzimuth = (!AzimuthRSLS) && (AzimuthValid) && (AzimuthInBeam); // dle osobni konzultace s VBi 2017-02-14
//  bool UsableAzimuth = (!det->AzimuthRSLS) && (det->AzimuthValid) && (det->AzimuthInBeam) && (det->AzimuthDiffOk);

    if (RollCallNotReceived)
    {
     // dotaz bez odpovedi

        UFItems.Data_to_Items(Uplink.Data);          // rozdekodujeme Uplink
        Message[0] = CommandNumber;
        Message[1] = ThetaEncoder * 0x10000 /(2*pi);    //prevod rad na 360/65536 stupne
        Message[2] = Rho * 256 /1852;                   //prevod m na 1/256Nm
//        Message[3] = BDS2 & BDS1<<4;
        Message[3] =  (RollCallNotReceived<<15) | ((!UsableAzimuth) << 9) | (LockoutLockedOut << 8) | (UFItems.Get_Expect_BDS() & 0xFF);
        //printf("Flag=%x\n",Message[3]);
        Message[3]&=0x9FFF;

        Message[4] = UFItems.UF << 11;
        Message[5]=0;
        uint32_t _AA = UFItems.Get_AP();
        Message[6]=(_AA >> 8) & 0xffff;
        Message[7]=(_AA << 8) & 0xff00;
        Message[8]=0;
        Message[9]=0;
        Message[10]=0;
//        if(!Get_Datalink_Control(Message[11])) return false;
        Datalink_control=(CommA_TMS_temp<<8)|(dwnELMctrl_temp<<4)|(upELMctrl_temp<<2)|CommBctrl_temp;
        Message[11] = Datalink_control;
        Message[12]=0;
//        if(!Get_Track_Number(Message[13])) Message[13]=0xFFFF;
        if (TrackNumber.Present) Message[13] = TrackNumber.Number; else Message.pop_back();

    }
    else
    {
    //odpoved byla prijata
        uint32_t CRC_Rest = DfCorrected.Compute_DF_CRC_Rest();
        DFItems.Data_to_Items(DfCorrected.Data);     // rozdekodujeme Downlink
        UFItems.Data_to_Items(Uplink.Data);          // rozdekodujeme Uplink
        uint8_t _DF;
        _DF = DFItems.Get_DF();
        if (_DF==11)
        {
            Message[0] = SweepNumber;
            Message[1] = Theta * 0x10000 /(2*pi);    //prevod rad na 360/65536 stupne
            Message[2] = Rho * 256 /1852;                   //prevod m na 1/256Nm
            Message[3] =  (RollCallNotReceived<<15) | ((!UsableAzimuth) << 9) | (LockoutLockedOut << 8) | (UFItems.Get_Expect_BDS() & 0xFF);
            Message[3]&=0x9FFF;
            uint8_t _CA;
            _CA = DFItems.Get_CA();
            uint8_t _AA[3];
            if(!DFItems.Get_AA(_AA)) return false;
//            printf("DF=%x CA=%x\n",_DF,_CA);
            Message[4]= (_DF<<11) | ((_CA&0x7)<<8) | _AA[0];
            Message[5]= (_AA[1]<<8) | _AA[2];
//            uint8_t _AP_PI[3];
//            if(!Items.Get_AP_PI(_AP_PI)) return false;
//            Message[6]= (_AP_PI[2]<<8) | _AP_PI[1];
//            Message[7]= _AP_PI[0] << 8;
            Message[6]= CRC_Rest >> 8;
            Message[7]= (CRC_Rest & 0xFF) << 8;
            Message[8]=0;
            Message[9]=0;
            Message[10]=0;
//            if(!Get_Datalink_Control(Message[11])) return false;                         //Datalink control
            Datalink_control=(CommA_TMS_temp<<8)|(dwnELMctrl_temp<<4)|(upELMctrl_temp<<2)|CommBctrl_temp;
            Message[11] = Datalink_control;
            Message[12]=0;
//            if(!Get_Track_Number(Message[13])) Message[13]=0xFFFF;
            if (TrackNumber.Present) Message[13] = TrackNumber.Number; else Message.pop_back();
            return true;
        }

        if ((_DF==4) || (_DF==5) || (_DF==20) || (_DF==21))
        {
//            if(!Get_Command_Number(Message[0])) return false;
            Message[0] = CommandNumber;
//            if (!Get_Azimuth_Monopulse(Message[1])) return false;
            Message[1] = Theta * 0x10000 /(2*pi);    //prevod rad na 360/65536 stupne
//            if(!Get_Range(Message[2])) return false;
            Message[2] = Rho * 256 /1852;                   //prevod m na 1/256Nm
//            if(!Get_Reply_Flags_BDS(Message[3])) return false;
            Message[3] =  (RollCallNotReceived<<15) | (!(UsableAzimuth) << 9) | (LockoutLockedOut << 8) | ((UFItems.Get_Expect_BDS()==0xFFFF) ? 0x00 : UFItems.Get_Expect_BDS() & 0xFF);
            Message[3]&=0x9FFF;
            uint8_t _FS;
            _FS = DFItems.Get_FS();

            uint8_t _DR;
            _DR = DFItems.Get_DR();
            uint8_t _UM;
            _UM = DFItems.Get_UM();
            Message[4]=(_DF<<11) | ((_FS & 0x7)<<8) | ((_DR & 0x1F)<<3) | ((_UM & 0x3F)>>3);
            if((_DF==4) || (_DF==20))
            {
                uint16_t _AC;
                _AC = DFItems.Get_AC();
                Message[5]= ((_UM & 0x3F)<<13) | (_AC & 0x1FFF);
            }
            else
            {
                uint16_t _ID;
                _ID = DFItems.Get_ID();
                Message[5]= ((_UM & 0x3F)<<13) | (_ID & 0x1FFF);
            }
//            uint8_t _AP_PI[3];
//            if(!Items.Get_AP_PI(_AP_PI)) return false;
//            Message[6]= (_AP_PI[2]<<8) | _AP_PI[1];
//            Message[7]= _AP_PI[0] << 8;
//            Message[6]= CRC_Rest >> 8;
//            Message[7]= (CRC_Rest & 0xFF) << 8;
            uint32_t _AA = UFItems.Get_AP();
            Message[6]=(_AA >> 8) & 0xffff;
            Message[7]=(_AA << 8) & 0xff00;


            if((_DF==4) || (_DF==5))
            {
                Message[8]=0;
                Message[9]=0;
                Message[10]=0;
            }
            else
            {
                uint8_t _MB[7];
                if(!DFItems.Get_MB(_MB)) return false;
                Message[7]|= _MB[0];
                Message[8]=(_MB[1]<<8) | (_MB[2]);
                Message[9]=(_MB[3]<<8) | (_MB[4]);
                Message[10]=(_MB[5]<<8) | (_MB[6]);
            }

            //synteza datalink control
            uint8_t MBS_subfield = UFItems.Get_MBS();
            uint8_t MES_subfield = UFItems.Get_MES();
            uint8_t RSS_subfield = UFItems.Get_RSS();
            uint8_t PC_field = UFItems.Get_PC();
            CommA_TMS_temp = UFItems.Get_TMS();
            if (CommA_TMS_temp == 0xFF) CommA_TMS_temp&=0xF;

            if (MBS_subfield==MBS_Air_Init_CommB) {CommBctrl_temp=reserv_req;}
            if (MBS_subfield==MBS_Closeout_CommB) {CommBctrl_temp=reserv_closeout;}
            //definice podle PC
            if (PC_field==PC_Closeout_CommB) {CommBctrl_temp=reserv_closeout;}
            if (PC_field==PC_Uplink_Closeout) {upELMctrl_temp=reserv_closeout;}
            if (PC_field==PC_Downlink_Closeout) {dwnELMctrl_temp=reserv_closeout;}
            //definice podle MES
            if (MES_subfield==MES_Uplink_Reserv) {upELMctrl_temp=reserv_req;}
            if (MES_subfield==MES_Uplink_Closeout) {upELMctrl_temp=reserv_closeout;}
            if (MES_subfield==MES_Downlink_Reserv) {dwnELMctrl_temp=reserv_req;}
            if (MES_subfield==MES_Downlink_Closeout) {dwnELMctrl_temp=reserv_closeout;}
            if (MES_subfield==MES_Uplink_Reserv_Downlink_Closeout) {upELMctrl_temp=reserv_req; dwnELMctrl_temp=reserv_closeout;}
            if (MES_subfield==MES_Uplink_Closeout_Downlink_Reserv) {upELMctrl_temp=reserv_closeout; dwnELMctrl_temp=reserv_req;}
            if (MES_subfield==MES_Uplink_Downlink_Closeout) {upELMctrl_temp=reserv_closeout; dwnELMctrl_temp=reserv_closeout;}
            //definice podle RSS
            if (RSS_subfield==RSS_CommB_Reserv_Status_Req) {CommBctrl_temp=reserv_stat_req;}
            if (RSS_subfield==RSS_Uplink_ELM_Reserv_Status_Req) {upELMctrl_temp=reserv_stat_req;}
            if (RSS_subfield==RSS_Downlink_ELM_Reserv_Status_Req) {dwnELMctrl_temp=reserv_stat_req;}
            //synteza datalink control
            Datalink_control=(CommA_TMS_temp<<8)|(dwnELMctrl_temp<<4)|(upELMctrl_temp<<2)|CommBctrl_temp;
            Message[11] = Datalink_control;
            Message[12]=0;
//            if(!Get_Track_Number(Message[13])) Message[13]=0xFFFF;
            if (TrackNumber.Present) Message[13] = TrackNumber.Number; else Message.pop_back();
            return true;
        }
        if (_DF==24)
        {
            Message[0] = CommandNumber;
            Message[1] = Theta * 0x10000 /(2*pi);    //prevod rad na 360/65536 stupne
            Message[2] = Rho * 256 /1852;                   //prevod m na 1/256Nm
            Message[3] =  (RollCallNotReceived<<15) | ((!UsableAzimuth) << 9) | (LockoutLockedOut << 8) | (UFItems.Get_Expect_BDS() & 0xFF);
            Message[3]&=0x9FFF;
            uint8_t _KE;
            _KE = DFItems.Get_KE();
            uint8_t _ND;
            _ND = DFItems.Get_ND();
//            uint8_t _AP_PI[3];
//            if(!Items.Get_AP_PI(_AP_PI)) return false;
//            Message[4]= 0xE000 | ((_KE & 0x1)<<12) | (_AP_PI[2]);
//            Message[5]= ((_AP_PI[1])<<8) | _AP_PI[0];
            uint32_t _AA = UFItems.Get_AP();
            Message[4]= 0xE000 | ((_KE & 0x1)<<12) | ((_ND & 0xf)<<8) | (_AA>>16);
            Message[5]= _AA &0xFFFF;
//            Message[4]= 0xE000 | ((_KE & 0x1)<<12) | ((_ND & 0xf)<<8) | (CRC_Rest>>16);
//            Message[5]= CRC_Rest &0xFFFF;
            uint8_t _MD[10];
            if(!DFItems.Get_MD(_MD)) return false;
            Message[6]= ((_MD[0])<<8) | _MD[1];
            Message[7]= ((_MD[2])<<8) | _MD[3];
            Message[8]= ((_MD[4])<<8) | _MD[5];
            Message[9]= ((_MD[6])<<8) | _MD[7];
            Message[10]=((_MD[8])<<8) | _MD[9];
//            if(!Get_Datalink_Control(Message[11])) return false;
            Datalink_control=(CommA_TMS_temp<<8)|(dwnELMctrl_temp<<4)|(upELMctrl_temp<<2)|CommBctrl_temp;
            Message[11] = Datalink_control;
            Message[12]=0;
//            if(!Get_Track_Number(Message[13])) Message[13]=0xFFFF;
            if (TrackNumber.Present) Message[13] = TrackNumber.Number; else Message.pop_back();
            return true;
        }
    }

    return true;
}
