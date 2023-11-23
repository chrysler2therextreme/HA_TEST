#pragma warning (disable : 4996)
#include "menu.h"
#include <chrono>

SMSManager sms;

enum { SENDAIR = 1, COOLER = 2, HEATER = 3 };
enum { NO_OPERATION = 1, WARM = 2, COOK = 3, HEAT= 4 };
time_t power_global=0;

class Shape
{
	std::vector<std::shared_ptr<Shape>>  v;
protected:
	std::string machineName;
	int type;
	bool powerFlag;
	int powerConsumption; 
	int state1; 
	int state2;
	time_t startHour;
	time_t endHour;
	std::vector<std::shared_ptr<time_t>>  useHour;// [endhour- starthour] 계산해서 off될때마다 등록

public:

	Shape(std::string name, int type, int power) : machineName(name), type(type), powerConsumption(power) { 
		sms.add_sms_number("AIRCONDITION_POWER_ON", "010-111-2222"); 
		sms.add_sms_number("RICECOOKER_POWER_ON", "010-222-3333");
		sms.add_sms_number("RICECOOKER_POWER_ON", "010-333-4444");
		sms.add_sms_number("WASHER_POWER_ON", "010-333-4444");
		sms.add_sms_number("LIGHT_POWER_ON", "010-333-4444");
	}
	virtual ~Shape() {}

	void save(FILE* f)
	{
		fprintf(f, "%d ", type);
		fprintf(f, "%s %d %d %d %d %d %lld %lld %d\n", 
			machineName.c_str(), type, powerFlag, powerConsumption, state1, state2, startHour, endHour, useHour.size());
		
		for (const auto& hour : useHour)	{fprintf(f, "%lld ", *hour);}// 사용 시간 저장

		fprintf(f, "\n");
	}

	void load_state(FILE* f) {
		
		char buffer[100]; // 이름값을 buffer에 저장 후 대입
		
		fscanf(f, "%s",&buffer);
		std::cout << "machineName :" << buffer << std::endl;
		machineName = buffer;
		
		fscanf(f, "%d %d %d %d %d %lld %zu",   //나머지 인자 load
			&type, &powerFlag, &powerConsumption, &state1, &state2, &startHour, &endHour);

		size_t numHours;  // vector변수 usehour 별도 처리
		fscanf(f, "%zu", &numHours);
		useHour.resize(numHours);

		for (auto& hour : useHour) {
			hour = std::make_shared<time_t>();
			fscanf(f, "%lld", hour.get());
		}
	}


	void add(std::shared_ptr<Shape> p) { v.push_back(p); }
	
	std::vector<std::shared_ptr<Shape>>& get_v() { return v; }

	virtual int change_state() { return -1; }

	virtual void showstatus() = 0;

	void calc_power() 
	{
		std::shared_ptr<time_t> watt = std::make_shared<time_t>();
		*watt = get_endHour() - get_startHour();
		std::cout << "## \"" << get_name() <<"\"" << "의 " << (useHour.size()) + 1 << "번째 소비전력 : " << *watt << std::endl;
		useHour.push_back(watt);
	}

	
	void set_name(const std::string c) { machineName = c; }
	std::string get_name() { return machineName; }

	void set_state1(int num) { state1 = num; }
	int get_state1() { return state1; }
	void set_state2(int num) { state2 = num; }
	int get_state2() { return state2; }

	void set_powerconsumption(int num) { powerConsumption = num; }
	int get_powerconsumption() { return powerConsumption; }

