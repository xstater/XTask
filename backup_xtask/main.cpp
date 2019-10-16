#include <functional>
#include <iostream>
#include <future>
#include <thread>
#include <vector>
#include "ThreadPool.h"
#include "Task.h"
#include "When.h"

using namespace std;

int main(){
    xtask::ThreadPool::instance();
    /*
    auto t = xtask::Task<int>([]()->int{
        cout<<"1nmsl1\n";
        throw std::runtime_error("asdasd");
        return 1;
    }).then([](int x)->int{
        cout<<"1nmsl2\n";
        return x+1;
    });
    //for(volatile int i = 1;i<1000000000;i++){}
    t.then([](int x)->void{
        cout<<"1nmsl3\n";
        cout<<x<<endl;
    },[](std::exception_ptr expt)->void{
        try{
            std::rethrow_exception(expt);
        }catch(std::exception &e){
            cout<<e.what()<<endl;
        }
    });
*/
    xtask::Task<std::vector<int>> task([]()->std::vector<int>{
        std::vector<int> data;
        for(int i = 1;i <= 100; ++i)data.push_back(i);
        return data;
    });
    auto cont = task.then([](const std::vector<int> &v)->void{
        for(int i:v){
            cout<<i<<' ';
        }
    });
    try{
        cont.get();
    }catch(const std::exception &e){
        cout<<e.what()<<endl;
    }


    cout<<"nmsl"<<endl;
/*
    xtask::Task<void> task([]()->void{
        cout<<"nmsl"<<endl;
        },false,xtask::Policy::synchronized);

    xtask::Task<void> task2(std::bind([](const xtask::Task<void> &task)->void{
        auto _task = task;
        cout<<"nmsl"<<endl;
        _task.run();
    },std::ref(task)),false,xtask::Policy::synchronized);

    task2.run();
*/
/*
    xtask::Task<void> tasks[7] = {
            xtask::Task<void>([]()->void{
                for(volatile int i = 1;i<500000000;i++){}
                cout<<"nmsl1\n";
            },true,xtask::Policy::pool),
            xtask::Task<void>([]()->void{
                for(volatile int i = 1;i<500000000;i++){}
                cout<<"nmsl2\n";
            },true,xtask::Policy::pool),
            xtask::Task<void>([]()->void{
                for(volatile int i = 1;i<500000000;i++){}
                cout<<"nmsl3\n";
            },true,xtask::Policy::pool),
            xtask::Task<void>([]()->void{
                for(volatile int i = 1;i<500000000;i++){}
                cout<<"nmsl4\n";
            },true,xtask::Policy::pool),
            xtask::Task<void>([]()->void{
                for(volatile int i = 1;i<500000000;i++){}
                cout<<"nmsl5\n";
            },true,xtask::Policy::pool),
            xtask::Task<void>([]()->void{
                for(volatile int i = 1;i<500000;i++){}
                cout<<"nmsl6\n";
            },true,xtask::Policy::thread),
            xtask::Task<void>([]()->void{
                for(volatile int i = 1;i<500000000;i++){}
                cout<<"nmsl7\n";
            },true,xtask::Policy::pool),
    };

    xtask::when_any(std::begin(tasks),std::end(tasks)).then([]()->void{
       cout<<"done\n";
    },xtask::Policy::synchronized).wait();
*/
    xtask::ThreadPool::instance().quit();

    return 0;
}