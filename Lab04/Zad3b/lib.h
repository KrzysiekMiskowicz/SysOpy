#ifndef SYSTEMYOPERACYJNE_LIB_H
#define SYSTEMYOPERACYJNE_LIB_H
    enum MODE{KILL, SIGQUEUE, SIGRT};
    void send_signal(enum MODE send_mode, int sender_pid, int sig);
#endif //SYSTEMYOPERACYJNE_LIB_H
