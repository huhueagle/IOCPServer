#ifndef __IOCP_SERVER_H__
#define __IOCP_SERVER_H__
#include "Addr.h"
#include "PerSocketContext.h"
#include <mswsock.h>
#include <vector>
#include <list>

//�����߳��˳���־
constexpr int EXIT_THREAD = 0;
constexpr int MAX_POST_ACCEPT = 10;
constexpr int DEFAULT_PORT = 10240; //Ĭ�϶˿ں�

struct ListenContext;
struct ClientContext;
struct AcceptIoContext;

class IocpServer
{
private:
	bool m_bIsShutdown; //�ر�ʱ���˳������߳�
	short m_listenPort; //�����������ļ����˿ں�
	LONG m_nMaxConnClientCnt; //���ͻ�������
	LONG m_nConnClientCnt; //�����ӿͻ�������
	LONG m_nWorkerCnt; //io�����߳�����
	HANDLE m_hIOCompletionPort; //��ɶ˿�
	HANDLE m_hExitEvent; //�˳��߳��¼�
	std::vector<HANDLE> m_hWorkerThreads; //�����߳̾���б�
	LPFN_GETACCEPTEXSOCKADDRS m_lpfnGetAcceptExAddr;
	LPFN_ACCEPTEX m_lpfnAcceptEx; //acceptEx����ָ��
	ListenContext* m_pListenCtx; //����������
	CRITICAL_SECTION m_csClientList; //�����ͻ�������std::list<ClientContext*>
	std::list<ClientContext*> m_connectedClientList; //�����ӿͻ�������
	std::list<ClientContext*> m_freeClientList; //���е�ClientContext����
	std::vector<AcceptIoContext*> m_acceptIoCtxList; //�������ӵ�IO�������б�

public:
	IocpServer(short listenPort = DEFAULT_PORT, int maxConnectionCount = 10000);
	IocpServer(const IocpServer&) = delete;
	IocpServer& operator=(const IocpServer&) = delete;
	virtual ~IocpServer();

	bool Start();
	bool Stop();
	bool Shutdown();
	bool Send(ClientContext* pConnClient, PBYTE pData, UINT len);

	// ��ȡ��ǰ������
	int GetConnectCount() { return m_nConnClientCnt; }
	// ��ȡ��ǰ�����˿�
	unsigned int GetPort() { return m_listenPort; }

protected:
	//����Ҫstatic _beginthreadex���ܷ���
	static unsigned WINAPI IocpWorkerThread(LPVOID arg);

	bool getAcceptExPtr();
	bool getAcceptExSockAddrs();
	bool setKeepAlive(ClientContext* pConnClient, 
		LPOVERLAPPED lpOverlapped, int time = 1, int interval = 1);

	bool createListenSocket(short listenPort);
	bool createIocpWorker();
	bool exitIocpWorker();
	bool initAcceptIoContext();

	bool postAccept(AcceptIoContext* pIoCtx);
	PostResult postRecv(ClientContext* pConnClient);
	PostResult postSend(ClientContext* pConnClient);

	bool handleAccept(LPOVERLAPPED lpOverlapped, DWORD dwBytesTransferred);
	bool handleRecv(ClientContext* pClientCtx, LPOVERLAPPED lpOverlapped,
		DWORD dwBytesTransferred);
	bool handleSend(ClientContext* pClientCtx, LPOVERLAPPED lpOverlapped,
		DWORD dwBytesTransferred);
	bool handleClose(ClientContext* pClientCtx);

	// Used to avoid access violation.
	void enterIoLoop(ClientContext* pClientCtx);
	int exitIoLoop(ClientContext* pClientCtx);

	void CloseClient(ClientContext* pConnClient);

	//���������ӿͻ����������̰߳�ȫ
	void addClientCtx(ClientContext* pConnClient);
	void removeClientCtx(ClientContext* pConnClient);
	void removeAllClientCtxs();

	ClientContext* allocateClientContext(SOCKET s);
	void releaseClientCtx(ClientContext* pConnClient);

	void echo(ClientContext* pConnClient);

	//�ص�����
	virtual void notifyNewConnection(ClientContext* pConnClient);
	//virtual void notifyDisconnected(ClientContext* pConnClient);
	virtual void notifyDisconnected(SOCKET s, Addr addr);
	virtual void notifyPackageReceived(ClientContext* pConnClient);
	virtual void notifyWritePackage();
	virtual void notifyWriteCompleted();
	virtual void showMessage(const char* szFormat, ...) {};
};

#endif // !__IOCP_SERVER_H__