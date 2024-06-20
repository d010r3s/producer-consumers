#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

class Order {
public:
    virtual bool process() = 0;
    virtual ~Order() {}
};

class OnlineOrder : public Order {
private:
    std::mutex& io_mutex;
public:
    OnlineOrder(std::mutex& mtx) : io_mutex(mtx) {}

    bool process() override {
        int processTime = rand() % 5 + 1;
        {
            std::lock_guard<std::mutex> lock(io_mutex);
            std::cout << "Processing for " << processTime << " seconds..." << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(processTime));
        {
            std::lock_guard<std::mutex> lock(io_mutex);
            std::cout << "Complete" << std::endl;
        }
        return true;
    }
};

// Блокирующая очередь
class BlockingQueue {
private:
    std::queue<Order*> queue;
    std::mutex mtx;
    std::condition_variable cv;
public:
    void put(Order* item) {
        std::unique_lock<std::mutex> lock(mtx);
        queue.push(item);
        cv.notify_one();
    }

    Order* take() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !queue.empty(); });
        Order* item = queue.front();
        queue.pop();
        return item;
    }
};

// Производитель
class OrderProducer {
private:
    BlockingQueue& queue;
    int numberOfOrders;
    std::mutex& io_mutex;
public:
    OrderProducer(BlockingQueue& q, int num, std::mutex& mtx) : queue(q), numberOfOrders(num), io_mutex(mtx) {}

    void operator()() {
        for (int i = 0; i < numberOfOrders; ++i) {
            Order* order = new OnlineOrder(io_mutex);
            queue.put(order);
            {
                std::lock_guard<std::mutex> lock(io_mutex);
                std::cout << "Created order " << i + 1 << std::endl;
            }
        }
    }
};

// Потребитель
class OrderConsumer {
private:
    BlockingQueue& queue;
    std::string name;
    std::mutex& io_mutex;
public:
    OrderConsumer(BlockingQueue& q, const std::string& n, std::mutex& mtx) : queue(q), name(n), io_mutex(mtx) {}

    void operator()() {
        while (true) {
            Order* order = queue.take();
            {
                std::lock_guard<std::mutex> lock(io_mutex);
                std::cout << name << " is processing an order..." << std::endl;
            }
            order->process();
            delete order;
        }
    }
};

int main() {
    srand(time(0));
    BlockingQueue queue;
    std::mutex io_mutex;

    OrderProducer producer(queue, rand() % 10 + 5, io_mutex);
    std::thread producerThread(producer);
    
    OrderConsumer consumer1(queue, "Consumer 1", io_mutex);
    OrderConsumer consumer2(queue, "Consumer 2", io_mutex);
    std::thread consumerThread1(consumer1);
    std::thread consumerThread2(consumer2);
    
    producerThread.join();
    
    consumerThread1.detach();
    consumerThread2.detach();

    std::this_thread::sleep_for(std::chrono::seconds(60)); 

    return 0;
}
