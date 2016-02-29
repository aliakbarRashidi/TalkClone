#include "socket.h"

sockaddr_in makeIpAddress(const std::string& ip_address, int port){
    sockaddr_in tmp{};

    tmp.sin_family = AF_INET;
    const char * c = ip_address.c_str();
    inet_aton(c,&tmp.sin_addr);
    tmp.sin_port = htons(port);

    return tmp;
}

Socket::Socket()
{
    /*
     * We have to start the file descriptor to a non
     * valid value
     */
    fd_ = -1;
}

Socket::Socket(const sockaddr_in& address)
{
    // Crear el socket local
    fd_ = socket(AF_INET, SOCK_DGRAM, 0);

    if ( fd_ < 0 )
        throw std::system_error(errno, std::system_category(),
                                "no se pudo crear el socket");

    //Asignar la dirección con bind
    int result = bind(fd_, reinterpret_cast<const sockaddr*>(&address),
                      sizeof(address));

    if ( result < 0 )
        throw std::system_error(errno, std::system_category(),"falló bind: ");

}

Socket::Socket(Socket &older)
{
    fd_= older.getFd();
    older.setFd(-1);
}

Socket::~Socket()
{
    //Finalizamos el descriptor del fichero para terminar la conexion
    close(fd_);
}

int Socket::getFd()
{
    return fd_;
}

void Socket::setFd(int newFd)
{
    fd_ = newFd;
}

void Socket::sendTo(const Message& message, const sockaddr_in& address)
{
    int result = sendto(fd_, static_cast<const void*>(&message), sizeof(message),
                        0,reinterpret_cast<const sockaddr*>(&address),sizeof(address));

    if ( result < 0 )
        throw std::system_error(errno, std::system_category(), "falló sendto: ");
}

void Socket::receiveFrom(Message& message, sockaddr_in& address)
{
    socklen_t src_len = sizeof(address);

    int result = recvfrom(fd_,static_cast<void*>(&message), sizeof(message), 0,
        reinterpret_cast<sockaddr*>(&address), &src_len);

    if (result < 0)
        throw std::system_error(errno, std::system_category(), "falló recvfrom: ");
}

Socket& Socket::operator=(Socket& older)
{
    fd_=older.getFd();
    older.setFd(-1);

    return *this;
}
