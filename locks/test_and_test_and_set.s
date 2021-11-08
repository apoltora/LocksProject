#  void Lock(int* lock) { while (1) {
#      while (*lock != 0);
#      if (test_and_set(*lock) == 0) return;
#      } }
#  void Unlock(volatile int* lock) {
#      *lock = 0;
#  }
#
#
#
#
#


