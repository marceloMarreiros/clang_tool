#ifndef CATCH_BY_VALUE_FINDER_HPP
#define CATCH_BY_VALUE_FINDER_HPP

#include "finder.h"

namespace clang
{
    class ASTContext;
}

class CatchByValueFinder : public Finder
{
  public:
    explicit CatchByValueFinder(clang::ASTContext &context);

    virtual void start() override;
    virtual void run(const clang::ast_matchers::MatchFinder::MatchResult &result);
};

#endif