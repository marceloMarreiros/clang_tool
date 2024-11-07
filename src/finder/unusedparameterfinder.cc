#include "unusedparameterfinder.h"
#include <clang/AST/ASTContext.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Tooling/Tooling.h>
#include <filesystem>

using namespace clang;
using namespace clang::ast_matchers;
namespace fs = std::filesystem;

UnusedParameterFinder::UnusedParameterFinder(ASTContext &context, const std::string &directory)
    : context(context), rewriter(context.getSourceManager(), context.getLangOpts()), directory(directory) {}

void UnusedParameterFinder::start() {
    MatchFinder finder;

    // Define the matcher for function and method declarations
    auto matcher = functionDecl().bind("functionDecl");
    auto methodMatcher = cxxMethodDecl().bind("methodDecl");

    // Register the matchers
    finder.addMatcher(matcher, this);
    finder.addMatcher(methodMatcher, this);

    // Run the matcher on the AST
    finder.matchAST(context);
}

void UnusedParameterFinder::run(const MatchFinder::MatchResult &result) {
    const SourceManager &sourceManager = result.Context->getSourceManager();

    if (const auto *func = result.Nodes.getNodeAs<FunctionDecl>("functionDecl")) {
        std::string filePath = sourceManager.getFilename(func->getLocation()).str();
        if (filePath.find(directory) == std::string::npos || fs::path(filePath).extension() != ".cpp") {
            return;
        }
        for (const auto *param : func->parameters()) {
            if (!param->isUsed() && !param->getNameAsString().empty()) {
                FullSourceLoc fullLoc(param->getBeginLoc(), sourceManager);
                llvm::outs() << "Unused parameter '" << param->getNameAsString()
                             << "' in function '" << func->getNameAsString()
                             << "' at " << filePath << ":"
                             << fullLoc.getSpellingLineNumber() << ":"
                             << fullLoc.getSpellingColumnNumber() << "\n";
                ++unusedParameterCount; // Increment the count of unused parameters
            }
        }
    } else if (const auto *method = result.Nodes.getNodeAs<CXXMethodDecl>("methodDecl")) {
        std::string filePath = sourceManager.getFilename(method->getLocation()).str();
        if (filePath.find(directory) == std::string::npos || fs::path(filePath).extension() != ".cpp") {
            return;
        }
        for (const auto *param : method->parameters()) {
            if (!param->isUsed() && !param->getNameAsString().empty()) {
                FullSourceLoc fullLoc(param->getBeginLoc(), sourceManager);
                llvm::outs() << "Unused parameter '" << param->getNameAsString()
                             << "' in method '" << method->getNameAsString()
                             << "' at " << filePath << ":"
                             << fullLoc.getSpellingLineNumber() << ":"
                             << fullLoc.getSpellingColumnNumber() << "\n";
                ++unusedParameterCount; // Increment the count of unused parameters
            } 
        }
    }
}

int UnusedParameterFinder::getUnusedParameterCount() const {
    return unusedParameterCount;
}