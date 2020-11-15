#include <Windows.h>
#include <stdio.h>

bool loadSysFile(char * driverName)
{
	char sPath[1024];
	char sCurrentDirectory[512];

	SC_HANDLE sh = OpenSCManager(
		NULL,								// Selects local computer
		NULL,								// SERVICES_ACTIVE_DATABASE by default
		SC_MANAGER_ALL_ACCESS);				// All access on the service manager

	if (!sh)
		return false;

	GetCurrentDirectory(512, sCurrentDirectory);

	// sPath will contain the path to the .sys file
	
	_snprintf((char *)(sPath), 1024, "%s\\%s.sys", sCurrentDirectory, driverName);

	printf("Loading %s\n", sPath);

	SC_HANDLE rh = CreateService(
		sh,
		driverName,							// Service name
		driverName,							// Display name
		SERVICE_ALL_ACCESS,					// Desired access
		SERVICE_KERNEL_DRIVER,				// Service type
		SERVICE_DEMAND_START,				// Start type
		SERVICE_ERROR_NORMAL,				// Error control
		sPath,								// Binary path name
		NULL,
		NULL,
		NULL,
		NULL,
		NULL);

	if (!rh)
	{
		if (GetLastError() == ERROR_SERVICE_EXISTS)
		{
			// The service is already registered
			rh = OpenService(sh, driverName, SERVICE_ALL_ACCESS);

			if (!rh)
			{
				CloseServiceHandle(sh);
				return false;
			}
		}
		else
		{
			CloseServiceHandle(sh);
			return false;
		}
	}

	// Start the drivers
	if (rh)
	{
		if (0 == StartService(rh, 0, NULL))
		{
			if (ERROR_SERVICE_ALREADY_RUNNING == GetLastError())
			{
				// Since it is already running, there is no problem
			}
			else
			{
				CloseServiceHandle(sh);
				CloseServiceHandle(rh);
				return false;
			}
		}

		CloseServiceHandle(sh);
		CloseServiceHandle(rh);

	}

	return true;
}

int main()
{
	loadSysFile("KernelName");
	return 0;
}
