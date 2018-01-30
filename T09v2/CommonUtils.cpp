#include "CommonUtils.h"
#include <iostream>

using namespace std;

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
}