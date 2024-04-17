#pragma once

#include "../includes/context/Context.hpp"

typedef std::string str_t;

class Command {
protected:
    Context* context;
public:
    Command(Context* ctx): context(ctx) { }
    virtual ~Command() { };
    virtual void apply() = 0;
    
};
