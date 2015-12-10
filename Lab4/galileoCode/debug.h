#ifndef DEBUG
# define DEBUG

# define pthread_mutex_lock printf("%d", phread_self());\
pthread_mutex_lock

#endif
