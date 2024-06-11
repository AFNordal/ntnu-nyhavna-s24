import serial
import serial.tools.list_ports

def get_url():
    ports = serial.tools.list_ports.comports()
    for i, p in enumerate(ports):
        # print(p.description)
        if "USB" in p.description:
            print(f"Found port {p.name}:{p.description}")
            return ports[i].device
    raise Exception("No serial port found")