

//#ifndef false
//#define false ((bool)0)
//#endif

//#ifndef true
//#define true ((bool)1)
//#endif

#include <stdbool.h>
#include "cm_base_functs.hh"
#include "cm_string.hh"
#include "cm_std_string.hh"
//#include "mup_interface.hh"
#include "ParserRDF.hh"
#include <stdbool.h>


int PARSER_RDF_ID_SIC = -1;
int PARSER_RDF_ID_SAC = -1;
int PARSER_RDF_ID_TOD = -1;
int PARSER_RDF_ID_AZ = -1;
int PARSER_RDF_ID_RNG = -1;
int PARSER_RDF_ID_M3A = -1;
int PARSER_RDF_ID_M3A_G = -1;
int PARSER_RDF_ID_M3A_I = -1;
int PARSER_RDF_ID_M3A_T = -1;
int PARSER_RDF_ID_MC = -1;
int PARSER_RDF_ID_MC_G = -1;
int PARSER_RDF_ID_MC_I = -1;
int PARSER_RDF_ID_MC_T = -1;
int PARSER_RDF_ID_M1 = -1;
int PARSER_RDF_ID_M1_G = -1;
int PARSER_RDF_ID_M1_I = -1;
int PARSER_RDF_ID_M1_T = -1;
int PARSER_RDF_ID_M2 = -1;
int PARSER_RDF_ID_M2_G = -1;
int PARSER_RDF_ID_M2_I = -1;
int PARSER_RDF_ID_M2_T = -1;
int PARSER_RDF_ID_FL = -1;
int PARSER_RDF_ID_FL_G = -1;
int PARSER_RDF_ID_FL_I = -1;
int PARSER_RDF_ID_FL_25 = -1;
int PARSER_RDF_ID_FlagTST = -1;
int PARSER_RDF_ID_FlagSIM = -1;
int PARSER_RDF_ID_FlagSPI = -1;
int PARSER_RDF_ID_FlagRAB = -1;
int PARSER_RDF_ID_SADDR = -1;
int PARSER_RDF_ID_CallSign = -1;
int PARSER_RDF_ID_UserText = -1;
int PARSER_RDF_ID_TargetTYPE = -1;
int PARSER_RDF_ID_BDS = -1;
int PARSER_RDF_ID_WE = -1;
int PARSER_RDF_ID_SSR_LEN = -1;
int PARSER_RDF_ID_SSR_REP = -1;
int PARSER_RDF_ID_SSR_AMP = -1;
int PARSER_RDF_ID_PSR_LEN = -1;
int PARSER_RDF_ID_PSR_AMP = -1;
int PARSER_RDF_ID_PSR_DIFF_AZ = -1;
int PARSER_RDF_ID_PSR_DIFF_RNG = -1;
int PARSER_RDF_ID_PSR_Doppler = -1;
int PARSER_RDF_ID_PSR_CalcDoppler = -1;
int PARSER_RDF_ID_PSR_CalcDoppler_V = -1;
int PARSER_RDF_ID_MDS_MSP = -1;
int PARSER_RDF_ID_MDS_Capa_CS = -1;
int PARSER_RDF_ID_MDS_ACAS = -1;
int PARSER_RDF_ID_MDS_Level = -1;
int PARSER_RDF_ID_MDS_FS = -1;
int PARSER_RDF_ID_BDSage = -1;
int PARSER_RDF_ID_TrNum = -1;
int PARSER_RDF_ID_TrNum_Tmp = -1;

int PARSER_RDF_ID_TrFlagInit = -1;
int PARSER_RDF_ID_TrFlagCancel = -1;
int PARSER_RDF_ID_TrFlagTent = -1;
int PARSER_RDF_ID_TrFlagCorr = -1;
int PARSER_RDF_ID_TrFlagCST = -1;
int PARSER_RDF_ID_TrFlagMono = -1;
int PARSER_RDF_ID_TrFlagDOU = -1;
int PARSER_RDF_ID_TrFlagMAH = -1;
int PARSER_RDF_ID_TrFlagSUP = -1;
int PARSER_RDF_ID_TrFlagPSR = -1;
int PARSER_RDF_ID_TrFlagSSR = -1;
int PARSER_RDF_ID_TrFlagMDS = -1;
int PARSER_RDF_ID_TrFlagTurn = -1;
int PARSER_RDF_ID_TrFlagSpeed = -1;
int PARSER_RDF_ID_TrFlagClimb = -1;
int PARSER_RDF_ID_TrClimb = -1;
int PARSER_RDF_ID_TrX = -1;
int PARSER_RDF_ID_TrY = -1;
int PARSER_RDF_ID_TrSpeed = -1;
int PARSER_RDF_ID_TrHeading = -1;
int PARSER_RDF_ID_TrAccelX = -1;
int PARSER_RDF_ID_TrAccelY = -1;

int PARSER_RDF_ID_ServiceTYPE = -1;
int PARSER_RDF_ID_NORTH_POS3D_LAT = -1;
int PARSER_RDF_ID_NORTH_POS3D_LON = -1;
int PARSER_RDF_ID_NORTH_POS3D_ALT = -1;
int PARSER_RDF_ID_NORTH_ROTSPD = -1;


struct sPARSER_RDF_FixConfigData
{
  std::string Name;
  std::string Name_MUP;
  std::string RDFTypes; // P=Plot, T=Track, S=Service,          A=ADSB
  int *ID_PTR;
  std::string Description;
};
sPARSER_RDF_FixConfigData PARSER_RDF_FixConfigData [] = {
 {  "SAC"                 , "SAC"                 , "PTS", &PARSER_RDF_ID_SAC                 }
,{  "SIC"                 , "SIC"                 , "PTS", &PARSER_RDF_ID_SIC                 }
,{  "TOD"                 , "TOD"                 , "PTS", &PARSER_RDF_ID_TOD                 }
,{  "AZ"                  , "AZ"                  , "PTS", &PARSER_RDF_ID_AZ                  }
,{  "RNG"                 , "RNG"                 , "PT" , &PARSER_RDF_ID_RNG                 }
,{  "M3A"                 , "M3A"                 , "PT" , &PARSER_RDF_ID_M3A                 }
,{  "M3A_G"               , "M3A_G"               , "PT" , &PARSER_RDF_ID_M3A_G               }
,{  "M3A_I"               , "M3A_I"               , "PT" , &PARSER_RDF_ID_M3A_I               }
,{  "M3A_T"               , "M3A_T"               , "PT" , &PARSER_RDF_ID_M3A_T               }
,{  "MC"                  , "MC"                  , "PT" , &PARSER_RDF_ID_MC                  }
,{  "MC_G"                , "MC_G"                , "PT" , &PARSER_RDF_ID_MC_G                }
,{  "MC_I"                , "MC_I"                , "PT" , &PARSER_RDF_ID_MC_I                }
,{  "MC_T"                , "MC_T"                , "PT" , &PARSER_RDF_ID_MC_T                }
,{  "M1"                  , "M1"                  , "PT" , &PARSER_RDF_ID_M1                  }
,{  "M1_G"                , "M1_G"                , "PT" , &PARSER_RDF_ID_M1_G                }
,{  "M1_I"                , "M1_I"                , "PT" , &PARSER_RDF_ID_M1_I                }
,{  "M1_T"                , "M1_T"                , "PT" , &PARSER_RDF_ID_M1_T                }
,{  "M2"                  , "M2"                  , "PT" , &PARSER_RDF_ID_M2                  }
,{  "M2_G"                , "M2_G"                , "PT" , &PARSER_RDF_ID_M2_G                }
,{  "M2_I"                , "M2_I"                , "PT" , &PARSER_RDF_ID_M2_I                }
,{  "M2_T"                , "M2_T"                , "PT" , &PARSER_RDF_ID_M2_T                }
,{  "FL"                  , "FL"                  , "PT" , &PARSER_RDF_ID_FL                  }
,{  "FL_G"                , "FL_G"                , "PT" , &PARSER_RDF_ID_FL_G                }
,{  "FL_I"                , "FL_I"                , "PT" , &PARSER_RDF_ID_FL_I                }
,{  "FL_25"               , "FL_25"               , "PT" , &PARSER_RDF_ID_FL_25               }
,{  "FlagTST"             , "FlagTST"             , "PT" , &PARSER_RDF_ID_FlagTST             }
,{  "FlagSIM"             , "FlagSIM"             , "PT" , &PARSER_RDF_ID_FlagSIM             }
,{  "FlagSPI"             , "FlagSPI"             , "PT" , &PARSER_RDF_ID_FlagSPI             }
,{  "FlagRAB"             , "FlagRAB"             , "PT" , &PARSER_RDF_ID_FlagRAB             }
,{  "SADDR"               , "SADDR"               , "PT" , &PARSER_RDF_ID_SADDR               }
,{  "CallSign"            , "CallSign"            , "PT" , &PARSER_RDF_ID_CallSign            }
,{  "UserText"            , "UserText"            , "PT" , &PARSER_RDF_ID_UserText            }
,{  "TargetTYPE"          , "TargetTYPE"          , "PT" , &PARSER_RDF_ID_TargetTYPE          }
,{  "BDS"                 , "BDS"                 , "PT" , &PARSER_RDF_ID_BDS                 }
,{  "WE"                  , "WE"                  , "PT" , &PARSER_RDF_ID_WE                  } // WarningError
,{  "SSR_LEN"             , "SSR_LEN"             , "PT" , &PARSER_RDF_ID_SSR_LEN             }
,{  "SSR_REP"             , "SSR_REP"             , "PT" , &PARSER_RDF_ID_SSR_REP             }
,{  "SSR_AMP"             , "SSR_AMP"             , "PT" , &PARSER_RDF_ID_SSR_AMP             }
,{  "PSR_LEN"             , "PSR_LEN"             , "PT" , &PARSER_RDF_ID_PSR_LEN             }
,{  "PSR_AMP"             , "PSR_AMP"             , "PT" , &PARSER_RDF_ID_PSR_AMP             }
,{  "PSR_DIFF_AZ"         , "PSR_DIFF_AZ"         , "PT" , &PARSER_RDF_ID_PSR_DIFF_AZ         }
,{  "PSR_DIFF_RNG"        , "PSR_DIFF_RNG"        , "PT" , &PARSER_RDF_ID_PSR_DIFF_RNG        }
,{  "PSR_Doppler"         , "PSR_Doppler"         , "PT" , &PARSER_RDF_ID_PSR_Doppler         }
,{  "PSR_CalcDoppler"     , "PSR_CalcDoppler"     , "PT" , &PARSER_RDF_ID_PSR_CalcDoppler     }
,{  "PSR_CalcDoppler_V"   , "PSR_CalcDoppler_V"   , "PT" , &PARSER_RDF_ID_PSR_CalcDoppler_V   }
,{  "MDS_MSP"             , "MDS_MSP"             , "PT" , &PARSER_RDF_ID_MDS_MSP             }
,{  "MDS_Capa_CS"         , "MDS_Capa_CS"         , "PT" , &PARSER_RDF_ID_MDS_Capa_CS         }
,{  "MDS_ACAS"            , "MDS_ACAS"            , "PT" , &PARSER_RDF_ID_MDS_ACAS            }
,{  "MDS_Level"           , "MDS_Level"           , "PT" , &PARSER_RDF_ID_MDS_Level           }
,{  "MDS_FS"              , "MDS_FS"              , "PT" , &PARSER_RDF_ID_MDS_FS              }
,{  "BDSage"              , "BDSage"              , "PT" , &PARSER_RDF_ID_BDSage              }
,{  "TrNum"               , "TrNum"               , "T"  , &PARSER_RDF_ID_TrNum               }
,{  "TrNum_Tmp"           , "TrNum_Tmp"           , "T"  , &PARSER_RDF_ID_TrNum_Tmp           }

,{  "TrFlagInit"          , "TrFlagInit"          , "T"  , &PARSER_RDF_ID_TrFlagInit          }
,{  "TrFlagCancel"        , "TrFlagCancel"        , "T"  , &PARSER_RDF_ID_TrFlagCancel        }
,{  "TrFlagTent"          , "TrFlagTent"          , "T"  , &PARSER_RDF_ID_TrFlagTent          }
,{  "TrFlagCorr"          , "TrFlagCorr"          , "T"  , &PARSER_RDF_ID_TrFlagCorr          }
,{  "TrFlagCST"           , "TrFlagCST"           , "T"  , &PARSER_RDF_ID_TrFlagCST           }
,{  "TrFlagMono"          , "TrFlagMono"          , "T"  , &PARSER_RDF_ID_TrFlagMono          }
,{  "TrFlagDOU"           , "TrFlagDOU"           , "T"  , &PARSER_RDF_ID_TrFlagDOU           }
,{  "TrFlagMAH"           , "TrFlagMAH"           , "T"  , &PARSER_RDF_ID_TrFlagMAH           }
,{  "TrFlagSUP"           , "TrFlagSUP"           , "T"  , &PARSER_RDF_ID_TrFlagSUP           }
,{  "TrFlagPSR"           , "TrFlagPSR"           , "T"  , &PARSER_RDF_ID_TrFlagPSR           }
,{  "TrFlagSSR"           , "TrFlagSSR"           , "T"  , &PARSER_RDF_ID_TrFlagSSR           }
,{  "TrFlagMDS"           , "TrFlagMDS"           , "T"  , &PARSER_RDF_ID_TrFlagMDS           }
,{  "TrFlagTurn"          , "TrFlagTurn"          , "T"  , &PARSER_RDF_ID_TrFlagTurn          }
,{  "TrFlagSpeed"         , "TrFlagSpeed"         , "T"  , &PARSER_RDF_ID_TrFlagSpeed         }
,{  "TrFlagClimb"         , "TrFlagClimb"         , "T"  , &PARSER_RDF_ID_TrFlagClimb         }
,{  "TrClimb"             , "TrClimb"             , "T"  , &PARSER_RDF_ID_TrClimb             }
,{  "TrX"                 , "TrX"                 , "T"  , &PARSER_RDF_ID_TrX                 }
,{  "TrY"                 , "TrY"                 , "T"  , &PARSER_RDF_ID_TrY                 }
,{  "TrSpeed"             , "TrSpeed"             , "T"  , &PARSER_RDF_ID_TrSpeed             }
,{  "TrHeading"           , "TrHeading"           , "T"  , &PARSER_RDF_ID_TrHeading           }
,{  "TrAccelX"            , "TrAccelX"            , "T"  , &PARSER_RDF_ID_TrAccelX            }
,{  "TrAccelY"            , "TrAccelY"            , "T"  , &PARSER_RDF_ID_TrAccelY            }

,{  "ServiceTYPE"         , "ServiceTYPE"         , "S"  , &PARSER_RDF_ID_ServiceTYPE         }
,{  "NORTH_POS3D_LAT"     , "NORTH_POS3D_LAT"     , "S"  , &PARSER_RDF_ID_NORTH_POS3D_LAT     }
,{  "NORTH_POS3D_LON"     , "NORTH_POS3D_LON"     , "S"  , &PARSER_RDF_ID_NORTH_POS3D_LON     }
,{  "NORTH_POS3D_ALT"     , "NORTH_POS3D_ALT"     , "S"  , &PARSER_RDF_ID_NORTH_POS3D_ALT     }
,{  "NORTH_ROTSPD"        , "NORTH_ROTSPD"        , "S"  , &PARSER_RDF_ID_NORTH_ROTSPD        }
,{  ""                    , ""                    , ""   , NULL                               } // all empty at end
};





