#include <zator.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

enum payload_type {
    ptask,
    pfile,
    pnetwork
};

typedef struct event {
    struct {
        enum payload_type type;
        int time;
        char* origin;
    } header;
    int itime;
    char* file;
    int line;
    void* payload;
} event_t;

typedef struct task_payload {
    int pid;
    int ppid;
    char* imagefilename;
    unsigned int uid;
} task_payload_t;

typedef struct file_payload {
    int mtime;
    int ctime;
    char* filename;
    int atime;
    long long size;
    void* sig;
    long long sig_length;
} file_payload_t;

typedef struct network_payload {
    int p_size;
    char source[4];
    unsigned short source_port;
    char remote[4];
    unsigned short remote_port;
} network_payload_t;

void generate_event(event_t* event, int type)
{
    event->header.type = type;
    event->header.time = time(NULL);
    event->header.origin = (char*) __FUNCTION__; // static storage duration
    event->itime = 1488;
    event->line = __LINE__;
    event->file = (char*) __FILE__;
    switch(type) {
    case ptask: {
        task_payload_t* task = (task_payload_t*) malloc(sizeof(task_payload_t));
        if (!task) {
            printf("Cannot allocate memory!\n");
            exit(1);
        }
        task->pid = 228;
        task->ppid = 227;
        task->imagefilename = (char*) "/usr/bin/hohojajaolala"; // static storage duration
        task->uid = 1000;
        event->payload = task;
    }
    break;
    case pfile: {
        file_payload_t* file = (file_payload_t*) malloc(sizeof(file_payload_t));
        if (!file) {
            printf("Cannot allocate memory!\n");
            exit(1);
        }
        file->mtime = 12927312;
        file->ctime = 12930429;
        file->filename = NULL;
        file->atime = 0;
        file->size = 128;
        file->sig = (void*) "\x99\u9231\u1238\u3812\u2317\u2137\x99";
        file->sig_length = 3;
        event->payload = file;
    }
    break;
    case pnetwork: {
        network_payload_t* network = (network_payload_t*) malloc(sizeof(network_payload_t));
        if (!network) {
            printf("Cannot allocate memory!\n");
            exit(1);
        }
        network->p_size = 1488;
        memcpy(network->source, "123", 4);
        network->source_port = 5343;
        memcpy(network->remote, "321", 4);
        network->remote_port = 32123;
        event->payload = network;
    }
    break;
    default:
    break;
    }
}

event_t* serialize_event(event_t* event, void* where, unsigned long max, int dry)
{
    unsigned long event_length = 0, payload_length;

    _Z_SERIALIZE_STRUCT(where, event, dry, event_length);
    Z_SERIALIZE_STRUCT_STR_FIELD(header.origin);
    Z_SERIALIZE_STRUCT_STR_FIELD(file);

    //printf("Event + header length: %lu\n", event_length);

    payload_length = event_length;
    switch (event->header.type) {
    case ptask:
        _Z_SERIALIZE_STRUCT_NESTED(payload, task_payload_t);
        Z_SERIALIZE_STRUCT_STR_FIELD(imagefilename);
        Z_SERIALIZE_STRUCT_NESTED_END();
        break;
    case pfile:
        _Z_SERIALIZE_STRUCT_NESTED(payload, file_payload_t);
        Z_SERIALIZE_STRUCT_STR_FIELD(filename);
        Z_SERIALIZE_STRUCT_PTR_FIELD(sig, ((file_payload_t*) event->payload)->sig_length);
        Z_SERIALIZE_STRUCT_NESTED_END();
        break;
    case pnetwork:
        _Z_SERIALIZE_STRUCT_NESTED(payload, network_payload_t);
        Z_SERIALIZE_STRUCT_NESTED_END();
        break;
    default:
        break;
    }

    Z_SERIALIZE_STRUCT_END();

    //printf("Payload length: %lu\n", event_length - payload_length);
    printf("Event + header + payload length: %lu\n", event_length);
}

void deserialize_event(event_t* event)
{
    Z_DESERIALIZE_STRUCT(event);
    Z_DESERIALIZE_STRUCT_FIELD(header.origin);
    Z_DESERIALIZE_STRUCT_FIELD(file);
    Z_DESERIALIZE_STRUCT_FIELD(payload);

    switch (event->header.type) {
    case ptask: {
        task_payload_t* payload = event->payload;
        Z_DESERIALIZE_STRUCT(payload);
        Z_DESERIALIZE_STRUCT_FIELD(imagefilename);
        Z_DESERIALIZE_STRUCT_END();
    } break;
    case pfile: {
        file_payload_t* payload = event->payload;
        Z_DESERIALIZE_STRUCT(payload);
        Z_DESERIALIZE_STRUCT_FIELD(filename);
        Z_DESERIALIZE_STRUCT_FIELD(sig);
        Z_DESERIALIZE_STRUCT_END();
    } break;
    default:
    break;
    }

    Z_DESERIALIZE_STRUCT_END();
}

void print_event(struct event* event)
{
    printf("Event:\n\titime: %d\n\tfile: %s\n\tline: %d\n", event->itime, event->file, event->line);
    printf("Header:\n\ttype: %d\n\ttime: %d\n\torigin: %s\n", event->header.type, event->header.time, event->header.origin);
    switch (event->header.type) {
    case ptask: {
        task_payload_t* payload = event->payload;
        if (payload)
            printf("TaskPayload:\n\tpid: %d\n\tppid: %d\n\timagefilename: %s\n\tuid: %d\n", payload->pid, payload->ppid, payload->imagefilename, payload->uid);
    } break;
    case pfile: {
        file_payload_t* payload = event->payload;
        if (payload)
            printf("FilePayload:\n\tmtime: %d\n\tctime: %d\n\tfilename: %s\n\tatime: %d\n\tsize: %ld\n\tsig_length: %lu\n", payload->mtime, payload->ctime, payload->filename, payload->atime, payload->size, payload->sig_length);
    } break;
    case pnetwork: {
        network_payload_t* payload = event->payload;
        if (payload)
            printf("NetworkPayload:\n\tp_size: %d\n\tsource: %s\n\tsource_port: %d\n\tremote: %s\n\tremote_port: %d\n", payload->p_size, payload->source, payload->source_port, payload->remote, payload->remote_port);
    } break;
    default:
    break;
    }
}

int main()
{
    const int es = 33 * 3;
    event_t stack_events[es];
    event_t* heap_events;
    char* buffer;
    int i;

    heap_events = (event_t*) malloc(sizeof(event_t) * es);
    if (!heap_events) {
        printf("Couldn't allocate memory!\n");
        return -1;
    }

    buffer = (char*) malloc(1 << 22); 
    if (!buffer) {
        printf("Couldn't allocate memory!\n");
        free(heap_events);
        return -1;
    }
    memset(buffer, 0, 1 << 22);

    for (i = 0; i < es; i++) {
        generate_event(&stack_events[i], i / 33);
        generate_event(&heap_events[i], i / 33);
    }

    for (i = 0; i < es; i++) {
        serialize_event(&stack_events[i], buffer + i * 400, 400, 0);
        free(stack_events[i].payload);
        serialize_event(&heap_events[i], buffer + es * 400 + i * 400, 400, 0);
        free(heap_events[i].payload);
    }

    for (i = 0; i < es; i++) {
        struct event* event = (event_t*) (buffer + i * 400);
        deserialize_event(event);
        print_event(event);
        event = (event_t*) (buffer + es * 400 + i * 400);
        deserialize_event(event);
        print_event(event);
    }

    free(heap_events);
    free(buffer);

    return 0;
}