#ifndef DEBUG
# define DEBUG

# define pthread_mutex_lock printf("%d", pthread_self());\
pthread_mutex_lock

#endif
