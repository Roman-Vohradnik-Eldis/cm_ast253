#ifndef SETCMD_HH_
#define SETCMD_HH_

#include <vector>
#include "cm_header_internal.hh"
#include "cm_base_variable.hh"

// SetCmdStatus je pomocna struktura, ktera udrzuje totalni funkci:
// Nat x Nat -> Nat.
//
// Hodnoty jsou ulozeme v std::map. Pokud tam nejaka dvojice neni, tak
// ma hodnotu default_id.
//
// IndexMapTotal
class SetCmdStatus
{
    unsigned default_id;
    typedef std::map<std::pair<unsigned, unsigned>, unsigned> Map;
    Map setcmd_ids;

public:
    SetCmdStatus()
        : default_id(0)
    {}
    
    unsigned get_max_value() const 
    {
	unsigned max = default_id;
	CM_for (Map::value_type const &p, setcmd_ids)
	    max = std::max(p.second, max);
	return max;
    }

    unsigned get_min_value() const 
    {
	unsigned min = default_id;
	CM_for (Map::value_type const &p, setcmd_ids)
	    min = std::min(p.second, min);
	return min;
    }

    unsigned get_index(unsigned y, unsigned x)
    {
        Map::iterator it = setcmd_ids.find(std::make_pair(y, x));
	if (it != setcmd_ids.end())
	    return it->second;
	return default_id;
    }

    void set_index(unsigned y, unsigned x, unsigned value)
    {
	// Create or update
	setcmd_ids[std::make_pair(y, x)] = value;
    }

    void set_all(unsigned value)
    {
	setcmd_ids.clear();
	default_id = value;
    }

    // void set(std::map<std::pair<unsigned, unsigned>, unsigned> const &other_map)
    // {
    //     for (auto &p : other_map)
    //         setcmd_ids.erase(p.first);
    //     setcmd_ids.insert(other_map.begin(), other_map.end());
    // }
};

// CmSetCmdMap: Tahle struktura resi kolize, pokud ja a muj partner
// zaroven zmenime jednu promennou. Pro kazdeho parterna mame vlastni
// strukturu CmSetCmdMap. Pri odesilani zpravy SET,DEF,UDNEF posilame
// set_id. Pri prijmani zpravy MOD nam partner posila set_id.  Tahle
// struktura udrzuje aktualni poslane set_id pro kazdou promennou a
// index jejiho pole.
//
// Pokud odesilame zpravu SET,DEF nebo UNDEF, tak ji posilame spolecne
// s cislem, ktere ziskame pomoci get_next_id().
// 
// Kdyz tu spravu uspesne odesleme, tak:
// - V pripade SET zavolame update_from_variable() na EventFields, ktere jsme menili.
// - V pripade DEF a UNDEF zavolame update_fully();
//
// Kdyz dostaneme od partnera zpravu MOD: Muze nastat situace, ze
// mezitim promenna zanikla, nebo se zmenil je typ. To zjistime pomoci
// is_safe_to_apply. Pokud vrati false, musime tu modifikaci zahodit.
//
// Pokud je ta modifikace safe to apply, tak pomoci diff_event_fields
// vyhodime ty EventFields, ktere uz byly prepsany nekym jinym. Takto
// orezanou modifikaci muzeme aplikovat.

class CmSetCmdMap
{
    // VariableName -> SetCmdStatus
    std::map<std::string, SetCmdStatus> m_map;

public:

    // Tohle zavolame, pokud sestavuejme zpravu SET,DEF nebo UNDEF.
    unsigned get_next_id(std::string const &var_name) const;

    // Tohle zavolame, pokud jsme odeslali zpravu SET (s danymi polozkami pole)
    void update_from_variable(std::string name, unsigned y_size, unsigned x_size, std::list<unsigned> const &EventFields, unsigned id);

    // Tohle zavolame, pokud jsme odeslali zpravu DEF,UNDEF
    void update_fully(std::string name, unsigned id);

    // Pouzijeme na zpravu MOD s set_id je received_id.
    bool is_safe_to_apply(std::string name, unsigned received_id);

    // Pouzijeme na zpravu MOD s set_id je received_id, pokud je
    // is_safe_to_apply.  Vrati true, pokud z modifikace neco
    // zbylo. Pokud vrati false, neni potreba modifikaci provadet.
    bool diff_event_fields(std::string name, unsigned y_size, unsigned x_size, std::list<unsigned> &EventFields, unsigned received_id);
};

#endif // CM_SETCMD_HH_
