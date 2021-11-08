#  struct lock {
#  volatile padded_int status[P]; volatile int head;
#  };
#  int my_element;
#  void Lock(lock* l) {
#    my_element = atomic_circ_increment(&l->head);
#    while (l->status[my_element] == 1);
#  }
#  void unlock(lock* l) { l->status[my_element] = 1; l->status[circ_next(my_element)] = 0;
#  }