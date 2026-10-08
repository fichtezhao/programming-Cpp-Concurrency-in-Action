class interrupt_flag
{
    std::atomic<bool> flag;
    std::condition_variable* thread_cond;
    std::condition_variable_any* thread_cond_any;
    std::mutex set_clear_mutex;

public:
    interrupt_flag():
        Interrupting threads
        thread_cond(0),thread_cond_any(0)
    {}

	// release signal
    void set()
    {
        flag.store(true,std::memory_order_relaxed);
        std::lock_guard<std::mutex> lk(set_clear_mutex);
        if(thread_cond)
        {
            thread_cond->notify_all();
        }
        else if(thread_cond_any)
        {
            thread_cond_any->notify_all();
        }
    }

	// wait until...
    template<typename Lockable>
    void wait(std::condition_variable_any& cv,Lockable& lk)
    {
        struct custom_lock
        {
            interrupt_flag* self;
            Lockable& lk;

            custom_lock(interrupt_flag* self_,
                        std::condition_variable_any& cond,
                        Lockable& lk_):
                self(self_),lk(lk_)
            {
                self->set_clear_mutex.lock();
                self->thread_cond_any=&cond;
            }

            void unlock()
            {
                lk.unlock();
                self->set_clear_mutex.unlock();
            }

            void lock()
            {
                std::lock(self->set_clear_mutex,lk);
            }

            ~custom_lock()
            {
                self->thread_cond_any=0;
                self->set_clear_mutex.unlock();
            }
        };

        custom_lock cl(this,cv,lk);
        interruption_point();
        cv.wait(cl);
        interruption_point();
    }

    // rest as before
};

thread_local interrupt_flag this_thread_interrupt_flag;

// where use it ??
template<typename Lockable>
void interruptible_wait(std::condition_variable_any& cv,
                        Lockable& lk)
{
    this_thread_interrupt_flag.wait(cv,lk);
}

void interruption_point()
{
	if(this_thread_interrupt_flag.is_set())
	{
		throw thread_interrupted();
	}
}

void interruptible_wait(std::condition_variable& cv,
                        std::unique_lock<std::mutex>& lk)
{
    interruption_point();
    this_thread_interrupt_flag.set_condition_variable(cv);
    interrupt_flag::clear_cv_on_destruct guard;
    interruption_point();
    cv.wait_for(lk,std::chrono::milliseconds(1));
    interruption_point();
}


std::mutex config_mutex;
std::vector<interruptible_thread> background_threads;// 9.09

void background_thread(int disk_id)
{
    while(true)
    {
        interruption_point(); //??
        fs_change fsc=get_fs_changes(disk_id); //??
        if(fsc.has_changes())
        {
            update_index(fsc);
        }
    }
}

void start_background_processing()
{
    background_threads.push_back(
        interruptible_thread(background_thread,disk_1));
    background_threads.push_back(
        interruptible_thread(background_thread,disk_2));
}

int main()
{
    start_background_processing();
    process_gui_until_exit(); //
    std::unique_lock<std::mutex> lk(config_mutex);
    for(unsigned i=0;i<background_threads.size();++i)
    {
        background_threads[i].interrupt();
    }
    for(unsigned i=0;i<background_threads.size();++i)
    {
        background_threads[i].join();
    }
}