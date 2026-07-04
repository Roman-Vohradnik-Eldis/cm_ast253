#include "cm_var_changes.hh"
#include "cm_var_utils.hh"

// was_created: Pokud ji nezname, tak jestli ji mame dat do created nebo do updated
void CmVarChanges::add_foreign_updates(VarMap const &src, bool was_created)
{
    CM_for (VarMap::value_type const &p, src)
    {
	deleted.erase(p.first);
	
        VarMap::iterator it = created.find(p.first);
        if (it != created.end())
        {
	    // Je to redefinice
            it->second = p.second;
            it->second.EventFields.clear();
        }
        else
        {
            VarMap::iterator it = updated.find(p.first);
	    if (it != updated.end())
	    {
		if (it->second.compareTypeAndLimits(p.second))
		{
		    // Zmenime existujici update
		    it->second = p.second;
		    it->second.EventFields.clear(); // TODO: Melo by by union
		}
		else
		{
		    // Udelame z ni redefinici
		    updated.erase(it);
		    created.insert(p);
		    created.at(p.first).EventFields.clear();
		}
	    }
            else
            {
		// Neni ani v created, ani v updated.
		if (was_created)
		{
		    created.insert(p);
		    created.at(p.first).EventFields.clear();
		}
		else
		{
		    updated.insert(p);
		}
            }
        }
    }	
}

bool CmVarChanges::add_foreign_changes(VarMap const &c, VarMap const &u, VarMap const &d)
{
    CM_for (VarMap::value_type const &p, d)
    {
        created.erase(p.first);
        updated.erase(p.first);
        deleted.erase(p.first);
        deleted.insert(p);
    }
    add_foreign_updates(c, true);
    add_foreign_updates(u, false);
    return true;
}

// Pokud promenna nikde neni, tak nic nemenime
//
// Pokud je promenna v created nebo updated, tak ji radeji celou
// nahradime. Lepsi abychom dostali zpetne nasi zmenu, nez neco
// zapomneli.
//
void CmVarChanges::add_my_updates(VarMap const &src)
{
    CM_for (VarMap::value_type const &p, src)
    {
	// Smazeme z deleted, protoze promennou ja redefinuju.
	deleted.erase(p.first);
        VarMap::iterator it = created.find(p.first);
	if (it != created.end())
	{
	    it->second = p.second;
	    it->second.EventFields.clear(); // radsi vsechno
	}
	else
	{
            VarMap::iterator it = updated.find(p.first);
	    if (it != updated.end())
	    {
		if (it->second.compareTypeAndLimits(p.second))
		{
		    // Existuje update, aktualizuj ho
		    it->second = p.second;
		    it->second.EventFields.clear(); // radsi vsechno
		}
		else
		{
		    // Celou predefinuj
		    updated.erase(it);
		    created.insert(p);
		    created.at(p.first).EventFields.clear();
		}
	    }
	}
    }
}

bool CmVarChanges::add_my_changes(VarMap const &c, VarMap const &u, VarMap const &d)
{
    CM_for (VarMap::value_type const &p, d)
    {
	// Pokud jsem ja oddefinoval promennou, tak o tom vim, a tedy
	// vsechny predchazejici zmeny me nezajimaji.
	created.erase(p.first);
	updated.erase(p.first);
	deleted.erase(p.first);
    }
    add_my_updates(c);
    add_my_updates(u);
    return true; // TODO: Dodelat
}

// std::string print_changes(CmVarChanges &ch)
// {
//     std::stringstream ss;
//     ss << "C=" << print_map(ch.created) << ", ";
//     ss << "U=" << print_map(ch.updated) << ", ";
//     ss << "D=" << print_map(ch.deleted);
//     return ss.str();
// }

void apply_var_changes(VarMap &vars, VarMap const &c, VarMap const &u, VarMap const &d)
{
    CM_for (VarMap::value_type const &p, d)
    {
        vars.erase(p.first);
    }
    CM_for (VarMap::value_type const &p, c)
    {
        // assert(p.second.EventFields.empty());
        vars.erase(p.first);
        vars.insert(p);
    }
    CM_for (VarMap::value_type const &p, u)
    {
        vars.erase(p.first);
        vars.insert(p);
	// vars.at(p.first).EventFields.clear();
    }
}

void move_changes(VarMap &dest, VarMap &src, std::string const &pattern)
{
    std::vector<VarMap::iterator> to_delete;
    for (VarMap::iterator it = src.begin(); it != src.end(); ++it)
    {
        if (fnmatch(pattern.c_str(), it->first.c_str(), 0) == 0)
        {
            dest.insert(*it);
            to_delete.push_back(it);
        }
    }
    CM_for (VarMap::iterator it, to_delete)
        src.erase(it);
}

void CmVarChanges::move_some_changes(std::string const &pattern, VarMap &c, VarMap &u, VarMap &d)
{
    c.clear();
    u.clear();
    d.clear();
    move_changes(c, created, pattern);
    move_changes(u, updated, pattern);
    move_changes(d, deleted, pattern);
}

void CmVarChanges::move_all_changes(VarMap &c, VarMap &u, VarMap &d)
{
    c.clear();
    u.clear();
    d.clear();
    std::swap(created, c);
    std::swap(updated, u);
    std::swap(deleted, d);
}


