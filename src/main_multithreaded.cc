#include <clang/Tooling/CommonOptionsParser.h>
#include <llvm/Support/CommandLine.h>
#include <filesystem>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <condition_variable>

#include "actions/frontendaction.h"
#include "utils/utils.h"

#include <string>

using namespace std;
using namespace llvm;
using namespace clang;
using namespace clang::tooling;
namespace fs = std::filesystem;

std::mutex io_mutex;
std::mutex queue_mutex;
std::condition_variable cv;
std::queue<std::string> fileQueue;
bool done = false;

void processFile(const string &filePath, CommonOptionsParser &optionsParser, const char *argv0) {
    if (!utils::fileExists(filePath)) {
        std::lock_guard<std::mutex> lock(io_mutex);
        llvm::errs() << "File: " << filePath << " does not exist!\n";
        return;
    }

    auto sourcetxt = utils::getSourceCode(filePath);
    auto compileCommands = optionsParser.getCompilations().getCompileCommands(getAbsolutePath(filePath));

    std::vector<std::string> compileArgs = utils::getCompileArgs(compileCommands);
    compileArgs.push_back("-I" + utils::getClangBuiltInIncludePath(argv0));

    auto xfrontendAction = std::make_unique<XFrontendAction>();
    utils::customRunToolOnCodeWithArgs(std::move(xfrontendAction), sourcetxt, compileArgs, filePath);
}

void worker(CommonOptionsParser &optionsParser, const char *argv0) {
    while (true) {
        std::string filePath;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            cv.wait(lock, [] { return !fileQueue.empty() || done; });

            if (fileQueue.empty() && done) {
                return;
            }

            filePath = fileQueue.front();
            fileQueue.pop();
        }
        processFile(filePath, optionsParser, argv0);
    }
}

void processDirectory(const string &directory, CommonOptionsParser &optionsParser, const char *argv0) {
    for (const auto &entry : fs::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file() && (entry.path().extension() == ".cc" || entry.path().extension() == ".cpp")) {
            {
                std::lock_guard<std::mutex> lock(queue_mutex);
                fileQueue.push(entry.path().string());
            }
            cv.notify_one();
        }
    }
}

int main(int argc, const char **argv) {
    auto start = chrono::high_resolution_clock::now();

    llvm::cl::OptionCategory ctCategory("clang-tool options");
    auto expectedParser = CommonOptionsParser::create(argc, argv, ctCategory);
    if (!expectedParser) {
        llvm::errs() << expectedParser.takeError();
        return -1;
    }

    CommonOptionsParser &optionsParser = expectedParser.get();
    const unsigned int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;

    for (unsigned int i = 0; i < numThreads; ++i) {
        threads.emplace_back(worker, std::ref(optionsParser), argv[0]);
    }

    for (auto &sourcePath : optionsParser.getSourcePathList()) {
        if (fs::exists(sourcePath)) {
            if (fs::is_directory(sourcePath)) {
                processDirectory(sourcePath, optionsParser, argv[0]);
            } else if (fs::is_regular_file(sourcePath)) {
                {
                    std::lock_guard<std::mutex> lock(queue_mutex);
                    fileQueue.push(sourcePath);
                }
                cv.notify_one();
            } else {
                llvm::errs() << "Path: " << sourcePath << " is not a valid file or directory!\n";
                return -1;
            }
        } else {
            llvm::errs() << "Path: " << sourcePath << " does not exist!\n";
            return -1;
        }
    }

    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        done = true;
    }
    cv.notify_all();

    for (auto &thread : threads) {
        if (thread.joinable()) {
            thread.join();
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