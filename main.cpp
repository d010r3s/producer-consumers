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
        std::cout << "Processing for " << processTime << " seconds..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(processTime));
        std::cout << "Complete" << std::endl;
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
public:
    OrderProducer(BlockingQueue& q, int num) : queue(q), numberOfOrders(num) {}

    void operator()() {
        for (int i = 0; i < numberOfOrders; ++i) {
            Order* order = new OnlineOrder();
            queue.put(order);
            std::cout << "Created order #" << i + 1 << std::endl;
        }
    }
};

// Потребитель
class OrderConsumer {
private:
    BlockingQueue& queue;
    std::string name;
public:
    OrderConsumer(BlockingQueue& q, const std::string& n) : queue(q), name(n) {}

    void operator()() {
        while (true) {
            Order* order = queue.take();
            std::cout << name << " is processing..." << std::endl;
            order->process();
            delete order;
        }
    }
};

int main() {
    srand(time(0));
    BlockingQueue queue;

    OrderProducer producer(queue, rand() % 10 + 5);
    std::thread producerThread(producer);

    OrderConsumer consumer1(queue, "Consumer 1");
    OrderConsumer consumer2(queue, "Consumer 2");
    std::thread consumerThread1(consumer1);
    std::thread consumerThread2(consumer2);

    producerThread.join();

    consumerThread1.detach();
    consumerThread2.detach();

    std::this_thread::sleep_for(std::chrono::seconds(60));

    return 0;
}