	void set_power(bool flag) {

		if (flag == false) {
			if (powerFlag == true)
			{
				set_endHour(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
				calc_power();
			}
			else {
				;
			}
			powerFlag = false;

		}
		else if ((powerFlag == true && flag == true) || (powerFlag == false && flag == false)) {
			std::cout << "입력한 전원상태가 동일합니다" << std::endl;
		}
		else if (flag == true) {
			set_startHour(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
			powerFlag = true;
			if (type == 1) {
				sms.send_sms("AIRCONDITION_POWER_ON", "AIRCONDITION이 켜졌습니다.");
			}
			else if (type == 2) {
				sms.send_sms("RICECOOKER_POWER_ON", "RICECOOKER이 켜졌습니다.");
			}
			else if (type == 3) {
				sms.send_sms("WASHER_POWER_ON", "WASHER이 켜졌습니다.");
			}
			else if (type == 4) {
				sms.send_sms("LIGHT_POWER_ON", "LIGHT이 켜졌습니다.");
			}
		}

		else          //get_power() == flag
			return;
	}
	bool get_power() { return powerFlag; }

	void set_startHour(time_t num) { startHour = num; }
	time_t get_startHour() { return startHour; }
	void set_endHour(time_t num) { endHour = num; }
	time_t get_endHour() { return endHour; }
	std::vector<std::shared_ptr<time_t>>& get_useHour() { return useHour; }
	
};

class Aircondition : public Shape
{
protected:
	
public:
	Aircondition(const std::string& s, int type, int power) : Shape(s, type, power) { set_power(true); 
	set_startHour(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())); // 생성자에서 startHour 변수에 현재시각입력
	set_state1(-1); set_state2(-1);
	}

	//~Aircondition();
	

	int change_state(void) override
	{
		int num1;
		if (get_power() == false)
		{
			std::cout << "에어컨 전원을 켜시겠습니까?(YES :1, NO : 2) : " << std::endl;
			num1 = ErrorHandling::getIntInput("정수를 입력하세요: ");
			
			if (num1 == 2)
			{
				std::cout << "상태변경이 불가합니다. 메인메뉴로 돌아갑니다. " << std::endl; return -3;
			}
			else if (num1 == 1)
			{
				std::cout << "에어컨 전원이 켜졌습니다 " << std::endl;
				set_power(true);
			}
			else
			{
				std::cout << "입력값이 잘못되었습니다 " << std::endl; return -2;
			}
		}
		else
		{
			std::cout << "에어컨 전원을 꺼시겠습니까?(YES :1, NO : 2) : " << std::endl;
			num1 = ErrorHandling::getIntInput("정수를 입력하세요: ");
			if (num1 == 2)
			{
				std::cout << "상태변경이 불가합니다. 메인메뉴로 돌아갑니다. " << std::endl; return -3;
			}
			else if ((num1 == 1))
			{
				std::cout << "에어컨 전원을 껐습니다 " << std::endl;
				set_power(false);
				//return 1;    //전원이 꺼진상태에서도 state1, state2 제어가능하도록
			}
			else
			{
				std::cout << "입력값이 잘못되었습니다 " << std::endl; return -2;
			}


		}

		int num2;
		std::cout << "에어컨 상태 선택하세요(SENDAIR = 1, COOLER = 2, HEATER = 3) : ";
		
		num2 = ErrorHandling::getIntInput("정수를 입력하세요: ");
		set_state1(num2);
		std::cout << "에어컨 온도 입력하세요 : ";
		
		num2 = ErrorHandling::getIntInput("정수를 입력하세요: ");
		set_state2(num2);

		return state1;
	}
	void showstatus() override
	{
		std::cout << "[에어컨] 이름 :" <<get_name() << std::endl << "전원상태 :" <<get_power() << 
			"  동작상태 :" << get_state1() << "  설정온도 :" << get_state2() << std::endl;
	}


};

class Ricecooker : public Shape
{
protected:
	
public:
	Ricecooker(const std::string& s, int type, int power) : Shape(s, type, power){
		set_power(false); set_startHour(0); set_state1(-1);	}
	

	int change_state(void) override
	{
		int num1;
		if (get_power() == false)
		{
			std::cout << "밥솥 전원을 켜시겠습니까?(YES :1, NO : 2) : "<<std::endl;
			num1 = ErrorHandling::getIntInput("정수를 입력하세요: ");
			if (num1 == 2)
			{
				std::cout << "상태변경이 불가합니다. 메인메뉴로 돌아갑니다. " << std::endl; return -3;
			}
			else if (num1 == 1)
			{
				std::cout << "밥솥 전원이 켜졌습니다 " << std::endl;
				set_power(true);
			}
			else
			{
				std::cout << "입력값이 잘못되었습니다 " << std::endl; return -2;
			}
		}
		else
		{
			std::cout << "밥솥 전원을 꺼시겠습니까?(YES :1, NO : 2) : " << std::endl;
			num1 = ErrorHandling::getIntInput("정수를 입력하세요: ");
			if (num1 == 2)
			{
				std::cout << "상태변경이 불가합니다. 메인메뉴로 돌아갑니다. " << std::endl; return -3;
			}
			else if ((num1 == 1))
			{
				std::cout << "밥솥 전원을 껐습니다 " << std::endl;
				set_power(false);
				return 1;
			}
			else
			{
				std::cout << "입력값이 잘못되었습니다 " << std::endl; return -2;
			}
			

		}

		/* state 1 설정 */
		int num2;
		std::cout <<  "밥솥 상태 선택하세요(NO_OPERATION = 1, WARM = 2, COOK = 3, HEAT= 4) : ";
		num2 = ErrorHandling::getIntInput("정수를 입력하세요: ");
		set_state1(num2);
		if (get_state1() == 1)
		{
			set_power(false);
		}

		return state1;
	}
	void showstatus()override
	{
		std::cout << "[밥솥] 이름 :" << get_name() << std::endl << "전원상태 :" << get_power() << 
			"  동작상태 :" << get_state1() << std::endl;
	}
	
};

class Washer : public Shape
{
protected:

public:
	Washer(const std::string& s, int type, int power) : Shape(s, type, power) {
		set_power(false); set_startHour(0); set_state1(-1);}

