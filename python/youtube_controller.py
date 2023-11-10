import pyautogui
import serial
import argparse
import time
import logging
from ctypes import cast, POINTER
from comtypes import CLSCTX_ALL
from pycaw.pycaw import AudioUtilities, IAudioEndpointVolume


class MyControllerMap:
    def __init__(self):
         self.button = {
            'VERDE': 'A',
            'VERMELHO': 'S',
            'AMARELO': 'J',
            'AZUL': 'K',
            'POWER': 'space'
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
        self.but = 0
        self.afec = 0

        
    
    
    
    def update(self):
        
        handshake = False;
        while True:
            while not handshake:
                byte = self.ser.read()
                # logging.debug(f"Received byte : {byte} with type {type(byte)}")
                if byte == b'z':
                    self.ser.write(b'z')
                    handshake = True
            
            
            self.incoming = self.ser.read()
            
            if self.incoming == b'S':
                self.afec = self.ser.read(3)
                volume.SetMasterVolumeLevelScalar(float(self.afec), None)
                
                print(self.afec)
                
            if self.incoming == b'Z':
                self.but = self.ser.read(2)
                print(self.but)
            
            if self.but == b'11':
                # logging.info("KEYDOWN VERDE")
                pyautogui.keyDown(self.mapping.button['VERDE'])
            else:
                # logging.info("KEYUP VERDE")
                pyautogui.keyUp(self.mapping.button['VERDE'])
            
            if self.but == b'22':
                # logging.info("KEYDOWN VERMELHO")
                pyautogui.keyDown(self.mapping.button['VERMELHO'])
            else:
                # logging.info("KEYUP VERMELHO")
                pyautogui.keyUp(self.mapping.button['VERMELHO'])

            if self.but == b'44':
                # logging.info("KEYDOWN AMARELO")
                pyautogui.keyDown(self.mapping.button['AMARELO'])
            else:
                # logging.info("KEYUP AMARELO")
                pyautogui.keyUp(self.mapping.button['AMARELO'])
            
            if self.but == b'88':
                # logging.info("KEYDOWN AMARELO")
                pyautogui.keyDown(self.mapping.button['AZUL'])
            else:
                # logging.info("KEYUP AMARELO")
                pyautogui.keyUp(self.mapping.button['AZUL'])
            

        

        
    #         # Aguarda até receber o caractere de início 'S'
    #     while self.incoming == b'S':
    #         self.incoming = self.ser.read()
    #         logging.debug("Received INCOMING: {}".format(self.incoming))

    # # Lê a parte do valor ADC do pacote até encontrar 'Z'
    #     adc_value_str = self.ser.read_until(b'Z')[:-1]  # Lê até 'Z' e remove 'Z' do final
    #     try:
    #         adc_value = float(adc_value_str)  # Converte para float
    #         volume_level = self.normalize_volume(adc_value)  # Normaliza o valor
    #         self.volume.SetMasterVolumeLevel(volume_level, None)  # Ajusta o volume
    #         logging.debug("Volume ajustado para: {}".format(volume_level))
    #     except ValueError:
    #         logging.error("Invalid ADC value")

    #     # Lê o estado dos botões
    #     button_data = self.ser.read(2)  # Lê dois bytes para o estado dos botões
    #     logging.debug("Received Button DATA: {}".format(button_data))

    #     # Processa o estado dos botões
    #     data = ord(button_data[0]) 
            
    #     ## Sync protocolfv z
    #     while self.incoming != b'X':
    #         self.incoming = self.ser.read()
    #         logging.debug("Received INCOMING: {}".format(self.incoming))

    #     data = self.ser.read()
    #     logging.debug("Received DATA: {}".format(data))

    #     # Aqui devemos usar o método ord() para converter o byte para um inteiro
    #     data = ord(data)

    #     if data & 0b00000001:  # Botão VERDE
    #         logging.info("KEYDOWN VERDE")
    #         pyautogui.keyDown(self.mapping.button['VERDE'])
    #     else:
    #         logging.info("KEYUP VERDE")
    #         pyautogui.keyUp(self.mapping.button['VERDE'])

    #     if data & 0b00000010:  # Botão VERMELHO
    #         logging.info("KEYDOWN VERMELHO")
    #         pyautogui.keyDown(self.mapping.button['VERMELHO'])
    #     else:
    #         logging.info("KEYUP VERMELHO")
    #         pyautogui.keyUp(self.mapping.button['VERMELHO'])

    #     if data & 0b00000100:
    #         logging.info("KEYDOWN AMARELO")
    #         pyautogui.keyDown(self.mapping.button['AMARELO'])
    #     else:
    #         logging.info("KEYUP AMARELO")
    #         pyautogui.keyUp(self.mapping.button['AMARELO'])

    #     if data & 0b00001000:
    #         logging.info("KEYDOWN AZUL")
    #         pyautogui.keyDown(self.mapping.button['AZUL'])
    #     else:
    #         logging.info("KEYUP AZUL")
    #         pyautogui.keyUp(self.mapping.button['AZUL'])

    #     if data &0b00001010:
    #         logging.info("KEYDOWN POWER")
    #         pyautogui.keyDown(self.mapping.button['POWER'])
    #     else:
    #         logging.info("KEYUP POWER")
    #         pyautogui.keyUp(self.mapping.button['POWER'])

    #     eof = self.ser.read()
    #     if eof != b'X':
    #         logging.error("EOF not found where expected")

    #     self.incoming = None

    def normalize_volume(self, adc_value):
        # Supondo que adc_value esteja no intervalo de 0.0 a 1.0
        # Adapte esses valores conforme necessário
        min_volume = -65.25  # Mínimo volume para pycaw
        max_volume = 0       # Máximo volume para pycaw
        return min_volume + (max_volume - min_volume) * adc_value


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
    devices = AudioUtilities.GetSpeakers()
    interface = devices.Activate(IAudioEndpointVolume._iid_, CLSCTX_ALL, None)
    volume = cast(interface, POINTER(IAudioEndpointVolume))

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
