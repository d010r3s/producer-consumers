# producer-consumers
2nd architecture project in the AMI's sophomore year
## Theory
The concept is a multithreaded order processing system for an online store  
The producer creates orders and places them into a blocking queue making access thread-safe. Consumers then retrieve orders from the queue and "process" them within random time.
## Structure
- [x] Producer: the thread that makes orders and places them in a blocking queue
- [x] Consumers: several threads that retrieve and process orders from the queue
- [x] class Order as an interface
- [x] class OnlineOrder as a processing simulation
- [x] BlockingQueue (using example from presentation)
      ![image](https://github.com/d010r3s/producer-consumers/assets/104917935/08698415-1702-43b1-b777-a9e33a0d2a3f)
## Example
![image](https://github.com/d010r3s/producer-consumers/assets/104917935/bf41d818-b3b4-4721-9528-88314f6b0805)
