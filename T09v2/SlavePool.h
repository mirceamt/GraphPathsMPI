#pragma once
#include <vector>

enum SlaveStatus
{
	Working,
	Idle,
	Stopped,
};


class SlavePool
{
public:
	SlavePool();
	void StopIdleSlaves();
	int DoJob(std::vector<int>& startingPath);
	void ChangeSlaveStatus(int slave, SlaveStatus oldStatus, SlaveStatus newStatus);
	int GetIdleSlavesCount();
	int GetStoppedSlavesCount();

private:
	int GetIdleSlave();
	int m_totalProcesses;
	std::vector<int> m_workingSlaves;
	std::vector<int> m_idleSlaves;
	std::vector<int> m_stoppedSlaves;

};