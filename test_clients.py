import socket
import threading

def run_client():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(("localhost", 6379))
    s.send(b"PING\r\n")
    data = s.recv(1024)
    print("Received:", data)
    s.close()

t1 = threading.Thread(target=run_client)
t2 = threading.Thread(target=run_client)

t1.start()
t2.start()
t1.join()
t2.join()
