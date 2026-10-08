class thread_pool
{
    std::atomic_bool done; // for simple type, convenient
	// since each thread execute the same function code, then must be mutual exclusive
    thread_safe_queue<std::function<void()> > work_queue; // just dummy function so far 
    std::vector<std::thread> threads;
    join_threads joiner; // ?

    void worker_thread()
    {
        while(!done)
        {
            std::function<void()> task;
            if(work_queue.try_pop(task))
            {
                task(); // one thread one task
            }
            else
            {
                std::this_thread::yield(); // wait until...??
            }
        }
    }
public:
    thread_pool():
        done(false),joiner(threads)
    {
        unsigned const thread_count=std::thread::hardware_concurrency();
        try
        {
            for(unsigned i=0;i<thread_count;++i)
            {
                threads.push_back(
                    std::thread(&thread_pool::worker_thread,this));
            }
        }
        catch(...)
        {
            done=true;
            throw;
        }
    }

    ~thread_pool()
    {
        done=true;
    }

	// fill task from external
    template<typename FunctionType>
    void submit(FunctionType f)
    {
        work_queue.push(std::function<void()>(f));
    }
};

// example

