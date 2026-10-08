
// 9.09
// the final is 9.12, 9.13
// use promise, and future to implement asynchronism, as the wrapper of interrupt_flag
// use mtx,cv to implementation mgr of interrupt_flag
// the standard promise and future, can not be interruptible 



// enhanced in 9.11
class interrupt_flag
{
public:
    void set();
    bool is_set() const;
};
thread_local interrupt_flag this_thread_interrupt_flag;

class interruptible_thread
{
    std::thread internal_thread;
    interrupt_flag* flag;
public:
    template<typename FunctionType>
    interruptible_thread(FunctionType f)
    {
        std::promise<interrupt_flag*> p;
        internal_thread=std::thread([f,&p]{
                p.set_value(&this_thread_interrupt_flag);
                f();
            });
        flag=p.get_future().get(); // ptr to interrupt_flag
    }
    void interrupt()
    {
        if(flag)
        {
            flag->set();
        }
    }
};
