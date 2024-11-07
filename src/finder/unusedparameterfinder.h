#ifndef UNUSED_PARAMETER_FINDER_H
#define UNUSED_PARAMETER_FINDER_H

#include <clang/AST/ASTContext.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Tooling/Tooling.h>
#include <string>

class UnusedParameterFinder : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
    UnusedParameterFinder(clang::ASTContext &context, const std::string &directory);

    void start();
    void run(const clang::ast_matchers::MatchFinder::MatchResult &result) override;
    int getUnusedParameterCount() const;

private:
    clang::ASTContext &context;
    clang::Rewriter rewriter;
    std::string directory; // Add this line to declare the directory member variable
    int unusedParameterCount;
};

#endif // UNUSED_PARAMETER_FINDER_H