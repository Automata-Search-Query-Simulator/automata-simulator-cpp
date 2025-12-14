#include <exception>
#include <iostream>

void runParserTests();
void runBuilderTests();
void runRunnerTests();
void runIntegrationTests();

int main() {
    try {
        runParserTests();
        runBuilderTests();
        runRunnerTests();
        runIntegrationTests();
    } catch (const std::exception& ex) {
        std::cerr << "Test failure: " << ex.what() << "\n";
        return 1;
    }
    std::cout << "All tests passed.\n";
    return 0;
}
