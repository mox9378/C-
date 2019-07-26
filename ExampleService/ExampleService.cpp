// ExampleService.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Windows.h>

SERVICE_STATUS_HANDLE ssh=NULL;
SERVICE_STATUS ss = {0};
void PrintError(wchar_t *err)
{
	printf("%s ErrorCode : %d\r\n",err,GetLastError());
}

BOOL InstallService()
{
	wchar_t DirBuf[1024]={0},SysDir[1024]={0};
	GetCurrentDirectory(1024,DirBuf);
	GetModuleFileName(NULL,DirBuf,sizeof(DirBuf));
	GetSystemDirectory(SysDir,sizeof(SysDir));
	wcscat_s(SysDir,L"\\ExampleService.exe");
	if(!CopyFile(DirBuf,SysDir,FALSE))
	{
		PrintError(L"CopyFile Fail");
		return FALSE;
	}

	SC_HANDLE sch = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	if(!sch)
	{
		PrintError(L"OpenSCManager Failed");
		return FALSE;
	}

	SC_HANDLE schNewSrv = CreateService(sch,L"ExampleService",L"SampleServiceApp",SERVICE_ALL_ACCESS,SERVICE_WIN32_OWN_PROCESS,SERVICE_AUTO_START,
		SERVICE_ERROR_NORMAL,SysDir,NULL,NULL,NULL,NULL,NULL);

	if(!schNewSrv)
	{
		PrintError(L"CreateService Failed");
		return FALSE;
	}

	SERVICE_DESCRIPTION sd;
	sd.lpDescription = L"A Sample Service , Test Example";
	
	ChangeServiceConfig2(schNewSrv,SERVICE_CONFIG_DESCRIPTION,&sd);
	CloseServiceHandle(schNewSrv);
	CloseServiceHandle(sch);

	printf("Install Service Success!");
	return TRUE;
}

BOOL UnInstallService()
{
	SC_HANDLE scm = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	if(!scm)
	{
		PrintError(L"OpenSCManager Failed");
		return FALSE;
	}

	SC_HANDLE scml = OpenService(scm,L"ExampleService",SC_MANAGER_ALL_ACCESS);
	if(!scml)
	{
		PrintError(L"OpenService Failed");
		return FALSE;
	}
	SERVICE_STATUS ss;
	if(!QueryServiceStatus(scml,&ss))
	{
		PrintError(L"QueryServiceStatus Failed");
		return FALSE;
	}
	if(ss.dwCurrentState != SERVICE_STOPPED)
	{
		if(!ControlService(scml,SERVICE_CONTROL_STOP,&ss) && ss.dwCurrentState !=SERVICE_CONTROL_STOP)
		{
			PrintError(L"ControlService Stop Failed");
			return FALSE;
		}
	}
	if(!DeleteService(scml))
	{
		PrintError(L"DeleteService Failed");
		return FALSE;
	}
	printf("Delete Service Success!");
	return TRUE;
}

void WINAPI ServiceCtrlHandler(DWORD dwOpcode)
{
	switch(dwOpcode)
	{
	case SERVICE_CONTROL_STOP:
		ss.dwCurrentState = SERVICE_STOPPED;
		break;
	case SERVICE_CONTROL_PAUSE:
		ss.dwCurrentState = SERVICE_PAUSED;
		break;
	case SERVICE_CONTROL_CONTINUE:
		ss.dwCurrentState = SERVICE_CONTINUE_PENDING;
		break;
	case SERVICE_CONTROL_INTERROGATE:
		break;
	case SERVICE_CONTROL_SHUTDOWN:
		break;
	default:
		PrintError(L"bad service request");
	}
	SetServiceStatus(ssh,&ss);
}

VOID WINAPI ServiceMain(
  DWORD dwArgc,     // number of arguments
  LPTSTR *lpszArgv  // array of arguments
)
{
	ss.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	ss.dwCurrentState = SERVICE_START_PENDING;
	ss.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;
	ss.dwCheckPoint = 0;
	ss.dwServiceSpecificExitCode = 0;
	ss.dwWaitHint = 0;
	ss.dwWin32ExitCode = 0;

	ssh = RegisterServiceCtrlHandler(L"ExampleService",ServiceCtrlHandler);

	if(!ssh)
	{
		PrintError(L"RegisterService Fail");
		return;
	}
	if(!SetServiceStatus(ssh,&ss))
	{
		PrintError(L"SetServiceStatus 0x01 Fail");
		return;
	}

	ss.dwWin32ExitCode = S_OK;
	ss.dwCheckPoint = 0;
	ss.dwWaitHint = 0;
	ss.dwCurrentState = SERVICE_RUNNING;
	if(!SetServiceStatus(ssh,&ss))
	{
		PrintError(L"SetServiceStatus 0x02 Fail");
		return;
	}
	//SC_HANDLE scm = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	//SC_HANDLE scml = OpenService(scm,L"ExampleService",SC_MANAGER_ALL_ACCESS);
	//StartService(scml,0,NULL);
	//CloseServiceHandle(scml);
	//CloseServiceHandle(scm);

	while(1)
	{
		//do something
	}
	
}

void usage()
{
	printf("[[-i Install],[-r UnInstall]]");
}

int _tmain(int argc, _TCHAR* argv[])
{
	if(argc == 2)
	{
		//if arguments has 2
		wchar_t buf[10]={0};
		wcscpy_s(buf,argv[1]);
		if(0 == wcscmp(buf,L"-i"))
		{
			if(!InstallService())
			{
				PrintError(L"Install Service Failed");
				return -1;
			}
		}
		else if(0 == wcscmp(buf,L"-r"))
		{
			if(!UnInstallService())
				return -1;
			else
				return 0;
		}
	}
	else if(argc > 2)
	{
		usage();
		return -1;
	}


	SERVICE_TABLE_ENTRY srvEntry[] = {
		{L"ExampleService",ServiceMain},
		{NULL,NULL}
	};
	StartServiceCtrlDispatcher(srvEntry);
	return 0;
}

