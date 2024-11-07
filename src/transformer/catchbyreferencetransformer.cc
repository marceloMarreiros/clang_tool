#include "catchbyreferencetransformer.h"
#include <clang/AST/ASTContext.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/Tooling/Tooling.h>

using namespace clang;
using namespace clang::ast_matchers;

CatchByReferenceTransformer::CatchByReferenceTransformer(ASTContext &context, Rewriter &rewriter)
    : Transformer(context, rewriter) {}

void CatchByReferenceTransformer::start() {
    MatchFinder catchFinder;

    // Define the matcher for catch clauses
    auto matcher = cxxCatchStmt().bind("catchStmt");

    // Register the matcher
    catchFinder.addMatcher(matcher, this);

    // Run the matcher on the AST
    catchFinder.matchAST(context);

    // Print transformed files at the end of the transformation
    for (const auto &file : transformedFiles) {
        llvm::outs() << "Transformed file: " << file << "\n";
    }
}

void CatchByReferenceTransformer::run(const MatchFinder::MatchResult &result) {

    if (const auto *catchStmt = result.Nodes.getNodeAs<CXXCatchStmt>("catchStmt")) {
        // Get the exception declaration
        if (VarDecl *exceptionDecl = catchStmt->getExceptionDecl()) {
            QualType exceptionType = exceptionDecl->getType();

            // Check if the exception type is already a reference or a pointer
            if (!exceptionType->isReferenceType() && !exceptionType->isPointerType()) {
                // Get the source range of the exception type
                SourceRange typeRange = exceptionDecl->getTypeSourceInfo()->getTypeLoc().getSourceRange();

                // Get the source code for the exception type
                std::string typeStr = std::string(Lexer::getSourceText(CharSourceRange::getTokenRange(typeRange), rewriter.getSourceMgr(), rewriter.getLangOpts()));

                // Append '&' to the exception type
                std::string newTypeStr = typeStr + "&";

                // Replace the old type with the new type
                rewriter.ReplaceText(typeRange, newTypeStr);

                // Add the file to the set of transformed files
                transformedFiles.insert(rewriter.getSourceMgr().getFilename(typeRange.getBegin()).str());
            }
        }
    }

    // Print transformed files at the end of the transformation
    // if (result.Context->getTranslationUnitDecl() == result.Context->getTranslationUnitDecl()->getASTContext().getTranslationUnitDecl()) {
    //     for (const auto &file : transformedFiles) {
    //         llvm::outs() << "Transformed file: " << file << "\n";
    //     }
    // }
}

void CatchByReferenceTransformer::print(raw_ostream &stream) {
    rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID()).write(stream);
}