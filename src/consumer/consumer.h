#ifndef CONSUMER_HPP
#define CONSUMER_HPP

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <string>

namespace clang {
    class ASTContext;
}

class XConsumer : public clang::ASTConsumer {
private:
    clang::Rewriter rewriter;
    std::string directory;

public:
    XConsumer(clang::ASTContext &context, const std::string &directory);
    virtual void HandleTranslationUnit(clang::ASTContext &context) override;
};

#endif // CONSUMER_HPP