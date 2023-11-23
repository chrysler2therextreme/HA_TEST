#include "smsManager.h"


	void SMSManager::add_sms_number(const std::string& key, const PHONENUMBER& number)
	{
		auto it = m.find(key);

		if (it != m.end()) //키값이 기등록되었다면
		{
			return;
		}
		else {				//기등록된 키값이 없다면
			m[key].push_back(number);
		}
		
	}

	// #. 이전 버전에서 이 함수 부분만 변경했습니다.
	void SMSManager::send_sms(const std::string& key, const std::string& msg)
	{
		//		auto& v = m[key]; // 기본 코드

				// 변경된 코드
		auto it = m.find(key);

		if (it != m.end())
		{
			for (const PHONENUMBER& number : it->second)
			{
				std::cout << number << " : " << msg << std::endl;
			}
		}
	}