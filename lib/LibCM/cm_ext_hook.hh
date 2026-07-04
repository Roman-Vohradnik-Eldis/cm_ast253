#ifndef CM_EXT_HOOK_HH_
#define CM_EXT_HOOK_HH_

#include "cm_connection.hh"
#include "cm_live_debug.hh"
#include "cm_scheduler.hh"

// CCmExtHook:
//
// Pozor: Metody se mohou volat z ruznych threadu zaroven a je potreba
// vse synchronizovat. Napriklad LibCM vola neco z CallbackThreadu,
// PollThreadu, apod.
//
class CCmExtHook
{
public:
    virtual ~CCmExtHook() {}
    
    // RefreshExtFilters: Vola se, pokud si uzivatel zmeni filtry.
    //
    // myFiltersIn jsou nove uzivatelovy filtry, nesmime to menit
    // myFiltersExt jsou soucasne ext filtry.
    //
    // Muzeme zmenit myFiltersExt. Pokud to udelame, musime vratit true. Jinak false.
    virtual bool RefreshExtFilters(CCmFiltersSet &myFiltersIn, CCmFiltersSet &myFiltersExt) = 0;

    // RegenaratePartnerFiltersExt: Vola se, pokud si nejaky partner zmeni filtry.
    //
    // V PartnerFiltersActive je sjednoceni filtru od vsech partneru, nesmime to menit.
    // V myFiltersExt jsou soucasne ext filtry.
    //
    // Muzeme zmenit myFiltersExt. Pokud to udelame, musime vratit true. Jinak false.
    virtual bool RegenaratePartnerFiltersExt(CCmFiltersSet &PartnerFiltersActive, CCmFiltersSet &myFiltersExt) = 0;

    // internalChangePartnerVariable: Tohle se vola po tom, co
    // uzivatel zmeni partnerovu promennou.
    virtual bool internalChangePartnerVariable(CCmVariable const *var) = 0;

    // internalChangeMyVariable: Tohle se vola po tom, co uzivatel
    // nebo partner zmenil nasi promennou.
    virtual bool internalChangeMyVariable(CCmVariable const *var, int EventWhat) = 0;

    // VariableGoCB: Tohle se vola pred tim, nez to jde uzivatelovi do
    // callbacku.  Tohle pred tim, nez jsou promenne do callbacku,
    // updatne stav muparseru.
    //
    // Poznamka: pokud nekdo cizi zmeni nasi promennou, tak se zavola
    // internalChangeMyvariable a zaroven se pak pozdeji objevi tady,
    // takze zmenu vlastne dostaneme dvakrat...
    virtual bool VariableGoCB(VarMap *VariablesCreated,
			       VarMap *VariablesUpdated,
			       VarMap *VariablesDeleted) = 0;

    // EvalPollExt: Tohle vola CM v PollThreadu. CM je odemcena, preda
    // sama sebe jako CCmConnection a EvalPollExt muze definovat/menit
    // proměnné, apod.
    virtual void EvalPollExt(CCmConnection &conn, PollCond &pc, double ActTime) = 0;

    // Tohle se vola pri LibCM start
    virtual void Start() = 0;
};

#endif // CM_EXT_HOOK_HH_
