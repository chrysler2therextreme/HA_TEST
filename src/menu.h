#include <iostream>
#include <vector>
#include <string>
#include <conio.h>
#include <functional>
#include <memory>
#include "ErrorHandling.h"
#include "smsManager.h"
using namespace std::placeholders;



class BaseMenu
{
	std::string title;
public:
	BaseMenu(const std::string& s) : title(s) {}

	std::string get_title() { return title; }

	virtual ~BaseMenu() {}
	virtual void command() = 0;
};



class PopupMenu : public BaseMenu
{

	std::vector<std::shared_ptr<BaseMenu>> v;
public:
	PopupMenu(const std::string& s) : BaseMenu(s) {}

	void add(std::shared_ptr<BaseMenu> p) { v.push_back(p); }

	void command()
	{
		while (1)
		{
			//system("cls");
			//std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

			size_t sz = v.size();
			for (int i = 0; i < sz; i++)
			{
				std::cout << i + 1 << ". " << v[i]->get_title() << std::endl;

			}
			//std::cout << sz + 1 << ". 종료\n";


			int cmd;
			std::cout << "메뉴를 선택하세요 >> ";
			//std::cin >> cmd;
			cmd = ErrorHandling::getIntInput("정수를 입력하세요: ");

			

			if (cmd < 1 || cmd > sz  )
				continue;

			v[cmd - 1]->command();

			if (cmd == sz)
				break;

			std::cout << std::endl;

		}
	}
};

class MenuItem : public BaseMenu
{
	int id;
	using HANDLER = std::function<void()>;

	HANDLER handler; // 메뉴 선택시 호출될 함수

public:
	MenuItem(const std::string& s, int n, HANDLER h = nullptr)
		: BaseMenu(s), id(n), handler(h) {}

	void command()
	{
		// 메뉴 선택시 등록된 함수 호출!
		if (handler != nullptr)
			handler();

	}
};

