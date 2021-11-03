#include <iostream>
#include <vector>
#include <algorithm>

class InputParser {
public:
    InputParser (int &argc, char **argv) {
        for (int i=1; i < argc; ++i)
            this->tokens.emplace_back(argv[i]);
    }

    const std::string& getCmdOption(const std::string &option) const {
        std::vector<std::string>::const_iterator itr;
        itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
        if (itr != this->tokens.end() && ++itr != this->tokens.end())
            return *itr;
        static const std::string empty_string;
        return empty_string;
    }

    bool cmdOptionExists(const std::string &option) const {
        return std::find(this->tokens.begin(), this->tokens.end(), option) != this->tokens.end();
    }
private:
    std::vector <std::string> tokens;
};


void help() {
    std::cout << "usages: RM0_Encoder.exe [-h] [{-v, -q}] -f <FILE> [-o <OUTPUT_FILE>]\n\n"
              << "This piece of software encodes haptic files into the RM0 format submitted to the MPEG CfP call for Haptic standardization\n"
              << "\npositional arguments:\n"
              << "\t-f, --file <FILE>\t\tfile to convert\n"
              << "\noptional arguments:\n"
              << "\t-h, --help\t\t\tshow this help message and exit\n"
              << "\t-v, --verbose\t\t\tbe more verbose\n"
              << "\t-q, --quiet\t\t\tbe more quiet\n"
              << "\t-o, --output<OUTPUT_FILE>\toutput file\n"
            ;
}


int main(int argc, char * argv[]) {
    InputParser inputParser(argc, argv);
    if(inputParser.cmdOptionExists("-h") || inputParser.cmdOptionExists("--help")) {
        help();
        return EXIT_SUCCESS;
    }

    std::string filename = inputParser.getCmdOption("-f");
    if (filename.empty()) filename = inputParser.getCmdOption("--file");
    if (filename.empty()) {
        help();
        return EXIT_FAILURE;
    }

    std::cout << "The file to process is : " << filename << "\n";
    std::string output = inputParser.getCmdOption("-o");
    if (output.empty()) output = inputParser.getCmdOption("--output");
    if (!output.empty())
        std::cout << "The generated file will be : " << output << "\n";
    return EXIT_SUCCESS;
}