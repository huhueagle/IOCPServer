#include "MyServerEngine.h"
#include "..\ServerUtil\pub.h"


namespace MyServer{

	MyServerEngine* g_pServerEngine = NULL;

	HANDLE MyServerEngine::m_hMutexServerMsgs = CreateMutex(NULL,FALSE,"m_hMutexServerMsgs");

	LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
		// Don't start processing messages until the application has been created.
		if( g_pServerEngine != 0 ){
			return g_pServerEngine->msgProc(msg, wParam, lParam);
		}else{
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}

	}


	MyServerEngine::MyServerEngine(HINSTANCE hInstance){
		mhAppInst = hInstance;
		mhMainWnd = 0;

		mServerPort = 12345;	//�����˿�

		hEB_InputServerIP	= NULL;
		hEB_InputServerPort	= NULL;

		hLB_Output			= NULL;
		hST_TextServerIP	= NULL;
		hST_TextServerPort	= NULL;
		lpszApplicationName = "GameServer";
		lpszTitle			= "GameServer Window";


		hBtnStart = NULL;
		hBtnStop = NULL;
		hBtnExit	= NULL;


		//��ʼ������
		this->InitMainWindow();
	
	}


	MyServerEngine::~MyServerEngine(){
		//
	}


	HINSTANCE MyServerEngine::getAppInst(){
		return mhAppInst;
	}

	HWND MyServerEngine::getMainWnd(){
		return mhMainWnd;
	}

	void MyServerEngine::Init(){
		// ��ʼ��Socket��
		if( m_IOCP.LoadSocketLib() == false){
			MessageBox(NULL,"����Winsock 2.2ʧ�ܣ����������޷����У�","��ʾ!",MB_OK);
			PostQuitMessage(0);
		}

		//��ʼ�������ӿؼ�
		this->InitControls(mhMainWnd);

		//���ü����˿�
		this->m_IOCP.SetPort(this->mServerPort);

		//ip��ַ�ͼ����˿�
		SetWindowText(hEB_InputServerIP,m_IOCP.GetLocalIP().c_str());
		SetWindowText(hEB_InputServerPort,(pub::Int2Str(this->mServerPort)).c_str());
		EnableWindow(hEB_InputServerIP,false);
	}

	void MyServerEngine::ShutDown(){
		m_IOCP.Stop();
	}

	int MyServerEngine::Run(){
		MSG  msg;
		msg.message = WM_NULL;

		static long last_time = timeGetTime();

		while(msg.message != WM_QUIT){

			// If there are Window messages then process them.
			if(PeekMessage( &msg, 0, 0, 0, PM_REMOVE )){
				TranslateMessage( &msg );
				DispatchMessage( &msg );

			}else{
				//������Ϣ��ʾ����
				this->SettleServerMsgs();
				
			}
		}
		return (int)msg.wParam;
	}

	void MyServerEngine::InitMainWindow(){

		WNDCLASSEX	wndclass;

		wndclass.cbSize			= sizeof(wndclass);
		wndclass.style			= CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc	= MainWndProc;
		wndclass.cbClsExtra		= 0;
		wndclass.cbWndExtra		= 0;
		wndclass.hInstance		= mhAppInst;
		wndclass.hIcon			= LoadIcon( NULL, IDI_APPLICATION );
		wndclass.hIconSm		= LoadIcon( NULL, IDI_APPLICATION );
		wndclass.hCursor		= LoadCursor( NULL, IDC_ARROW );
		wndclass.hbrBackground	= (HBRUSH)(COLOR_WINDOW);
		wndclass.lpszMenuName	= NULL;
		wndclass.lpszClassName	= lpszApplicationName;	// Registered Class Name
		

		if( RegisterClassEx( &wndclass ) == 0 ) {
			MessageBox(0, "RegisterClass FAILED", 0, 0);
			exit(1);
		}

		// Create the window
		mhMainWnd = CreateWindow(
			lpszApplicationName,		// Application Name
			lpszTitle,					// Name Displayed on Title Bar
			WS_OVERLAPPEDWINDOW&~WS_MAXIMIZEBOX&~WS_MINIMIZEBOX,
			200,
			200,
			800,
			600,
			NULL,
			NULL,
			mhAppInst,
			NULL );


		if( !mhMainWnd ){
			MessageBox(0, "CreateWindow FAILED", 0, 0);
			PostQuitMessage(0);
		}


		ShowWindow(mhMainWnd, SW_SHOW);
		UpdateWindow(mhMainWnd);

	}

	void MyServerEngine::InitControls(HWND hWnd){
		// hEB_InputServerIP
		hST_TextServerIP = CreateWindow(
			"static","Server IP",
			WS_CHILD | SS_CENTER | WS_VISIBLE,
			5,
			5,
			120,
			28,
			hWnd,(HMENU)IDC_hST_TextServerIP,mhAppInst,NULL);

		// hEB_InputServerIP
		hST_TextServerPort = CreateWindow(
			"static","Port",
			WS_CHILD | SS_CENTER | WS_VISIBLE,
			125,
			5,
			50,
			28,
			hWnd,(HMENU)IDC_hST_TextServerPort,mhAppInst,NULL);



		// hEB_InputServerIP
		hEB_InputServerIP = CreateWindowEx(
			WS_EX_CLIENTEDGE,
			"EDIT","192.168.0.2",
			WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
			5,
			20,
			120,
			28,
			hWnd,(HMENU)IDC_hEB_InputServerIP,mhAppInst,NULL);

		// hEB_InputServerPort
		hEB_InputServerPort = CreateWindowEx(
			WS_EX_CLIENTEDGE,
			"EDIT","6000",
			WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
			125,
			20,
			50,
			28,
			hWnd,(HMENU)IDC_hEB_InputServerPort,mhAppInst,NULL);

		// hLB_Output
		hLB_Output = CreateWindowEx(
			WS_EX_CLIENTEDGE,
			"LISTBOX",
			NULL,
			WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL|WS_HSCROLL| WS_BORDER,
			5,
			50,
			780,
			480,
			hWnd,(HMENU)IDC_hLB_Output,mhAppInst,NULL);	


		//��ʼ����
		hBtnStart = CreateWindow(
			"BUTTON",
			"��ʼ����",
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			5,
			530,
			100,
			28,
			hWnd,(HMENU)IDC_hBtn_Start,mhAppInst,NULL);

		//ֹͣ����
		hBtnStop = CreateWindow(
			"BUTTON",
			"ֹͣ����",
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			150,
			530,
			100,
			28,
			hWnd,(HMENU)IDC_hBtn_Stop,mhAppInst,NULL);

		//�˳���ť
		hBtnExit = CreateWindow(
			"BUTTON",
			"�˳�",
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			650,
			530,
			100,
			28,
			hWnd,(HMENU)IDC_hBtn_Exit,mhAppInst,NULL);

	}

	//���ӷ�������Ϣ
	void MyServerEngine::AddServerMsgs(string& msg){
		WaitForSingleObject(m_hMutexServerMsgs,INFINITE);
		m_vtServerMsgs.push_back(msg);
		ReleaseMutex(m_hMutexServerMsgs);
	}

	//������������Ϣ
	void MyServerEngine::SettleServerMsgs(){
		WaitForSingleObject(m_hMutexServerMsgs,INFINITE);
		if(m_vtServerMsgs.size() > 0){
			for(DWORD i = 0; i < m_vtServerMsgs.size(); i ++){
				this->ShowText(m_vtServerMsgs.at(i));
			}
		}else{
			Sleep(10);
		}
		m_vtServerMsgs.clear();
		
		ReleaseMutex(m_hMutexServerMsgs);

	}

	void MyServerEngine::ShowText(string& msg){
		int Line;

		// add string to the listbox
		SendMessage(hLB_Output,LB_ADDSTRING,0,(LPARAM) msg.c_str());

		// determine number of items in listbox
		Line = SendMessage(hLB_Output,LB_GETCOUNT,0,0);

		// flag last item as the selected item, to scroll listbox down
		SendMessage(hLB_Output,LB_SETCURSEL,Line-1,0);

		// unflag all items to eliminate negative highlite
		SendMessage(hLB_Output,LB_SETCURSEL,-1,0);
	}




	LRESULT MyServerEngine::msgProc(UINT msg, WPARAM wParam, LPARAM lParam){
		
		switch( msg ){

				case WM_COMMAND:
					switch(LOWORD(wParam))
					{
					case IDC_hBtn_Start:
						//ShowText(string("Connecting to Server"));
						//���ü����˿�
						char aa[64];
						GetWindowText(hEB_InputServerPort,aa,64);
						this->mServerPort = pub::str2int(string(aa));
						this->m_IOCP.SetPort(this->mServerPort);
						this->m_IOCP.Start();
						EnableWindow(hBtnStart,false);
						break;

					case IDC_hBtn_Stop:
						this->m_IOCP.Stop();
						EnableWindow(hBtnStart,true);
						break;

					case IDC_hBtn_Exit:
						PostQuitMessage(0);
						break;
					}

					break;

				case WM_CLOSE:
					{
						DestroyWindow(mhMainWnd);
					}
					return 0;

				case WM_DESTROY:
					{
						PostQuitMessage(0);
					}

					return 0;
		}
		return DefWindowProc(mhMainWnd, msg, wParam, lParam);
	}


}