	int change_state(void) override
	{
		int num1;
		if (get_power() == false)
		{
			std::cout << "세탁기 전원을 켜시겠습니까?(YES :1, NO : 2) : " << std::endl;
			num1 = ErrorHandling::getIntInput("정수를 입력하세요: ");
			if (num1 == 2)
			{
				std::cout << "상태변경이 불가합니다. 메인메뉴로 돌아갑니다. " << std::endl; return -3;
			}
			else if (num1 == 1)
			{
				std::cout << "세탁기 전원이 켜졌습니다 " << std::endl;
				set_power(true);
			}
			else
			{
				std::cout << "입력값이 잘못되었습니다 " << std::endl; return -2;
			}
		}
		else
		{
			std::cout << "세탁기 전원을 꺼시겠습니까?(YES :1, NO : 2) : " << std::endl;
			num1 = ErrorHandling::getIntInput("정수를 입력하세요: ");
			if (num1 == 2)
			{
				std::cout << "상태변경이 불가합니다. 메인메뉴로 돌아갑니다. " << std::endl; return -3;
			}
			else if ((num1 == 1))
			{
				std::cout << "세탁기 전원을 껐습니다 " << std::endl;
				set_power(false);
				return state1;
			}
			else
			{
				std::cout << "입력값이 잘못되었습니다 " << std::endl; return -2;
			}


		}

		/* state 1 */

		std::cout << "세탁기 상태 선택하세요(NO_OPERATION = 1, GENERAL = 2, BOIL = 3, DRY= 4) : ";
		int num2 = ErrorHandling::getIntInput("정수를 입력하세요: ");
		set_state1(num2);
		if (get_state1() == 1)
		{
			set_power(false);
		}
		return state1;

	}

	void showstatus()override
	{
		std::cout << "[세탁기] 이름 :" << get_name() << std::endl << "전원상태 :" << get_power() <<
			"  동작상태 :" << get_state1() << std::endl;
	}

};

class Light : public Shape
{
protected:

public:
	Light(const std::string& s, int type, int power) : Shape(s, type, power) { set_power(false); set_startHour(0); }

	int change_state() override
	{
		int num1;
		if (get_power() == false)
		{
			std::cout << "전등 전원을 켜시겠습니까?(YES :1, NO : 2) : " << std::endl;
			num1 = ErrorHandling::getIntInput("정수를 입력하세요: ");

			if (num1 == 2)
			{
				std::cout << "상태변경이 불가합니다. 메인메뉴로 돌아갑니다. " << std::endl; return -3;
			}
			else if (num1 == 1)
			{
				std::cout << "전등 전원이 켜졌습니다 " << std::endl;
				set_power(true);
			}
			else
			{
				std::cout << "입력값이 잘못되었습니다 " << std::endl; return -2;
			}
		}
		else
		{
			std::cout << "전등 전원을 꺼시겠습니까?(YES :1, NO : 2) : " << std::endl;
			num1 = ErrorHandling::getIntInput("정수를 입력하세요: ");
			if (num1 == 2)
			{
				std::cout << "상태변경이 불가합니다. 메인메뉴로 돌아갑니다. " << std::endl; return -3;
			}
			else if ((num1 == 1))
			{
				std::cout << "전등 전원을 껐습니다 " << std::endl;
				set_power(false);
				return 1;
			}
			else
			{
				std::cout << "입력값이 잘못되었습니다 " << std::endl; return -2;
			}


		}

		return 1;
	}

	void showstatus() override
	{
		std::cout << "[전등] 이름 :" << get_name() << std::endl << "전원상태 :" << get_power() 
			<< std::endl;
	}

};



class HomeAutomationSystem
{
private:
	std::vector<std::shared_ptr<Shape>> v; // 제품객체 보관할 vector
public:
	HomeAutomationSystem() { init_menu(); }

