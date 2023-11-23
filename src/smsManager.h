
#ifndef SMSMANAGER_H
#define SMSMANAGER_H

#include <vector>
#include <string>
#include <map>
#include <iostream>
class SMSManager
{
	using PHONENUMBER = std::string;

	std::map<std::string, std::vector<PHONENUMBER>> m;
public:
	void add_sms_number(const std::string& key, const PHONENUMBER& number);


	// #. ���� �������� �� �Լ� �κи� �����߽��ϴ�.
	void send_sms(const std::string& key, const std::string& msg);

};
#endif // ERROR_HANDLING_H