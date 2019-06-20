#include <stdlib.h>
#include <stdio.h>
#include ".\include\7186e.h"
#include ".\include\xc100.h"

#define  __DEBUG     0

typedef unsigned char uint8_t;
typedef unsigned int uint16_t;

#define RS232_COMM_PORT    1
#define SERIAL_TIMEOUT     10 //ms
#define RS232_BAUDRATE     38400
#define CAN_BAUDRATE       250000UL

struct comm_mapping {
   char *serial_cmd;
   unsigned long can_id;
};

const struct comm_mapping comm_map_table[] = {
   {"AYC1\n", 0x0101}, //channel 1 current data 
   {"AYX1\n", 0x0102}, //channel 1 maximum data
   {"AYN1\n", 0x0103}, //channel 1 minimum data
   {"AYH1\n", 0x0104}, //channel 1 held data 
   {"AYT1\n", 0x0105}, //channel 1 tare data
   {"AYC2\n", 0x0201}, //channel 2 current data 
   {"AYX2\n", 0x0202}, //channel 2 maximum data
   {"AYN2\n", 0x0203}, //channel 2 minimum data
   {"AYH2\n", 0x0204}, //channel 2 held data 
   {"AYT2\n", 0x0205}, //channel 2 tare data
   {"AYC3\n", 0x0301}, //channel 3 current data 
   {"AYX3\n", 0x0302}, //channel 3 maximum data
   {"AYN3\n", 0x0303}, //channel 3 minimum data
   {"AYH3\n", 0x0304}, //channel 3 held data 
   {"AYT3\n", 0x0305}, //channel 3 tare data
};   

uint8_t can_tx_flag = 0, serial_rx_flag = 0;

#define SERIAL_BUFF_LENGTH    100
char serial_buff[SERIAL_BUFF_LENGTH];

uint8_t can_buff[8] = {0};

/*If the UserCANInt() function is not used, please don't remove it.*/		  
void UserCANInt(char CANInt)
{
   if (CANInt == 0x02) {
      if (!can_tx_flag) {
         can_tx_flag = 1;
         L1On();
      }   
   }   
}

void timer_handler()
{
   static uint8_t run_flag = 0, can_tx_counter = 0, run_counter = 0,
         serial_rx_counter = 0;
   
   if (run_counter++ == 50) {   
      run_counter = 0;
      if (!run_flag) 
         LedOn();
      else 
         LedOff();   
   
      run_flag ^= 1;
   }   
   
   if (can_tx_flag) {
      can_tx_counter++;
      if (can_tx_counter == 5) {
         L1Off();
      } 
      if (can_tx_counter == 10) {
         can_tx_counter = 0;
         can_tx_flag = 0;
      }         
   } 
   
   if (serial_rx_flag) {
      serial_rx_counter++;
      if (serial_rx_counter == 5) {
         L2Off();
      }   
      if (serial_rx_counter == 10) {
         serial_rx_counter = 0;
         serial_rx_flag = 0;
      }   
   }   
}

int serial_comm(char *cmd, char *response, int len)
{
   long start_ticks, current_ticks;
   uint8_t s_data, buff_idx = 0;
   
   printCom(RS232_COMM_PORT, cmd);
  
   start_ticks = GetTimeTicks();
   while (1) {
      current_ticks = GetTimeTicks();
      if (current_ticks - start_ticks > SERIAL_TIMEOUT) 
         return -1;
      
      if (buff_idx == len-1)
         return -1;
      
      if (IsCom(RS232_COMM_PORT)) {
         s_data = ReadCom(RS232_COMM_PORT);
         response[buff_idx++] = s_data;
         if (s_data == '\r' || s_data == '\n') {
            response[buff_idx] = '\0';
            break;
         }      
      }   
   }

   return buff_idx + 1;   
}
   
void main(void)
{
   int ret, i, j;
   uint8_t can_status, overflow_status;
         
   InitLib();
   
   EnableWDT();
   
   Init5DigitLed();
   Disable5DigitLed();
   
   L1Off();
   L2Off();
   L3Off();   
   
   if (InstallCom(RS232_COMM_PORT, RS232_BAUDRATE, 8, 0, 1) != 0) {
      Print("Fail to initialize serial port %d\n", RS232_COMM_PORT);
      goto _exit1;
   }  
   
   ClearCom(RS232_COMM_PORT);
   ClearTxBuffer(RS232_COMM_PORT);
   	
   ret=XC100Init(1, 0x03, CAN_BAUDRATE, 0, 0, 0x00000000UL, 0xffffffffUL);
   if (ret != CAN_NoError) {
      Print("Fail to initialize CAN controller, program exit\n");
      goto _exit2;
   }   
   
   CAN_CreateBuffer(0, 1000);   
   CAN_CreateBuffer(1, 1000);   
   CAN_InstallIrq();
   
   SetUserTimer(0);
   SetDelayTimer(1);
   InstallUserTimerFunction_ms(10, timer_handler);
      
   while (1) {
      GetStatus(&can_status, &overflow_status);
      if (can_status&0x80) {
         Print("CAN bus off detected\n");
         CAN_BusOff_Recovery();
      }   
      
      for (i = 0; i < sizeof(comm_map_table)/sizeof(comm_map_table[0]); i++) {
#if __DEBUG         
         Print("Send serial cmd: %s", comm_map_table[i].serial_cmd);
#endif         
         ret = serial_comm(comm_map_table[i].serial_cmd, serial_buff, 
                     SERIAL_BUFF_LENGTH);
         if (ret == -1) {
#if __DEBUG            
            Print("Command %s   -- Timeout\n", comm_map_table[i].serial_cmd);
#endif            
            DelayMs(10);
            continue;
         }  
         
#if __DEBUG         
         Print("  -- %s\n", serial_buff);
#endif         
         if (!serial_rx_flag) {
            serial_rx_flag = 1;
            L2On();
         }   
         
         sscanf(serial_buff, "%02X%02X%02X%02X,%02X%02X", 
               can_buff, can_buff+1, can_buff+2, can_buff+3,
               can_buff+4, can_buff+5);
         
         SendCANMsg(1, comm_map_table[i].can_id, 0, 6, can_buff);       
      }     
       
      RefreshWDT();        
      
	   if (Kbhit()) { 
         /*if press any key, exit the program*/
         break;
	   }
   }
 
   StopUserTimerFun();
    
   L1Off();
   L2Off();
   L3Off();
   LedOff();
    
   CAN_Restore();
_exit2:
   RestoreCom(RS232_COMM_PORT);   
_exit1:   
   Enable5DigitLed();
   DisableWDT();
   
   Print("Exit this program!\n");
}
