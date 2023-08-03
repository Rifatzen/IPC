# Debugging with Threads

- Compilation: `g++ -g project.cpp`
- Starting gdb: `gdb a.out`

Some commands that I used:
- `layout src`
- `b main`
- `b do_stuff`
- `run`
- `set scheduler-locking step`
- `bt` // optional, if you want to see the stack trace of the current thread
- `thread apply all bt` // optional, if you want to see the stack trace of all threads
- `info thr` // show all running threads
- `thread THREAD_ID` // switch to thread with id THREAD_ID, e.g: `thread 2`
- `until LINE_NUM` // continue until hitting LINE_NUM, e.g: `until 20`