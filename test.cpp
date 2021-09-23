#include <string>
#include <iostream>
#include "TPL.h"

int main(int /*argc*/, char const**/*argv*/)
{
    TPL::TaskPool pool(4);

    for (size_t i = 0; i < 100; i++) {
        pool.enqueue([](int a) { 
            
            std::this_thread::sleep_for(std::chrono::seconds(random() % 5));
            printf("%d, get_id=%ld\n", a, std::this_thread::get_id());
        }, i);

        printf("main=%d\n", i);
    }


    getchar();
    return 0;
}
