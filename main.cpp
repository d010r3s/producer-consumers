#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

// Заказ
class Order {
public:
    virtual bool process() = 0;
    virtual ~Order() {}
};

class OnlineOrder : public Order {
public:
    bool process() override {
        int processTime = rand() % 10 + 5;
        std::cout << "Processing order for " << processTime << " seconds..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(processTime));
        std::cout << "Order processing complete" << std::endl;
        return true;
    }
};

// Очередь
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
public:
    OrderProducer(BlockingQueue& q, int num) : queue(q), numberOfOrders(num) {}

    void operator()() {
        for (int i = 0; i < numberOfOrders; ++i) {
            Order* order = new OnlineOrder();
            queue.put(order);
            std::cout << "Created order " << i + 1 << std::endl;
        }
    }
};

// Потребитель
class OrderProcessor {
private:
    BlockingQueue& queue;
    std::string name;
public:
    OrderProcessor(BlockingQueue& q, const std::string& n) : queue(q), name(n) {}

    void operator()() {
        while (true) {
            Order* order = queue.take();
            std::cout << name << " is processing an order." << std::endl;
            order->process();
            delete order;
        }
    }
};

int main() {
    srand(time(0));
    BlockingQueue queue;

    OrderProducer producer(queue, 10);
    std::thread producerThread(producer);

    OrderProcessor processor1(queue, "Processor 1");
    OrderProcessor processor2(queue, "Processor 2");
    std::thread processorThread1(processor1);
    std::thread processorThread2(processor2);

    producerThread.join();

    processorThread1.detach();
    processorThread2.detach();

    std::this_thread::sleep_for(std::chrono::seconds(60));

    return 0;
}
