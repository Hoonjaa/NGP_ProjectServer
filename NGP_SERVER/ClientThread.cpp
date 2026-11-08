#include "Header/ClientThread.h"
#include "Header/ServerTypes.h"

#include <memory>

DWORD WINAPI ProcessClient(LPVOID arg)
{
    std::shared_ptr<ClientSession> session(static_cast<ClientSession*>(arg));
	session->workContainer->job_push(PacketJob{ session->socket, std::vector<unsigned char>() }); 
    closesocket(session->socket);
    return 0;
}
