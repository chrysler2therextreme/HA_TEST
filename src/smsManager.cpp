#include "smsManager.h"


	void SMSManager::add_sms_number(const std::string& key, const PHONENUMBER& number)
	{
		auto it = m.find(key);

		if (it != m.end()) //Ű���� ���ϵǾ��ٸ�
		{
			return;
		}
		else {				//���ϵ� Ű���� ���ٸ�
			m[key].push_back(number);
		}
		
	}

	// #. ���� �������� �� �Լ� �κи� �����߽��ϴ�.
	void SMSManager::send_sms(const std::string& key, const std::string& msg)
	{
		//		auto& v = m[key]; // �⺻ �ڵ�

				// ����� �ڵ�
		auto it = m.find(key);

		if (it != m.end())
		{
			for (const PHONENUMBER& number : it->second)
			{
				std::cout << number << " : " << msg << std::endl;
			}
		}
	}