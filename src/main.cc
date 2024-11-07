#include <clang/Tooling/CommonOptionsParser.h>
#include <llvm/Support/CommandLine.h>
#include <filesystem>
#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>

#include "actions/frontendaction.h"
#include "utils/utils.h"
#include "finder/unusedparameterfinder.h"

#include <string>

using namespace std;
using namespace llvm;
using namespace clang;
using namespace clang::tooling;
namespace fs = std::filesystem;

void processFile(const string &filePath, CommonOptionsParser &optionsParser, const char *argv0, const string &directory) {
    if (!utils::fileExists(filePath)) {
        llvm::errs() << "File: " << filePath << " does not exist!\n";
        return;
    } 
    else {
        llvm::outs() << "Processing file: " << filePath << "\n";
    }

    auto sourcetxt = utils::getSourceCode(filePath);
    auto compileCommands = optionsParser.getCompilations().getCompileCommands(getAbsolutePath(filePath));

    std::vector<std::string> compileArgs = utils::getCompileArgs(compileCommands);
    compileArgs.push_back("-I" + utils::getClangBuiltInIncludePath(argv0));

    auto xfrontendAction = std::make_unique<XFrontendAction>(directory);
    utils::customRunToolOnCodeWithArgs(move(xfrontendAction), sourcetxt, compileArgs, filePath);
}

void processDirectory(const string &directory, CommonOptionsParser &optionsParser, const char *argv0) {
    for (const auto &entry : fs::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".cpp") {
            processFile(entry.path().string(), optionsParser, argv0, directory);
        }
    }
}

int main(int argc, const char **argv) {
    // Print a start message
    cout << "========================================" << endl;
    cout << " Starting Unused Parameter Checker" << endl;
    cout << "========================================" << endl;

    auto start = chrono::high_resolution_clock::now();
    
    llvm::cl::OptionCategory ctCategory("clang-tool options");
    auto expectedParser = CommonOptionsParser::create(argc, argv, ctCategory);
    if (!expectedParser) {
        llvm::errs() << expectedParser.takeError();
        return -1;
    }

    CommonOptionsParser &optionsParser = expectedParser.get();
    if (argc < 2) {
        llvm::errs() << "Usage: " << argv[0] << " <directory>\n";
        return -1;
    }

    std::string directory = argv[1];

    // UnusedParameterFinder finder(optionsParser.getCompilations().getASTContext(), directory);

    for (auto &sourcePath : optionsParser.getSourcePathList()) {
        fs::path sourcePathFs(sourcePath);
        if (fs::exists(sourcePathFs)) {
            if (fs::is_directory(sourcePathFs)) {
                llvm::outs() << "Analyzing directory: " << sourcePathFs << "\n";
                processDirectory(sourcePath, optionsParser, argv[0]);
            } else if (fs::is_regular_file(sourcePathFs) && sourcePathFs.extension() == ".cpp") {
                llvm::outs() << "Accessing file: " << sourcePathFs << "\n";
                processFile(sourcePath, optionsParser, argv[0], sourcePathFs.parent_path().string());
            } else {
                llvm::errs() << "Path: " << sourcePath << " is not a valid .cpp file or directory!\n";
                return -1;
            }
        } else {
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

    cout << "========================================" << endl;
    cout << " Unused Parameter Checker completed" << endl;
    cout << "========================================" << endl;
    // cout << "Total unused parameters found: " << finder.getUnusedParameterCount() << endl;
    cout << "Execution time: " << hours.count() << " hours "
         << minutes.count() << " minutes "
         << seconds.count() << " seconds" << endl;

    return 0;
}