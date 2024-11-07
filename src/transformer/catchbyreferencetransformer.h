#ifndef CATCHBYREFERENCETRANSFORMER_HPP
#define CATCHBYREFERENCETRANSFORMER_HPP

#include "transformer.h"
#include <clang/ASTMatchers/ASTMatchFinder.h>

namespace clang {
    class ASTContext;
    class raw_ostream;
    class Rewriter;
}

class CatchByReferenceTransformer : public Transformer {
public:
    explicit CatchByReferenceTransformer(clang::ASTContext &context, clang::Rewriter &rewriter);

    void start() override;
    void run(const clang::ast_matchers::MatchFinder::MatchResult &result) override;
    void print(clang::raw_ostream &stream) override;

private:
    std::set<std::string> transformedFiles;
    bool fileContainsCatch(const std::string &filePath);
};

#endif