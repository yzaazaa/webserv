import socket
import threading
import sys

# Global flag and lock to indicate an error
error_occurred = False
error_lock = threading.Lock()

def send_requests(host, port):
    global error_occurred
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        s.connect((host, port))
        for _ in range(100):  # Adjust the number of messages as needed
            try:
                s.sendall(b'Test message')
                response = s.recv(1024)
                print(f"Received: {response}")
            except socket.error as e:
                print(f"Socket error: {e}")
                with error_lock:
                    error_occurred = True
                break
    except socket.error as e:
        print(f"Connection error: {e}")
        with error_lock:
            error_occurred = True
    finally:
        s.close()

def run_load_test(host, port, num_threads):
    global error_occurred
    threads = []
    for _ in range(num_threads):  # Number of concurrent clients
        t = threading.Thread(target=send_requests, args=(host, port))
        t.start()
        threads.append(t)
    
    for t in threads:
        t.join()
    
    if error_occurred:
        print("An error occurred. Exiting...")
        sys.exit(1)

# Run the load test
run_load_test('localhost', 8080, 5000)  # 50 concurrent clients
