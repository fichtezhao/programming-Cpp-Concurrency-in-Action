// enhancement on 9.01
// e.g. 9.05
void thread_pool::run_pending_task()
{
    function_wrapper task; // 9.02
    if(work_queue.try_pop(task)) // get task and wrap it, prons :
    {
        task();
    }
    else
    {
        std::this_thread::yield();
    }
}
