from machine import Pin, SoftI2C, UART
from dht import DHT11
from pico_i2c_lcd import I2cLcd
import time

class MonitoringSystem:
    def __init__(self, dht_pin=6, uart_baudrate=115200):
        # Inicializar el sensor DHT11
        self.sensor_dht = DHT11(Pin(dht_pin))
        
        # Inicializar UART
        self.uart = UART(0, baudrate=uart_baudrate, tx=Pin(0), rx=Pin(1))
        
        # Inicializar LCD
        self.i2c = SoftI2C(sda=Pin(4), scl=Pin(5), freq=400000)
        self.lcd = I2cLcd(self.i2c, 0x27, 2, 16)
        self.lcd.clear()
        
        # Umbrales de temperatura
        self.TEMP_HIGH = 30
        self.TEMP_LOW = 20
        
        # Umbrales de peso
        self.WEIGHT_CHANGE_THRESHOLD = 500  # Cambio significativo de 500g
        self.LOW_STOCK_THRESHOLD = 500      # Alerta de reabastecimiento en 500g
        self.PRODUCT_WEIGHT = 1800          # Peso esperado del producto
        self.WEIGHT_TOLERANCE = 50          # Tolerancia para considerar peso normal
        
        # Variables de estado
        self.last_weight = 0
        self.alert_active = False           # Estado de alerta de peso
        self.temp_alert = False             # Estado de alerta de temperatura
        
    def mostrar_mensaje(self, linea1="", linea2=""):
        """Muestra un mensaje en el LCD"""
        self.lcd.clear()
        if linea1:
            self.lcd.move_to(0, 0)
            self.lcd.putstr(linea1[:16])
        if linea2:
            self.lcd.move_to(0, 1)
            self.lcd.putstr(linea2[:16])
            
    def leer_temperatura_y_humedad(self):
        """Lee la temperatura y la humedad del sensor DHT11"""
        try:
            self.sensor_dht.measure()
            return self.sensor_dht.temperature(), self.sensor_dht.humidity()
        except OSError as e:
            print("Error al leer el sensor DHT11:", e)
            return None, None
            
    def check_weight_change(self, current_weight):
        """Verifica si hay un cambio significativo en el peso"""
        return abs(current_weight - self.last_weight) >= self.WEIGHT_CHANGE_THRESHOLD
        
    def check_stock_level(self, weight):
        """Verifica si el stock está bajo"""
        return weight < self.LOW_STOCK_THRESHOLD
        
    def is_normal_weight(self, weight):
        """Verifica si el peso está dentro del rango normal"""
        return abs(weight - self.PRODUCT_WEIGHT) <= self.WEIGHT_TOLERANCE
        
    def procesar_peso(self):
        """Procesa la lectura de peso del UART"""
        if self.uart.any():
            try:
                weight_str = self.uart.readline().decode('utf-8').strip()
                current_weight = float(weight_str)
                
                # Verificar si el peso ha vuelto a la normalidad
                if self.is_normal_weight(current_weight):
                    self.alert_active = False
                    if not self.temp_alert:  # Solo mostrar si no hay alerta de temperatura
                        self.mostrar_mensaje(
                            "Sistema OK",
                            "Peso Normal"
                        )
                
                # Si hay una alerta activa o se detecta un cambio, mostrar alerta
                elif self.alert_active or self.check_weight_change(current_weight):
                    self.alert_active = True
                    if current_weight < self.LOW_STOCK_THRESHOLD:
                        self.mostrar_mensaje(
                            "REABASTECER!",
                            f"Peso: {current_weight}g"
                        )
                    else:
                        self.mostrar_mensaje(
                            "ALERTA CAMBIO!",
                            f"Peso: {current_weight}g"
                        )
                
                self.last_weight = current_weight
                return True
                
            except ValueError as e:
                print(f"Error al convertir el peso: {e}")
                return False
        return False
        
    def monitorear_sistema(self):
        """Función principal que monitorea el peso"""
        print("Sistema iniciado...")
        self.mostrar_mensaje("Sistema listo", "Monitoreando...")
        
        try:
            while True:
                # Procesar peso si hay datos disponibles (prioridad alta)
                if self.uart.any():
                    self.procesar_peso()
                    continue  # Si hay datos de peso, skip temperatura
                
                # Solo procesar temperatura si no hay alertas de peso activas
                if not self.alert_active:
                    temperatura, humedad = self.leer_temperatura_y_humedad()
                    print(f"Temp: {temperatura}C")
                    if temperatura is not None and humedad is not None:
                        if temperatura > self.TEMP_HIGH:
                            self.temp_alert = True
                            self.mostrar_mensaje("ALERTA TEMP:", "Muy alta!")
                        elif temperatura < self.TEMP_LOW:
                            self.temp_alert = True
                            self.mostrar_mensaje("ALERTA TEMP:", "Muy baja!")
                        else:
                            self.temp_alert = False

                
                # Pequeña pausa para no saturar el sistema
                time.sleep(0.1)
                
        except KeyboardInterrupt:
            self.mostrar_mensaje("Programa", "Terminado")
            time.sleep(2)
            self.lcd.clear()
        except Exception as e:
            print(f"Error: {e}")
            self.mostrar_mensaje("Error sistema", "Reiniciando...")
            time.sleep(3)
            machine.reset()

if __name__ == '__main__':
    sistema = MonitoringSystem()
    sistema.monitorear_sistema()