class CPARSER_RDF_FixConfig
{
  public:
    std::map<int, std::string> id_to_Name;
    std::map<int, std::string> id_to_Name_MUP;
    std::map<std::string, int> Name_to_id;
    std::map<std::string, int> Name_MUP_to_id;
    CPARSER_RDF_FixConfig();
};

CPARSER_RDF_FixConfig::CPARSER_RDF_FixConfig()
{
  int i=0;

  while (!PARSER_RDF_FixConfigData[i].Name.empty())
  {
    if (PARSER_RDF_FixConfigData[i].ID_PTR != NULL) *(PARSER_RDF_FixConfigData[i].ID_PTR) = i;
    id_to_Name[i]     = PARSER_RDF_FixConfigData[i].Name;
    id_to_Name_MUP[i] = PARSER_RDF_FixConfigData[i].Name_MUP;

    Name_to_id    [PARSER_RDF_FixConfigData[i].Name    ] = i;
    Name_MUP_to_id[PARSER_RDF_FixConfigData[i].Name_MUP] = i;
//    printf("Id=%d Name=%s, Name_MUP=%s\n",i, PARSER_RDF_FixConfigData[i].Name.c_str(), PARSER_RDF_FixConfigData[i].Name_MUP.c_str());
    i++;
  }
//  printf("Id's=%d \n",i);
}


CPARSER_RDF_FixConfig PARSER_RDF_FixConfig;




// ******************************************************************************************************************************************************
// ******************************************************************************************************************************************************







CParserRDFany::CParserRDFany()
{
}


bool CParserRDFany::prepare_expr(class C_CM_MUP_Interface *MUP, std::string &func, std::string &ErrorMessage, std::set<std::string> *_needVars, std::set<std::string> *_modifyVars)
{
  std::string RetVal = func; // replace \n,\r\,t ' '
  gchar *no_returns = cm_string_strdup_replace_outside_string(func.c_str(), "\t\n\r\f\v", ' ');
  if (no_returns)
  {
    RetVal = std::string(no_returns);
    g_free(no_returns);
  }
  func = RetVal;
  std::list<std::string> OutList;
  std::list<std::string> RqList;
  if (!RetVal.empty())
  {
    if (!MUP->GetUsedVariables(RetVal, &OutList, false, true, false, false)) // output
    {
      ErrorMessage = "MUP::ERROR:GetUsedVariables \"" + RetVal + "\" -> \'" + MUP->GetLastError() + "\'";
//      cms_ns_if_print("libparserrdf",1, "%s",ErrorMessage.c_str());
      return false;
    }
    if (!MUP->GetUsedVariables(RetVal, &RqList, false, false, true)) // input
//    if (!MUP->GetUsedVariables(RetVal, &RqList, false, (bool)false, (bool)true)) // input
    {
      ErrorMessage = "MUP::ERROR:GetUsedVariables \"" + RetVal + "\" -> \'" + MUP->GetLastError() + "\'";
//      cms_ns_if_print("libparserrdf",1, "%s",ErrorMessage.c_str());
      return false;
    }
  }

//      printf("Prepare(%d):\"%s\"\n",cmdType,RetVal.c_str());

  for (std::list<std::string>::iterator it = OutList.begin(); it != OutList.end(); it++)
  {
    needVars.insert(*it);
    modifyVars.insert(*it);
//        printf("   ***OUT:%s\n",it->c_str());
  }
  for (std::list<std::string>::iterator it = RqList.begin(); it != RqList.end(); it++)
  {
    needVars.insert(*it);
//        printf("   ***IN:%s\n",it->c_str());
  }

  return true;
}

bool CParserRDFany::define_set(class C_CM_MUP_Interface *MUP, std::string func, std::string &ErrorMessage, bool &Empty)
{
  cmdType=E_CMD_TYPE_SET;
  Expr = func;
  bool RetVal = prepare_expr(MUP, Expr, ErrorMessage, &needVars, &modifyVars);
  if (Expr.empty()) Empty = true; else Empty = false;
  return ( Empty ? true : RetVal);
}

bool CParserRDFany::define_undef(class C_CM_MUP_Interface *MUP, std::set<std::string> &_vars, std::string &ErrorMessage)
{
  cmdType=E_CMD_TYPE_UNDEF;
  needVars = _vars;
  modifyVars = _vars;
  return true;
}

bool CParserRDFany::define_if(class C_CM_MUP_Interface *MUP, std::string func, std::string &ErrorMessage, std::vector<class CParserRDFprocedure> &_Then, std::vector<class CParserRDFprocedure> &_Else)
{
  cmdType=E_CMD_TYPE_IF;
  std::string IfExpr1 = "(" + func + ")";
  if (!prepare_expr(MUP, IfExpr1, ErrorMessage, &needVars, &modifyVars)) return false;
  IfExpr.push_back(IfExpr1);
  Then = _Then;
  Else = _Else;
  return true;
}

bool CParserRDFany::define_case(class C_CM_MUP_Interface *MUP, std::vector<std::string> &_func, std::string &ErrorMessage, std::vector<class CParserRDFprocedure> &_Then, std::vector<class CParserRDFprocedure> &_Else)
{
  cmdType=E_CMD_TYPE_CASE;
  for (unsigned i=0;i<_func.size();i++)
  {
    std::string IfExpr1 = "(" + _func[i] + ")";
    if (!prepare_expr(MUP, IfExpr1, ErrorMessage, &needVars, &modifyVars)) return false;
    IfExpr.push_back(IfExpr1);
  }
  Then = _Then;
  Else = _Else;
  return true;
}

bool CParserRDFany::define_for(class C_CM_MUP_Interface *MUP, std::string funcStart, std::string funcCompare, std::string funcIncrement, std::string &ErrorMessage, std::vector<class CParserRDFprocedure> &_Then)
{
  cmdType=E_CMD_TYPE_FOR;

  std::string IfExpr1;
  IfExpr1 = funcStart;
  if (!prepare_expr(MUP, IfExpr1, ErrorMessage, &needVars, &modifyVars)) return false;
  IfExpr.push_back(IfExpr1);

  IfExpr1 = "(" + funcCompare + ")";
  if (!prepare_expr(MUP, IfExpr1, ErrorMessage, &needVars, &modifyVars)) return false;
  IfExpr.push_back(IfExpr1);

  IfExpr1 = funcIncrement;
  if (!prepare_expr(MUP, IfExpr1, ErrorMessage, &needVars, &modifyVars)) return false;
  IfExpr.push_back(IfExpr1);

  Then = _Then;
  return true;
}

std::string CParserRDFany::print_me(unsigned verbosity, std::string act_tab, std::string one_tab)
{
  std::string RetVal = std::string();
  if (cmdType==E_CMD_TYPE_SET) // set
  {
    RetVal += act_tab + Expr + "\n";
  }
  if (cmdType==E_CMD_TYPE_UNDEF) // undef
  {
    RetVal += act_tab + "undef";
    for (std::set<std::string>::iterator it = modifyVars.begin(); it != modifyVars.end(); it++)
    {
       RetVal += "," + (*it);
    }
    for (std::set<int>::iterator it = modifyVars_i.begin(); it != modifyVars_i.end(); it++)
    {
      if (*it >= 0) RetVal += "," + PARSER_RDF_FixConfigData[*it].Name_MUP;
    }
    RetVal += act_tab + "\n";
  }
  if (cmdType==E_CMD_TYPE_IF) // if
  {
    RetVal += act_tab + "if," + IfExpr[0] + "\n";
    RetVal += Then[0].print_me(verbosity, act_tab + one_tab, one_tab);
    if (Else.size()>0)
    {
      RetVal += act_tab + "else" + "\n";
      RetVal += Else[0].print_me(verbosity, act_tab + one_tab, one_tab);
    }
  }
  if (cmdType==E_CMD_TYPE_CASE) // case
  {
    for (unsigned c = 0; c < MIN(IfExpr.size(), Then.size()); c++)
    {
      if (c==0)
      {
        RetVal += act_tab + "case," + IfExpr[c] + "\n";
      } else {
        RetVal += act_tab + "case-next," + IfExpr[c] + "\n";
      }
      RetVal += Then[c].print_me(verbosity, act_tab + one_tab, one_tab);
    }
    if (Else.size()>0)
    {
      RetVal += act_tab + "case-default" + "\n";
      RetVal += Else[0].print_me(verbosity, act_tab + one_tab, one_tab);
    }
  }
  if (cmdType==E_CMD_TYPE_FOR) // for
  {
    RetVal += act_tab + "for";
    for (unsigned c = 0; c < 3; c++)
    {
      RetVal += "," + IfExpr[c];
    }
    RetVal += "\n";
    RetVal += Then[0].print_me(verbosity, act_tab + one_tab, one_tab);
  }
  return RetVal;
}

void CParserRDFany::cumulateVars(std::set<std::string> &_needVars, std::set<std::string> &_modifyVars)
{
  for (std::set<std::string>::iterator it = needVars.begin(); it != needVars.end(); it++)
  {
    _needVars.insert(*it);
  }
  for (std::set<std::string>::iterator it = modifyVars.begin(); it != modifyVars.end(); it++)
  {
    _modifyVars.insert(*it);
  }
  if ((cmdType==E_CMD_TYPE_IF) || (cmdType==E_CMD_TYPE_CASE) || (cmdType==E_CMD_TYPE_FOR)) // if or case
  {
    for (unsigned u = 0; u<Then.size(); u++) Then[u].cumulateVars();
    if (Else.size()>0) Else[0].cumulateVars();
  }
}

void CParserRDFany::optimizeVars(std::set<std::string> *parentVars, std::map<std::string, class CCmVariable> *GlobalVariables, std::map<std::string, class CCmVariable> *InOutVariables)
{
  if ((cmdType==E_CMD_TYPE_IF) || (cmdType==E_CMD_TYPE_CASE) || (cmdType==E_CMD_TYPE_FOR)) // if or case
  {
    for (unsigned u = 0; u<Then.size(); u++) Then[u].optimizeVars(parentVars,GlobalVariables,InOutVariables);
    if (Else.size()>0) Else[0].optimizeVars(parentVars,GlobalVariables,InOutVariables);
  }
  if (cmdType==E_CMD_TYPE_UNDEF)
  {
    std::set<std::string> mVars = modifyVars;
    modifyVars.clear();
    for (std::set<std::string>::iterator it = mVars.begin(); it != mVars.end(); it++)
    {
      std::map<std::string, int>::iterator it_id = PARSER_RDF_FixConfig.Name_to_id.find(*it);
      if (it_id != PARSER_RDF_FixConfig.Name_to_id.end())
      {
        modifyVars_i.insert(it_id->second);
      } else {
        modifyVars.insert(*it);
      }
    }
  }
}

// ************************************************************************************************
// ************************************************************************************************
// ************************************************************************************************
// ************************************************************************************************



CParserRDFprocedure::CParserRDFprocedure()
{
  clear();
}

bool CParserRDFprocedure::add_set(class C_CM_MUP_Interface *MUP, std::string func, std::string &ErrorMessage, bool &Empty)
{
  CParserRDFany n;
  bool RetVal = n.define_set(MUP, func, ErrorMessage, Empty);
  if ((RetVal)&&(!Empty))
  {
    AllCMDs.push_back(n);
  }
  return RetVal;
}

bool CParserRDFprocedure::add_undef(class C_CM_MUP_Interface *MUP, std::set<std::string> _vars, std::string &ErrorMessage)
{
  CParserRDFany n;
  bool RetVal = n.define_undef(MUP, _vars, ErrorMessage);
  AllCMDs.push_back(n);
  return RetVal;
}

bool CParserRDFprocedure::add_if(class C_CM_MUP_Interface *MUP, std::string func, std::string &ErrorMessage, std::vector<class CParserRDFprocedure> &_Then, std::vector<class CParserRDFprocedure> &_Else)
{
  CParserRDFany n;
  bool RetVal = n.define_if(MUP, func, ErrorMessage, _Then, _Else);
  AllCMDs.push_back(n);
  return RetVal;
}

bool CParserRDFprocedure::add_case(class C_CM_MUP_Interface *MUP, std::vector<std::string> &_func, std::string &ErrorMessage, std::vector<class CParserRDFprocedure> &_Then, std::vector<class CParserRDFprocedure> &_Else)
{
  CParserRDFany n;
  bool RetVal = n.define_case(MUP, _func, ErrorMessage, _Then, _Else);
  AllCMDs.push_back(n);
  return RetVal;
}

bool CParserRDFprocedure::add_for(class C_CM_MUP_Interface *MUP, std::string funcStart, std::string funcCompare, std::string funcIncrement, std::string &ErrorMessage, std::vector<class CParserRDFprocedure> &_Then)
{
  CParserRDFany n;
  bool RetVal = n.define_for(MUP, funcStart, funcCompare, funcIncrement, ErrorMessage, _Then);
  AllCMDs.push_back(n);
  return RetVal;
}

void CParserRDFprocedure::clear(void)
{
  modifyVars.clear();
  needVars.clear();
  modifyVars_i.clear();
  needAddVars.clear();
  needAddVars_i.clear();
  AllCMDs.clear();
}

void CParserRDFprocedure::cumulateVars(void)
{
  for (unsigned i=0; i<AllCMDs.size();i++)
  {
    AllCMDs[i].cumulateVars(needVars, modifyVars);
  }
}

