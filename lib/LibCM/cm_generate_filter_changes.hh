#ifndef CM_GENERATE_FILTER_CHANGES_HH_
#define CM_GENERATE_FILTER_CHANGES_HH_

#include "cm_base_filters_set.hh"
#include "cm_send_buffer_state.hh"

// `onlyPrefix`: Pokud je neprazdny, uvazuj pouze filtry s timto
// prefixem. To se pouziva, pokud ma partner nastaven MyPrefix,
// protoze jine promenne nam dat nemuze.
//
// `myVariables`: Uvazuj pouze, ktere matchuji nebo mimo
// myVariables. Sem se typicky davaji vsechny nase promenne.
//
struct FilterParams
{
    std::string onlyPrefix;
    VarMap const &myVariables;
};

// Zkusi co nejvice filtru "presunout" ze `src` to `dst`. Pritom
// vygeneruje zpravu, co je ma poslat partnerovi. Mnozstvi presunutych
// filtru je limitovano delkou zpravy.
//
// `src`: const: Filterset, ktery chceme nastavit.
//
// `dst`: Filterset, ktery ted zna nas Partner.
//
// Vrati true, pokud se presunuly vsechny zmeny, a tedy po odeslani
// zpravy bude partner vsechny nase filtry znat. V opacnem pripade se
// poslala jenom cast a bude to potreba zavolat jeste jednou
//
bool CompareFiltersAndGenerateChangeMessage(SendBufferState &buffer,
                                            CCmFiltersSet &src,
                                            CCmFiltersSet &dst,
                                            FilterParams const &params);

#endif // CM_GENERATE_FILTER_CHANGES_HH_
