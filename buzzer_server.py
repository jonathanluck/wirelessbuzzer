import socket
import select
import time

clients = {}
buzzed = False
lastbuzz = 0

host = '0.0.0.0'
buzzport = 200
hbport = 400
clientport = 500
bufsiz = 1024
clienttimeout = 6.5 #TODO change this back for non-testing purposes
lockstr = b'\x01'
buzzstr = b'\x00'
clearstr = b'\x02'


def updateClients(d, addr):
    t = time.time()
    d[addr] = int(t)
    keys = list(d.keys())
    for k in keys:
        if((t - d[k]) > clienttimeout):
            d.pop(k)

def sendBuzz(d, buzzer):
    updateClients(d, buzzer)
    for k in d:
        s = socket.socket()
        s.connect((k, clientport))
        if(k != buzzer):
            s.send(lockstr)
        else:
            s.send(buzzstr)
        s.close()

def sendClear(d):
    for k in d:
        try:
            s = socket.socket()
            s.settimeout(0.5)
            s.connect((k, clientport))
            s.send(clearstr);
            s.close()
        except ConnectionRefusedError:
            print("connection error")
        except TimeoutError:
            print("timeout error")
        except socket.timeout:
            print("timeout error")

buzzserver = socket.socket()
heartbeatserver = socket.socket()
buzzserver.bind((host, buzzport))
heartbeatserver.bind((host, hbport))
buzzserver.listen(0)
heartbeatserver.listen(8)

while True:
    if(buzzed and (int(time.time()) - lastbuzz) >= 6):
        buzzed = False
        sendClear(clients)
    r, w, e = select.select([buzzserver, heartbeatserver], [], [], 0)
    for server in r:
        client, address = server.accept()
        data = client.recv(bufsiz)
        if(server == buzzserver and not buzzed):
            buzzed = True
            lastbuzz = int(time.time())
            sendBuzz(clients, address[0])
        elif(server == heartbeatserver):
            updateClients(clients, address[0])
        client.close()