void CParserRDFprocedure::optimizeVars(std::set<std::string> *parentVars, std::map<std::string, class CCmVariable> *GlobalVariables, std::map<std::string, class CCmVariable> *InOutVariables)
{
  std::set<std::string> ALLparentVars;
  if (parentVars!=NULL) ALLparentVars = *parentVars;

  for (std::set<std::string>::iterator it = needVars.begin(); it != needVars.end(); it++)
  {
    bool use = true;
    if ((parentVars != NULL)&&(use))
    {
      std::set<std::string>::iterator itm = parentVars->find(*it);
      if (itm != parentVars->end()) use = false;
    }
    if ((GlobalVariables != NULL)&&(use))
    {
      std::map<std::string, class CCmVariable>::iterator itm = GlobalVariables->find(*it);
      if (itm != GlobalVariables->end()) use = false;
    }

    if (use)
    {
      std::map<std::string, int>::iterator it_id = PARSER_RDF_FixConfig.Name_to_id.find(*it);
      if (it_id != PARSER_RDF_FixConfig.Name_to_id.end())
      {
//        printf("  found numeric ID=%d from string %s\n",it_id->second, it->c_str());
        needAddVars_i.insert(it_id->second);
      } else {
        if (InOutVariables != NULL)
        {
          if (InOutVariables->find(*it) != InOutVariables->end()) needAddVars.insert(*it);
        } else {
          // local variable
        }
      }
      ALLparentVars.insert(*it);
    }
  }

  std::set<std::string> mVars = modifyVars;
  modifyVars.clear();

  for (std::set<std::string>::iterator it = mVars.begin(); it != mVars.end(); it++)
  {
    bool used = false;
    if ((GlobalVariables != NULL))
    {
      std::map<std::string, class CCmVariable>::iterator itm = GlobalVariables->find(*it);
      if (itm != GlobalVariables->end())
      {
        used = true;
        modifyVars.insert(*it);
      }
    }
    if ((!used)&&(InOutVariables != NULL))
    {
      std::map<std::string, class CCmVariable>::iterator itm = InOutVariables->find(*it);
      if (itm != InOutVariables->end())
      {
        used = true;
        modifyVars.insert(*it);
      }
    }
    if (!used)
    {
      std::map<std::string, int>::iterator it_id = PARSER_RDF_FixConfig.Name_to_id.find(*it);
      if (it_id != PARSER_RDF_FixConfig.Name_to_id.end())
      {
        modifyVars_i.insert(it_id->second);
      } else {
        // local variable
      }
    }
  }
  for (unsigned i=0; i<AllCMDs.size();i++)
  {
    AllCMDs[i].optimizeVars(&ALLparentVars, GlobalVariables, InOutVariables);
  }
}

std::string CParserRDFprocedure::print_me(unsigned verbosity, std::string act_tab, std::string one_tab)
{
  std::string RetVal = std::string();

  if (verbosity > 5)
  {
    std::string needStr   = std::string();
    std::string needAddStr   = std::string();
    std::string modifyStr = std::string();
    for (std::set<std::string>::iterator it = needVars.begin(); it != needVars.end(); it++)
    {
      needStr += (needStr.empty() ? "" : ",")  + std::string("\'") + (*it) + std::string("\'");
    }
    for (std::set<std::string>::iterator it = needAddVars.begin(); it != needAddVars.end(); it++)
    {
      needAddStr += (needAddStr.empty() ? "" : ",")  + std::string("\'") + (*it) + std::string("\'");
    }
    for (std::set<int>::iterator it = needAddVars_i.begin(); it != needAddVars_i.end(); it++)
    {
      std::string wstr;
      std::map<int, std::string>::iterator it_id = PARSER_RDF_FixConfig.id_to_Name.find(*it);
      if (it_id != PARSER_RDF_FixConfig.id_to_Name.end())
      {
        wstr = it_id->second.c_str();
      } else {
        char ws[20];
        sprintf(ws,"(%d)",*it);
        wstr = std::string(ws);
      }
      needAddStr += (needAddStr.empty() ? "" : ",") + wstr;
    }
    for (std::set<std::string>::iterator it = modifyVars.begin(); it != modifyVars.end(); it++)
    {
      modifyStr += (modifyStr.empty() ? "" : ",") + std::string("\'") + (*it) + std::string("\'");
    }
    for (std::set<int>::iterator it = modifyVars_i.begin(); it != modifyVars_i.end(); it++)
    {
      std::string wstr;
      std::map<int, std::string>::iterator it_id = PARSER_RDF_FixConfig.id_to_Name.find(*it);
      if (it_id != PARSER_RDF_FixConfig.id_to_Name.end())
      {
        wstr = it_id->second.c_str();
      } else {
        char ws[20];
        sprintf(ws,"(%d)",*it);
        wstr = std::string(ws);
      }
      modifyStr += (modifyStr.empty() ? "" : ",") + wstr;
    }


    if (verbosity > 8)
    {
      if (!needStr.empty())  RetVal += act_tab + "... need:"  + needStr   + "\n";
    }
    if (!needAddStr.empty()) RetVal += act_tab + "... add:"   + needAddStr   + "\n";
    if (!modifyStr.empty())  RetVal += act_tab + "... modif:" + modifyStr + "\n";
  }

  for (unsigned i=0; i<AllCMDs.size();i++)
  {
    RetVal += AllCMDs[i].print_me(verbosity, act_tab, one_tab);
  }
  return RetVal;
}




// ************************************************************************************************
// ************************************************************************************************
// ************************************************************************************************
// ************************************************************************************************

bool CParserRDF::set_process_one_block(std::string func, class C_CM_MUP_Interface *MUP, class CParserRDFprocedure &current_proc)
{
  std::string func_without_brackets = func;
  std::deque<std::string> OutParams;
  unsigned previous_size = 0;
  unsigned found_items = 1;
  unsigned found_params = 1;
  std::string ErrorMessage;
  do
  {
    previous_size = func_without_brackets.size();
    OutParams.clear();
    cm_std_string_SplitLineAndInterpretStringsInQuotes(func_without_brackets, OutParams, ";", "[](){}", true);
    found_items = OutParams.size();
    if (found_items<2)
    {
      OutParams.clear();
      cm_std_string_SplitLineAndInterpretStringsInQuotes(func_without_brackets, OutParams, ",", "[](){}", true);
      found_params = OutParams.size();
      if (found_items<2)
      {
        func_without_brackets  = cm_std_string_remove_first_brackets_pair_or_interpret_string(func_without_brackets, "()[]{}", " \t\n\r\f\v");
      }
    }
  } while ((previous_size != func_without_brackets.size()) && (found_items<2) && (found_params<2));

  if (found_items>1)
  {
    OutParams.clear();
    cm_std_string_SplitLineAndInterpretStringsInQuotes(func_without_brackets, OutParams, ";", "[](){}", true);
    for (unsigned i=0;i<OutParams.size();i++)
    {
      if (!set_process_one_block(OutParams[i], MUP, current_proc)) return false;
    }
  } else {
    if (found_params>1)
    {
      OutParams.clear();
      cm_std_string_SplitLineAndInterpretStringsInQuotes(func_without_brackets, OutParams, ",", "[](){}", true);
      if (OutParams.size()>0)
      {
               if (OutParams[0] == "if")
        {
          if ((OutParams.size()==3)||(OutParams.size()==4))
          {
            std::vector<class CParserRDFprocedure> Then; Then.clear();
            std::vector<class CParserRDFprocedure> Else; Else.clear();
            class CParserRDFprocedure Then1;
            Then1.clear();
            if (!set_process_one_block(OutParams[2], MUP, Then1)) return false;
            Then.push_back(Then1);
            if (OutParams.size()==4)
            {
              class CParserRDFprocedure Else1;
              Else1.clear();
              if (!set_process_one_block(OutParams[3], MUP, Else1)) return false;
              Else.push_back(Else1);
            }
            if (!current_proc.add_if(MUP, OutParams[1], ErrorMessage, Then, Else))
            {
              SetLastError(ErrorMessage);
              return false;
            }
          } else {
//            cms_ns_if_print("libparserrdf",1, "WRONG IF (param nums) \"%s\"\n",func_without_brackets.c_str());
            SetLastError("WRONG IF (param nums) \"" + func_without_brackets + "\"");
            return false;
          }
        } else if (OutParams[0] == "case")
        {
          if (OutParams.size()>=3)
          {
            std::vector<class CParserRDFprocedure> Then; Then.clear();
            std::vector<class CParserRDFprocedure> Else; Else.clear();
            std::vector<std::string> func; func.clear();
            unsigned cases = (OutParams.size() - 1) / 2;
            for (unsigned actcase=0; actcase<cases; actcase++)
            {
              class CParserRDFprocedure Then1;
              Then1.clear();
              if (!set_process_one_block(OutParams[1+(actcase*2)+1], MUP, Then1)) return false;
              Then.push_back(Then1);
              func.push_back(OutParams[1+(actcase*2)+0]);
            }
            if (((OutParams.size() - 1) % 2)==1)
            {
              class CParserRDFprocedure Else1;
              Else1.clear();
              if (!set_process_one_block(OutParams[OutParams.size()-1], MUP, Else1)) return false;
              Else.push_back(Else1);
            }
            if (!current_proc.add_case(MUP, func, ErrorMessage, Then, Else))
            {
              SetLastError(ErrorMessage);
              return false;
            }
          } else {
//            cms_ns_if_print("libparserrdf",1, "WRONG CASE (param nums) \"%s\"\n",func_without_brackets.c_str());
            SetLastError("WRONG CASE (param nums) \"" + func_without_brackets + "\"");
            return false;
          }
        } else if (OutParams[0] == "for")
        {
          if (OutParams.size()==5)
          {
            std::vector<class CParserRDFprocedure> Then; Then.clear();
            std::vector<class CParserRDFprocedure> Else; Else.clear();
            class CParserRDFprocedure Then1;
            Then1.clear();
            if (!set_process_one_block(OutParams[4], MUP, Then1)) return false;
            Then.push_back(Then1);
            if (!current_proc.add_for(MUP, OutParams[1],OutParams[2],OutParams[3], ErrorMessage, Then))
            {
              SetLastError(ErrorMessage);
              return false;
            }
          } else {
//            cms_ns_if_print("libparserrdf",1, "WRONG FOR (param nums) \"%s\"\n",func_without_brackets.c_str());
            SetLastError("WRONG FOR (param nums) \"" + func_without_brackets + "\"");
            return false;
          }
        } else if ((OutParams[0] == "undef")||(OutParams[0] == "unset"))
        {
          std::set<std::string> vars; vars.clear();
          for (unsigned p=1; p<OutParams.size(); p++)
          {
            vars.insert(OutParams[p]);
          }
          if (vars.size()>0)
          {
            if (!current_proc.add_undef(MUP, vars, ErrorMessage))
            {
              SetLastError(ErrorMessage);
              return false;
            }
          } else {
//            cms_ns_if_print("libparserrdf",1, "WRONG UNDEF (no parameters, but minimally one is required) \"%s\"\n",func_without_brackets.c_str());
            SetLastError("WRONG UNDEF (no parameters, but minimally one is required) \"" + func_without_brackets + "\"");
            return false;
          }
        } else {
          SetLastError("UNKNOWN COMMAND \"" + OutParams[0] + "\" ::: \"" + func_without_brackets + "\"");
//          cms_ns_if_print("libparserrdf",1, "UNKNOWN COMMAND (%s) \"%s\"", OutParams[0].c_str(), func_without_brackets.c_str());
          return false;
        }
      }
    } else {
      if ((func.size()==func_without_brackets.size()))
      {
        bool Empty;
        if (!current_proc.add_set(MUP, func_without_brackets, ErrorMessage, Empty))
        {
          SetLastError(ErrorMessage);
          return false;
        }
      } else {
      // again
        if (!set_process_one_block(func_without_brackets, MUP, current_proc)) return false;
      }
    }


  }
  return true;
}

bool CParserRDF::set(std::string func, class C_CM_MUP_Interface *MUP, std::map<std::string, class CCmVariable> *GlobalVariables, std::map<std::string, class CCmVariable> *InOutVariables, bool _SaveAndRestoreGlobals, bool _BreakIfRemoveIsTrue)
{
  main.clear();
  SaveAndRestoreGlobals = _SaveAndRestoreGlobals;
  BreakIfRemoveIsTrue = _BreakIfRemoveIsTrue;
  FirstRunParse = true;
  LastError.clear();
  bool RetVal = set_process_one_block(func, MUP, main);
  if (RetVal)
  {
    main.cumulateVars();
    main.optimizeVars(NULL, GlobalVariables, InOutVariables);
  }
  return RetVal;

}


bool CParserRDF::tryLoadVariableInOut(class C_CM_MUP_Interface *MUP, std::map<std::string, class CCmVariable> *InOutVariables,
                                    std::map<std::string, class CCmVariable> *GlobalVariables, std::set<std::string> &LoadedVariables, std::string varname)
{
  if (LoadedVariables.find(varname) != LoadedVariables.end()) return true; // already loaded
  if (InOutVariables!=NULL)
  {
    std::map<std::string, class CCmVariable>::iterator it = InOutVariables->find(varname);
    if (it != InOutVariables->end())
    {
      MUP->SetVariable(&it->second);
      LoadedVariables.insert(varname);
      return true;
    }
  }
  return false;
}