	void init_menu()
	{

		std::shared_ptr<PopupMenu> root = std::make_shared<PopupMenu>("ROOT");
		root->add(std::make_shared<MenuItem>("제품등록", 11, std::bind(&HomeAutomationSystem::add_product, this)));
		root->add(std::make_shared<MenuItem>("제품삭제", 12, std::bind(&HomeAutomationSystem::remove_product, this)));
		root->add(std::make_shared<MenuItem>("제품목록출력", 13, std::bind(&HomeAutomationSystem::print_product, this)));
		root->add(std::make_shared<MenuItem>("제품상태변경", 14, std::bind(&HomeAutomationSystem::change_status, this)));
		root->add(std::make_shared<MenuItem>("전원일괄제어", 15, std::bind(&HomeAutomationSystem::overall_power_control, this)));
		root->add(std::make_shared<MenuItem>("파일저장된 객체불러오기", 16, std::bind(&HomeAutomationSystem::load_objects, this)));
		root->add(std::make_shared<MenuItem>("종료", 17, std::bind(&HomeAutomationSystem::terminate, this)));
		root->command();


	}

	void load_objects() {
		FILE* f = fopen("object_info.txt", "r");
		if (f == nullptr) {
			std::cerr << "파일을 열 수 없습니다." << std::endl;
			return;
		}

		int num_objects;
		while (fscanf(f, "%d", &num_objects) == 1) //EOF가 아니면 반복
		{
		
			std::cout << "num_objects :" << num_objects << std::endl;

			std::shared_ptr<Shape> newObject;
			switch (num_objects) {
			case 1:
				newObject = std::make_shared<Aircondition>("", 0, 0);
				break;
			case 2:
				newObject = std::make_shared<Ricecooker>("", 0, 0);
				break;
			case 3:
				newObject = std::make_shared<Washer>("", 0, 0);
				break;
			case 4:
				newObject = std::make_shared<Light>("", 0, 0);
				break;

			}
						
			newObject->load_state(f);// 읽어온 객체의 상태를 복원
			v.push_back(newObject);
		}

		fclose(f);
	}

	void add_product()
	{
			
			std::cout << "등록할 제품을 숫자로 입력하세요 : " << std::endl <<
				"(AirConditioner = 1, RiceCooker = 2, Washer = 3, Light = 4)" << std::endl;
			int select = ErrorHandling::getIntInput("정수를 입력하세요: ");
			

			if (select < 1 || select > 4) {
				std::cout << " 범위내의 숫자를 입력해주세요" << std::endl;
				add_product();
			}
			int flag_check = 0;

			std::string s;
			std::cout << "가전의 이름을 입력하세요" << std::endl;
			while (1) {   /*이름중복에 대한 유효성체크*/
				std::cin >> s;
				for (auto it = v.begin(); it != v.end(); ++it)
				{
					if ((*it)->get_name() == s)
					{
						std::cout << "범위내의 동일한 이름의 아이템이 존재합니다. 다시 입력해주세요." << std::endl;
						flag_check = 1;
					}
				}
				if (flag_check == 0){	break;	}
				flag_check = 0;
			
			}
		


			std::cout << "가전의 일일 소비전력량를 숫자로 입력하세요[kW]" << std::endl;
			int watt = ErrorHandling::getIntInput("정수를 입력하세요: ");
			
			std::shared_ptr<Shape>  pm1 = nullptr;
			
			//Shape* pm1 = nullptr;
			switch (select) {
			case 1:
				pm1 = std::make_shared<Aircondition> (s,select, watt);
				break;
			case 2:
				pm1 = std::make_shared<Ricecooker> (s, select, watt);
				break;
			case 3:
				pm1 = std::make_shared<Washer> (s, select, watt);
				break;
			case 4:
				pm1 = std::make_shared<Light> (s, select, watt);
				break;
			default:
				;
				std::cout << "등록가능한 가전의 번호를 입력해주세요" << std::endl;;
				break;
			}
			
			std::cout << std::endl;

			v.push_back(pm1);
			
			FILE* f = fopen("object_info.txt", "a");  // 'a' 모드로 열어서 기존 파일 끝에 추가

			if (f != nullptr)
			{
				for (auto e : v)
				{
					e->save(f); // 객체 정보 저장
				}

				fclose(f);  
			}
			else
			{
				std::cerr << "파일을 열 수 없습니다." << std::endl;
			}
	}
	
