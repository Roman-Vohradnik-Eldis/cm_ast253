#include "cm_var_utils.hh"

template <typename T>
void print_variable_value(std::ostream &o, CCmVariable const &v, unsigned y, unsigned x, bool (CCmVariable::*get)(T &, unsigned, unsigned) const)
{
    T val;
    if ((v.*get)(val, y, x))
	o << val;
    else
	o << "<bad-value>";
}

void print_any_variable(std::ostream &o, CCmVariable const &v, unsigned y, unsigned x)
{
    switch (v.getType())
    {
    case 'i':
	return print_variable_value<libcm_integer>(o, v, y, x, &CCmVariable::getVariableInt);
    case 's':
	return print_variable_value<std::string>(o, v, y, x, &CCmVariable::getVariableString);
    case 'b':
	return print_variable_value<bool>(o, v, y, x, &CCmVariable::getVariableBool);
    case 'e':
	return print_variable_value<std::string>(o, v, y, x, &CCmVariable::getVariableEnum);
    case 'f':
	return print_variable_value<double>(o, v, y, x, &CCmVariable::getVariableFloat);
    default:
	o << "<not-set>";
    }
}

std::string print_var(CCmVariable const &v)
{
    std::stringstream ss;
    ss << v.getName();
    ss << "/" << v.getType();
    ss << ":";
    bool first = true;
    unsigned x_size = v.getArrayX();
    unsigned y_size = v.getArrayY();
    for (unsigned x = 0; x < x_size; ++x)
    {
        for (unsigned y = 0; y < y_size; ++y)
        {
            if (first)
                first = false;
            else
                ss << ",";
	    print_any_variable(ss, v, y, x);
        }
    }
    CM_for (unsigned i, v.EventFields)
        ss << "+" << i;
    return ss.str();
}

std::string print_var(VariableEvent const &v)
{
    return print_var(v.variable);
}

std::string print_var(CCmFilter const &f)
{
    return f.toString();
}

std::string print_var(CCmFiltersSet const &set)
{
    return set.toString();
}
