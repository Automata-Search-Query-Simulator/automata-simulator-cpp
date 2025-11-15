#pragma once

#include <memory>
#include <string>

#include "AutomatonPlan.hpp"
#include "IRunner.hpp"
#include "automata/builders/Builders.hpp"

namespace automata {

class NfaRunner : public IRunner {
  public:
    explicit NfaRunner(Nfa nfa, bool trace);
    RunResult run(const std::string& input) override;

  private:
    Nfa nfa_;
    bool trace_;
};

class DfaRunner : public IRunner {
  public:
    explicit DfaRunner(Dfa dfa, bool trace);
    RunResult run(const std::string& input) override;

  private:
    Dfa dfa_;
    bool trace_;
};

class EfaRunner : public IRunner {
  public:
    explicit EfaRunner(Efa efa, bool trace);
    RunResult run(const std::string& input) override;

  private:
    Efa efa_;
    bool trace_;
};

class PdaRunner : public IRunner {
  public:
    PdaRunner(Pda pda, bool trace);
    RunResult run(const std::string& input) override;

  private:
    Pda pda_;
    bool trace_;
};

class RunnerFactory {
  public:
    RunnerPtr create(const AutomatonPlan& plan, const RegexParser& parser) const;
};

}  // namespace automata
