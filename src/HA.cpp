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
	std::vector<std::shared_ptr<time_t>>  useHour;// [endhour- starthour] ����ؼ� off�ɶ����� ���

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
		
		for (const auto& hour : useHour)	{fprintf(f, "%lld ", *hour);}// ��� �ð� ����

		fprintf(f, "\n");
	}

	void load_state(FILE* f) {
		
		char buffer[100]; // �̸����� buffer�� ���� �� ����
		
		fscanf(f, "%s",&buffer);
		std::cout << "machineName :" << buffer << std::endl;
		machineName = buffer;
		
		fscanf(f, "%d %d %d %d %d %lld %zu",   //������ ���� load
			&type, &powerFlag, &powerConsumption, &state1, &state2, &startHour, &endHour);

		size_t numHours;  // vector���� usehour ���� ó��
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
		std::cout << "## \"" << get_name() <<"\"" << "�� " << (useHour.size()) + 1 << "��° �Һ����� : " << *watt << std::endl;
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
			std::cout << "�Է��� �������°� �����մϴ�" << std::endl;
		}
		else if (flag == true) {
			set_startHour(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
			powerFlag = true;
			if (type == 1) {
				sms.send_sms("AIRCONDITION_POWER_ON", "AIRCONDITION�� �������ϴ�.");
			}
			else if (type == 2) {
				sms.send_sms("RICECOOKER_POWER_ON", "RICECOOKER�� �������ϴ�.");
			}
			else if (type == 3) {
				sms.send_sms("WASHER_POWER_ON", "WASHER�� �������ϴ�.");
			}
			else if (type == 4) {
				sms.send_sms("LIGHT_POWER_ON", "LIGHT�� �������ϴ�.");
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
	set_startHour(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())); // �����ڿ��� startHour ������ ����ð��Է�
	set_state1(-1); set_state2(-1);
	}

	//~Aircondition();
	

	int change_state(void) override
	{
		int num1;
		if (get_power() == false)
		{
			std::cout << "������ ������ �ѽðڽ��ϱ�?(YES :1, NO : 2) : " << std::endl;
			num1 = ErrorHandling::getIntInput("������ �Է��ϼ���: ");
			
			if (num1 == 2)
			{
				std::cout << "���º����� �Ұ��մϴ�. ���θ޴��� ���ư��ϴ�. " << std::endl; return -3;
			}
			else if (num1 == 1)
			{
				std::cout << "������ ������ �������ϴ� " << std::endl;
				set_power(true);
			}
			else
			{
				std::cout << "�Է°��� �߸��Ǿ����ϴ� " << std::endl; return -2;
			}
		}
		else
		{
			std::cout << "������ ������ ���ðڽ��ϱ�?(YES :1, NO : 2) : " << std::endl;
			num1 = ErrorHandling::getIntInput("������ �Է��ϼ���: ");
			if (num1 == 2)
			{
				std::cout << "���º����� �Ұ��մϴ�. ���θ޴��� ���ư��ϴ�. " << std::endl; return -3;
			}
			else if ((num1 == 1))
			{
				std::cout << "������ ������ �����ϴ� " << std::endl;
				set_power(false);
				//return 1;    //������ �������¿����� state1, state2 ������ϵ���
			}
			else
			{
				std::cout << "�Է°��� �߸��Ǿ����ϴ� " << std::endl; return -2;
			}


		}

		int num2;
		std::cout << "������ ���� �����ϼ���(SENDAIR = 1, COOLER = 2, HEATER = 3) : ";
		
		num2 = ErrorHandling::getIntInput("������ �Է��ϼ���: ");
		set_state1(num2);
		std::cout << "������ �µ� �Է��ϼ��� : ";
		
		num2 = ErrorHandling::getIntInput("������ �Է��ϼ���: ");
		set_state2(num2);

		return state1;
	}
	void showstatus() override
	{
		std::cout << "[������] �̸� :" <<get_name() << std::endl << "�������� :" <<get_power() << 
			"  ���ۻ��� :" << get_state1() << "  �����µ� :" << get_state2() << std::endl;
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
			std::cout << "��� ������ �ѽðڽ��ϱ�?(YES :1, NO : 2) : "<<std::endl;
			num1 = ErrorHandling::getIntInput("������ �Է��ϼ���: ");
			if (num1 == 2)
			{
				std::cout << "���º����� �Ұ��մϴ�. ���θ޴��� ���ư��ϴ�. " << std::endl; return -3;
			}
			else if (num1 == 1)
			{
				std::cout << "��� ������ �������ϴ� " << std::endl;
				set_power(true);
			}
			else
			{
				std::cout << "�Է°��� �߸��Ǿ����ϴ� " << std::endl; return -2;
			}
		}
		else
		{
			std::cout << "��� ������ ���ðڽ��ϱ�?(YES :1, NO : 2) : " << std::endl;
			num1 = ErrorHandling::getIntInput("������ �Է��ϼ���: ");
			if (num1 == 2)
			{
				std::cout << "���º����� �Ұ��մϴ�. ���θ޴��� ���ư��ϴ�. " << std::endl; return -3;
			}
			else if ((num1 == 1))
			{
				std::cout << "��� ������ �����ϴ� " << std::endl;
				set_power(false);
				return 1;
			}
			else
			{
				std::cout << "�Է°��� �߸��Ǿ����ϴ� " << std::endl; return -2;
			}
			

		}

		/* state 1 ���� */
		int num2;
		std::cout <<  "��� ���� �����ϼ���(NO_OPERATION = 1, WARM = 2, COOK = 3, HEAT= 4) : ";
		num2 = ErrorHandling::getIntInput("������ �Է��ϼ���: ");
		set_state1(num2);
		if (get_state1() == 1)
		{
			set_power(false);
		}

		return state1;
	}
	void showstatus()override
	{
		std::cout << "[���] �̸� :" << get_name() << std::endl << "�������� :" << get_power() << 
			"  ���ۻ��� :" << get_state1() << std::endl;
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
			std::cout << "��Ź�� ������ �ѽðڽ��ϱ�?(YES :1, NO : 2) : " << std::endl;
			num1 = ErrorHandling::getIntInput("������ �Է��ϼ���: ");
			if (num1 == 2)
			{
				std::cout << "���º����� �Ұ��մϴ�. ���θ޴��� ���ư��ϴ�. " << std::endl; return -3;
			}
			else if (num1 == 1)
			{
				std::cout << "��Ź�� ������ �������ϴ� " << std::endl;
				set_power(true);
			}
			else
			{
				std::cout << "�Է°��� �߸��Ǿ����ϴ� " << std::endl; return -2;
			}
		}
		else
		{
			std::cout << "��Ź�� ������ ���ðڽ��ϱ�?(YES :1, NO : 2) : " << std::endl;
			num1 = ErrorHandling::getIntInput("������ �Է��ϼ���: ");
			if (num1 == 2)
			{
				std::cout << "���º����� �Ұ��մϴ�. ���θ޴��� ���ư��ϴ�. " << std::endl; return -3;
			}
			else if ((num1 == 1))
			{
				std::cout << "��Ź�� ������ �����ϴ� " << std::endl;
				set_power(false);
				return state1;
			}
			else
			{
				std::cout << "�Է°��� �߸��Ǿ����ϴ� " << std::endl; return -2;
			}


		}

		/* state 1 */

		std::cout << "��Ź�� ���� �����ϼ���(NO_OPERATION = 1, GENERAL = 2, BOIL = 3, DRY= 4) : ";
		int num2 = ErrorHandling::getIntInput("������ �Է��ϼ���: ");
		set_state1(num2);
		if (get_state1() == 1)
		{
			set_power(false);
		}
		return state1;

	}

	void showstatus()override
	{
		std::cout << "[��Ź��] �̸� :" << get_name() << std::endl << "�������� :" << get_power() <<
			"  ���ۻ��� :" << get_state1() << std::endl;
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
			std::cout << "���� ������ �ѽðڽ��ϱ�?(YES :1, NO : 2) : " << std::endl;
			num1 = ErrorHandling::getIntInput("������ �Է��ϼ���: ");

			if (num1 == 2)
			{
				std::cout << "���º����� �Ұ��մϴ�. ���θ޴��� ���ư��ϴ�. " << std::endl; return -3;
			}
			else if (num1 == 1)
			{
				std::cout << "���� ������ �������ϴ� " << std::endl;
				set_power(true);
			}
			else
			{
				std::cout << "�Է°��� �߸��Ǿ����ϴ� " << std::endl; return -2;
			}
		}
		else
		{
			std::cout << "���� ������ ���ðڽ��ϱ�?(YES :1, NO : 2) : " << std::endl;
			num1 = ErrorHandling::getIntInput("������ �Է��ϼ���: ");
			if (num1 == 2)
			{
				std::cout << "���º����� �Ұ��մϴ�. ���θ޴��� ���ư��ϴ�. " << std::endl; return -3;
			}
			else if ((num1 == 1))
			{
				std::cout << "���� ������ �����ϴ� " << std::endl;
				set_power(false);
				return 1;
			}
			else
			{
				std::cout << "�Է°��� �߸��Ǿ����ϴ� " << std::endl; return -2;
			}


		}

		return 1;
	}

	void showstatus() override
	{
		std::cout << "[����] �̸� :" << get_name() << std::endl << "�������� :" << get_power() 
			<< std::endl;
	}

};



