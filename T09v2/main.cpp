#include <iostream>
#include "mpi.h"

using namespace std;

int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);

	int rank;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	if (rank == 0)
	{
		char helloStr[] = "Hello World";
		MPI_Send(helloStr, _countof(helloStr), MPI_CHAR, 1, 0, MPI_COMM_WORLD);
	}
	else if (rank == 1)
	{
		char helloStr[12];
		MPI_Recv(helloStr, _countof(helloStr), MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		cout << "Process " << rank << " received from process 0: ";
		cout << "\"" << helloStr << "\"";
		cout << "\n";
	}

	MPI_Finalize();
	return 0;
}