#include <iostream>
#include <cstddef>
#include <vector>
#include <fstream>
#include <chrono>
#include <pthread.h>

class Integer{
  private:
    int _value;
    pthread_spinlock_t* _spinlock;

public:
    Integer(int value): _value{value}, _spinlock{new pthread_spinlock_t{}} {
        pthread_spin_init(_spinlock, PTHREAD_PROCESS_PRIVATE);
    }   
    
    int get_value() const {
        return _value;
    }

    void set_value(int value) {
        _value = value;
    }

    void operator++(int) {
        pthread_spin_lock(_spinlock);
        this->_value++;
        pthread_spin_unlock(_spinlock);
    }

    ~Integer() {
        pthread_spin_destroy(_spinlock);
        delete _spinlock;
    }
};


Integer integer{0};
void* increment(void* arg) {
    const std::size_t count = 100000;
    for (std::size_t i{}; i < count; i++){
       integer++;
    }
    return nullptr;
}

int main() {
    auto start = std::chrono::high_resolution_clock::now();
    std::size_t thread_count = 100;
    std::vector<pthread_t> threads(thread_count);

    for (std::size_t i = 0; i < thread_count; i++) {
        if (pthread_create(
                &threads[i],
                nullptr,
                increment,
                nullptr
            ) != 0) {
            return 1;
        }
    }

    for (std::size_t i = 0; i < thread_count; i++) {
        if (pthread_join(threads[i], nullptr) != 0){
            return 2;
        }
    }
    std::cout << integer.get_value() << std::endl;

    auto stop = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        stop - start
    );

    std::ofstream time_file{"time.txt", std::ios_base::app};
    time_file << "mutex: " << duration.count() << " milliseconds" << std::endl;
}
