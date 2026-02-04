#include "param_sfo.cpp"
#include <exception>
#define HEX "\\x"

int main(int argc, char* argv[]) {
    try {
        if(argc > 4)
            throw std::invalid_argument("Invalid command");

        std::string path;
        bool hex = false;

        for(int i = 1; i < argc; i++) {
            std::string cmd(argv[i]);
            if(cmd == HEX)
                hex = true;
            else
                path = cmd;
        }

        if(path.size() == 0)
            throw std::invalid_argument("Invalid PARAM.SFO file path");

        param_sfo::param_sfo_file sfo_file(path);
        sfo_file.print(hex);
    }
    catch(const std::exception& e) {
        std::cout << e.what() << "\n";
    }

    return 0;
}