// Injector.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "../../include/DLLInjector.h"
#include <iostream>

int main()
{
	DLLInjector injector;
	if (injector.inject(L"ExampleDLL.dll", L"InjectTarget.exe"))
	{
		std::cout << "DLL successfully injected!" << std::endl;
	}
	else
	{
		std::cout << "DLL injection failed! Error code : " << injector.GetErrorCode() << std::endl;
	}
	system("pause");
	return 0;
}