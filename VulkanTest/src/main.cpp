#include <stdexcept>
#include <cstdlib>
#include <Application.h>

/**
 * 程序的main函数
 *
 * @return 如果程序正常允许返回EXIT_SUCCESS , 否则EXIT_FAILURE.
 *
 * @throws 如果程序出错抛出std::exception
 */
int main() {
	EngineCore::MioEngine engine = EngineCore::MioEngine();

	try
	{
		engine.run();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}
	

	return EXIT_SUCCESS;
}
