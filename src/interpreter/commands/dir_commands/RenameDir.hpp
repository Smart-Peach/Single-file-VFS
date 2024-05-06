
#include "../Command.hpp"
#include "../../includes/Context.hpp"

class RenameDir: public Command {
public:
    RenameDir() { }
    RenameDir(const RenameDir& other) = default;
    RenameDir& operator=(const RenameDir& other) = default;
    ~RenameDir() = default;

    void apply(Context* context) override {
        std::cout << "RenameDir command was applied to \"" << context->get_basic_src_name() << "\"" << std::endl;
        return;
    }
};