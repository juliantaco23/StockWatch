from machine import Pin, SoftI2C
from hx711 import *
from pico_i2c_lcd import I2cLcd
import time

class Balanza:
    def __init__(self, pin_sck=20, pin_dt=21):
        # Inicializar HX711
        self.hx = hx711(Pin(pin_sck), Pin(pin_dt))
        self.factor_calibracion = 1
        self.offset = 0
        
        # Inicializar LCD
        self.i2c = SoftI2C(sda=Pin(4), scl=Pin(5), freq=400000)
        self.lcd = I2cLcd(self.i2c, 0x27, 2, 16)
        self.lcd.clear()
        
        # Configuración inicial
        self.mostrar_mensaje("Iniciando", "sensor...")
        self.hx.set_power(hx711.power.pwr_up)
        time.sleep(0.5)
        self.hx.set_gain(hx711.gain.gain_128)
        self.hx.set_power(hx711.power.pwr_down)
        hx711.wait_power_down()
        self.hx.set_power(hx711.power.pwr_up)
        hx711.wait_settle(hx711.rate.rate_10)
        self.mostrar_mensaje("Sensor iniciado", "correctamente")
        time.sleep(2)

    def mostrar_mensaje(self, linea1="", linea2=""):
        """Muestra un mensaje en el LCD"""
        self.lcd.clear()
        if linea1:
            self.lcd.move_to(0, 0)
            self.lcd.putstr(linea1[:16])  # Limitar a 16 caracteres
        if linea2:
            self.lcd.move_to(0, 1)
            self.lcd.putstr(linea2[:16])  # Limitar a 16 caracteres

    def obtener_lectura(self):
        """Obtiene una lectura válida del sensor"""
        intentos = 0
        while intentos < 5:
            if val := self.hx.get_value_timeout(250000):
                return float(val)
            intentos += 1
            time.sleep_ms(100)
        return None

    def obtener_promedio(self, muestras=10):
        """Obtiene un promedio de varias lecturas"""
        valores = []
        intentos = 0
        while len(valores) < muestras and intentos < muestras * 2:
            if val := self.obtener_lectura():
                valores.append(val)
            intentos += 1
            time.sleep_ms(100)

        if not valores:
            self.mostrar_mensaje("Error:", "No hay lecturas")
            return None

        if len(valores) > 3:
            valores.remove(max(valores))
            valores.remove(min(valores))

        return sum(valores) / len(valores)

    def tara(self):
        """Establece el punto cero"""
        self.mostrar_mensaje("Estableciendo", "tara...")
        time.sleep(1)

        offset = self.obtener_promedio(20)
        if offset is not None:
            self.offset = offset
            self.mostrar_mensaje("Tara establecida", "correctamente")
            time.sleep(2)
            return True
        else:
            self.mostrar_mensaje("Error al", "establecer tara")
            time.sleep(2)
            return False

    def calibrar(self):
        """Proceso de calibración"""
        self.mostrar_mensaje("=== CALIBRACION ===", "Retire peso")
        input("Presione Enter...")

        if not self.tara():
            return False

        self.mostrar_mensaje("Coloque peso", "conocido")
        peso_conocido = float(input("Peso en gramos: "))

        self.mostrar_mensaje("Tomando lecturas", "espere...")
        time.sleep(2)

        lectura_peso = self.obtener_promedio(20)
        if lectura_peso is None:
            self.mostrar_mensaje("Error en", "calibracion")
            return False

        lectura_peso_tara = lectura_peso - self.offset
        if lectura_peso_tara == 0:
            self.mostrar_mensaje("Error:", "Lectura = tara")
            return False

        self.factor_calibracion = lectura_peso_tara / peso_conocido
        self.mostrar_mensaje("Calibracion OK", f"Factor: {self.factor_calibracion:.2f}")
        time.sleep(2)

        self.mostrar_mensaje("Probando", "calibracion...")
        for _ in range(5):
            peso = self.obtener_peso()
            if peso is not None:
                self.mostrar_mensaje("Peso:", f"{peso:.1f}g")
            else:
                self.mostrar_mensaje("Error en", "lectura")
            time.sleep(1)

        return True

    def obtener_peso(self):
        """Obtiene el peso actual en gramos"""
        lectura = self.obtener_lectura()
        if lectura is None:
            return None
        return (lectura - self.offset) / self.factor_calibracion

def medir_continuamente():
    try:
        balanza = Balanza()

        if input("¿Calibrar? (s/n): ").lower() == 's':
            if not balanza.calibrar():
                balanza.mostrar_mensaje("Fallo calibrac.", "Revisar conexion")
                return
        else:
            if not balanza.tara():
                balanza.mostrar_mensaje("Error tara", "Revisar conexion")
                return

        balanza.mostrar_mensaje("=== MIDIENDO ===", "Ctrl+C = Salir")
        time.sleep(2)

        while True:
            peso = balanza.obtener_peso()
            if peso is not None:
                balanza.mostrar_mensaje("Peso:", f"{peso:.1f}g")
            else:
                balanza.mostrar_mensaje("Error en", "lectura")
            time.sleep(1)

    except KeyboardInterrupt:
        balanza.mostrar_mensaje("Programa", "terminado")
        time.sleep(2)
        balanza.lcd.clear()
        balanza.lcd.backlight_off()
    except Exception as e:
        balanza.mostrar_mensaje("Error:", str(e)[:16])
        time.sleep(2)
        balanza.lcd.clear()
        balanza.lcd.backlight_off()

if __name__ == '__main__':
    medir_continuamente()
