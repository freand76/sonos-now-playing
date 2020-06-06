/* Include used libraries */
#include "WiFi.h"
#include "ESPmDNS.h"
#include "MCUFRIEND_kbv.h"

/* Here we have the wifi credentials and network name */
#include "network.h"

#include "ready_image.h"

MCUFRIEND_kbv tft;
WiFiServer server(80);

/* HEADER_DATA pkg_length, x, y, width, height */
#define HEADER_LENGTH (1 + 1 + 1 + 1 + 1)
#define BUFFER_V_SIZE 80
#define BUFFER_H_SIZE 480


static uint16_t dataBuffer16[HEADER_LENGTH + BUFFER_V_SIZE*BUFFER_H_SIZE];

#define BLACK   0x0000

void setup()
{
    Serial.begin(115200);
    uint16_t ID = tft.readID();

    Serial.print(F("ID = 0x"));
    Serial.println(ID, HEX);

    tft.begin(ID);
    tft.setRotation(3);
    tft.fillScreen(BLACK);

    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.setHostname(device_name);
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    tft.drawRGBBitmap(0, 0, image_data, IMAGE_X_SIZE, IMAGE_Y_SIZE);

    // Print local IP address and start web server
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println(WiFi.getHostname());
    server.begin();
}

WiFiClient client;
size_t readPos = 0;

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
			  
                            tft.drawRGBBitmap(dataBuffer16[1], dataBuffer16[2], &dataBuffer16[5], dataBuffer16[3], dataBuffer16[4]);
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
