#include <iostream>
#include <string>
#include <functional>

std::function<void()> g_callback;

class Foo
{
public:
	Foo()
	{
		std::cout << "Foo created\n";

		m_str = "Hello, World!";
	}

	~Foo()
	{
		std::cout << "Foo destroyed\n";
	}

	void Register()
	{
		// this만 캡처 (수명 보장 없음)
		g_callback = [this]()
			{
				std::cout << "Foo::Run(), this = " << this << "\n";
				DoSomething();
			};
	}

	void DoSomething()
	{
		std::cout << "Doing something\n";

		std::cout << m_str << "\n";
	}


private:

	std::string m_str;
};

int main()
{
	Foo* f = new Foo();

	f->Register();

	delete f;

	std::cout << "Calling callback...\n";


	g_callback();

	return 0;
}
