#include "catchbyvaluefinder.h"

#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>
#include <clang/ASTMatchers/ASTMatchers.h>

CatchByValueFinder::CatchByValueFinder(clang::ASTContext &context) : Finder(context)
{
}

void CatchByValueFinder::start()
{
    using namespace clang::ast_matchers;

    MatchFinder catchFinder;

    auto catchByValueMatcher = cxxCatchStmt(has(varDecl(unless(hasType(referenceType()))))).bind("catchByValue");
    catchFinder.addMatcher(catchByValueMatcher, this);

    catchFinder.matchAST(context);
}

void CatchByValueFinder::run(const clang::ast_matchers::MatchFinder::MatchResult &result)
{
    using namespace clang;

    if (const CXXCatchStmt *catchStmt = result.Nodes.getNodeAs<CXXCatchStmt>("catchByValue"))
    {
        if (result.SourceManager->isInSystemHeader(catchStmt->getSourceRange().getBegin()))
            return;

        llvm::outs() << "Catch by value found at: " << catchStmt->getBeginLoc().printToString(*result.SourceManager) << "\n";
    }
}