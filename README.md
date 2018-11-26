# XTask

### Future
Future表示一个在将来被赋值的值<br>
Future<T>::get()阻塞当前线程直到值可用，并返回该值，如果有异常，则会抛出异常<br>
Future<T>::wait()阻塞当前线程直到值可用<br>
Future<T>::wait_for()等待一段时间，并返回等待后的状态(Status)<br>
Future<T>::wait_until()等待到某个时间点，并返回等待后的状态(Status)<br>
Future<T>::then()添加一个Continuation到当前Future，Future的值被设置之后会调用该Continuation，该函数会返回一个新的Future供链式调用<br>

### Promise
Promise<T>::set可以设置一个值或一个异常<br>
Promise<T>::get_future()返回一个对应的Future<T><br>

### Task
(Contructor)构造函数接受一个Callable对象<br>
Task<T>::get_future()返回一个对应的Future<T><br>

### when
when_all接受[begin,end)的迭代器，等待所有的Future的值被设置，返回一个Future<void>，其值会在所有Future被设置之后被设置<br>
when_all接受[begin,end)的迭代器，等待其中一个Future的值被设置，返回一个Future<void>，其值会在有一个Future被设置后被设置

## Examples
```
#include <iostream>
#include "Task.h"

using namespace std;

int main(){
    xtask::Task<int> task([]()->int{
        cout<<"Task"<<endl;
        return 1;
    });
    auto f = task.get_future().then([](Future<int> f)->int{
        cout<<"Task:"<<f.get()<<endl;
        return f.get()+1;
    }).then([](Future<int> f)->int{
        cout<<"Task:"<<f.get()<<endl;
        return f.get()+1;
    });
    task.run();
    f.get();
    return 0;
}

```