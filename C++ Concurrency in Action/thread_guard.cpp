#include <thread>

//follows RAII by ensuring that the thread object is dealt with when end of scope reached
class thread_guard
{
    std::thread t;

    public:
        explicit thread_guard(std::thread& t_) : t(t_){}
        
        ~thread_guard()
        {
            if (t.joinable())
            {
                t.join();
            }
        }

        thread_guard(thread_guard const&)=delete;
        thread_guard& operator=(thread_guard const&)=delete;
};

//this transfers ownership of the thread to the guard, and ensures no one else can join or detach it
class scoped_thread
{
    std::thread t;

    public:
        explicit scoped_thread(std::thread t_) : t(std::move(t))
        {
            if (!t.joinable())
            {
                throw std::logic_error("no thread");
            }
        }

        ~scoped_thread()
        {
            t.join();
        }

        //then handle copy and copy assignment operators as needed

};

struct func;

int main(int argc, char** argv)
{
    int local_state = 0;

    func my_func(local_state);

    std::thread t(my_func);

    thread_guard g(t);

    return 0;
}