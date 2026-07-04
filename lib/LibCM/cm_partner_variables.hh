#ifndef CM_PARTNER_VARIABLES_HH_
#define CM_PARTNER_VARIABLES_HH_

#include "cm_setcmd.hh"
#include "cm_base_varevent.hh"
#include "cm_fragments.hh"
#include "cm_variables_to_send.hh"

// PartnerVariablesState obsahuje:
//
// (1) Co jsme partnerovi poslali za svoje promenne
// (2) Co nam on poslal za svoje promenne
//
// (3) Mame pripravene zmeny svych promennych, ktere chceme odeslat
// (4) Mame pripravene zmeny jeho promennych, ktere chceme odeslat

struct PartnerVariablesState
{
    // Pokud je zprava ohledne zmeny/definice zmeny promenne moc
    // dlouha, tak ji celou dame sem.  Odtud ji po castech postupne
    // bereme a posilame. Partner si tyto kousky sklada.
    Maybe<FragmentsToSend> fragments_to_send;
    
    // Kazda promenna, ktera nekdy byla v hisVariables, ma prirazene tzv. set_id. Na
    // zacatku ma set_id rovno nule. Pokud promenna AA opousti
    // hisVariablesToSend, tak se ji priradi nove set_id (z nejakeho citace). S
    // timto novym set_id je odeslana zprava SET.
    CmSetCmdMap my_vars_setcmd_id;

    // Pro kazdou jeho promennou si pamatuje posledni set_id, ktery nam poslal
    // partner pomoci zpravy SET na tuto promennou. Nastaveno na 0, pokud nam
    // jeste zadny SET neposlal. Kdyz odesilame zpravu MOD, tak mu toto posledni
    // set_id posilame.
    std::map<std::string, unsigned> his_vars_setcmd_id;

    // Aktualni definice promennych, ktere jsme poslali partnerovi.
    // Po odeslani definice/redefinice/undefinice to tady menime.
    std::map<std::string, MyVariableState> myVariables_State;

    // Fronta toho, co se ma posilat. Kdyz my nebo nektery z partneru
    // zmeni promennou, tak se to tady prida/zmeni/vyhodi.  Jednou za
    // cas se z toho vytvori zprava k odeslani a poslane veci se z
    // fronty vyhodi.
    MyVariablesToSend myVariablesToSend; 

    // hisVariables: obsahuje vsechny jeho promenne, ktere zname. Kdyz
    // nam on posle zmenu, dame ji sem. Pokud my nastavime jeho
    // promennou, zmenu pridame sem (a updatneme
    // hisVariablesToSend). EventFields jsou zde nepouzite.  Pokud
    // pouziva MyPrefix, jsou zde i s prefixem.
    //
    // hisVariablesToSend: obsahuje pouze ty promenne z hisVariables, ktere je
    // mu potreba poslat. Pri pridavani se beru v uvahu priorita promenne.
    // Pouzivaji se EventFields, ktere polozky pole se maji posilat.
    //
    VarMap hisVariables;
    std::list<CCmVariable> his_variables_to_send;

    // Tohle je mnozina jeho promennych.  Pokud je promenna tady, tak
    // to znamena ze Ja nebo jiny partner definoval stejnou promennou,
    // a ta nase se nepouziva. Pokud Ja tu svoji oddefinuju nebo jiny
    // partner ji oddefinuje, tak potom se mozna bude pouzivat.
    std::set<std::string> his_colliding_variables;
}; 

#endif // CM_PARTNER_VARIABLES_HH_