class HomeAutomationSystem
{
private:
	std::vector<std::shared_ptr<Shape>> v; // ��ǰ��ü ������ vector
public:
	HomeAutomationSystem() { init_menu(); }

	void init_menu()
	{

		std::shared_ptr<PopupMenu> root = std::make_shared<PopupMenu>("ROOT");
		root->add(std::make_shared<MenuItem>("��ǰ���", 11, std::bind(&HomeAutomationSystem::add_product, this)));
		root->add(std::make_shared<MenuItem>("��ǰ����", 12, std::bind(&HomeAutomationSystem::remove_product, this)));
		root->add(std::make_shared<MenuItem>("��ǰ������", 13, std::bind(&HomeAutomationSystem::print_product, this)));
		root->add(std::make_shared<MenuItem>("��ǰ���º���", 14, std::bind(&HomeAutomationSystem::change_status, this)));
		root->add(std::make_shared<MenuItem>("�����ϰ�����", 15, std::bind(&HomeAutomationSystem::overall_power_control, this)));
		root->add(std::make_shared<MenuItem>("��������� ��ü�ҷ�����", 16, std::bind(&HomeAutomationSystem::load_objects, this)));
		root->add(std::make_shared<MenuItem>("����", 17, std::bind(&HomeAutomationSystem::terminate, this)));
		root->command();


	}

