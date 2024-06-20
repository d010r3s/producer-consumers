#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <random>

using namespace std;

class Order {
public:
    virtual bool process() = 0;
    virtual ~Order() {}
};

class OnlineOrder : public Order {
private:
    mutex& io_mutex;
public:
    OnlineOrder(mutex& mtx) : io_mutex(mtx) {}

    bool process() override {
        // Генератор случайных чисел и распределение для каждого объекта
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(1, 5);
        int processTime = dis(gen);

        {
            lock_guard<mutex> lock(io_mutex);
            cout << "Processing for " << processTime << " seconds..." << endl;
        }
        this_thread::sleep_for(chrono::seconds(processTime));
        {
            lock_guard<mutex> lock(io_mutex);
            cout << "Complete" << endl;
        }
        return true;
    }
};

// Блокирующая очередь
class BlockingQueue {
private:
    queue<Order*> queue;
    mutex mtx;
    condition_variable cv;
public:
    void put(Order* item) {
        unique_lock<mutex> lock(mtx);
        queue.push(item);
        cv.notify_one();
    }

    Order* take() {
        unique_lock<mutex> lock(mtx);
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
    mutex& io_mutex;
public:
    OrderProducer(BlockingQueue& q, int num, mutex& mtx) : queue(q), numberOfOrders(num), io_mutex(mtx) {}

    void operator()() {
        for (int i = 0; i < numberOfOrders; ++i) {
            Order* order = new OnlineOrder(io_mutex);
            queue.put(order);
            {
                lock_guard<mutex> lock(io_mutex);
                cout << "Created order " << i + 1 << endl;
            }
        }
    }
};

// Потребитель
class OrderConsumer {
private:
    BlockingQueue& queue;
    string name;
    mutex& io_mutex;
public:
    OrderConsumer(BlockingQueue& q, const string& n, mutex& mtx) : queue(q), name(n), io_mutex(mtx) {}

    void operator()() {
        while (true) {
            Order* order = queue.take();
            {
                lock_guard<mutex> lock(io_mutex);
                cout << name << " is processing an order..." << endl;
            }
            order->process();
            delete order;
        }
    }
};

int main() {
    srand(time(0));
    BlockingQueue queue;
    mutex mtx;

    OrderProducer producer(queue, rand() % 10 + 5, mtx);
    thread producerThread(producer);

    OrderConsumer consumer1(queue, "Consumer 1", mtx);
    OrderConsumer consumer2(queue, "Consumer 2", mtx);
    thread consumerThread1(consumer1);
    thread consumerThread2(consumer2);

    producerThread.join();

    consumerThread1.detach();
    consumerThread2.detach();

    this_thread::sleep_for(chrono::seconds(30));

    return 0;
}
