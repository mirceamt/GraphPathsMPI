#pragma once

class CommonUtils
{
public:
	static int GetNrProcesses();
	static void SetNrProcesses(int nrProcesses);
	static void ShowError(int errorCode, char *errorMsg = nullptr);
private:
	static int s_nrProcesses;
};