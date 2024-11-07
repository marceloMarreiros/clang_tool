#include <clang/Tooling/CommonOptionsParser.h>
#include <llvm/Support/CommandLine.h>
#include <filesystem>
#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>

#include "actions/frontendaction.h"
#include "utils/utils.h"

#include <string>

using namespace std;
using namespace llvm;
using namespace clang;
using namespace clang::tooling;
namespace fs = std::filesystem;

bool fileContainsCatch(const string &filePath) {
    std::ifstream file(filePath);
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("catch") != std::string::npos) {
            return true;
        }
    }
    return false;
}

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

void processDirectory(const string &directory, CommonOptionsParser &optionsParser, const char *argv0, int &totalFiles, int &filesWithCatch, vector<string> &filesToProcess) {
    for (const auto &entry : fs::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file() && (entry.path().extension() == ".cc" || entry.path().extension() == ".cpp")) {
            totalFiles++;
            if (fileContainsCatch(entry.path().string())) {
                filesWithCatch++;
                filesToProcess.push_back(entry.path().string());
            }
        }
    }
}

int main(int argc, const char **argv) {
    // Print a start message
    cout << "========================================" << endl;
    cout << " Starting Catch By Reference Transformer" << endl;
    cout << "========================================" << endl;

    auto start = chrono::high_resolution_clock::now();
    
    llvm::cl::OptionCategory ctCategory("clang-tool options");
    auto expectedParser = CommonOptionsParser::create(argc, argv, ctCategory);
    if (!expectedParser) {
        llvm::errs() << expectedParser.takeError();
        return -1;
    }

    CommonOptionsParser &optionsParser = expectedParser.get();
    int totalFiles = 0;
    int filesWithCatch = 0;
    vector<string> filesToProcess;

    for (auto &sourcePath : optionsParser.getSourcePathList()) {
        if (fs::exists(sourcePath)) {
            if (fs::is_directory(sourcePath)) {
                processDirectory(sourcePath, optionsParser, argv[0], totalFiles, filesWithCatch, filesToProcess);
            } else if (fs::is_regular_file(sourcePath)) {
                totalFiles++;
                if (fileContainsCatch(sourcePath)) {
                    filesWithCatch++;
                    filesToProcess.push_back(sourcePath);
                }
            } else {
                llvm::errs() << "Path: " << sourcePath << " is not a valid file or directory!\n";
                return -1;
            }
        } else {
            llvm::errs() << "Path: " << sourcePath << " does not exist!\n";
            return -1;
        }
    }

    // Show the user the percentage of files that have catch
    if (totalFiles > 0) {
        double percentage = (static_cast<double>(filesWithCatch) / totalFiles) * 100;
        cout << "Total .cpp and .cc files: " << totalFiles << endl;
        cout << "Files with 'catch': " << filesWithCatch << endl;
        cout << "Percentage of files with 'catch': " << percentage << "%" << endl;
    } else {
        cout << "No .cpp or .cc files found." << endl;
    }

    // Process only the files that contain 'catch'
    if (filesWithCatch > 0) {
        for (const auto &file : filesToProcess) {
            processFile(file, optionsParser, argv[0]);
        }
    } else {
        cout << "No files with 'catch' statements found." << endl;
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    auto hours = chrono::duration_cast<chrono::hours>(duration);
    duration -= hours;
    auto minutes = chrono::duration_cast<chrono::minutes>(duration);
    duration -= minutes;
    auto seconds = chrono::duration_cast<chrono::seconds>(duration);

    cout << "========================================" << endl;
    cout << " Catch By Reference Transformer completed" << endl;
    cout << "========================================" << endl;
    cout << "Execution time: " << hours.count() << " hours "
         << minutes.count() << " minutes "
         << seconds.count() << " seconds" << endl;

    return 0;
}