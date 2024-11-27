#include <WiFi.h>
#include <HTTPClient.h>
// Definimos el pin de salida del sensor
#define UV_PIN 32  // GPIO 32

// Variables para calibración
float voltajeSinUV = 0.77;  // Voltaje base sin luz UV
float VDD = 3.0;  // Voltaje de referencia
int uvIndex;

const char* networkName = "YOUR_NETWORK_NAME";
const char* networkPassword = "YOUR_PASSWORD_NETWORK";

//Endpoint para enviar la data del sensor
const char* serverName = "http://192.168.1.62:3000/api/sensor-data";

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);  // Resolución de 12 bits para ESP32

  WiFi.begin(networkName, networkPassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("Conectando a la red WiFi...: ");
    Serial.println(networkName);
  }
  Serial.println("Conectado a la red WiFi exitosamente!!");
}

// Función para leer el sensor UV y calcular irradiancia e índice UV
void loop() {

  if ((WiFi.status() == WL_CONNECTED)) {
		
		HTTPClient http;
    http.begin(serverName); 
    http.addHeader("Content-Type", "application/json"); 

    int lectura = analogRead(UV_PIN);  // Leemos el valor analógico (0 - 4095)
  
    // Convertimos la lectura en voltaje
	  float voltaje = (lectura / 4095.0) * VDD;
	  
	  // Calibración: restamos el valor base de voltaje
	  float voltajeUV = voltaje - voltajeSinUV;
	  if (voltajeUV < 0) voltajeUV = 0;  // Aseguramos que no sea negativo
	  
	  // Relacionamos el voltaje con la intensidad de la irradiancia UV (basado en el gráfico de sparkfun)
	  // De acuerdo con el gráfico, se puede estimar que 1V ≈ 3 mW/cm^2S
	  float irradiancia = voltajeUV * 3;  // mW/cm²

	  // Calcular el índice UV (UV index)
	  // Se considera que 0.1 mW/cm² es aproximadamente igual a 1 UV index  
	  uvIndex = (int)(irradiancia / 0.12);
	  
	  // Mostramos los resultados
	  Serial.print("Voltaje: ");
	  Serial.print(voltajeUV, 2);
	  Serial.print(" V, Irradiancia: ");
	  Serial.print(irradiancia, 2);
	  Serial.print(" mW/cm², Índice UV: ");
	  Serial.println(uvIndex);  // Imprimir UV index en entero

    // Crear el payload JSON con los datos del sensor
	  String payload = "{\"uvIndex\": " + String(uvIndex) + "}";
	   
	  int httpResponseCode = http.POST(payload);
    Serial.println(httpResponseCode);

	  if (httpResponseCode > 0) {
	    String response = http.getString();
	    Serial.println(httpResponseCode);
	    Serial.println(response);
	  } else {
	    Serial.println("Error en la solicitud HTTP");
	   }

	  http.end(); 
	}
  //El método se repite cada minuto
  delay(60000);
}
