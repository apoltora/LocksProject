#  AcquireQLock(*glock, *mlock)
#  {
#  mlock->next = NULL; mlock->state = UNLOCKED; ATOMIC();
#  prev = glock
#  *glock = mlock
#  END_ATOMIC();
#  if (prev == NULL) return; mlock->state = LOCKED; prev->next = mlock;
#  while (mlock->state == LOCKED)
#  ; // SPIN
#  }

#  ReleaseQLock(*glock, *mlock)
#  {
#  do {
#  if (mlock->next == NULL) {
#      x = CMPXCHG(glock, mlock, NULL);
#      if (x == mlock) return;
#    }
#  else {
#      mlock->next->state = UNLOCKED;
#  return; }
#  } while (1);
#  }