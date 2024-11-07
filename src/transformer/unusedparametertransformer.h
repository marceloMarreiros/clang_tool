#ifndef UNUSED_PARAMETER_TRANSFORMER_H
#define UNUSED_PARAMETER_TRANSFORMER_H

#include "../finder/unusedparameterfinder.h"

class UnusedParameterTransformer : public UnusedParameterFinder {
public:
    UnusedParameterTransformer(clang::ASTContext &context, clang::Rewriter &rewriter, const std::string &directory);

    void run(const clang::ast_matchers::MatchFinder::MatchResult &result) override;

private:
    clang::Rewriter &rewriter;
};

#endif // UNUSED_PARAMETER_TRANSFORMER_H