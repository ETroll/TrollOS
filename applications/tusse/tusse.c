#include <stdio.h>
#include <string.h>
#include <trlib/system.h>
#include <trlib/mq.h>
#include <trlib/trui.h>

file_t* stdout = NULL;
file_t* stdin = NULL;

int main(int argc, char** argv)
{
    int parent = system_parentpid();

    // trui_clientmessage_t message = {
    //     .command = TRUI_SYSLOG,
    //     .id = 0,
    //     .width = 0,
    //     .height = 0,
    //     .x = 0,
    //     .y = 0
    // };

    // strcpy(message.text, "Hello parent!\0");

    system_debug(0x02);
    // char* data = "Hello parent!\0";
    // mq_send(parent, data, strlen(data), MQ_NOFLAGS);
    // mq_send(parent, &message, 124, MQ_NOFLAGS);
    // if(argc > 0)
    // {
    //     sprintf(message.text, "Got %d arguments\0", argc);
    //     // mq_send(parent, buffer, strlen(buffer), MQ_NOFLAGS);
    //     mq_send(parent, &message, sizeof(trui_clientmessage_t), MQ_NOFLAGS);
    //
    //     for(int i = 0; i<argc; i++)
    //     {
    //         sprintf(message.text, "Arg%d: %s\0", i, argv[i]);
    //         // mq_send(parent, buffer, strlen(buffer), MQ_NOFLAGS);
    //         mq_send(parent, &message, sizeof(trui_clientmessage_t), MQ_NOFLAGS);
    //     }
    // }
    // else
    // {
    //     // char* error = "NO ARGUMENTS!\0";
    //     strcpy(message.text, "NO ARGUMENTS!\0");
    //     // mq_send(parent, error, strlen(error), MQ_NOFLAGS);
    //     mq_send(parent, &message, sizeof(trui_clientmessage_t), MQ_NOFLAGS);
    //
    // }

    system_exit(1);
    return 0;
}
