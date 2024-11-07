#include "consumer.h"

#include "../finder/integervariablefinder.h"
#include "../transformer/functioncalltransformer.h"
#include "../transformer/catchbyreferencetransformer.h"
#include "../finder/catchbyvaluefinder.h"
#include "../finder/unusedparameterfinder.h"

XConsumer::XConsumer(clang::ASTContext &context, const std::string &directory)
    : directory(directory) {
}

void XConsumer::HandleTranslationUnit(clang::ASTContext &context) {
    rewriter.setSourceMgr(context.getSourceManager(), context.getLangOpts());

    // Example of using the UnusedParameterFinder
    UnusedParameterFinder unusedParamFinder(context, directory);
    unusedParamFinder.start();

    auto buffer = rewriter.getRewriteBufferFor(context.getSourceManager().getMainFileID());

    if (buffer != nullptr) {
        std::error_code EC;
        const auto &sourceManager = context.getSourceManager();
        const auto mainFileID = sourceManager.getMainFileID();
        const auto mainFileEntry = sourceManager.getFileEntryForID(mainFileID);
        const auto originalFilePath = mainFileEntry->getName();

        llvm::raw_fd_ostream outFile(originalFilePath, EC, llvm::sys::fs::OF_None);
        if (!EC) {
            buffer->write(outFile);
        } else {
            llvm::errs() << "Error saving transformed file: " << EC.message() << "\n";
        }
    }
}