import pyautogui
import serial
import argparse
import time
import logging

class MyControllerMap:
    def __init__(self):
         self.button = {
            'VERDE': 'A',
            'VERMELHO': 'S',
            'AMARELO': 'J',
            'AZUL': 'K',
            'POWER': 'shift'
        }

class SerialControllerInterface:
    # Protocolo
    # byte 1 -> Botão 1 (estado - Apertado 1 ou não 0)
    # byte 2 -> EOP - End of Packet -> valor reservado 'X'

    def __init__(self, port, baudrate):
        self.ser = serial.Serial(port, baudrate=baudrate)
        self.mapping = MyControllerMap()
        self.incoming = '0'
        pyautogui.PAUSE = 0  ## remove delay
    
    def update(self):
        ## Sync protocolfv z
        while self.incoming != b'X':
            self.incoming = self.ser.read()
            logging.debug("Received INCOMING: {}".format(self.incoming))

        data_byte = self.ser.read()
        if data_byte:  # Make sure that a byte was actually read.
            data = data_byte[0]  # Convert from bytes to int
            logging.debug("Received DATA: {}".format(data))


        if data & 0b00000001:
            logging.info("KEYDOWN VERDE")
            pyautogui.keyDown(self.mapping.button['VERDE'])
        else:
            logging.info("KEYUP VERDE")
            pyautogui.keyUp(self.mapping.button['VERDE'])

        if data & 0b00000010:
            logging.info("KEYDOWN VERMELHO")
            pyautogui.keyDown(self.mapping.button['VERMELHO'])
        else:
            logging.info("KEYUP VERMELHO")
            pyautogui.keyUp(self.mapping.button['VERMELHO'])

        if data & 0b00000100:
            logging.info("KEYDOWN AMARELO")
            pyautogui.keyDown(self.mapping.button['AMARELO'])
        else:
            logging.info("KEYUP AMARELO")
            pyautogui.keyUp(self.mapping.button['AMARELO'])

        if data & 0b00001000:
            logging.info("KEYDOWN AZUL")
            pyautogui.keyDown(self.mapping.button['AZUL'])
        else:
            logging.info("KEYUP AZUL")
            pyautogui.keyUp(self.mapping.button['AZUL'])

        if data & 0b00010000:
            logging.info("KEYDOWN POWER")
            pyautogui.keyDown(self.mapping.button['POWER'])
        else:
            logging.info("KEYUP POWER")
            pyautogui.keyUp(self.mapping.button['POWER'])

        self.incoming = self.ser.read()


class DummyControllerInterface:
    def __init__(self):
        self.mapping = MyControllerMap()

    def update(self):
        pyautogui.keyDown(self.mapping.button['VERDE'])
        time.sleep(0.1)
        pyautogui.keyUp(self.mapping.button['VERDE'])
        logging.info("[Dummy] Pressed A button")
        time.sleep(1)

        pyautogui.keyDown(self.mapping.button['VERMELHO'])
        time.sleep(0.1)
        pyautogui.keyUp(self.mapping.button['VERMELHO'])
        logging.info("[Dummy] Pressed A button")
        time.sleep(1)

        pyautogui.keyDown(self.mapping.button['AMARELO'])
        time.sleep(0.1)
        pyautogui.keyUp(self.mapping.button['AMARELO'])
        logging.info("[Dummy] Pressed A button")
        time.sleep(1)

        pyautogui.keyDown(self.mapping.button['AZUL'])
        time.sleep(0.1)
        pyautogui.keyUp(self.mapping.button['AZUL'])
        logging.info("[Dummy] Pressed A button")
        time.sleep(1)

        pyautogui.keyDown(self.mapping.button['POWER'])
        time.sleep(0.1)
        pyautogui.keyUp(self.mapping.button['POWER'])
        logging.info("[Dummy] Pressed A button")
        time.sleep(1)

if __name__ == '__main__':
    interfaces = ['dummy', 'serial']
    argparse = argparse.ArgumentParser()
    argparse.add_argument('serial_port', type=str)
    argparse.add_argument('-b', '--baudrate', type=int, default=9600)
    argparse.add_argument('-c', '--controller_interface', type=str, default='serial', choices=interfaces)
    argparse.add_argument('-d', '--debug', default=False, action='store_true')
    args = argparse.parse_args()
    if args.debug:
        logging.basicConfig(level=logging.DEBUG)

    print("Connection to {} using {} interface ({})".format(args.serial_port, args.controller_interface, args.baudrate))
    if args.controller_interface == 'dummy':
        controller = DummyControllerInterface()
    else:
        controller = SerialControllerInterface(port=args.serial_port, baudrate=args.baudrate)

    while True:
        controller.update()
