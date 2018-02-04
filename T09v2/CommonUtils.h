#pragma once
#include <string>
#define SEND_MESSAGE_TAG 1
#define INF 1000000000

class CommonUtils
{
public:
	static int GetNrProcesses();
	static void SetNrProcesses(int nrProcesses);
	static void ShowError(int errorCode, char *errorMsg = nullptr);
	static int StringToInt(char *s);
	static std::string IntToString(int x);
	static std::string IntsToString(int *v, int n);
	static std::string BoolToString(bool x);
	static std::string BoolsToString(bool * v, int n);
	static int GetMasterRank();
private:
	static const int m_masterRank = 0;
	static int s_nrProcesses;
};
