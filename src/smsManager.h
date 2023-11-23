
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


	// #. 이전 버전에서 이 함수 부분만 변경했습니다.
	void send_sms(const std::string& key, const std::string& msg);

};
#endif // ERROR_HANDLING_H