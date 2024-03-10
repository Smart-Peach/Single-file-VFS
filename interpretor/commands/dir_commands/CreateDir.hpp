#include "../Command.hpp"

class CreateDir: public Command {

public:
    CreateDir() = default;
    CreateDir(str_t dir_name):
        dir_name(dir_name) {};
    ~CreateDir() {};

    void apply() override {
        std::cout << "directory " << dir_name << " created" << std::endl;
        return;
    }

private:
    str_t dir_name;
};