#include "unusedparametertransformer.h"
#include <clang/AST/ASTContext.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Tooling/Tooling.h>
#include <filesystem>

using namespace clang;
using namespace clang::ast_matchers;
namespace fs = std::filesystem;

UnusedParameterTransformer::UnusedParameterTransformer(ASTContext &context, Rewriter &rewriter, const std::string &directory)
    : UnusedParameterFinder(context, directory), rewriter(rewriter) {}

void UnusedParameterTransformer::run(const MatchFinder::MatchResult &result) {
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

                // Comment out the unused parameter
                SourceLocation paramStartLoc = param->getBeginLoc();
                std::string paramName = param->getNameAsString();
                std::string replacementText = "/*" + paramName + "*/";
                rewriter.ReplaceText(paramStartLoc, paramName.length(), replacementText);

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

                // Comment out the unused parameter
                SourceLocation paramStartLoc = param->getBeginLoc();
                std::string paramName = param->getNameAsString();
                std::string replacementText = "/*" + paramName + "*/";
                rewriter.ReplaceText(paramStartLoc, paramName.length(), replacementText);

            }
        }
    }
}