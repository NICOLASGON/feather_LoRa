#include <SPI.h>
#include <RH_RF95.h>
 
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3
 
#define RF95_FREQ 868.0
 
RH_RF95 rf95(RFM95_CS, RFM95_INT);

int16_t packetnum = 0;
uint8_t current_buffer[RH_RF95_MAX_MESSAGE_LEN];
uint8_t current_buffer_len = sizeof(current_buffer);

#define SERIAL_BUFFER_MAXSIZE 100
uint8_t serial_buffer_in[SERIAL_BUFFER_MAXSIZE];
uint8_t serial_buffer_out[SERIAL_BUFFER_MAXSIZE];
uint8_t serial_buffer_index = 0;

uint8_t str_to_uint8(uint8_t *str)
{
    uint8_t high = *str, low = *(str+1);
    
    if (low >= '0' && low <= '9') low = low - '0';
    else if (low >= 'a' && low <='f') low = low - 'a' + 10;
    else if (low >= 'A' && low <='F') low = low - 'A' + 10;    
   
    if (high >= '0' && high <= '9') high = high - '0';
    else if (high >= 'a' && high <='f') high = high - 'a' + 10;
    else if (high >= 'A' && high <='F') high = high - 'A' + 10;    
    
    return (high << 4) | (low & 0xF); 
}

void setup() 
{
    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);
 
    while(!Serial);
    Serial.begin(115200);
    delay(100);
 
    digitalWrite(RFM95_RST, LOW);
    delay(10);
    digitalWrite(RFM95_RST, HIGH);
    delay(10);
 
    if(!rf95.init())
        while (1);
 
    if(!rf95.setFrequency(RF95_FREQ))
        while (1);
  
    rf95.setTxPower(23, false);
}

void parse_serial_line(void)
{
    uint8_t *pos = NULL;
    uint16_t i = 0;
    float freq = 0.0;
    uint16_t power = 0;

    switch(serial_buffer_in[0])
    {
        case 'S':
            pos = &serial_buffer_in[2];

            while(*pos != '\0')
            {
                current_buffer[i++] = str_to_uint8(pos);
                pos += 2;
            }
            rf95.send(current_buffer, i);
            rf95.waitPacketSent();
            break;
        case 'F':
            freq = strtof((char *)&serial_buffer_in[2], NULL);
            rf95.setFrequency(freq);
            break;
        case 'P':
            sscanf((char *)&serial_buffer_in[2], "%hu", &power);
            rf95.setTxPower(power, false);
            break;
    }
}

void loop()
{
    if( Serial.available() > 0 )
    {
        serial_buffer_in[serial_buffer_index] = Serial.read(); 
        
        if( serial_buffer_in[serial_buffer_index] == '\r' ||
            serial_buffer_in[serial_buffer_index] == '\n')
        {
            serial_buffer_in[serial_buffer_index] = '\0';
            parse_serial_line();
            serial_buffer_index = 0;
            serial_buffer_in[serial_buffer_index] = '\0';
        }
        else
        {
        
            if( serial_buffer_index == (SERIAL_BUFFER_MAXSIZE-1) )
            {
                serial_buffer_index = 0;
            }
            else
            {
                serial_buffer_index++;
            }
        }
    }

 
    if(rf95.available() > 0)
    { 
        current_buffer_len = sizeof(current_buffer);
        rf95.recv(current_buffer, &current_buffer_len);
        serial_buffer_out[0] = '\0';
        sprintf((char *)serial_buffer_out, "R ");
        size_t i = 0;
        uint8_t hex_char[2];
        for(i=0; i<current_buffer_len; i++)
        {
            sprintf((char *)hex_char, "%02x", current_buffer[i]);
            strncat((char *)serial_buffer_out, (char *)hex_char, SERIAL_BUFFER_MAXSIZE);
        }
        Serial.println((char *)serial_buffer_out);
    }
}
