/* Include used libraries */
#include "WiFi.h"
#include "ESPmDNS.h"
#include "MCUFRIEND_kbv.h"

/* Here we have the wifi credentials and network name */
#include "network.h"

#include "ready_image.h"

MCUFRIEND_kbv tft;
WiFiServer server(80);

/* HEADER_DATA pkg_length, row_index, col_offset, col_length */
#define HEADER_LENGTH (1 + 1 + 1 + 1)
#define BUFFER_H_SIZE 480


static uint16_t dataBuffer16[HEADER_LENGTH + BUFFER_H_SIZE];

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GREY    0x8410
#define ORANGE  0xE880

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

void loop(void)
{
  WiFiClient client = server.available();
  if (client)
    {
      Serial.println("Connected");
      size_t readPos = 0;
      uint8_t* dataBuffer8 = (uint8_t*)dataBuffer16;
      memset(dataBuffer16, 0, sizeof(dataBuffer16));
      while(client.connected())
	{
	  if (client.available())
	    {
	      int readBytes = client.read(&dataBuffer8[readPos], sizeof(dataBuffer16) - readPos);
	      if (readBytes > 0)
		{
		  readPos += readBytes;
		  
		  if (dataBuffer16[0] == readPos)
		    {
		      if (dataBuffer16[3] == 0)
			{
			  tft.fillScreen(BLACK);
			}
		      else
			{
			  
			  tft.drawRGBBitmap(0, dataBuffer16[1], &dataBuffer16[4], dataBuffer16[3], 1);
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
			  client.stop();
			  continue;
			}
		      else if (readPos > dataBuffer16[0])
			{
			  Serial.println("Protocol error (pkg error), stop!");
			  client.stop();
			  continue;
			}
		    }
		}
	    }
	}
      client.stop();
    }
    else
    {

    }

}
