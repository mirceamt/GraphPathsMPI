#include "SlavePool.h"
#include "CommonUtils.h"
#include "Master.h"
#include "mpi.h"
#include <iostream>

using namespace std;

SlavePool::SlavePool()
{
	m_totalProcesses = CommonUtils::GetNrProcesses();
	for (int i = 1; i < m_totalProcesses; ++i)
	{
		m_idleSlaves.push_back(i);
	}
}

void SlavePool::StopIdleSlaves()
{
	//Master::Log("\n\n\n==================================================\n\n\n");
	//ShowVectors();

	while (m_idleSlaves.size() > 0)
	{
		int x = m_idleSlaves.back();
		int message[2];
		message[0] = 0;
		message[1] = 0;
		MPI_Send(message, 2, MPI_INT, x, SEND_MESSAGE_TAG, MPI_COMM_WORLD);
		Master::Log("Sent message to " + CommonUtils::IntsToString(message, 2));
		ChangeSlaveStatus(x, SlaveStatus::Idle, SlaveStatus::Stopped);
	}

	//ShowVectors();
	//Master::Log("\n\n\n==================================================\n\n\n");
}

int SlavePool::DoJob(std::vector<int>& startingPath)
{
	Master::Log("Entered SlavePool::DoJob");
	int slave = GetIdleSlave();
	if (slave < 0)
	{
		return -1;
	}
	int message[2];
	message[0] = 1;
	message[1] = startingPath.size();
	MPI_Send(message, 2, MPI_INT, slave, SEND_MESSAGE_TAG, MPI_COMM_WORLD);
	Master::Log(string("Sent message to ") + CommonUtils::IntToString(slave) + "; message: " + CommonUtils::IntsToString(message, 2));
	MPI_Send(startingPath.data(), startingPath.size(), MPI_INT, slave, SEND_MESSAGE_TAG, MPI_COMM_WORLD);
	Master::Log(string("Sent message to ") + CommonUtils::IntToString(slave) + "; message: " + CommonUtils::IntsToString(startingPath.data(), startingPath.size()));
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

	//Master::Log("\n\n\n$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n\n");
	//Master::Log("Working slaves: " + CommonUtils::IntsToString(m_workingSlaves.data(), m_workingSlaves.size()));
	//Master::Log("Idle slaves: " + CommonUtils::IntsToString(m_idleSlaves.data(), m_idleSlaves.size()));
	//Master::Log("Stopped slaves: " + CommonUtils::IntsToString(m_stoppedSlaves.data(), m_stoppedSlaves.size()));


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

	//Master::Log("Working slaves: " + CommonUtils::IntsToString(m_workingSlaves.data(), m_workingSlaves.size()));
	//Master::Log("Idle slaves: " + CommonUtils::IntsToString(m_idleSlaves.data(), m_idleSlaves.size()));
	//Master::Log("Stopped slaves: " + CommonUtils::IntsToString(m_stoppedSlaves.data(), m_stoppedSlaves.size()));
	//Master::Log("\n\n\n$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n\n");


}

int SlavePool::GetIdleSlavesCount()
{
	return m_idleSlaves.size();
}

int SlavePool::GetStoppedSlavesCount()
{
	return m_stoppedSlaves.size();
}

void SlavePool::ShowVectors()
{
	Master::Log(" ! Working slaves: " + CommonUtils::IntsToString(m_workingSlaves.data(), m_workingSlaves.size()));
	Master::Log(" ! Idle slaves: " + CommonUtils::IntsToString(m_idleSlaves.data(), m_idleSlaves.size()));
	Master::Log(" ! Stopped slaves: " + CommonUtils::IntsToString(m_stoppedSlaves.data(), m_stoppedSlaves.size()));
}

int SlavePool::GetIdleSlave()
{
	if (m_idleSlaves.size() > 0)
	{
		return m_idleSlaves.back();
	}
	return -1;
}
