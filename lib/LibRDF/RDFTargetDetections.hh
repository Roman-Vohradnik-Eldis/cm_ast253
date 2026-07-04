#ifndef _RDF_TARGET_SIF_H
#define _RDF_TARGET_SIF_H

#include "RDFHeader.hh"
#include "RDFTargetStruct.hh"
#include "UF_DF_Data.hh"
#include <vector>
#include <bitset>

enum PULSE
{
	PULSE_F1,
	PULSE_C1,
	PULSE_A1,
	PULSE_C2,
	PULSE_A2,
	PULSE_C4,
	PULSE_A4,
	PULSE_X,
	PULSE_B1,
	PULSE_D1,
	PULSE_B2,
	PULSE_D2,
	PULSE_B4,
	PULSE_D4,
	PULSE_F2,
	PULSE_SPI,
	PULSES
};

enum UVD_PULSE {
    PULSE_RK1 = 0,
    PULSE_RK2,
    PULSE_RK3,
    PULSE_RK11,
    PULSE_RK12,
    PULSE_RK13
};

struct RDFSumPulse
{
	unsigned short Amplitude : 12;
	unsigned short OtherUsed : 1;
	unsigned short OtherMatched : 1;
	unsigned short AmplitudeNotMatched : 1;
	unsigned short Garbled : 1;

	RDFSumPulse () { init (); }
	void init ();
};

struct RDFDiffPulse
{
	unsigned short Amplitude : 12;
	unsigned short Signum : 1;
	unsigned short RSLS : 1;

	RDFDiffPulse () { init (); }
	void init ();
};

struct RDFSifDetections
{
	RDFTime Time;

	bool AzimuthNotValid : 1;
	bool AzimuthRSLSOut : 1;
    bool AzimuthValid : 1;
    bool AzimuthInBeam : 1;
    bool AzimuthDiffOk : 1;
    bool AzimuthRSLS : 1;

	unsigned char PeriodMode;
	unsigned short SweepNumber;

	bool RawCodeX;
	bool RawCodeSPI;
	unsigned short RawCodeValue;

	unsigned short SumValue : 14;
	unsigned short SumSignum : 1;
	unsigned short DiffValue : 14;
	unsigned short DiffSignum : 1;
    uint16_t DiffRSLS : 1;
    uint16_t PhaseValue: 14;

	float Rho;
	float Theta;
	float ThetaEncoder;
    uint8_t AzimuthQuality;

	RDFSumPulse SumPulse[16];
	RDFDiffPulse DiffPulse[16];

   std::vector<uint16_t> SumSamples;
   std::vector<uint16_t> DiffSamples;
   std::vector<uint16_t> PhaseSamples;

	RDFSifDetections () { init (); }
	void init ();
};

struct RDFUvdDetections
{
    RDFTime Time;

    uint8_t ReplyReportDescriptor;

    uint8_t InterrogationMode;

    uint16_t SweepNumber;

    uint64_t RawCode : 40;

    uint16_t SumValue : 14;
    uint16_t DiffValue : 14;
    uint16_t DiffRSLS : 1;
    uint16_t DiffSignum : 1;
    uint16_t PhaseValue: 14;

    float Rho;
    float Theta;
    float ThetaEncoder;
    uint8_t ThetaValid : 1;
    uint8_t ThetaInBeam : 1;
    uint8_t ThetaRSLS : 1;
    uint8_t ThetaDiffOk : 1;
    uint8_t ThetaQuality;

    RDFSumPulse SumPulse[6];
    RDFDiffPulse DiffPulse[6];

    std::vector<uint16_t> SumSamples;
    std::vector<uint16_t> DiffSamples;
    std::vector<uint16_t> PhaseSamples;
};

const std::map<int8_t,std::string> TargetBDS_to_String = map_list_of
    (0,"For_asterix_048_and_018")
    (1,"For_asterix_048")
    (2,"For_asterix_018")
    (3,"Unused");


struct RDFModeSDetections
{
	RDFTime Time;

	bool RollCallNotReceived : 1;
	bool ReplyCorrected : 1;
	bool LockoutLockedOut : 1;
	bool InvalidBDS : 1;
    bool AzimuthValid : 1;
    bool AzimuthInBeam : 1;
    bool AzimuthDiffOk : 1;
    bool AzimuthRSLS : 1;
	unsigned char TargetBDS : 2;

	uf_df_data Uplink;
	uf_df_data DfRaw;
	uf_df_data DfCorrected;
	vector<unsigned char> DfConfidence;

	float Rho;
	float Theta;
	float ThetaEncoder;
    uint8_t AzimuthQuality;

	unsigned short SumValue : 14;
	unsigned short SumSignum : 1;
	unsigned short DiffValue : 14;
	unsigned short DiffSignum : 1;
    uint16_t DiffRSLS : 1;
    uint16_t PhaseValue: 14;

	unsigned char PeriodType;
	unsigned short SweepNumber;
	unsigned short CommandNumber;

	RDFTrackNumber TrackNumber;

    RDFExpectedBDS ExpectedBDS;
	
    RDFDatalinkControl DatalinkControl;

   uint16_t SweepLength;
   bool SweepLengthPresent : 1;

   uint16_t InterrogationDelay;
   bool InterrogationDelayPresent :1;

   std::vector<uint16_t> SumSamples;
   std::vector<uint16_t> DiffSamples;
   std::vector<uint16_t> PhaseSamples;

	RDFModeSDetections () { init (); }
	void init ();
	bool DF_to_Message(std::vector<uint16_t> &Message);  // funkce na prevod detekceS do stareho formatu ,funkce neni vyzkousena!!!!
};

#endif /* _RDF_TARGET_H */
