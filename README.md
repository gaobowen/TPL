# TPL
C++ Task Parallel Library.   
- Suitable for short tasks.
## example
```cpp
    TPL::TaskPool pool(4);

    for (size_t i = 0; i < 100; i++) {
        pool.enqueue([](int a) { 
            
            std::this_thread::sleep_for(std::chrono::seconds(random() % 5));
            printf("%d, get_id=%ld\n", a, std::this_thread::get_id());
        }, i);

        printf("main=%d\n", i);
    }
```
## reference
- [cameron314/concurrentqueue](https://github.com/cameron314/concurrentqueue)
- [progschj/ThreadPool](https://github.com/progschj/ThreadPool)