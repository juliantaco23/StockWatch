# StockWatch
Lider: Julian Tamara Cordoba 
Integrantes:
-	Julian Tamara Cordoba 
-	Gilber Camilo Agaton Samboni


Sistema de Inventario Basado en Peso
Título: Sistema de Monitoreo de Inventario en Tiempo Real Basado en Peso con Raspberry Pi.

Descripción: Este proyecto propone desarrollar un sistema de monitoreo de inventario en tiempo real utilizando la Raspberry Pi Pico como unidad de control principal. El sistema empleará sensores de peso para rastrear continuamente los niveles de inventario en estanterías o contenedores, proporcionando actualizaciones instantáneas y alertas de reabastecimiento.

Requisitos Funcionales: 
-	Medir el peso de los productos en tiempo real con una precisión de ±100g.
-	Actualizar automáticamente el inventario basado en cambios de peso.
-	Generar alertas cuando los niveles de inventario caigan por debajo de umbrales predefinidos.
-	Transmitir datos a un servicio en la nube para análisis y almacenamiento.
-	Proporcionar una interfaz web para visualización de datos y gestión de inventario.
-	Sistema de alarma, cuando pase cierto tiempo o cuando se detecte un peso debajo del umbral
-	Las alertas se muestran a traves de un LCD
-	Leds indicadores del estado del sistema (activo o inactivo)
-	Los pesos se muestran en un LCD diferente
-	El sistema arroja una alerta si la temperatura para un producto, esta por debajo o por encima de valores predefinidos
-	Para activar el sistema se debe ingresar un codigo predefinido en una matriz numérica

Requisitos No Funcionales:

-	Precisión: Detección de cambios de peso con un margen de error no superior a ±100g.
-	Fiabilidad: Funcionamiento continuo sin fallos durante al menos 2 horas.
-	Latencia: Actualizaciones de inventario en menos de 10 segundos tras un cambio de peso.
-	Escalabilidad: Capacidad para manejar hasta 4 unidades de almacenamiento simultáneamente.
-	Seguridad: Implementa control de acceso para evitar revelar la informacion a terceros




Hardware Necesario:
-	2 Raspberry Pi Pico RP2040 W
-	4 celdas de carga (10kg cada una)
-	4 amplificadores HX711 para las celdas de carga
-	Fuente de alimentación estable
-	2 Display LCD 16x2
-	2 Leds 
-	Sensor de temperatura lm35
-	Matriz numérica


Diagrama de la Solución:
General:
 

Inicio del sistema:
 



Adquisición de Datos:
 


Procesamiento, Almacenamiento y analisis:
 

Datos a Medir:
-	Peso total en cada unidad de almacenamiento
-	Cambios de peso en el tiempo
-	Frecuencia de reabastecimiento
-	Tiempo entre reabastecimientos
-	Temperatura de cada unidad de almacenamiento
Dashboard: El dashboard generado incluirá:
1.	Visualización en tiempo real de los niveles de inventario por producto/estantería
2.	Gráficos de tendencias de consumo de productos
Aplicaciones:
-	Tiendas minoristas para gestión automática de inventario
-	Almacenes para monitoreo de stock en tiempo real
-	Industrias manufactureras para control de materias primas
-	Farmacias para seguimiento de medicamentos











