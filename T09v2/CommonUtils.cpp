#include "CommonUtils.h"
#include <iostream>
#include <string>

using namespace std;

int CommonUtils::s_nrProcesses;

void CommonUtils::SetNrProcesses(int nrProcesses)
{
	s_nrProcesses = nrProcesses;
}

int CommonUtils::GetNrProcesses()
{
	return s_nrProcesses;
}

void CommonUtils::ShowError(int errorCode, char *errorMsg)
{
	cout << "\n\n";
	cout << "Error: ";
	if (errorMsg == nullptr)
	{
		cout << "NULL";
	}
	else
	{
		cout << errorMsg;
	}
	cout << "\n";
	cout << "Error code: " << errorCode;
	cout << "\n\n";
	cout.flush();
}

int CommonUtils::StringToInt(char * s)
{
	int ret = 0;
	for (int i = 0; s[i] != 0; ++i)
	{
		ret = ret * 10 + s[i] - '0';
	}
	return ret;
}

string CommonUtils::IntsToString(int *v, int n)
{
	string ret = "";
	ret += IntToString(v[0]);
	for (int i = 1; i < n; ++i)
	{
		ret += " ";
		ret += IntToString(v[i]);
	}
	return ret;
}

string CommonUtils::IntToString(int x) 
{
	string ret = "";
	if (x == 0)
	{
		ret = '0';
	}
	else
	{
		int semn = 1;
		if (x < 0)
		{
			semn = -1;
			x = -x;
		}
		while (x > 0)
		{
			ret += (char)(x % 10 + '0');
			x /= 10;
		}
		reverse(ret.begin(), ret.end());
		if (semn < 0)
		{
			ret = "-" + ret;
		}
	}
	return ret;
}