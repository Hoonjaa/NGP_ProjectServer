#include "Header/ClientThread.h"
#include "Header/ServerTypes.h"

#include <memory>

DWORD WINAPI ProcessClient(LPVOID arg)
{
    std::unique_ptr<ClientSession> session(static_cast<ClientSession*>(arg));

    // Client receive and send loop goes here.
    closesocket(session->socket);
    return 0;
}
