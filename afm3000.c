#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>

#define OFFSET 32000.0
#define FLOW_COEFFICIENT 140.0
#define POLYNOMIAL  0x131
char flow_cmd[2] = {0x10, 0x00};
char DeviceID_cmd[2] = {0x31,0xAE};
char reset_cmd[2] = {0x20,0x00};
char Afmdata[3] = {0};
float Afm_output = 0;
float flowData;
int file;
char *bus = "/dev/i2c-1"; ////// I2c line address


int start_bus()
{
        if((file = open(bus, O_RDWR)) < 0)
        {
                printf("Failed to open the bus. \n");
        //      exit(1);
         //       return FILE_OPEN_ERROR_AMS;
        }
        return 0;
}



int crc_check(uint8_t data[], uint8_t nbrOfBytes, uint8_t checksum){
//==============================================================================
        uint8_t bit;     // bit mask
        uint8_t crc = 0; // calculated checksum
        uint8_t byteCtr; // byte counter

        // calculates 8-Bit checksum with given polynomial
        for(int byteCtr = 0; byteCtr < nbrOfBytes; byteCtr++)
        {
                crc ^= (data[byteCtr]);
                for(bit = 8; bit > 0; --bit)
                {
                          if(crc & 0x80)
                                  crc = (crc << 1) ^ POLYNOMIAL;
                          else
                                  crc = (crc << 1);
                }
        }


        // verify checksum
        if(crc != checksum)
                return 0;
        else
                return 1;

}

float flow()
{

                        ioctl(file, I2C_SLAVE, 0x50);

                        // Sending Flow Collection command
                        int maxRetries = 4;
                        if (write(file, flow_cmd, sizeof(flow_cmd)) != sizeof(flow_cmd))
                        {
				printf("Error: Input/Output error\n");
                        }
                        while(maxRetries--)
                        {
                                // Reading the 2-byte Flow data
                                if (read(file, Afmdata, sizeof(Afmdata)) != sizeof(Afmdata))
                                {
                                        printf("Error: Input/Output error\n");

                                }
                                if(crc_check(Afmdata,2,Afmdata[2]))
                                {
                                        flowData = (Afmdata[0] << 8) | Afmdata[1];
                                        Afm_output = ((float)(flowData - OFFSET)/FLOW_COEFFICIENT);
                                        printf("flow  %f \n",Afm_output);
                                        break;
                                }
                                usleep(200);
                        }




}
int main()
{
        start_bus();
	while(1)
	{
		flow();
		usleep(50000);
	}
}