template <typename T> bool CParserRDF::do_Procedure(class CParserRDFprocedure &proc, class C_CM_MUP_Interface *MUP, std::map<std::string, class CCmVariable> *InOutVariables,
                              std::map<std::string, class CCmVariable> *GlobalVariables,
                              std::set<std::string> &LoadedVariables, std::set<int> &LoadedVariables_i, std::set<std::string> &ChangedVariables,std::set<int> &ChangedVariables_i,
                              bool &DoBreak, T &RDFstruct)
{
  for (std::set<std::string>::iterator it = proc.needAddVars.begin(); it != proc.needAddVars.end(); it++)
  {
    std::string varname = *it;
    tryLoadVariableInOut(MUP, InOutVariables, GlobalVariables, LoadedVariables, varname);
    LoadedVariables.insert(varname); // was tried
  }
  for (std::set<int>::iterator it = proc.needAddVars_i.begin(); it != proc.needAddVars_i.end(); it++)
  {
    if (*it >= 0)
    {
      tryLoadVariablesRDF(*it, MUP, LoadedVariables_i, &RDFstruct);
      LoadedVariables_i.insert(*it); // was tried
    }
  }

  unsigned tests;
  for (unsigned i=0;i<proc.AllCMDs.size();i++)
  {
    switch (proc.AllCMDs[i].cmdType)
    {
      case E_CMD_TYPE_SET:
        if (!MUP->Evaluate(proc.AllCMDs[i].Expr))
        {
          SetLastError("MUP::Error (SET plot)" + proc.AllCMDs[i].Expr + " :: " + MUP->GetLastError());
          return false;
        }
        break;
      case E_CMD_TYPE_UNDEF:
        for (std::set<std::string>::iterator it = proc.AllCMDs[i].modifyVars.begin(); it != proc.AllCMDs[i].modifyVars.end(); it++)
        {
          MUP->RemoveVariable(*it);
        }
        for (std::set<int>::iterator it = proc.AllCMDs[i].modifyVars_i.begin(); it != proc.AllCMDs[i].modifyVars_i.end(); it++)
        {
          if (*it >= 0) MUP->RemoveVariable(PARSER_RDF_FixConfigData[*it].Name_MUP);
        }
        break;
      case E_CMD_TYPE_IF:
        tests = MIN(proc.AllCMDs[i].IfExpr.size(), proc.AllCMDs[i].Then.size());
        if (tests>0)
        {
          bool result = false;
          if (!MUP->Evaluate(proc.AllCMDs[i].IfExpr[0], result))
          {
            SetLastError("MUP::Error (IF plot)" + proc.AllCMDs[i].IfExpr[0] + " :: " + MUP->GetLastError());
            return false;
          }
          if (result)
          {
            if (!do_Procedure(proc.AllCMDs[i].Then[0], MUP, InOutVariables, GlobalVariables, LoadedVariables, LoadedVariables_i, ChangedVariables, ChangedVariables_i, DoBreak, RDFstruct)) return false;
          } else if (proc.AllCMDs[i].Else.size()>0)
          {
            if (!do_Procedure(proc.AllCMDs[i].Else[0], MUP, InOutVariables, GlobalVariables, LoadedVariables, LoadedVariables_i, ChangedVariables, ChangedVariables_i, DoBreak, RDFstruct)) return false;
          }
        } else return false;
        if (DoBreak) return true;
        break;
      case E_CMD_TYPE_CASE:
        tests = MIN(proc.AllCMDs[i].IfExpr.size(), proc.AllCMDs[i].Then.size());
        if (tests>0)
        {
          bool result = false;
          for (unsigned u=0;(u<tests)&&(!result);u++)
          {
            if (!MUP->Evaluate(proc.AllCMDs[i].IfExpr[u], result))
            {
              SetLastError("MUP::Error (CASE plot)" + proc.AllCMDs[i].IfExpr[u] + " :: " + MUP->GetLastError());
              return false;
            }
            if (result)
            {
              if (!do_Procedure(proc.AllCMDs[i].Then[u], MUP, InOutVariables, GlobalVariables, LoadedVariables, LoadedVariables_i, ChangedVariables, ChangedVariables_i, DoBreak, RDFstruct)) return false;
            }
          }
          if ((!result)&&(proc.AllCMDs[i].Else.size()>0))
          {
            if (!do_Procedure(proc.AllCMDs[i].Else[0], MUP, InOutVariables, GlobalVariables, LoadedVariables, LoadedVariables_i, ChangedVariables, ChangedVariables_i, DoBreak, RDFstruct)) return false;
          }
        } else return false;
        if (DoBreak) return true;
        break;
      case E_CMD_TYPE_FOR:
        if (!MUP->Evaluate(proc.AllCMDs[i].IfExpr[0])) // start
        {
          SetLastError("MUP::Error (FOR:start plot)" + proc.AllCMDs[i].IfExpr[0] + " :: " + MUP->GetLastError());
          return false;
        }
        bool can_continue = true;
        while (can_continue)
        {
          bool result = false;
          if (!MUP->Evaluate(proc.AllCMDs[i].IfExpr[1], result))
          {
            SetLastError("MUP::Error (FOR:cond plot)" + proc.AllCMDs[i].IfExpr[1] + " :: " + MUP->GetLastError());
            return false;
          }
          if (!result) can_continue = false;
          if (can_continue)
          {
            if (!do_Procedure(proc.AllCMDs[i].Then[0], MUP, InOutVariables, GlobalVariables, LoadedVariables, LoadedVariables_i, ChangedVariables, ChangedVariables_i, DoBreak, RDFstruct)) return false;
            if (DoBreak) return true;
            if (!MUP->Evaluate(proc.AllCMDs[i].IfExpr[2])) // increment
            {
              SetLastError("MUP::Error (FOR:incr plot)" + proc.AllCMDs[i].IfExpr[2] + " :: " + MUP->GetLastError());
              return false;
            }
          }
        }
        if (DoBreak) return true;
        break;
    }

    for (std::set<std::string>::iterator it = proc.AllCMDs[i].modifyVars.begin(); it != proc.AllCMDs[i].modifyVars.end(); it++)
    {
      ChangedVariables.insert(*it);
      if ((BreakIfRemoveIsTrue)&&(*it == "REMOVE")&&(InOutVariables!=NULL))
      {
        std::map<std::string, class CCmVariable>::iterator itv = InOutVariables->find(*it);
        if (itv != InOutVariables->end())
        {
          MUP->GetVariable(&itv->second);
          if (itv->second.getVariableBool(DoBreak))
          {
          }
          if (DoBreak) return true;
        }
      }
    }
  }
  for (std::set<int>::iterator it = proc.modifyVars_i.begin(); it != proc.modifyVars_i.end(); it++)
  {
    ChangedVariables_i.insert(*it);
  }
  return true;
}







template <typename T> bool CParserRDF::do_parse(T &RDFstruct, class C_CM_MUP_Interface *MUP, std::map<std::string, class CCmVariable> *InOutVariables, std::map<std::string, class CCmVariable> *GlobalVariables)
{
  std::set<std::string> LoadedVariables;
  std::set<std::string> ChangedVariables;
  std::set<int> LoadedVariables_i;
  std::set<int> ChangedVariables_i;
//  parse_Start(MUP, InOutVariables, GlobalVariables, LoadedVariables, ChangedVariables);

  if (GlobalVariables != NULL)
  {
    if ((SaveAndRestoreGlobals) || (FirstRunParse))
    {
      for (std::map<std::string, class CCmVariable>::iterator it = GlobalVariables->begin(); it != GlobalVariables->end(); it++)
      {
        MUP->SetVariable(&it->second);
      }
      if (!SaveAndRestoreGlobals)
      {
        FixLoadedGlobalVariables = *GlobalVariables;
        FirstRunParse = false;
      }
    }
  }


  bool DoBreak = false;

  bool RetVal = do_Procedure(main, MUP, InOutVariables, GlobalVariables, LoadedVariables, LoadedVariables_i, ChangedVariables, ChangedVariables_i, DoBreak, RDFstruct);

  if (RetVal)
  {
//    parse_Stop1(MUP, InOutVariables, GlobalVariables, LoadedVariables, LoadedVariables_i, ChangedVariables, ChangedVariables_i);

  // save globals
    for (std::set<std::string>::iterator it = ChangedVariables.begin(); it != ChangedVariables.end(); it++)
    {
      LoadedVariables.insert(*it);
    }
    for (std::set<int>::iterator it = ChangedVariables_i.begin(); it != ChangedVariables_i.end(); it++)
    {
      LoadedVariables_i.insert(*it);
    }

    if (GlobalVariables != NULL)
    {
      if (SaveAndRestoreGlobals)
      {
        for (std::map<std::string, class CCmVariable>::iterator it = GlobalVariables->begin(); it != GlobalVariables->end(); it++)
        {
          std::string name = it->second.getName();
          if (ChangedVariables.find(name) != ChangedVariables.end())
          {
            MUP->GetVariable(&it->second);
          }
        }
      }
    }
    // save in/out
    if (InOutVariables!=NULL)
    {
      for (std::map<std::string, class CCmVariable>::iterator it = InOutVariables->begin(); it != InOutVariables->end(); it++)
      {
        if (ChangedVariables.find(it->second.getName()) != ChangedVariables.end())
        {
          MUP->GetVariable(&it->second);
        }
      }
    }

    // save RDF
    while (!ChangedVariables_i.empty())
    {
      std::set<int>::iterator firstIt = ChangedVariables_i.begin();
      int chVar = *(firstIt);
      ChangedVariables_i.erase(firstIt);
      if (chVar >= 0) tryStoreVariableRDF(chVar, MUP, ChangedVariables_i, &RDFstruct);
    }
//    parse_Stop2(MUP, InOutVariables, GlobalVariables, LoadedVariables, LoadedVariables_i, ChangedVariables, ChangedVariables_i);

    for (std::set<int>::iterator it = LoadedVariables_i.begin(); it != LoadedVariables_i.end(); it++)
    {
      if (*it >= 0)
      {
        MUP->RemoveVariable(PARSER_RDF_FixConfigData[*it].Name_MUP);
      }
    }
    for (std::set<std::string>::iterator it = LoadedVariables.begin(); it != LoadedVariables.end(); it++)
    {
      bool rem = true;
      if ((!SaveAndRestoreGlobals)&&(FixLoadedGlobalVariables.size()>0))
      {
        if (FixLoadedGlobalVariables.find(*it) != FixLoadedGlobalVariables.end()) rem = false;
      }
      if (rem) MUP->RemoveVariable(*it);
    }

  }
  return RetVal;
}



void CParserRDF::get_globals(class C_CM_MUP_Interface *MUP, std::map<std::string, class CCmVariable> *GlobalVariables)
{
  if (!SaveAndRestoreGlobals)
  {
    if (GlobalVariables!=NULL)
    {
      for (std::map<std::string, class CCmVariable>::iterator it = GlobalVariables->begin(); it != GlobalVariables->end(); it++)
      {
        MUP->GetVariable(&it->second);
      }
    }
  } else {
    // impossible , all was erased
  }
}

std::string CParserRDF::print_me(unsigned verbosity)
{
  std::string RetVal = std::string();
  RetVal += main.print_me(verbosity, "", "    ");
  return RetVal;
}

void CParserRDF::SetLastError(std::string Inp)
{
  LastError = Inp;
//  cms_ns_if_print("libparserrdf",1, "%s",Inp.c_str());
}

std::string CParserRDF::GetLastError(void)
{
  std::string RetVal = LastError;
  LastError.clear();
  return RetVal;
}

// ------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------------------------------------
// ************************************************************  RDFRadarService ************************************************************

bool CParserRDF::parse(RDFRadarService &RDFstruct, class C_CM_MUP_Interface *MUP, std::map<std::string, class CCmVariable> *InOutVariables, std::map<std::string, class CCmVariable> *GlobalVariables)
{
  return do_parse(RDFstruct, MUP, InOutVariables, GlobalVariables);
}

bool CParserRDF::tryLoadVariablesRDF(int var_i, class C_CM_MUP_Interface *MUP, std::set<int> &LoadedVariables_i, RDFRadarService  *RDFstruct)
{
      if (var_i<0) return false;

      if (var_i == PARSER_RDF_ID_SIC)
      {
        MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,(int)RDFstruct->SIC);
        return true;
      }
      if (var_i == PARSER_RDF_ID_SAC)
      {
        MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,(int)RDFstruct->SAC);
        return true;
      }
      if (var_i == PARSER_RDF_ID_TOD)
      {
        if (RDFstruct->Time.Present) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,((double)RDFstruct->Time.Time) / 1000.);
        return true;
      }
      if (var_i == PARSER_RDF_ID_ServiceTYPE)
      {
        int SType = 0;
             if (RDFstruct->Type == RADAR_SERVICE_TYPE_NORTH ) SType = 1;
        else if (RDFstruct->Type == RADAR_SERVICE_TYPE_SECTOR) SType = 2;
        MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, SType);
        return true;
      }
      if (RDFstruct->Type == RADAR_SERVICE_TYPE_NORTH )
      {
        if (var_i == PARSER_RDF_ID_NORTH_POS3D_LAT)
        {
          if (RDFstruct->Position3D != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, (RDFstruct->Position3D->Latitude  * (180.0 / M_PI)));
          return true;
        }
        if (var_i == PARSER_RDF_ID_NORTH_POS3D_LON)
        {
          if (RDFstruct->Position3D != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, (RDFstruct->Position3D->Longitude * (180.0 / M_PI)));
          return true;
        }
        if (var_i == PARSER_RDF_ID_NORTH_POS3D_ALT)
        {
          if (RDFstruct->Position3D != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, RDFstruct->Position3D->Height);
          return true;
        }
        if (var_i == PARSER_RDF_ID_NORTH_ROTSPD)
        {
          if (RDFstruct->AntennaRotation != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, RDFstruct->AntennaRotation->Speed);
          return true;
        }

      } else if (RDFstruct->Type == RADAR_SERVICE_TYPE_SECTOR )
      {
        if (var_i == PARSER_RDF_ID_AZ)
        {
          MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, (RDFstruct->Sector * (180.0 / M_PI)));
          return true;
        }
      }

  return false;
}

bool CParserRDF::tryStoreVariableRDF(int var_i, class C_CM_MUP_Interface *MUP, std::set<int> &ChangedVariables_i, RDFRadarService *RDFstruct)
{
      if (var_i<0) return false;

      if (var_i == PARSER_RDF_ID_SIC)
      {
        int val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->SIC = val;
        } else {
          RDFstruct->SIC = 0; // undefined
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_SAC)
      {
        int val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->SAC = val;
        } else {
          RDFstruct->SAC = 0; // undefined
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_TOD)
      {
        double val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->Time.Present = 1;
          RDFstruct->Time.Time = (unsigned)((val * 1000.) + 0.5);
        } else {
          RDFstruct->Time.Present = 0;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_ServiceTYPE)
      {
        int val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
               if (val == 1) RDFstruct->Type = RADAR_SERVICE_TYPE_NORTH;
          else if (val == 2) RDFstruct->Type = RADAR_SERVICE_TYPE_SECTOR;
        } else {
          // undefined
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_NORTH_ROTSPD)
      {
        double val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->AntennaRotation == NULL) RDFstruct->AntennaRotation = new RDFAntennaRotation;
          RDFstruct->AntennaRotation->Speed = val;
        } else {
          if (RDFstruct->AntennaRotation) delete RDFstruct->AntennaRotation;
          RDFstruct->AntennaRotation = NULL;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_NORTH_POS3D_LAT)
      {
        double val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->Position3D == NULL) RDFstruct->Position3D = new RDFPosition3D;
          RDFstruct->Position3D->Latitude = val / (180.0 / M_PI);
        } else {
          if (RDFstruct->Position3D) delete RDFstruct->Position3D;
          RDFstruct->Position3D = NULL;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_NORTH_POS3D_LON)
      {
        double val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->Position3D == NULL) RDFstruct->Position3D = new RDFPosition3D;
          RDFstruct->Position3D->Longitude = val / (180.0 / M_PI);
        } else {
          if (RDFstruct->Position3D) delete RDFstruct->Position3D;
          RDFstruct->Position3D = NULL;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_NORTH_POS3D_ALT)
      {
        double val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->Position3D == NULL) RDFstruct->Position3D = new RDFPosition3D;
          RDFstruct->Position3D->Height = val ;
        } else {
          if (RDFstruct->Position3D) delete RDFstruct->Position3D;
          RDFstruct->Position3D = NULL;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_AZ)
      {
        double val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->Sector = val / (180.0 / M_PI);
        } else {
          // undefined
        }
        return true;
      }


   return false;
}

