#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TextTestRunner.h>

int main(int argc, char **argv) {
        CppUnit::TextTestRunner runner;
	runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
	if(runner.run()) {
		return EXIT_SUCCESS;
	} else {
		return EXIT_FAILURE;
	}
}
