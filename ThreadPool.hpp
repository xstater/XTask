#ifndef _XTASK_THREAD_POOL_H_
#define _XTASK_THREAD_POOL_H_

#include <queue>
#include <condition_variable>
#include <thread>
#include <mutex>
#include <future>
#include <type_traits>
#include <vector>
#include <functional>
#include <utility>

class ThreadPool{
    public:
        using uint = unsigned int;
    
        static ThreadPool &instance()
        {
            static ThreadPool pool;
            return pool;
        }

        enum class TaskPriority{
            NOW = 0,
            HIGH = 1,
            DEFAULT = 2,
            LOW = 3
        };

        ThreadPool(uint size = 0);
        ~ThreadPool();
        
        void quit();

        template <class Function,class... ArgsType>
        auto addTask(Function &&f,ArgsType &&...args)
            -> std::future<typename std::result_of<Function(ArgsType...)>::type>;

        template <class Function,class... ArgsType>
        auto addTask(TaskPriority priority,Function &&f,ArgsType &&...args)
            -> std::future<typename std::result_of<Function(ArgsType...)>::type>;
    protected:
    private:
        struct Task{
            Task(TaskPriority priority,std::function<void()> task)
                :m_priority(priority),m_task(std::move(task)){}
            friend bool operator<(Task a,Task b){
                return a.m_priority > b.m_priority;
            }
            TaskPriority m_priority;
            std::function<void()> m_task;
        };
        std::priority_queue<Task> m_tasks;
        std::vector<std::thread> m_threads;
        std::condition_variable m_cond;
        std::mutex m_mutex;
        bool m_is_quit;
};

ThreadPool::ThreadPool(uint size)
    :m_is_quit(false){
    if(size == 0){
        size = std::thread::hardware_concurrency();
        size = size<=1?1:size-1;
    }

    for(uint i=0;i<size;++i){
        m_threads.emplace_back([this]()->void{
            std::function<void()> task;
            while(true){
                {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    m_cond.wait(lock,
                        [this]()->bool{return m_is_quit || !m_tasks.empty();});
                    if(m_is_quit && m_tasks.empty())return;
                    task = m_tasks.top().m_task;
                    m_tasks.pop();
                }
                task();
            }
        });
    }
}

void ThreadPool::quit(){
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(m_is_quit)return;
        else m_is_quit = true;
    }
    m_cond.notify_all();
    for(auto &itr:m_threads){
        itr.join();
    }
}

inline ThreadPool::~ThreadPool(){
    quit();
}

template <class Function,class... ArgsType>
inline auto ThreadPool::addTask(Function &&f,ArgsType &&...args)
    -> std::future<typename std::result_of<Function(ArgsType...)>::type>{
    return addTask(TaskPriority::DEFAULT,std::forward<Function>(f),std::forward<ArgsType>(args)...);
}

template <class Function,class... ArgsType>
auto ThreadPool::addTask(TaskPriority priority,Function &&f,ArgsType &&...args)
    -> std::future<typename std::result_of<Function(ArgsType...)>::type>{
    using ReturnType = typename std::result_of<Function(ArgsType...)>::type;
    auto task = std::make_shared<std::packaged_task<ReturnType(ArgsType...)>>(std::bind(std::forward<Function>(f),std::forward<ArgsType>(args)...));
    std::future<ReturnType> ans = task->get_future();
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_tasks.emplace(priority,[task]()->void{(*task)();});
    }
    m_cond.notify_one();
    return ans;
}

#endif //_XTASK_THREAD_POOL_H_