// ------------------------------------------------------------------------------------------------------------------------------------------
// ************************************************************  RDFTargetPlot   ************************************************************

bool CParserRDF::parse(RDFTargetPlot   &RDFstruct, class C_CM_MUP_Interface *MUP, std::map<std::string, class CCmVariable> *InOutVariables, std::map<std::string, class CCmVariable> *GlobalVariables)
{
  return do_parse(RDFstruct, MUP, InOutVariables, GlobalVariables);
}

bool CParserRDF::tryLoadVariablesRDF(int var_i, class C_CM_MUP_Interface *MUP, std::set<int> &LoadedVariables_i, RDFTargetPlot  *RDFstruct)
{
      if (var_i<0) return false;

      if (var_i == PARSER_RDF_ID_SIC)
      {
        MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, (int)RDFstruct->SIC);
        return true;
      }

      if (var_i == PARSER_RDF_ID_SIC)
      {
        MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,(int)RDFstruct->SIC);
        return true;
      }

      if (var_i == PARSER_RDF_ID_SAC)
      {
        MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,(int)RDFstruct->SAC);
        return true;
      }
      if (var_i == PARSER_RDF_ID_TOD)
      {
        if (RDFstruct->Time.Present) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,((double)RDFstruct->Time.Time) / 1000.);
        return true;
      }
      if (var_i == PARSER_RDF_ID_M3A)
      {
        if (RDFstruct->Mode3A != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (((RDFstruct->Mode3A->Code >> 0) & 0x7) << 0) | (((RDFstruct->Mode3A->Code >> 3) & 0x7) << 4) |
                                     (((RDFstruct->Mode3A->Code >> 6) & 0x7) << 8) | (((RDFstruct->Mode3A->Code >> 9) & 0x7) << 12));
        return true;
      }
      if (var_i == PARSER_RDF_ID_M3A_G)
      {
        if (RDFstruct->Mode3A != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->Mode3A->Garbled ? true : false));
        return true;
      }
      if (var_i == PARSER_RDF_ID_M3A_I)
      {
        if (RDFstruct->Mode3A != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->Mode3A->Valid   ? false : true));
        return true;
      }
      if (var_i == PARSER_RDF_ID_M3A_T)
      {
        if (RDFstruct->Mode3A != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->Mode3A->Tracked ? true : false));
        return true;
      }
      if (var_i == PARSER_RDF_ID_MC)
      {
        if (RDFstruct->ModeC  != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,   (((RDFstruct->ModeC->Code >> 0) & 0x7) << 0) | (((RDFstruct->ModeC->Code >> 3) & 0x7) << 4) |
                                     (((RDFstruct->ModeC->Code >> 6) & 0x7) << 8) | (((RDFstruct->ModeC->Code >> 9) & 0x7) << 12));
        return true;
      }
      if (var_i == PARSER_RDF_ID_MC_G)
      {
        if (RDFstruct->ModeC != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->ModeC->Garbled ? true : false));
        return true;
      }
      if (var_i == PARSER_RDF_ID_MC_I)
      {
        if (RDFstruct->ModeC != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->ModeC->Valid   ? false : true));
        return true;
      }
      if (var_i == PARSER_RDF_ID_MC_T)
      {
        if (RDFstruct->ModeC != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->ModeC->Tracked ? true : false));
        return true;
      }
      if (var_i == PARSER_RDF_ID_M1)
      {
        if (RDFstruct->Mode1  != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,   (((RDFstruct->Mode1->Code >> 0) & 0x7) << 0) | (((RDFstruct->Mode1->Code >> 3) & 0x7) << 4) |
                                     (((RDFstruct->Mode1->Code >> 6) & 0x7) << 8) | (((RDFstruct->Mode1->Code >> 9) & 0x7) << 12));
        return true;
      }
      if (var_i == PARSER_RDF_ID_M1_G)
      {
        if (RDFstruct->Mode1 != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->Mode1->Garbled ? true : false));
        return true;
      }
      if (var_i == PARSER_RDF_ID_M1_I)
      {
        if (RDFstruct->Mode1 != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->Mode1->Valid   ? false : true));
        return true;
      }
      if (var_i == PARSER_RDF_ID_M1_T)
      {
        if (RDFstruct->Mode1 != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->Mode1->Tracked ? true : false));
        return true;
      }
      if (var_i == PARSER_RDF_ID_M2)
      {
        if (RDFstruct->Mode2  != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,   (((RDFstruct->Mode2->Code >> 0) & 0x7) << 0) | (((RDFstruct->Mode2->Code >> 3) & 0x7) << 4) |
                                     (((RDFstruct->Mode2->Code >> 6) & 0x7) << 8) | (((RDFstruct->Mode2->Code >> 9) & 0x7) << 12));
        return true;
      }
      if (var_i == PARSER_RDF_ID_M2_G)
      {
        if (RDFstruct->Mode2 != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->Mode2->Garbled ? true : false));
        return true;
      }
      if (var_i == PARSER_RDF_ID_M2_I)
      {
        if (RDFstruct->Mode2 != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->Mode2->Valid   ? false : true));
        return true;
      }
      if (var_i == PARSER_RDF_ID_M2_T)
      {
        if (RDFstruct->Mode2 != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->Mode2->Tracked ? true : false));
        return true;
      }
      if (var_i == PARSER_RDF_ID_FL)
      {
        if (RDFstruct->FlightLevel  != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  RDFstruct->FlightLevel->Height * ((1 / 0.3048) * 0.01));
        return true;
      }
      if (var_i == PARSER_RDF_ID_FL_G)
      {
        if (RDFstruct->FlightLevel != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->FlightLevel->Garbled ? true : false));
        return true;
      }
      if (var_i == PARSER_RDF_ID_FL_I)
      {
        if (RDFstruct->FlightLevel != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->FlightLevel->Valid   ? false : true));
        return true;
      }
      if (var_i == PARSER_RDF_ID_FL_25)
      {
        if (RDFstruct->CommCapabilityAndFlightStatus != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (bool)(RDFstruct->CommCapabilityAndFlightStatus->AltitudeCapability ? true : false));
        return true;
      }
      if (var_i == PARSER_RDF_ID_RNG)
      {
        if (RDFstruct->Polar != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->Polar->Rho / 1852.));
        return true;
      }
      if (var_i == PARSER_RDF_ID_AZ)
      {
        if (RDFstruct->Polar != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->Polar->Theta * (180. / M_PI)));
        return true;
      }
      if (var_i == PARSER_RDF_ID_FlagTST)
      {
        MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->Tested ? true : false));
        return true;
      }
      if (var_i == PARSER_RDF_ID_FlagSIM)
      {
        MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->Simulated ? true : false));
      }
      if (var_i == PARSER_RDF_ID_FlagSPI)
      {
        MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->SPI ? true : false));
      }
      if (var_i == PARSER_RDF_ID_FlagRAB)
      {
        MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->RAB ? true : false));
      }
      if (var_i == PARSER_RDF_ID_SADDR)
      {
        if (RDFstruct->AircraftAddress.Present) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (int)RDFstruct->AircraftAddress.Address);
      }
      if (var_i == PARSER_RDF_ID_CallSign)
      {
        if (!RDFstruct->AircraftIdentification.empty()) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  RDFstruct->AircraftIdentification);
        return true;
      }
      if (var_i == PARSER_RDF_ID_UserText)
      {
        if (!RDFstruct->UserText.empty()) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  RDFstruct->UserText);
        return true;
      }
      if (var_i == PARSER_RDF_ID_SSR_LEN)
      {
        if ((RDFstruct->TargetCharacteristics) && (RDFstruct->TargetCharacteristics->SsrRunlengthPresent))
            MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, ((double)(int)RDFstruct->TargetCharacteristics->SsrRunlength) * (360.0 / 8192.0));
        return true;
      }
      if (var_i == PARSER_RDF_ID_SSR_REP)
      {
        if ((RDFstruct->TargetCharacteristics) && (RDFstruct->TargetCharacteristics->SsrRepliesPresent))
            MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, (int)RDFstruct->TargetCharacteristics->SsrReplies);
        return true;
      }
      if (var_i == PARSER_RDF_ID_SSR_AMP)
      {
        if ((RDFstruct->TargetCharacteristics) && (RDFstruct->TargetCharacteristics->SsrAmplitudePresent))
            MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, (double)(int)RDFstruct->TargetCharacteristics->SsrAmplitude);
        return true;
      }
      if (var_i == PARSER_RDF_ID_PSR_LEN)
      {
        if ((RDFstruct->TargetCharacteristics) && (RDFstruct->TargetCharacteristics->PsrRunlengthPresent))
            MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, ((double)(int)RDFstruct->TargetCharacteristics->PsrRunlength) * (360.0 / 8192.0));
        return true;
      }
      if (var_i == PARSER_RDF_ID_PSR_AMP)
      {
        if ((RDFstruct->TargetCharacteristics) && (RDFstruct->TargetCharacteristics->PsrAmplitudePresent))
            MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, (double)(int)RDFstruct->TargetCharacteristics->PsrAmplitude);
        return true;
      }
      if (var_i == PARSER_RDF_ID_PSR_DIFF_AZ)
      {
        if ((RDFstruct->TargetCharacteristics) && (RDFstruct->TargetCharacteristics->PsrSsrAzimuthDiffPresent))
            MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, (int)RDFstruct->TargetCharacteristics->PsrSsrAzimuthDiff);
        return true;
      }
      if (var_i == PARSER_RDF_ID_PSR_DIFF_RNG)
      {
        if ((RDFstruct->TargetCharacteristics) && (RDFstruct->TargetCharacteristics->PsrSsrRangeDiffPresent))
            MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, (int)RDFstruct->TargetCharacteristics->PsrSsrRangeDiff);
        return true;
      }
      if (var_i == PARSER_RDF_ID_PSR_Doppler)
      {
        if (RDFstruct->DopplerSpeed.RawPresent) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, (double)  RDFstruct->DopplerSpeed.DopplerSpeed);
        return true;
      }
      if (var_i == PARSER_RDF_ID_PSR_CalcDoppler)
      {
        if (RDFstruct->DopplerSpeed.CalculatedPresent) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, (double)  RDFstruct->DopplerSpeed.CalculatedSpeed);
        return true;
      }
      if (var_i == PARSER_RDF_ID_PSR_CalcDoppler_V)
      {
        if (RDFstruct->DopplerSpeed.CalculatedPresent) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->DopplerSpeed.CalculatedValid ? true : false));
        return true;
      }

      if (var_i == PARSER_RDF_ID_MDS_MSP)
      {
        if (RDFstruct->CommCapabilityAndFlightStatus!=NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->CommCapabilityAndFlightStatus->ModeSSpecificService ? true : false));
        return true;
      }
      if (var_i == PARSER_RDF_ID_MDS_Capa_CS)
      {
        if (RDFstruct->CommCapabilityAndFlightStatus!=NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->CommCapabilityAndFlightStatus->IdentificationCapability ? true : false));
        return true;
      }
      if (var_i == PARSER_RDF_ID_MDS_ACAS)
      {
        if (RDFstruct->CommCapabilityAndFlightStatus!=NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->CommCapabilityAndFlightStatus->ACASOperational ? true : false));
        return true;
      }
      if (var_i == PARSER_RDF_ID_MDS_Level)
      {
        if (RDFstruct->CommCapabilityAndFlightStatus!=NULL)
        {
          int val = 1;
          switch (RDFstruct->CommCapabilityAndFlightStatus->CommCapability)
          {
            case COMM_CAPABILITY_NONE         : val = 1; break;
            case COMM_CAPABILITY_A_B          : val = 2; break;
            case COMM_CAPABILITY_A_B_ELMU     : val = 3; break;
            case COMM_CAPABILITY_A_B_ELMU_ELMD: val = 4; break;
            case COMM_CAPABILITY_LEVEL_5      : val = 5; break;
          }
          MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  val);
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_MDS_FS)
      {
        if (RDFstruct->CommCapabilityAndFlightStatus!=NULL)
        {
          int val = 0;
          switch (RDFstruct->CommCapabilityAndFlightStatus->FlightStatus)
          {
            case FLIGHT_STATUS_AIRBORNE             : val = 0; break;
            case FLIGHT_STATUS_ON_GROUND            : val = 1; break;
            case FLIGHT_STATUS_AIRBORNE_ALERT       : val = 2; break;
            case FLIGHT_STATUS_ON_GROUND_ALERT      : val = 3; break;
            case FLIGHT_STATUS_ALERT_SPI            : val = 4; break;
            case FLIGHT_STATUS_SPI                  : val = 5; break;
            case FLIGHT_STATUS_GENERAL_EMERGENCY    : val = 6; break;
            case FLIGHT_STATUS_LIFEGUARD            : val = 7; break;
            case FLIGHT_STATUS_MINIMUM_FUEL         : val = 8; break;
            case FLIGHT_STATUS_NO_COMMUNICATIONS    : val = 9; break;
            case FLIGHT_STATUS_UNLAWFUL_INTERFERENCE: val =10; break;
          }
          MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  val);
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_TargetTYPE)
      {
        int val = 0;
        switch (RDFstruct->TargetType)
        {
          case TARGET_TYPE_UNKNOWN            : val = 0; break;
          case TARGET_TYPE_PRIMARY            : val = 1; break;
          case TARGET_TYPE_SECONDARY          : val = 2; break;
          case TARGET_TYPE_COMBINED           : val = 3; break;
          case TARGET_TYPE_MODES_ALL_CALL     : val = 4; break;
          case TARGET_TYPE_MODES_ROLL_CALL    : val = 5; break;
          case TARGET_TYPE_MODES_ALL_CALL_PSR : val = 6; break;
          case TARGET_TYPE_MODES_ROLL_CALL_PSR: val = 7; break;
          case TARGET_TYPE_ADSB               : val = 8; break;
        }
        MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, val);
        return true;
      }
      if (var_i == PARSER_RDF_ID_WE)
      {
        std::deque<bool> WE;
        unsigned WE_size = 256; // 24 by melo stacit
        WE.resize(WE_size);
        for (unsigned i=0; i<WE_size; i++) WE[i]=false;
        for (unsigned i=0; i<RDFstruct->WarningError.size(); i++)
        {
          if (RDFstruct->WarningError[i] < WE_size) WE[RDFstruct->WarningError[i]]=true;
        }
        MUP->SetVariable(PARSER_RDF_FixConfigData[PARSER_RDF_ID_WE   ].Name_MUP,  WE, WE_size);
      }
      if ((var_i == PARSER_RDF_ID_BDS)||(var_i == PARSER_RDF_ID_BDSage))
      {
        if (var_i == PARSER_RDF_ID_BDS   ) LoadedVariables_i.insert(PARSER_RDF_ID_BDSage);
        if (var_i == PARSER_RDF_ID_BDSage) LoadedVariables_i.insert(PARSER_RDF_ID_BDS   );

        // SET it allways, if need BDS, because if target is without BDS, but need SET new, array must be prepared

//        if ((!RDFstruct->ModeS.empty()) || (!RDFstruct->ModeSAge.empty()) || (RDFstruct->ResolutionAdvisory.Present))
//        {

//          std::deque<uint64_t> BDS;
          UIntList BDS;
          std::deque<int> BDS_AGE;

          BDS.resize(256);
          BDS_AGE.resize(256);
          for (unsigned i=0;i<256;i++)
          {
            BDS[i]=(uint64_t)1<<57;
            BDS_AGE[i]=-1;
          }

          for (unsigned i=0;i<RDFstruct->ModeS.size();i++)
          {
            unsigned char Addr = RDFstruct->ModeS[i].Address;
            unsigned char *MB = RDFstruct->ModeS[i].MessageData;
            uint64_t val = (((uint64_t)(MB[0]))<<48) | (((uint64_t)(MB[1]))<<40) | (((uint64_t)(MB[2]))<<32) | (((uint64_t)(MB[3]))<<24) |
                           (((uint64_t)(MB[4]))<<16) | (((uint64_t)(MB[5]))<< 8) | (((uint64_t)(MB[6]))<< 0) ;
            BDS[Addr] = val;
          }

          if (RDFstruct->ResolutionAdvisory.Present)
          {
            unsigned char Addr = 0x30;
            unsigned char *MB = RDFstruct->ResolutionAdvisory.Data;
            uint64_t val = (((uint64_t)(MB[0]))<<48) | (((uint64_t)(MB[1]))<<40) | (((uint64_t)(MB[2]))<<32) | (((uint64_t)(MB[3]))<<24) |
                           (((uint64_t)(MB[4]))<<16) | (((uint64_t)(MB[5]))<< 8) | (((uint64_t)(MB[6]))<< 0) ;
            BDS[Addr] = val;
          }

          for (unsigned i=0;i<RDFstruct->ModeSAge.size();i++)
          {
            unsigned char Addr = RDFstruct->ModeSAge[i].Address;
            unsigned char *MB = RDFstruct->ModeSAge[i].MessageData;
            uint64_t val = (((uint64_t)(MB[0]))<<48) | (((uint64_t)(MB[1]))<<40) | (((uint64_t)(MB[2]))<<32) | (((uint64_t)(MB[3]))<<24) |
                           (((uint64_t)(MB[4]))<<16) | (((uint64_t)(MB[5]))<< 8) | (((uint64_t)(MB[6]))<< 0) ;
            BDS[Addr] = val;
            BDS_AGE[Addr] = RDFstruct->ModeSAge[i].Age;
          }

//        if (!RDFstruct->AircraftIdentification.empty()) 
          MUP->SetVariable(PARSER_RDF_FixConfigData[PARSER_RDF_ID_BDS   ].Name_MUP,  BDS, 256);
          MUP->SetVariable(PARSER_RDF_FixConfigData[PARSER_RDF_ID_BDSage].Name_MUP,  BDS_AGE, 256);
//        }
        return true;
      }
  return false;
}

