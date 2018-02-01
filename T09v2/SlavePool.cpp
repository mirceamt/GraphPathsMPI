#include "SlavePool.h"
#include "CommonUtils.h"
#include "mpi.h"
#include <iostream>

using namespace std;

SlavePool::SlavePool()
{
	m_totalProcesses = CommonUtils::GetNrProcesses();
	for (int i = 1; i <= m_totalProcesses; ++i)
	{
		m_idleSlaves.push_back(i);
	}
}

void SlavePool::StopIdleSlaves()
{
	for (auto x : m_idleSlaves)
	{
		int message[2];
		message[0] = 0;
		message[1] = 0;
		MPI_Send(message, 2, MPI_INT, x, MPI_ANY_TAG, MPI_COMM_WORLD);
		ChangeSlaveStatus(x, SlaveStatus::Idle, SlaveStatus::Stopped);
		m_stoppedSlaves.push_back(x);
	}
}

int SlavePool::DoJob(std::vector<int>& startingPath)
{
	int slave = GetIdleSlave();
	if (slave < 0)
	{
		return -1;
	}
	int message[2];
	message[0] = 1;
	message[1] = startingPath.size();
	MPI_Send(message, 2, MPI_INT, slave, MPI_ANY_TAG, MPI_COMM_WORLD);
	MPI_Send(startingPath.data(), startingPath.size(), MPI_INT, slave, MPI_ANY_TAG, MPI_COMM_WORLD);
	ChangeSlaveStatus(slave, SlaveStatus::Idle, SlaveStatus::Working);
	return slave;
}

void SlavePool::ChangeSlaveStatus(int slave, SlaveStatus oldStatus, SlaveStatus newStatus)
{
	vector<int>* oldVector = nullptr;
	vector<int>* newVector = nullptr;

	switch (oldStatus)
	{
	case Working:
		oldVector = &m_workingSlaves;
		break;
	case Idle:
		oldVector = &m_idleSlaves;
		break;
	case Stopped:
		oldVector = &m_stoppedSlaves;
		break;
	default:
		break;
	}

	switch (newStatus)
	{
	case Working:
		newVector = &m_workingSlaves;
		break;
	case Idle:
		newVector = &m_idleSlaves;
		break;
	case Stopped:
		newVector = &m_stoppedSlaves;
		break;
	default:
		break;
	}

	if (oldVector == nullptr || newVector == nullptr)
	{
		cout << "ERROR! oldVector or newVector is nullptr in SlavePool::ChangeSlaveStatus\n";
		cout.flush();
		exit(0);
	}

	for (int i = 0; i < (int)oldVector->size(); ++i)
	{
		if (oldVector->at(i) == slave)
		{
			oldVector->at(i) = oldVector->back();
			break;
		}
	}
	oldVector->pop_back();

	newVector->push_back(slave);
}

int SlavePool::GetIdleSlavesCount()
{
	return m_idleSlaves.size();
}

int SlavePool::GetStoppedSlavesCount()
{
	return m_stoppedSlaves.size();
}

int SlavePool::GetIdleSlave()
{
	if (m_idleSlaves.size() > 0)
	{
		return m_idleSlaves.back();
	}
	return -1;
}
