#include <clang/Tooling/CommonOptionsParser.h>
#include <llvm/Support/CommandLine.h>
#include <filesystem>
#include <chrono>
#include <iostream>

#include "actions/frontendaction.h"
#include "utils/utils.h"

#include <string>

using namespace std;
using namespace llvm;
using namespace clang;
using namespace clang::tooling;
namespace fs = std::filesystem;

void processFile(const string &filePath, CommonOptionsParser &optionsParser, const char *argv0) {
    if (!utils::fileExists(filePath)) {
        llvm::errs() << "File: " << filePath << " does not exist!\n";
        return;
    }

    auto sourcetxt = utils::getSourceCode(filePath);
    auto compileCommands = optionsParser.getCompilations().getCompileCommands(getAbsolutePath(filePath));

    std::vector<std::string> compileArgs = utils::getCompileArgs(compileCommands);
    compileArgs.push_back("-I" + utils::getClangBuiltInIncludePath(argv0));
    // for (auto &s : compileArgs)
    //     llvm::outs() << s << "\n";

    auto xfrontendAction = std::make_unique<XFrontendAction>();
    utils::customRunToolOnCodeWithArgs(move(xfrontendAction), sourcetxt, compileArgs, filePath);
}

void processDirectory(const string &directory, CommonOptionsParser &optionsParser, const char *argv0) {
    for (const auto &entry : fs::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file() && (entry.path().extension() == ".cc" || entry.path().extension() == ".cpp")) {
            processFile(entry.path().string(), optionsParser, argv0);
        }
    }
}

int main(int argc, const char **argv)
{
    auto start = chrono::high_resolution_clock::now();
    
    llvm::cl::OptionCategory ctCategory("clang-tool options");
    auto expectedParser = CommonOptionsParser::create(argc, argv, ctCategory);
    if (!expectedParser)
    {
        llvm::errs() << expectedParser.takeError();
        return -1;
    }

    CommonOptionsParser &optionsParser = expectedParser.get();
    for (auto &sourcePath : optionsParser.getSourcePathList())
    {
        if (fs::exists(sourcePath))
        {
            if (fs::is_directory(sourcePath))
            {
                processDirectory(sourcePath, optionsParser, argv[0]);
            }
            else if (fs::is_regular_file(sourcePath))
            {
                processFile(sourcePath, optionsParser, argv[0]);
            }
            else
            {
                llvm::errs() << "Path: " << sourcePath << " is not a valid file or directory!\n";
                return -1;
            }
        }
        else
        {
            llvm::errs() << "Path: " << sourcePath << " does not exist!\n";
            return -1;
        }
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    auto hours = chrono::duration_cast<chrono::hours>(duration);
    duration -= hours;
    auto minutes = chrono::duration_cast<chrono::minutes>(duration);
    duration -= minutes;
    auto seconds = chrono::duration_cast<chrono::seconds>(duration);

    cout << "Execution time: " << hours.count() << " hours "
         << minutes.count() << " minutes "
         << seconds.count() << " seconds" << endl;


    return 0;
}