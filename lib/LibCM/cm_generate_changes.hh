#ifndef CM_GENERATE_CHANGES_HH_
#define CM_GENERATE_CHANGES_HH_

#include "cm_fragments.hh"
#include "cm_partner_variables.hh"
#include "cm_output_buffer.hh"
#include "cm_send_buffer_state.hh"

struct DataToSendParams
{
    // unsigned max_item_length;
    // unsigned LengthIdeal;
    // unsigned LengthMax;

    bool UseSetIds;

    // Strip prefix from his variables 
    std::string PartnerVarPrefix;

    // for debug messages
    std::string my_libcm_name;
};

// Vybere promenne, ktere potrebuji poslat. Z nich vygenerujeme zpravu
// k odeslani. Zaroven aktualizuje stav techto odeslanych
// promennych. Tim padem tuto zpravu musime nutne dorucit partnerovi.
// Zpravy musime dorucit v tom poradi, v kterem jsme je vygenerovali.
//
// @return true if something was put to the `buffer`. Otherwise, there
// is nothing to send.
//
bool generate_data_to_send(SendBufferState &buffer,
                           PartnerVariablesState &variable_state,
                           DataToSendParams const &params,
                           double AtcTime);
   
#endif // CM_GENERATE_CHANGES_HH_
