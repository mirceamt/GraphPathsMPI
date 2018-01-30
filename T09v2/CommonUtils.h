#pragma once
#include <string>

class CommonUtils
{
public:
	static int GetNrProcesses();
	static void SetNrProcesses(int nrProcesses);
	static void ShowError(int errorCode, char *errorMsg = nullptr);
	static int StringToInt(char *s);
	static std::string IntsToString(int *v, int n);
	static std::string IntToString(int x);
private:
	static int s_nrProcesses;
};