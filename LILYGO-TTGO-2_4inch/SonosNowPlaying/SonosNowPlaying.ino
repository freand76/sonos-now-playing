/* Include used libraries */
#include "WiFi.h"
#include "ESPmDNS.h"
#include <TFT_eSPI.h>
#include <SPI.h>

#include "ready_image.h"

/* Here we have the wifi credentials and network name */
#include "network.h"

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library
WiFiServer server(80);

/* HEADER_DATA pkg_length, x, y, width, height */
#define HEADER_LENGTH (1 + 1 + 1 + 1 + 1)
#define BUFFER_V_SIZE 120
#define BUFFER_H_SIZE 320

static uint16_t dataBuffer16[HEADER_LENGTH + BUFFER_V_SIZE*BUFFER_H_SIZE];

#define BLACK   0x0000

void setup(void)
{
    Serial.begin(115200);
    Serial.println("Hello World!!!");
    tft.init();
    tft.setSwapBytes(true);
    tft.invertDisplay(false);
    tft.fillScreen(BLACK);
    tft.setRotation(1);

    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.setHostname(device_name);
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    tft.pushImage(0, 0, IMAGE_X_SIZE, IMAGE_Y_SIZE, image_data);

    // Print local IP address and start web server
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println(WiFi.getHostname());
    server.begin();
}

WiFiClient client;
uint32_t readPos = 0;

void loop(void)
{
    if (!client)
    {
        client = server.available();
        if (client)
	{
            Serial.println("Connected");
            readPos = 0;
            memset(dataBuffer16, 0, sizeof(dataBuffer16));
	}
    }
    else
    {
        uint8_t* dataBuffer8 = (uint8_t*)dataBuffer16;
        if (client.connected())
	{
            if (client.available())
	    {
                int readBytes = client.read(&dataBuffer8[readPos], sizeof(dataBuffer16) - readPos);
                if (readBytes > 0)
		{
                    readPos += readBytes;
                    if (dataBuffer16[0] == (readPos/2))
		    {
                        if ((dataBuffer16[3] == 0) && (dataBuffer16[4] == 0))
			{
                            tft.fillScreen(BLACK);
			}
                        else
			{
			  
                            tft.pushImage(dataBuffer16[1], dataBuffer16[2], dataBuffer16[3], dataBuffer16[4], &dataBuffer16[5]);
			}
                        readPos = 0;
                        memset(dataBuffer16, 0, sizeof(dataBuffer16));
                        client.write("ACK!");
		    }
                    else
		    {
                        if (readPos == sizeof(dataBuffer16))
			{
                            Serial.println("Protocol error (full buffer), stop!");
                            Serial.println(readPos);
                            client.stop();
                            readPos = 0;
			}
                        else if ((readPos/2) > dataBuffer16[0])
			{
                            Serial.println("Protocol error (pkg error), stop!");
                            Serial.println(dataBuffer16[0]);
                            Serial.println(readPos);
                            client.stop();
                            readPos = 0;
			}
		    }
		}
	    }
	}
        else
	{
            Serial.println("Connection lost");
            client.stop();
            readPos = 0;
	}
    }
}

