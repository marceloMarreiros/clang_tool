#ifndef FRONTENDACTION_H
#define FRONTENDACTION_H

#include <clang/Frontend/FrontendAction.h>
#include <string>

class XFrontendAction : public clang::ASTFrontendAction {
public:
    XFrontendAction(const std::string &directory) : directory(directory) {}

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &compiler, llvm::StringRef inFile) override;

private:
    std::string directory;
};

#endif // FRONTENDACTION_H