	void remove_product()
	{
		time_t temp_power = 0;
		int flag = 0;
		if (v.size() == 0)
		{
			std::cout << "등록된 아이템이 없습니다" << std::endl;
			return;
		}
		std::cout << "삭제할 제품 이름을 입력하세요" << std::endl;
		std::string s; std::cin >> s;
		
		for (auto it = v.begin(); it != v.end(); ++it)
		{
			if ((*it)->get_name() == s)
			{
				(*it)->set_power(false);
				
				for (auto it_re = (*it)->get_useHour().begin(); it_re != (*it)->get_useHour().end(); ++it_re)
				{
					temp_power += *(*it_re);
				}
				std::cout << "## \"" << (*it)->get_name()<< "\"의 삭제 전 소비전력 재확인 : "<< temp_power << std::endl;
				power_global += temp_power;

				v.erase(it);    // 벡터에서 객체 제거
				std::cout << "가전 제거 완료" << std::endl;
				
				return;
			}
		}
		std::cout << "일치하는 이름이 없습니다" << std::endl;

	}

	void print_product()
	{
		
		std::cout << "====================================================" << std::endl;
		for (auto& e : v)
		{
			e->showstatus();
			std::cout << std::endl;
		}
		std::cout << "====================================================" << std::endl;
	}

	void change_status()
	{
		print_product();
		std::cout << "제어할 아이템의 이름을 입력하세요" << std::endl;
		std::string s; std::cin >> s;
		for (auto it = v.begin(); it != v.end(); ++it)
		{
			if ((*it)->get_name() == s)
			{
				(*it)->change_state();
				std::cout << std::endl;
				return;
			}
		}
		std::cout << "해당하는 이름의 아이템이 없습니다." << std::endl;
		return;
	}

	int all_turnoff()
	{
		int count = 0;
		for (auto it = v.begin(); it != v.end(); ++it)
		{
			if ((*it)->get_power() == 1)
			{
				(*it)->set_power(0);
				std::cout << (*it)->get_name() << "의 전원 ON --> OFF" << std::endl;
				std::cout << std::endl <<"=== 변경된 상태 ====" << std::endl;
				(*it)->showstatus();
				count++;
				std::cout << std::endl;
			}
		}
		return count;
	}

	int all_turnon()
	{
		int count = 0;
		for (auto it = v.begin(); it != v.end(); ++it)
		{
			if ((*it)->get_power() == 0)
			{
				(*it)->set_power(1);
				std::cout << (*it)->get_name() << "의 전원 OFF --> ON " << std::endl;
				(*it)->showstatus();
				count++;
			}
		}
		return count;
	}

	void overall_power_control()
	{
		
		int select = ErrorHandling::getIntInput("전체 전원을 끄시겠습니까? (Yes : 1, Viceversa : 2) [정수를 입력하세요]: ");
		int cnt = 0;
		std::cout << "====================================================" << std::endl;
		if (select == 1) //전체 전원 OFF
		{
			cnt = all_turnoff();
			
		}
		else if(select == 2) //전체 전원 ON
		{
			cnt = all_turnon();
		}
		if (cnt == 0)
		{
			std::cout << "전원상태를 변경할 가전이 없습니다" << std::endl;
		}

		std::cout << "====================================================" << std::endl;
	}

	void terminate()
	{
		
		time_t single_consumption = 0;
		all_turnoff(); // 종료전 모든 객체의 전원을 OFF하여 전력량 계산
		for (auto it = v.begin(); it != v.end(); ++it)
		{
			for (auto it_re = (*it)->get_useHour().begin(); it_re != (*it)->get_useHour().end(); ++it_re)
			{
				single_consumption += *(*it_re);
			}
			
			std::cout << "## " << (*it)->get_name()<< "객체의 총 소비전력 : " << single_consumption << std::endl;
			power_global += single_consumption;
		}
		std::cout <<"## 전체 전력소비량 : " << power_global << std::endl;
	
		FILE* f = fopen("object_info.txt", "w");  // 'w' 모드로 열어서 파일을 새로 작성

		if (f != nullptr) {
			
			for (auto e : v)
			{
				e->save(f); // 객체 정보 저장
			}
			fclose(f); 
		}
		else
		{
			std::cerr << "파일을 열 수 없습니다." << std::endl;
		}
	}

	
};


int main()
{
	HomeAutomationSystem has;
	return 0;
}