	void load_objects() {
		FILE* f = fopen("object_info.txt", "r");
		if (f == nullptr) {
			std::cerr << "������ �� �� �����ϴ�." << std::endl;
			return;
		}

		int num_objects;
		while (fscanf(f, "%d", &num_objects) == 1) //EOF�� �ƴϸ� �ݺ�
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
						
			newObject->load_state(f);// �о�� ��ü�� ���¸� ����
			v.push_back(newObject);
		}

		fclose(f);
	}

	void add_product()
	{
			
			std::cout << "����� ��ǰ�� ���ڷ� �Է��ϼ��� : " << std::endl <<
				"(AirConditioner = 1, RiceCooker = 2, Washer = 3, Light = 4)" << std::endl;
			int select = ErrorHandling::getIntInput("������ �Է��ϼ���: ");
			

			if (select < 1 || select > 4) {
				std::cout << " �������� ���ڸ� �Է����ּ���" << std::endl;
				add_product();
			}
			int flag_check = 0;

			std::string s;
			std::cout << "������ �̸��� �Է��ϼ���" << std::endl;
			while (1) {   /*�̸��ߺ��� ���� ��ȿ��üũ*/
				std::cin >> s;
				for (auto it = v.begin(); it != v.end(); ++it)
				{
					if ((*it)->get_name() == s)
					{
						std::cout << "�������� ������ �̸��� �������� �����մϴ�. �ٽ� �Է����ּ���." << std::endl;
						flag_check = 1;
					}
				}
				if (flag_check == 0){	break;	}
				flag_check = 0;
			
			}
		


			std::cout << "������ ���� �Һ����·��� ���ڷ� �Է��ϼ���[kW]" << std::endl;
			int watt = ErrorHandling::getIntInput("������ �Է��ϼ���: ");
			
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
				std::cout << "��ϰ����� ������ ��ȣ�� �Է����ּ���" << std::endl;;
				break;
			}
			
			std::cout << std::endl;

			v.push_back(pm1);
			
			FILE* f = fopen("object_info.txt", "a");  // 'a' ���� ��� ���� ���� ���� �߰�

			if (f != nullptr)
			{
				for (auto e : v)
				{
					e->save(f); // ��ü ���� ����
				}

				fclose(f);  
			}
			else
			{
				std::cerr << "������ �� �� �����ϴ�." << std::endl;
			}
	}
	
	void remove_product()
	{
		time_t temp_power = 0;
		int flag = 0;
		if (v.size() == 0)
		{
			std::cout << "��ϵ� �������� �����ϴ�" << std::endl;
			return;
		}
		std::cout << "������ ��ǰ �̸��� �Է��ϼ���" << std::endl;
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
				std::cout << "## \"" << (*it)->get_name()<< "\"�� ���� �� �Һ����� ��Ȯ�� : "<< temp_power << std::endl;
				power_global += temp_power;

				v.erase(it);    // ���Ϳ��� ��ü ����
				std::cout << "���� ���� �Ϸ�" << std::endl;
				
				return;
			}
		}
		std::cout << "��ġ�ϴ� �̸��� �����ϴ�" << std::endl;

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
		std::cout << "������ �������� �̸��� �Է��ϼ���" << std::endl;
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
		std::cout << "�ش��ϴ� �̸��� �������� �����ϴ�." << std::endl;
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
				std::cout << (*it)->get_name() << "�� ���� ON --> OFF" << std::endl;
				std::cout << std::endl <<"=== ����� ���� ====" << std::endl;
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
				std::cout << (*it)->get_name() << "�� ���� OFF --> ON " << std::endl;
				(*it)->showstatus();
				count++;
			}
		}
		return count;
	}

	void overall_power_control()
	{
		
		int select = ErrorHandling::getIntInput("��ü ������ ���ðڽ��ϱ�? (Yes : 1, Viceversa : 2) [������ �Է��ϼ���]: ");
		int cnt = 0;
		std::cout << "====================================================" << std::endl;
		if (select == 1) //��ü ���� OFF
		{
			cnt = all_turnoff();
			
		}
		else if(select == 2) //��ü ���� ON
		{
			cnt = all_turnon();
		}
		if (cnt == 0)
		{
			std::cout << "�������¸� ������ ������ �����ϴ�" << std::endl;
		}

		std::cout << "====================================================" << std::endl;
	}

	void terminate()
	{
		
		time_t single_consumption = 0;
		all_turnoff(); // ������ ��� ��ü�� ������ OFF�Ͽ� ���·� ���
		for (auto it = v.begin(); it != v.end(); ++it)
		{
			for (auto it_re = (*it)->get_useHour().begin(); it_re != (*it)->get_useHour().end(); ++it_re)
			{
				single_consumption += *(*it_re);
			}
			
			std::cout << "## " << (*it)->get_name()<< "��ü�� �� �Һ����� : " << single_consumption << std::endl;
			power_global += single_consumption;
		}
		std::cout <<"## ��ü ���¼Һ� : " << power_global << std::endl;
	
		FILE* f = fopen("object_info.txt", "w");  // 'w' ���� ��� ������ ���� �ۼ�

		if (f != nullptr) {
			
			for (auto e : v)
			{
				e->save(f); // ��ü ���� ����
			}
			fclose(f); 
		}
		else
		{
			std::cerr << "������ �� �� �����ϴ�." << std::endl;
		}
	}

	
};


int main()
{
	HomeAutomationSystem has;
	return 0;
}
