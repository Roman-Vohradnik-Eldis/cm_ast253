#ifndef CM_BASE_VAREVENT_HH_
#define CM_BASE_VAREVENT_HH_

#include "cm_base_variable.hh"

class CmEventWhat
{
public:
    enum Kind
    {
	undefine = 0,
	define = 1,
	update = 2,
    };

private:
        Kind m_what;

public:
    CmEventWhat(Kind k) : m_what(k) {}

    friend bool operator==(CmEventWhat a, CmEventWhat b) {return a.m_what == b.m_what;}
    friend bool operator!=(CmEventWhat a, CmEventWhat b) {return a.m_what != b.m_what;}

    std::string to_string()
    {
	switch (m_what)
	{
	case undefine: return "undefine";
	case define: return "define";
	case update: return "update";
	default: throw std::logic_error("CmEventWhat: Unknown value");
	}
    }
};

struct VariableEvent
{
    VariableEvent(CmEventWhat what, CCmVariable v)
	: EventWhat(what)
    {
	// case what == -1 is used internally in cm_base_ext...
	std::swap(v, variable);
    }
    CmEventWhat EventWhat; // 0=undefine , 1=define , 2=update
    CCmVariable variable;
};

typedef std::map<std::string, VariableEvent> VarEventMap;

#endif
