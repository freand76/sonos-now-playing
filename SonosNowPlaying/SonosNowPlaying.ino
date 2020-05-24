/* Include used libraries */
#include "WiFi.h"
#include "MCUFRIEND_kbv.h"

/* Here we have the wifi credentials */
#include "credentials.h"

MCUFRIEND_kbv tft;
WiFiServer server(80);

#define BUFFER_H_SIZE 320
#define BUFFER_V_SIZE 100

static uint8_t bitmapMemory[BUFFER_H_SIZE*BUFFER_V_SIZE*2];

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
    tft.fillScreen(GREEN);

    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    tft.fillScreen(BLACK);
    // Print local IP address and start web server
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    server.begin();
}

void loop(void)
{
  WiFiClient client = server.available();
  if (client)
    {
      Serial.println("Connected");
      size_t bitmapPosition = 0;
      size_t recBytes = 0;
      size_t yPos = 80;
      memset(bitmapMemory, 0, sizeof(bitmapMemory));
      while(client.connected())
	{
	  if (client.available())
	    {
	      uint8_t read = client.read();
	      bitmapMemory[bitmapPosition] = read;
	      bitmapPosition++;
	      recBytes++;
	      if ((bitmapPosition % 100) == 0)
		{
		  Serial.print(".");
		}

	      if (bitmapPosition >= sizeof(bitmapMemory))
		{
		  Serial.println("Done");
		  tft.drawRGBBitmap(0, yPos, (uint16_t*)bitmapMemory, BUFFER_H_SIZE, BUFFER_V_SIZE);
		  yPos += BUFFER_V_SIZE;
		  memset(bitmapMemory, 0, sizeof(bitmapMemory));;
		  bitmapPosition = 0;
		}
	      if (recBytes == (320*320*2))
		{
		  break;
		}
	    }
	}
      client.stop();
      Serial.println("Redraw");
      tft.drawRGBBitmap(0, yPos, (uint16_t*)bitmapMemory, BUFFER_H_SIZE, BUFFER_V_SIZE);
    }
    else
    {

    }

}
