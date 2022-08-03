# sy
sync a drive for removal. Windows command line app.

The logic:

    Flush out buffers first in case lock fails and removal has to happen regardless.
    Lock the volume so no other apps can write to it.
    Flush again in case any other app writes snuck in before the lock.
    Dismount the volume so it can be unplugged.           

usage:  sy [X]

    Use sy.exe before unplugging an external drive
    arguments:  [X] -- drive letter to flush, lock, flush (again), then dismount.
    examples:   sy r
                sy f:
