#include "socket.h"
#include <thread>
#include <pthread.h>

#define LOCALPORT 6000
#define REMOTEPORT 5500

#define SUCCESS 0
#define ERR_SOCKET 3

/*===================================================
 * Thread's domain
 * ==================================================
 * For asynchronous communication 2 threads are launched,
 * one deals with getting input and sending messages,
 * the second one deals with receiving and showing them.
 */

void getandSendMessage(Socket *local, Message message, sockaddr_in remote,
                       std::string *message_text, bool *endOfLoop)
{
    //We should ead until read quit or eof
    while (!*endOfLoop) {
        std::getline(std::cin,*message_text);

        if(*message_text == "/quit" || std::cin.eof())
            *endOfLoop = true;

        if (!*endOfLoop) {
            message_text->copy(message.text, sizeof(message.text) - 1, 0);
            message.text[message_text->length()] = '\0';
            local->sendTo(reinterpret_cast<const Message&>(message));
        }
    }

}

void firsThread(Socket *local,Message message, sockaddr_in *sinRemote,
                std::string *message_text,bool *endOfLoop)
{
    try {
        getandSendMessage(&*local,message,*sinRemote,&*message_text,
                          &*endOfLoop);
    } catch (std::system_error& e) {
        std::cerr << program_invocation_name << ": " << e.what()
        << std::endl;
        *endOfLoop = true;
    }

}
//=======================================================================

void receiveAndShowMessage(Socket *socket, Message *message,
                           sockaddr_in sinRemote)
{
    while(1) {
        socket->receiveFrom(*message);

        // Display received messages
        char* remote_ip = inet_ntoa(sinRemote.sin_addr);
        int remote_port = ntohs(sinRemote.sin_port);

        /* In order to increase the security we add a \0 to limit
         * the lecture to the size of the buffer, maybe the message
         * its not delimited by default.
         */
        message->text[1023] = '\0';

        std::cout << "System " << remote_ip << ":" << remote_port <<
        " sent: '" << message->text << "'" << std::endl;
    }

}

void secondThread(Socket *local,Message *message,
                  sockaddr_in *sinRemote, bool *endOfLoop)
{
    try {
        receiveAndShowMessage(&*local,&*message,*sinRemote);
    } catch (std::system_error& e) {
        std::cout << "Connection was over" << std::endl;
        *endOfLoop = true;
    }

}

//===============================================================

//This function will shutdown the threads
void requestCancellation(std::thread& oneThread)
{
    pthread_cancel(oneThread.native_handle());
}

/*===============================================================
 * End of Thread's domain
 *===============================================================
 */

void setupSocket(Socket *local, sockaddr_in sinLocal,
                 sockaddr_in sinRemote, int *aux)
{
    try {
         *local = Socket(sinLocal,sinRemote);

    }catch (std::system_error& e) {

        if (errno == ENOBUFS || errno == ENOMEM) {
            std::cerr << program_invocation_name
                      << ": Not enough memory for creating Socket "
                      << std::endl;
        } else {
            std::cerr << program_invocation_name << ": " << e.what()
            << std::endl;
        }

        *aux = ERR_SOCKET; //Program won't finish with 0
    }
}

void startCommunication(Socket *local,sockaddr_in *sinRemote)
{
    Message message;
    std::string message_text(""); //String for input
    bool endOfLoop = false;

    if(local->actingLikeServer()) {
        try {
            local->handleConnections(&*sinRemote);
        } catch (std::system_error& e) {
            std::cerr << program_invocation_name << ": " << e.what()
            << std::endl;
        }
    }

    //First thread with get input and send messages
    std::thread hilo1(&firsThread,&*local,message,&*sinRemote,&message_text,
                      &endOfLoop);
    //Second thread will recv messages and print them
    std::thread hilo2(&secondThread,&*local,&message,&*sinRemote,&endOfLoop);
    //They shall no block main thread
    hilo1.detach();
    hilo2.detach();

    while(!endOfLoop)
        usleep(25000);
    //We must finish both threads gracefully!
    requestCancellation(hilo1);
    requestCancellation(hilo2);
}

int main(void){

    int aux = SUCCESS; //Return value (0)
    //Preparing both structs local and Remote
    sockaddr_in sinLocal = makeIpAddress("0.0.0.0",LOCALPORT);
    sockaddr_in sinRemote = makeIpAddress("0.0.0.0",REMOTEPORT);

    Socket local;

    setupSocket(&local,sinLocal,sinRemote,&aux);

    if(aux == SUCCESS)
        startCommunication(&local,&sinRemote);

    return aux;
}

