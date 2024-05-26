//spinlock status: 0 -> unlocked; 1 -> locked

//locks the spinlock; enters STANDBY mode if spinlock is already taken
void spin_lock(volatile UINTPTR *lock);

//unlocks the spinlock; wakes up cores on STANDBY
void spin_unlock(volatile UINTPTR *lock);
