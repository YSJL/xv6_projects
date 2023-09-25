# xv6_projects
-	Series of projects that implemented OS concepts on an educational version of UNIX, xv6 using C
-	Implemented: Stack backtrace, Dynamic RAM boot, Copy-on-Write fork, Zero-Initialized Data, Priority FIFO/RR scheduling, Kernel Threading, User Isolation, File Permissions, Login
## Lab1
- Implemented Stack Backtrace and Dynamic RAM boot
- Implemented a Backtrace method prints the stack trace of any functions run within the kernel by travelling up the stack.
- Edited BIOS and Bootloader so that it detects amount of RAM and allocates accordingly rather than the original implementation of setting a fixed value.