bool CParserRDF::tryStoreVariableRDF(int var_i, class C_CM_MUP_Interface *MUP, std::set<int> &ChangedVariables_i, RDFTargetPlot *RDFstruct)
{
      if (var_i<0) return false;

      if (var_i == PARSER_RDF_ID_SIC)
      {
        int val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->SIC = val;
        } else {
          RDFstruct->SIC = 0; // undefined
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_SAC)
      {
        int val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->SAC = val;
        } else {
          RDFstruct->SAC = 0; // undefined
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_TOD)
      {
        double val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->Time.Present = 1;
          RDFstruct->Time.Time = (unsigned)((val * 1000.) + 0.5);
        } else {
          RDFstruct->Time.Present = 0;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_TargetTYPE)
      {
        int val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
               if (val == 1) RDFstruct->TargetType = TARGET_TYPE_PRIMARY;
          else if (val == 2) RDFstruct->TargetType = TARGET_TYPE_SECONDARY;
          else if (val == 3) RDFstruct->TargetType = TARGET_TYPE_COMBINED;
          else if (val == 4) RDFstruct->TargetType = TARGET_TYPE_MODES_ALL_CALL;
          else if (val == 5) RDFstruct->TargetType = TARGET_TYPE_MODES_ROLL_CALL;
          else if (val == 6) RDFstruct->TargetType = TARGET_TYPE_MODES_ALL_CALL_PSR;
          else if (val == 7) RDFstruct->TargetType = TARGET_TYPE_MODES_ROLL_CALL_PSR;
          else if (val == 8) RDFstruct->TargetType = TARGET_TYPE_ADSB;
          else               RDFstruct->TargetType = TARGET_TYPE_UNKNOWN;
        } else {
          RDFstruct->TargetType = TARGET_TYPE_UNKNOWN; // undefined
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_M3A)
      {
        int val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->Mode3A == NULL) RDFstruct->Mode3A = new RDFTargetMode();
          RDFstruct->Mode3A->Code = (((val >> 0) & 0x7) << 0) | (((val >> 4) & 0x7) << 3) | (((val >> 8) & 0x7) << 6) | (((val >> 12) & 0x7) << 9);
        } else {
          if (RDFstruct->Mode3A != NULL) delete RDFstruct->Mode3A;
          RDFstruct->Mode3A = NULL;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_M3A_T)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->Mode3A != NULL) RDFstruct->Mode3A->Tracked = val;
        } else {
          if (RDFstruct->Mode3A != NULL) RDFstruct->Mode3A->Tracked = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_M3A_I)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->Mode3A != NULL) RDFstruct->Mode3A->Valid = (val ? false : true);
        } else {
          if (RDFstruct->Mode3A != NULL) RDFstruct->Mode3A->Valid = true;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_MC)
      {
        int val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->ModeC == NULL) RDFstruct->ModeC = new RDFTargetMode();
          RDFstruct->ModeC->Code = (((val >> 0) & 0x7) << 0) | (((val >> 4) & 0x7) << 3) | (((val >> 8) & 0x7) << 6) | (((val >> 12) & 0x7) << 9);
        } else {
          if (RDFstruct->ModeC != NULL) delete RDFstruct->ModeC;
          RDFstruct->ModeC = NULL;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_MC_G)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->ModeC != NULL) RDFstruct->ModeC->Garbled = val;
        } else {
          if (RDFstruct->ModeC != NULL) RDFstruct->ModeC->Garbled = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_MC_T)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->ModeC != NULL) RDFstruct->ModeC->Tracked = val;
        } else {
          if (RDFstruct->ModeC != NULL) RDFstruct->ModeC->Tracked = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_MC_I)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->ModeC != NULL) RDFstruct->ModeC->Valid = (val ? false : true);
        } else {
          if (RDFstruct->ModeC != NULL) RDFstruct->ModeC->Valid = true;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_M1)
      {
        int val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->Mode1== NULL) RDFstruct->Mode1 = new RDFTargetMode();
          RDFstruct->Mode1->Code = (((val >> 0) & 0x7) << 0) | (((val >> 4) & 0x7) << 3) | (((val >> 8) & 0x7) << 6) | (((val >> 12) & 0x7) << 9);
        } else {
          if (RDFstruct->Mode1 != NULL) delete RDFstruct->Mode1;
          RDFstruct->Mode1 = NULL;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_M1_G)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->Mode1 != NULL) RDFstruct->Mode1->Garbled = val;
        } else {
          if (RDFstruct->Mode1 != NULL) RDFstruct->Mode1->Garbled = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_M1_T)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->Mode1 != NULL) RDFstruct->Mode1->Tracked = val;
        } else {
          if (RDFstruct->Mode1 != NULL) RDFstruct->Mode1->Tracked = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_M1_I)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->Mode1 != NULL) RDFstruct->Mode1->Valid = (val ? false : true);
        } else {
          if (RDFstruct->Mode1 != NULL) RDFstruct->Mode1->Valid = true;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_M2)
      {
        int val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->Mode2== NULL) RDFstruct->Mode2 = new RDFTargetMode();
          RDFstruct->Mode2->Code = (((val >> 0) & 0x7) << 0) | (((val >> 4) & 0x7) << 3) | (((val >> 8) & 0x7) << 6) | (((val >> 12) & 0x7) << 9);
        } else {
          if (RDFstruct->Mode2 != NULL) delete RDFstruct->Mode2;
          RDFstruct->Mode2 = NULL;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_M2_G)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->Mode2 != NULL) RDFstruct->Mode2->Garbled = val;
        } else {
          if (RDFstruct->Mode2 != NULL) RDFstruct->Mode2->Garbled = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_M2_T)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->Mode2 != NULL) RDFstruct->Mode2->Tracked = val;
        } else {
          if (RDFstruct->Mode2 != NULL) RDFstruct->Mode2->Tracked = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_M2_I)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->Mode2 != NULL) RDFstruct->Mode2->Valid = (val ? false : true);
        } else {
          if (RDFstruct->Mode2 != NULL) RDFstruct->Mode2->Valid = true;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_FL)
      {
        double val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->FlightLevel== NULL) RDFstruct->FlightLevel = new RDFTargetFlightLevel();
          RDFstruct->FlightLevel->Height = val / ((1 / 0.3048) * 0.01);
        } else {
          if (RDFstruct->FlightLevel != NULL) delete RDFstruct->FlightLevel;
          RDFstruct->FlightLevel = NULL;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_FL_G)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->FlightLevel != NULL) RDFstruct->FlightLevel->Garbled = val;
        } else {
          if (RDFstruct->FlightLevel != NULL) RDFstruct->FlightLevel->Garbled = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_FL_I)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->FlightLevel != NULL) RDFstruct->FlightLevel->Valid = (val ? false : true);
        } else {
          if (RDFstruct->FlightLevel != NULL) RDFstruct->FlightLevel->Valid = true;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_FL_25)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->CommCapabilityAndFlightStatus == NULL) RDFstruct->CommCapabilityAndFlightStatus = new RDFCommCapabilityAndFlightStatus();
          RDFstruct->CommCapabilityAndFlightStatus->AltitudeCapability = val;
        } else {
          if (RDFstruct->CommCapabilityAndFlightStatus != NULL) RDFstruct->CommCapabilityAndFlightStatus->AltitudeCapability = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_FlagTST)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->Tested = (val ? 1 : 0);
        } else {
          RDFstruct->Tested = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_FlagSIM)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->Simulated = (val ? 1 : 0);
        } else {
          RDFstruct->Simulated = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_FlagSPI)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->SPI = (val ? 1 : 0);
        } else {
          RDFstruct->SPI = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_FlagRAB)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->RAB = (val ? 1 : 0);
        } else {
          RDFstruct->RAB = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_SSR_LEN)
      {
        double valD;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,valD))
        {
          if (!RDFstruct->TargetCharacteristics) RDFstruct->TargetCharacteristics = new RDFTargetCharacteristics;
          RDFstruct->TargetCharacteristics->SsrRunlengthPresent = true;
          int val = (valD / (360.0 / 8192.0)) + 0.5;
          RDFstruct->TargetCharacteristics->SsrRunlength = MIN(MAX(val,255),0);
        } else {
          RDFstruct->RAB = false;
          if (RDFstruct->TargetCharacteristics) RDFstruct->TargetCharacteristics->SsrRunlengthPresent = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_SSR_REP)
      {
        int val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (!RDFstruct->TargetCharacteristics) RDFstruct->TargetCharacteristics = new RDFTargetCharacteristics;
          RDFstruct->TargetCharacteristics->SsrRepliesPresent = true;
          RDFstruct->TargetCharacteristics->SsrReplies = MIN(MAX(val,255),0);
        } else {
          RDFstruct->RAB = false;
          if (RDFstruct->TargetCharacteristics) RDFstruct->TargetCharacteristics->SsrRepliesPresent = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_SSR_AMP)
      {
        double valD;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,valD))
        {
          if (!RDFstruct->TargetCharacteristics) RDFstruct->TargetCharacteristics = new RDFTargetCharacteristics;
          RDFstruct->TargetCharacteristics->SsrAmplitudePresent = true;
          int val = valD + 0.5;
          RDFstruct->TargetCharacteristics->SsrAmplitude = MIN(MAX(val,127),-127);
        } else {
          RDFstruct->RAB = false;
          if (RDFstruct->TargetCharacteristics) RDFstruct->TargetCharacteristics->SsrAmplitudePresent = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_PSR_LEN)
      {
        double valD;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,valD))
        {
          if (!RDFstruct->TargetCharacteristics) RDFstruct->TargetCharacteristics = new RDFTargetCharacteristics;
          RDFstruct->TargetCharacteristics->PsrRunlengthPresent = true;
          int val = (valD / (360.0 / 8192.0)) + 0.5;
          RDFstruct->TargetCharacteristics->PsrRunlength = MIN(MAX(val,255),0);
        } else {
          RDFstruct->RAB = false;
          if (RDFstruct->TargetCharacteristics) RDFstruct->TargetCharacteristics->PsrRunlengthPresent = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_PSR_AMP)
      {
        double valD;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,valD))
        {
          if (!RDFstruct->TargetCharacteristics) RDFstruct->TargetCharacteristics = new RDFTargetCharacteristics;
          RDFstruct->TargetCharacteristics->PsrAmplitudePresent = true;
          int val = valD + 0.5;
          RDFstruct->TargetCharacteristics->PsrAmplitude = MIN(MAX(val,127),-127);
        } else {
          RDFstruct->RAB = false;
          if (RDFstruct->TargetCharacteristics) RDFstruct->TargetCharacteristics->PsrAmplitudePresent = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_PSR_DIFF_AZ)
      {
        int val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (!RDFstruct->TargetCharacteristics) RDFstruct->TargetCharacteristics = new RDFTargetCharacteristics;
          RDFstruct->TargetCharacteristics->PsrSsrAzimuthDiffPresent = true;
          RDFstruct->TargetCharacteristics->PsrSsrAzimuthDiff = MIN(MAX(val,127),-127);
        } else {
          RDFstruct->RAB = false;
          if (RDFstruct->TargetCharacteristics) RDFstruct->TargetCharacteristics->PsrSsrAzimuthDiffPresent = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_PSR_DIFF_RNG)
      {
        int val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (!RDFstruct->TargetCharacteristics) RDFstruct->TargetCharacteristics = new RDFTargetCharacteristics;
          RDFstruct->TargetCharacteristics->PsrSsrRangeDiffPresent = true;
          RDFstruct->TargetCharacteristics->PsrSsrRangeDiff = MIN(MAX(val,127),-127);
        } else {
          RDFstruct->RAB = false;
          if (RDFstruct->TargetCharacteristics) RDFstruct->TargetCharacteristics->PsrSsrRangeDiffPresent = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_PSR_Doppler)
      {
        double val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->DopplerSpeed.RawPresent = true;
          RDFstruct->DopplerSpeed.DopplerSpeed = (val + 0.5); // zaokrouhlit
          // a co ostatni polozky??? nejak je snulovat???, asi jiz jsou konstruktorem RDFDopplerSpeed
        } else {
          RDFstruct->DopplerSpeed.RawPresent = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_PSR_CalcDoppler)
      {
        double val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->DopplerSpeed.CalculatedPresent = true;
          RDFstruct->DopplerSpeed.CalculatedSpeed = (val + 0.5); // zaokrouhlit
        } else {
          RDFstruct->DopplerSpeed.CalculatedPresent = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_PSR_CalcDoppler_V)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->DopplerSpeed.CalculatedPresent) RDFstruct->DopplerSpeed.CalculatedValid = val;
        } else {
          RDFstruct->DopplerSpeed.CalculatedPresent = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_MDS_MSP)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->CommCapabilityAndFlightStatus==NULL) RDFstruct->CommCapabilityAndFlightStatus = new RDFCommCapabilityAndFlightStatus;
          RDFstruct->CommCapabilityAndFlightStatus->ModeSSpecificService = val;
        } else {
          if (RDFstruct->CommCapabilityAndFlightStatus!=NULL)
          {
            delete RDFstruct->CommCapabilityAndFlightStatus;
            RDFstruct->CommCapabilityAndFlightStatus = NULL;
          }
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_MDS_Capa_CS)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->CommCapabilityAndFlightStatus==NULL) RDFstruct->CommCapabilityAndFlightStatus = new RDFCommCapabilityAndFlightStatus;
          RDFstruct->CommCapabilityAndFlightStatus->IdentificationCapability = val;
        } else {
          if (RDFstruct->CommCapabilityAndFlightStatus!=NULL)
          {
            delete RDFstruct->CommCapabilityAndFlightStatus;
            RDFstruct->CommCapabilityAndFlightStatus = NULL;
          }
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_MDS_ACAS)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->CommCapabilityAndFlightStatus==NULL) RDFstruct->CommCapabilityAndFlightStatus = new RDFCommCapabilityAndFlightStatus;
          RDFstruct->CommCapabilityAndFlightStatus->ACASOperational = val;
        } else {
          if (RDFstruct->CommCapabilityAndFlightStatus!=NULL)
          {
            delete RDFstruct->CommCapabilityAndFlightStatus;
            RDFstruct->CommCapabilityAndFlightStatus = NULL;
          }
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_MDS_Level)
      {
        int val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->CommCapabilityAndFlightStatus==NULL) RDFstruct->CommCapabilityAndFlightStatus = new RDFCommCapabilityAndFlightStatus;
          enum COMM_CAPABILITY valE = COMM_CAPABILITY_NONE;
               if (val==2) valE = COMM_CAPABILITY_A_B;
          else if (val==3) valE = COMM_CAPABILITY_A_B_ELMU;
          else if (val==4) valE = COMM_CAPABILITY_A_B_ELMU_ELMD;
          else if (val==5) valE = COMM_CAPABILITY_LEVEL_5;
          else             valE = COMM_CAPABILITY_NONE;
          RDFstruct->CommCapabilityAndFlightStatus->CommCapability = valE;
        } else {
          if (RDFstruct->CommCapabilityAndFlightStatus!=NULL)
          {
            delete RDFstruct->CommCapabilityAndFlightStatus;
            RDFstruct->CommCapabilityAndFlightStatus = NULL;
          }
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_MDS_FS)
      {
        int val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->CommCapabilityAndFlightStatus==NULL) RDFstruct->CommCapabilityAndFlightStatus = new RDFCommCapabilityAndFlightStatus;
          enum FLIGHT_STATUS valE = FLIGHT_STATUS_AIRBORNE;
               if (val== 1) valE = FLIGHT_STATUS_ON_GROUND;
          else if (val== 2) valE = FLIGHT_STATUS_AIRBORNE_ALERT;
          else if (val== 3) valE = FLIGHT_STATUS_ON_GROUND_ALERT;
          else if (val== 4) valE = FLIGHT_STATUS_ALERT_SPI;
          else if (val== 5) valE = FLIGHT_STATUS_SPI;
          else if (val== 6) valE = FLIGHT_STATUS_GENERAL_EMERGENCY;
          else if (val== 7) valE = FLIGHT_STATUS_LIFEGUARD;
          else if (val== 8) valE = FLIGHT_STATUS_MINIMUM_FUEL;
          else if (val== 9) valE = FLIGHT_STATUS_NO_COMMUNICATIONS;
          else if (val==10) valE = FLIGHT_STATUS_UNLAWFUL_INTERFERENCE;
          else              valE = FLIGHT_STATUS_AIRBORNE;
          RDFstruct->CommCapabilityAndFlightStatus->FlightStatus = valE;
        } else {
          if (RDFstruct->CommCapabilityAndFlightStatus!=NULL)
          {
            delete RDFstruct->CommCapabilityAndFlightStatus;
            RDFstruct->CommCapabilityAndFlightStatus = NULL;
          }
        }
        return true;
      }
      if ((var_i == PARSER_RDF_ID_RNG)||(var_i == PARSER_RDF_ID_AZ))
      {
        bool chRNG = (var_i == PARSER_RDF_ID_RNG);
        bool chAZ  = (var_i == PARSER_RDF_ID_AZ );
        if ((!chRNG) && (ChangedVariables_i.find(PARSER_RDF_ID_RNG) != ChangedVariables_i.end()))
        {
          ChangedVariables_i.erase(ChangedVariables_i.find(PARSER_RDF_ID_RNG));
          chRNG = true;
        }
        if ((!chAZ ) && (ChangedVariables_i.find(PARSER_RDF_ID_AZ) != ChangedVariables_i.end()))
        {
          ChangedVariables_i.erase(ChangedVariables_i.find(PARSER_RDF_ID_AZ));
          chAZ = true;
        }

        double valRNG;
        bool existRNG = false;
        if (chRNG) existRNG = MUP->GetVariable(PARSER_RDF_FixConfigData[PARSER_RDF_ID_RNG].Name_MUP,valRNG);
        double valAZ;
        bool existAZ = false;
        if (chAZ) existAZ = MUP->GetVariable(PARSER_RDF_FixConfigData[PARSER_RDF_ID_AZ].Name_MUP,valAZ);

        if (((chRNG)&&(!existRNG)) || ((chAZ)&&(!existAZ)))
        {
          if (RDFstruct->Polar != NULL) delete RDFstruct->Polar;
          RDFstruct->Polar = NULL;
        } else {
          if (RDFstruct->Polar == NULL) RDFstruct->Polar = new RDFCoorPolar();
          if (existRNG) RDFstruct->Polar->Rho = valRNG * 1852.;
          if (existAZ)  RDFstruct->Polar->Theta = valAZ / (180. / M_PI);
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_WE)
      {
        RDFstruct->WarningError.clear();
        std::deque<bool> valWE;
        int WE_y,WE_x;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[PARSER_RDF_ID_WE].Name_MUP,valWE,WE_y,WE_x))
        {
          for (unsigned i = 0; i<(unsigned)WE_y; i++)
          {
            if (valWE[i]) RDFstruct->WarningError.push_back(i);
          }
        }
      }
      if ((var_i == PARSER_RDF_ID_BDS)||(var_i == PARSER_RDF_ID_BDSage))
      {
        bool chBDS      = (var_i == PARSER_RDF_ID_BDS);
        bool chBDS_AGE  = (var_i == PARSER_RDF_ID_BDSage);
        if ((!chBDS) && (ChangedVariables_i.find(PARSER_RDF_ID_BDS) != ChangedVariables_i.end()))
        {
          ChangedVariables_i.erase(ChangedVariables_i.find(PARSER_RDF_ID_BDS));
          chBDS = true;
        }
        if ((!chBDS_AGE ) && (ChangedVariables_i.find(PARSER_RDF_ID_BDSage) != ChangedVariables_i.end()))
        {
          ChangedVariables_i.erase(ChangedVariables_i.find(PARSER_RDF_ID_BDSage));
          chBDS_AGE = true;
        }

//      std::deque<uint64_t> valBDS;
        UIntList valBDS;
        bool existBDS = false;
        int BDS_y,BDS_x;
        if (chBDS) existBDS = MUP->GetVariable(PARSER_RDF_FixConfigData[PARSER_RDF_ID_BDS].Name_MUP,valBDS,BDS_y,BDS_x);
        std::deque<int> valBDS_AGE;
        bool existBDS_AGE = false;
        int BDS_AGE_y,BDS_AGE_x;
        if (chBDS_AGE) existBDS_AGE = MUP->GetVariable(PARSER_RDF_FixConfigData[PARSER_RDF_ID_BDSage].Name_MUP,valBDS_AGE,BDS_AGE_y,BDS_AGE_x);
//        printf("BDS:  %d %d\n",BDS_y, BDS_AGE_y);

        RDFstruct->ModeS.clear();
        RDFstruct->ModeSAge.clear();
        RDFstruct->ResolutionAdvisory.Present = false;

        if (existBDS)
        {
          for (int i=0;i<BDS_y;i++)
          {
            if (!(valBDS[i] & ((uint64_t)1<<57)))
            {
              uint64_t BDSval = valBDS[i];
              int Age = -1;
              if ((existBDS_AGE) && (BDS_AGE_y>i)) Age = valBDS_AGE[i];
//              printf ("mam BDS %x Age=%d\n",i,Age);
              if (Age<0)
              {
                if (i == 0x30)
                {
                  RDFstruct->ResolutionAdvisory.Present = true;
                  unsigned char *MB = RDFstruct->ResolutionAdvisory.Data;
                  MB[0] = (BDSval >> 48) & 0xff;
                  MB[1] = (BDSval >> 40) & 0xff;
                  MB[2] = (BDSval >> 32) & 0xff;
                  MB[3] = (BDSval >> 24) & 0xff;
                  MB[4] = (BDSval >> 16) & 0xff;
                  MB[5] = (BDSval >>  8) & 0xff;
                  MB[6] = (BDSval >>  0) & 0xff;
                } else {
                  RDFTargetModeS item;
                  item.Address = i;
                  unsigned char *MB = item.MessageData;
                  MB[0] = (BDSval >> 48) & 0xff;
                  MB[1] = (BDSval >> 40) & 0xff;
                  MB[2] = (BDSval >> 32) & 0xff;
                  MB[3] = (BDSval >> 24) & 0xff;
                  MB[4] = (BDSval >> 16) & 0xff;
                  MB[5] = (BDSval >>  8) & 0xff;
                  MB[6] = (BDSval >>  0) & 0xff;
                  RDFstruct->ModeS.push_back(item);
                }
              } else {
                  RDFTargetModeS item;
                  item.Address = i;
                  item.Age = MIN(Age,255);
                  unsigned char *MB = item.MessageData;
                  MB[0] = (BDSval >> 48) & 0xff;
                  MB[1] = (BDSval >> 40) & 0xff;
                  MB[2] = (BDSval >> 32) & 0xff;
                  MB[3] = (BDSval >> 24) & 0xff;
                  MB[4] = (BDSval >> 16) & 0xff;
                  MB[5] = (BDSval >>  8) & 0xff;
                  MB[6] = (BDSval >>  0) & 0xff;
                  RDFstruct->ModeSAge.push_back(item);
              }
            }
          }
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_SADDR)
      {
        int val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->AircraftAddress.Present = true;
          RDFstruct->AircraftAddress.Address = val;
        } else {
          RDFstruct->AircraftAddress.Present = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_CallSign)
      {
        std::string val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->AircraftIdentification = val;
        } else {
          RDFstruct->AircraftIdentification.clear();
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_UserText)
      {
        std::string val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->UserText = val;
        } else {
          RDFstruct->UserText.clear();
        }
        return true;
      }
   return false;
}

// ------------------------------------------------------------------------------------------------------------------------------------------
// ************************************************************  RDFTargetTrack  ************************************************************

bool CParserRDF::parse(RDFTargetTrack  &RDFstruct, class C_CM_MUP_Interface *MUP, std::map<std::string, class CCmVariable> *InOutVariables, std::map<std::string, class CCmVariable> *GlobalVariables)
{
  return do_parse(RDFstruct, MUP, InOutVariables, GlobalVariables);
}

bool CParserRDF::tryLoadVariablesRDF(int var_i, class C_CM_MUP_Interface *MUP, std::set<int> &LoadedVariables_i, RDFTargetTrack  *RDFstruct)
{
  if (var_i<0) return false;

  if (tryLoadVariablesRDF(var_i, MUP, LoadedVariables_i, (RDFTargetPlot *)RDFstruct)) return true;

  // specialy pro track
      if (var_i == PARSER_RDF_ID_TrNum)
      {
        if (RDFstruct->TrackNumber.Present) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, (int)RDFstruct->TrackNumber.Number);
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrNum_Tmp)
      {
        if (RDFstruct->TrackNumber.Present) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, (RDFstruct->TrackNumber.Temporary ? true : false));
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagInit)
      {
        if (RDFstruct->Init) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, (RDFstruct->Init ? true : false));
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagCancel)
      {
        if (RDFstruct->Init) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, (RDFstruct->Cancel ? true : false));
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagTent)
      {
        if (RDFstruct->Init) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, (RDFstruct->Tentative ? true : false));
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagCorr)
      {
        if (RDFstruct->Init) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, (RDFstruct->Correlated ? true : false));
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagCST)
      {
        if (RDFstruct->Init) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, (RDFstruct->CST ? true : false));
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagMono)
      {
        if (RDFstruct->Init) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, (RDFstruct->Monosensor ? true : false));
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagDOU)
      {
        if (RDFstruct->Init) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, (RDFstruct->DOU ? true : false));
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagMAH)
      {
        if (RDFstruct->Init) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, (RDFstruct->MAH ? true : false));
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagSUP)
      {
        if (RDFstruct->Init) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, (RDFstruct->SUP ? true : false));
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagPSR)
      {
        if (RDFstruct->Init) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, (RDFstruct->PSR ? true : false));
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagSSR)
      {
        if (RDFstruct->Init) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, (RDFstruct->SSR ? true : false));
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagMDS)
      {
        if (RDFstruct->Init) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP, (RDFstruct->MDS ? true : false));
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagTurn)
      {
        int val = 1;
        switch (RDFstruct->TransversalAcceleration)
        {
          case TRANS_ACCEL_CONSTANT_COURSE : val = 0; break;
          case TRANS_ACCEL_RIGHT_TURN      : val = 1; break;
          case TRANS_ACCEL_LEFT_TURN       : val = 2; break;
          case TRANS_ACCEL_UNDETERMINED    : val = 3; break;
        }
        MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  val);
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagSpeed)
      {
        int val = 1;
        switch (RDFstruct->LongitudinalAcceleration)
        {
          case LONG_ACCEL_CONSTANT_GROUNDSPEED   : val = 0; break;
          case LONG_ACCEL_INCREASING_GROUNDSPEED : val = 1; break;
          case LONG_ACCEL_DECREASING_GROUNDSPEED : val = 2; break;
          case LONG_ACCEL_UNDETERMINED           : val = 3; break;
        }
        MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  val);
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagSpeed)
      {
        int val = 1;
        switch (RDFstruct->TrackClimb)
        {
          case TRACK_CLIMB_MAINTAINING : val = 0; break;
          case TRACK_CLIMB_CLIMBING    : val = 1; break;
          case TRACK_CLIMB_DESCENDING  : val = 2; break;
          case TRACK_CLIMB_UNKNOWN     : val = 3; break;
        }
        MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  val);
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrClimb)
      { // neznam jednotky a nevim cim delit/nasobit
        if (RDFstruct->ClimbSpeed  != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  RDFstruct->ClimbSpeed->Speed);
//        if (RDFstruct->ClimbSpeed  != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  RDFstruct->ClimbSpeed->Speed * ((1 / 0.3048) * 0.01));
        return true;
      }

      if (var_i == PARSER_RDF_ID_TrX)
      {
        if (RDFstruct->Cartesian != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->Cartesian->X / 1852.));
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrY)
      {
        if (RDFstruct->Cartesian != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->Cartesian->Y / 1852.));
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrSpeed)
      {
        double ms2knots = (3600.0 / 1852.0);
        if (RDFstruct->GroundSpeed != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->GroundSpeed->Speed * ms2knots));
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrHeading)
      {
        if (RDFstruct->GroundSpeed != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->GroundSpeed->Heading / (180. / M_PI)));
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrAccelX)
      { // neznam jednotky a nevim cim delit/nasobit
//        if (RDFstruct->Acceleration != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->Acceleration->X / 1852.));
        if (RDFstruct->Acceleration != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->Acceleration->X));
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrAccelY)
      { // neznam jednotky a nevim cim delit/nasobit
//        if (RDFstruct->Acceleration != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->Acceleration->Y / 1852.));
        if (RDFstruct->Acceleration != NULL) MUP->SetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,  (RDFstruct->Acceleration->Y));
        return true;
      }

  return false;
}

