#include <iostream>
#include "Master.h"
#include "Slave.h"
#include "mpi.h"
#include "CommonUtils.h"

using namespace std;

int main(int argc, char* argv[])
{
	//cout << "argc\t" << argc << "\n";
	//for (int i = 0; argv[i] != NULL; ++i)
	//{
	//	cout << "argv[" << i << "]\t" << argv[i] << "\n";
	//}
	//cout << "\n\nDONE\n\n";
	

	MPI_Init(&argc, &argv);

	int nrProcesses;
	MPI_Comm_size(MPI_COMM_WORLD, &nrProcesses);
	CommonUtils::SetNrProcesses(nrProcesses);
	
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0)
	{
		Master *master = new Master(rank);
		master->Init();
		master->Run();
		master->CleanUp();
		delete master;
	}
	else
	{
		Slave *slave = new Slave(rank);
		slave->Init();
		slave->Run();
		slave->CleanUp();
		delete slave;
	}

	MPI_Finalize();
	return 0;
}