#  struct lock {
#    volatile int next_ticket; volatile int now_serving;
#  };
#  void Lock(lock* l) {
#    int my_ticket = atomic_increment(&l->next_ticket);
#    while (my_ticket != l->now_serving);
#  }
#  void unlock(lock* l) {
#    l->now_serving++;
#  }