bool CParserRDF::tryStoreVariableRDF(int var_i, class C_CM_MUP_Interface *MUP, std::set<int> &ChangedVariables_i, RDFTargetTrack *RDFstruct)
{
      if (var_i<0) return false;

      if (var_i == PARSER_RDF_ID_TrNum)
      {
        int val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->TrackNumber.Present = 1;
          RDFstruct->TrackNumber.Number = val;
        } else {
          RDFstruct->TrackNumber.Present = 0;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrNum_Tmp)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->TrackNumber.Temporary = val;
        } else {
          RDFstruct->TrackNumber.Temporary = 0;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagInit)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->Init = val;
        } else {
          RDFstruct->Init = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagCancel)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->Cancel = val;
        } else {
          RDFstruct->Cancel = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagTent)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->Tentative = val;
        } else {
          RDFstruct->Tentative = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagCorr)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->Correlated = val;
        } else {
          RDFstruct->Correlated = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagCST)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->CST = val;
        } else {
          RDFstruct->CST = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagMono)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->Monosensor = val;
        } else {
          RDFstruct->Monosensor = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagDOU)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->DOU = val;
        } else {
          RDFstruct->DOU = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagMAH)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->MAH = val;
        } else {
          RDFstruct->MAH = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagSUP)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->SUP = val;
        } else {
          RDFstruct->SUP = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagPSR)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->PSR = val;
        } else {
          RDFstruct->PSR = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagSSR)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->SSR = val;
        } else {
          RDFstruct->SSR = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagMDS)
      {
        bool val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          RDFstruct->MDS = val;
        } else {
          RDFstruct->MDS = false;
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagTurn)
      {
        int val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          enum TRANS_ACCEL  valE = TRANS_ACCEL_UNDETERMINED;
               if (val==0) valE = TRANS_ACCEL_CONSTANT_COURSE;
          else if (val==1) valE = TRANS_ACCEL_RIGHT_TURN;
          else if (val==2) valE = TRANS_ACCEL_LEFT_TURN;
          else             valE = TRANS_ACCEL_UNDETERMINED;
          RDFstruct->TransversalAcceleration = valE;
        } else {
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagSpeed)
      {
        int val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          enum LONG_ACCEL  valE = LONG_ACCEL_UNDETERMINED;
               if (val==0) valE = LONG_ACCEL_CONSTANT_GROUNDSPEED;
          else if (val==1) valE = LONG_ACCEL_INCREASING_GROUNDSPEED;
          else if (val==2) valE = LONG_ACCEL_DECREASING_GROUNDSPEED;
          else             valE = LONG_ACCEL_UNDETERMINED;
          RDFstruct->LongitudinalAcceleration = valE;
        } else {
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrFlagClimb)
      {
        int val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          enum TRACK_CLIMB valE = TRACK_CLIMB_UNKNOWN;
               if (val==0) valE = TRACK_CLIMB_MAINTAINING;
          else if (val==1) valE = TRACK_CLIMB_CLIMBING;
          else if (val==2) valE = TRACK_CLIMB_DESCENDING;
          else             valE = TRACK_CLIMB_UNKNOWN;
          RDFstruct->TrackClimb = valE;
        } else {
        }
        return true;
      }
      if (var_i == PARSER_RDF_ID_TrClimb)
      {
        double val;
        if (MUP->GetVariable(PARSER_RDF_FixConfigData[var_i].Name_MUP,val))
        {
          if (RDFstruct->ClimbSpeed == NULL) RDFstruct->ClimbSpeed = new RDFTargetClimbSpeed();
          RDFstruct->ClimbSpeed->Speed = val; //  / ((1 / 0.3048) * 0.01);
        } else {
          if (RDFstruct->ClimbSpeed != NULL)
          {
            delete RDFstruct->ClimbSpeed;
            RDFstruct->ClimbSpeed = NULL;
          }
        }
        return true;
      }
      if ((var_i == PARSER_RDF_ID_TrX)||(var_i == PARSER_RDF_ID_TrY))
      {
        bool chTrX = (var_i == PARSER_RDF_ID_TrX);
        bool chTrY = (var_i == PARSER_RDF_ID_TrY);
        if ((!chTrX) && (ChangedVariables_i.find(PARSER_RDF_ID_TrX) != ChangedVariables_i.end()))
        {
          ChangedVariables_i.erase(ChangedVariables_i.find(PARSER_RDF_ID_TrX));
          chTrX = true;
        }
        if ((!chTrY) && (ChangedVariables_i.find(PARSER_RDF_ID_TrY) != ChangedVariables_i.end()))
        {
          ChangedVariables_i.erase(ChangedVariables_i.find(PARSER_RDF_ID_TrY));
          chTrY = true;
        }

        double valTrX;
        bool existTrX = false;
        if (chTrX) existTrX = MUP->GetVariable(PARSER_RDF_FixConfigData[PARSER_RDF_ID_TrX].Name_MUP,valTrX);
        double valTrY;
        bool existTrY = false;
        if (chTrY) existTrY = MUP->GetVariable(PARSER_RDF_FixConfigData[PARSER_RDF_ID_TrY].Name_MUP,valTrY);

        if (((chTrX)&&(!existTrX)) || ((chTrY)&&(!existTrY)))
        {
          if (RDFstruct->Cartesian != NULL) delete RDFstruct->Cartesian;
          RDFstruct->Cartesian = NULL;
        } else {
          if (RDFstruct->Cartesian == NULL) RDFstruct->Cartesian = new RDFCoorCartesian();
          if (existTrX) RDFstruct->Cartesian->X = valTrX * 1852.;
          if (existTrY) RDFstruct->Cartesian->Y = valTrY * 1852.;
        }
        return true;
      }
      if ((var_i == PARSER_RDF_ID_TrAccelX)||(var_i == PARSER_RDF_ID_TrAccelY))
      {
        bool chTrX = (var_i == PARSER_RDF_ID_TrAccelX);
        bool chTrY = (var_i == PARSER_RDF_ID_TrAccelY);
        if ((!chTrX) && (ChangedVariables_i.find(PARSER_RDF_ID_TrAccelX) != ChangedVariables_i.end()))
        {
          ChangedVariables_i.erase(ChangedVariables_i.find(PARSER_RDF_ID_TrAccelX));
          chTrX = true;
        }
        if ((!chTrY) && (ChangedVariables_i.find(PARSER_RDF_ID_TrAccelY) != ChangedVariables_i.end()))
        {
          ChangedVariables_i.erase(ChangedVariables_i.find(PARSER_RDF_ID_TrAccelY));
          chTrY = true;
        }

        double valTrX;
        bool existTrX = false;
        if (chTrX) existTrX = MUP->GetVariable(PARSER_RDF_FixConfigData[PARSER_RDF_ID_TrAccelX].Name_MUP,valTrX);
        double valTrY;
        bool existTrY = false;
        if (chTrY) existTrY = MUP->GetVariable(PARSER_RDF_FixConfigData[PARSER_RDF_ID_TrAccelY].Name_MUP,valTrY);

        if (((chTrX)&&(!existTrX)) || ((chTrY)&&(!existTrY)))
        {
          if (RDFstruct->Acceleration != NULL) delete RDFstruct->Acceleration;
          RDFstruct->Acceleration = NULL;
        } else {
          if (RDFstruct->Acceleration == NULL) RDFstruct->Acceleration = new RDFCoorCartesian();
          if (existTrX) RDFstruct->Acceleration->X = valTrX;// * 1852.; // neznam jednotky a nevim cim delit/nasobit
          if (existTrY) RDFstruct->Acceleration->Y = valTrY;// * 1852.; // neznam jednotky a nevim cim delit/nasobit
        }
        return true;
      }
      if ((var_i == PARSER_RDF_ID_TrSpeed)||(var_i == PARSER_RDF_ID_TrHeading))
      {
        bool chTrSpeed   = (var_i == PARSER_RDF_ID_TrSpeed  );
        bool chTrHeading = (var_i == PARSER_RDF_ID_TrHeading);
        if ((!chTrSpeed  ) && (ChangedVariables_i.find(PARSER_RDF_ID_TrSpeed  ) != ChangedVariables_i.end()))
        {
          ChangedVariables_i.erase(ChangedVariables_i.find(PARSER_RDF_ID_TrSpeed  ));
          chTrSpeed   = true;
        }
        if ((!chTrHeading) && (ChangedVariables_i.find(PARSER_RDF_ID_TrHeading) != ChangedVariables_i.end()))
        {
          ChangedVariables_i.erase(ChangedVariables_i.find(PARSER_RDF_ID_TrHeading));
          chTrHeading = true;
        }

        double valTrSpeed;
        bool existTrSpeed   = false;
        if (chTrSpeed  ) existTrSpeed   = MUP->GetVariable(PARSER_RDF_FixConfigData[PARSER_RDF_ID_TrSpeed  ].Name_MUP,valTrSpeed  );
        double valTrHeading;
        bool existTrHeading = false;
        if (chTrHeading) existTrHeading = MUP->GetVariable(PARSER_RDF_FixConfigData[PARSER_RDF_ID_TrHeading].Name_MUP,valTrHeading);

        if (((chTrSpeed)&&(!existTrSpeed)) || ((chTrHeading)&&(!existTrHeading)))
        {
          if (RDFstruct->GroundSpeed != NULL) delete RDFstruct->GroundSpeed;
          RDFstruct->GroundSpeed = NULL;
        } else {
          if (RDFstruct->GroundSpeed == NULL) RDFstruct->GroundSpeed = new RDFTargetGroundSpeed();
          double ms2knots = (3600.0 / 1852.0);
          if (existTrSpeed  ) RDFstruct->GroundSpeed->Speed   = valTrSpeed   / ms2knots;
          if (existTrHeading) RDFstruct->GroundSpeed->Heading = valTrHeading / (180. / M_PI);
        }
        return true;
      }


   return false;
}

// ------------------------------------------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------------------------
