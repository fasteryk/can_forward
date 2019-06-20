/******************************************************/
/*  XC100 head file                                   */
/*  For 7188XB, 7186EX                                */
/*                                                    */
/*  Note: head file version is different to           */
/*        lib file version.                           */
/*                                                    */
/******************************************************/
/*
	version 1.09 
  [30,Sep,2014] by Alan
    1.  Fix CAN interrupt mask problem in CAN ISR. [22,Jul,2014]
    2.  Fix SendCANMsg API can not send message out problem when CAN error happened.
    3.  Add API SendCANMsg_Recovery() for actively recovering CAN Bus Off status when CAN error happened.

	version 1.08
  [20,Apr,2013] by Alan
    1.  Add API SendCANMsg_NonBlock. [09,Oct,2012]
    2.  Add API GetNonBlockTxBufferLockedCount. [09,Oct,2012]
    3.  Add API GetNonBlockTxIncompleteCount. [09,Oct,2012] 
    4.  Add API CAN_BusOff_Recovery. [20,Apr,2013]
    5.  Modify Interrupt Mask, All interrupt Mask -> CAN interrupt Mask. [27,Nov,2013]
    6.  Add API XC100Init_Listen for initializing module on listen mode. [18,Mar,2014]
  
	version 1.07
  [23,Aug,2012] by Alan
    1.  Modify incorrect received timestamp accuracy on Receive Interrupt Mode and Receive Polling Mode.
    2.  Modify auto search CAN baudrate fail problem.
    	

	version 1.06
  [22,Jun,2012] by Alan
    1.  Modify SendCANMsg function for data lost in Tramsit Interrupt Mode.
    	  => "Transmit return no error count" is unequal to Transmit times.    	  

  version 1.05
  [01,Dec,2011] by Randy
    1.  Modify SendCANMsg function for data lost in Transmit Interrupt Enable with 
        Transmit Buffer mode.
  
  version 1.04
  [08,Dec,2010] by Andy
    1.  Add CalcCANBaud function for user to know what's the CAN baudrate
        of CAN BTR parameter.
  
  version 1.03
  [09,Nov,2010] by Andy
    1.  Modify XC100Init() function
        Old method => Use software BaudCalculate method to calculate CAN baudrate
        New method => Use defined CAN BTR0 and BTR1 for CAN baudrate:
                      5K, 10K, 20K, 50K, 100K, 125K, 200K, 250K, 500K, 800K, 1M

    2.  Modify delay time of SendCANMsg() function;
        Old method: wait for 1000 ms when CAN message can not be sent
        New method: add "WaitForCANMsgSending" variable for user to setting
                    default value is 100000 count
                    
    3.  Add a variable "CANAccFilterMode" for CAN Acceptance Filter Mode
        0 => Dual Acceptance Filter option 
        other value => Single Acceptance Filter option (Default) 
    
        The acceptance filter option will take effect when calling XC100Init function 
    
    
*/

#define CAN_NoError                     0
#define CAN_ResetError                  5
#define CAN_ConfigError                 8
#define CAN_SetACRError                 9
#define CAN_SetAMRError                10 
#define CAN_SetBaudRateError           11
#define CAN_InstallIrqFailure          14
#define CAN_RemoveIrqFailure           15
#define CAN_TransmitIncomplete         16
#define CAN_TransmitBufferLocked       17
#define CAN_ReceiveBufferEmpty         18
#define CAN_DataOverrun                19
#define CAN_ReceiveError               20
#define CAN_SoftBufferIsFull           21
#define CAN_SoftBufferIsEmpty          22
#define CAN_BaudNotSupport             23
#define CAN_DataLengthError            24
#define CAN_NotEnoughMemory            25
#define CAN_TypeOf7188Error            26
#define CAN_AIFreqError                40
#define CAN_AICntError                 41
#define CAN_AutoBaudTimeout            50
#define CAN_TxBufferNotZero						 51
#define CAN_BusOff        						 52



#define _TypeOf7188XB 0
#define _TypeOf7188EX 1


/*Wait for sending limit on polling mode*/
extern unsigned long WaitForCANMsgSending;
/*Transmit Retry Limit limit on Sending Non-Block Mode*/
extern unsigned long NonBlockTxIncompleteLimit;
/*Buffer Lock Retry Limit on Sending Non-Block Mode*/
extern unsigned long NonBlockBufferLockLimit;

/*
Variable CANAccFilterMode: 
0 => Dual Acceptance Filter option 
other value => Single Acceptance Filter option (Default)
*/ 
extern unsigned char CANAccFilterMode;

/*************************************************************************************
Function UserCANInt:
    Description:
        This function is used for the interrupt routine defined by users. When users 
        use this function, be careful for the size of program. The longer execution 
        time in the interrupt routine will let the system crash.
    Parameters:
        CANInt: This parameter will be passed by CAN controller, and indicates the 
                interrupt situation of CAN controller.
       
            CANIntMode Value    |         (Hex)	Meaning
        ------------------------+---------------------------------
                0x01	          |  Receive a message successfully
                0x02	          |  Transmit a message successfully
                0x04	          |  Error warring
                0x08	          |  Data Overrun
                0x10	          |  CAN controller wake-up
                0x20	          |  Bus Passive
                0x40	          |  Arbitration Lost
                0x80	          |  Bus Error
         
*************************************************************************************/
extern void UserCANInt(char CANInt);

/*************************************************************************************
CAN_Reset:
    Description:
        Reset the CAN controller.
*************************************************************************************/
void CAN_Reset(void);

/*************************************************************************************
XC100Init:
    Description:
        Initialize the software buffer and XC100 hardware, which includes CAN controller, 
        , L1 LED, L2 LED and L3 LED.
    Parameters:
        TypeOf7188: define which type of 7188 you use. 
          
                value  |      TypeOf7188
             ----------+------------------------
                  0    |  for 7188XB or 7188XBD
                  1    |  for 7188EX or 7188EXD

        IntMode: Set the CAN controller interrupt mode. Each bit of IntMode parameters 
                 indicates different function shown as follows.
                 
  	  	           	     Interrupt Type          |  Value of IntMode
  	  		    -----------------------------------+-------------------        
  	  	           Receive Interrupt Enable      |        0x01
   	    	         Transmit Interrupt Enable     |        0x02
  	       	    Error Warning Interrupt Enable   |        0x04
     	      	  Data Overrun Interrupt Enable    |        0x08
    	    	       Wake-up Interrupt Enable      |        0x10
      	  		  Error Passive Interrupt Enable   |        0x20
  	  	    	Arbitration Lost Interrupt Enable  |        0x40
      	          Bus Error Interrupt Enable     |        0x80
      	              
         	Receive Interrupt Enable: 
        		When a message has been received without errors, the receive 
            interrupt will be triggered.
          Transmit Inttrtupt Enable:
	        	When a message has been successfully transmitted or the transmit 
            buffer is accessible again, the transmit interrupt will be triggered. 
          Error Warning Enable:
            If the error or bus status is set or clear, the error interrupt 
            will be triggered. 
          Data Overrun Interrupt Enable: 
            If a message was lost because there was not enough space for that 
            message in the FIFO (FIFO has 64 bytes), the overrun interrupt 
            will be triggered.
          Wake-up Interrupt Enable:
            When the CAN controller is sleeping and bus activity is 
            detected. The Wake-up interrupt will be triggered.
          Error Passive Interrupt Enable:
            If CAN controller has at least one error counter exceeds
    	      the protocol-defined level of 127 or if the CAN controller
    	      is in the error passive status, the Error Passive Interrupt
    	      will be triggered.
          Arbitration Lost Interrupt Enable:
            When the CAN controller lost the arbitration and becomes a
            receiver. The Arbitration Lost Interrupt will be triggered.
          Bus Error Interrupt Enable: 	
            When the CAN controller detects an error on the CAN bus,
            the Bus Error Interrupt will be triggered.

                
   	  	  	Use one-byte value to implement the interrupt. For example, if error 
   	  	  	and overrun interrupt are needed in the BasicCAN(CAN 2.0A) mode. set 
   	  	  	the IntMode value to 0x18(That is 0x08+0x10.).
   	    CANBaud: Use a long int to set this parameter. For example, if users want to set
                  CAN baud to 125K bps. Use the value 125000UL.
        BT0, BT1: Set the special user-defined baudrate. Users can set arbitrary baud
                  with these parameters. But users need to have the background of SJA1000
                  CAN controller and 82C251 CAN transceiver, and calculate the values of 
                  BT0 and BT1 by themselves (The clock frequency of CAN controller 
                  is 16MHz.).
        AccCode,AccMask: The AccCode is used for deciding what kind of ID the
          CAN controller will accept. The AccMask is used for deciding which
          bit of ID will need to check with AccCode. If the bit of AccMask is set 
          to 0, it means that the bit in the same position of ID need to be checked,
          and the bit value ID need to match the bit of AccCode in the same position.
        
          Single Acceptance Filter option: (CANAccFilterMode != 0)
        
             If the CAN message ID is 11-bit ID:
        	
                       Register         | bits of register |     Filter Target
            ----------------------------+------------------+------------------------
             AccCode[0] and AccMask[0]  |   bit7 ~ bit0    |   bit10 ~ bit3 of ID
             AccCode[1] and AccMask[1]  |   bit7 ~ bit5    |   bit2 ~ bit0 of ID
             AccCode[1] and AccMask[1]  |   bit4           |   RTR
             AccCode[1] and AccMask[1]  |   bit3 ~ bit0    |   No use
             AccCode[2] and AccMask[2]  |   bit7 ~ bit0    |   bit7 ~ bit0 of first byte data
             AccCode[3] and AccMask[3]  |   bit7 ~ bit0    |   bit7 ~ bit0 of second byte data
               
             If the CAN message ID is 29-bit ID:
        	
                       Register         | bits of register |     Filter Target
            ----------------------------+------------------+------------------------
             AccCode[0] and AccMask[0]  |   bit7 ~ bit0    |   bit28 ~ bit21 of ID
             AccCode[1] and AccMask[1]  |   bit7 ~ bit0    |   bit20 ~ bit13 of ID
             AccCode[2] and AccMask[2]  |   bit7 ~ bit0    |   bit12 ~ bit5 of ID
             AccCode[3] and AccMask[3]  |   bit7 ~ bit3    |   bit4 ~ bit0 of ID
             AccCode[3] and AccMask[3]  |   bit2           |   RTR
             AccCode[3] and AccMask[3]  |   bit1 ~ bit0    |   No use
          
          --------------------------------------------------------------------------
          
          Dual Acceptance Filter option: (CANAccFilterMode == 0)
          
             If the CAN message ID is 11-bit ID:
        	
        	   filter 1:
                       Register         | bits of register |     Filter Target
            ----------------------------+------------------+------------------------
             AccCode[0] and AccMask[0]  |   bit7 ~ bit0    |   bit10 ~ bit3 of ID
             AccCode[1] and AccMask[1]  |   bit7 ~ bit5    |   bit2 ~ bit0 of ID
             AccCode[1] and AccMask[1]  |   bit4           |   RTR
             AccCode[1] and AccMask[1]  |   bit3 ~ bit0    |   bit7 ~ bit4 of first byte data
             AccCode[3] and AccMask[3]  |   bit3 ~ bit0    |   bit3 ~ bit0 of first byte data
             
             filter 2:
                       Register         | bits of register |     Filter Target
            ----------------------------+------------------+------------------------
             AccCode[2] and AccMask[2]  |   bit7 ~ bit0    |   bit10 ~ bit3 of ID
             AccCode[3] and AccMask[3]  |   bit7 ~ bit5    |   bit2 ~ bit0 of ID
             AccCode[3] and AccMask[3]  |   bit4           |   RTR
             
               
             If the CAN message ID is 29-bit ID:
             
        	   filter 1:
                       Register         | bits of register |     Filter Target
            ----------------------------+------------------+------------------------
             AccCode[0] and AccMask[0]  |   bit7 ~ bit0    |   bit28 ~ bit21 of ID
             AccCode[1] and AccMask[1]  |   bit7 ~ bit0    |   bit20 ~ bit13 of ID
             
             filter 2:
                       Register         | bits of register |     Filter Target
            ----------------------------+------------------+------------------------             
             AccCode[2] and AccMask[2]  |   bit7 ~ bit0    |   bit28 ~ bit21 of ID
             AccCode[3] and AccMask[3]  |   bit7 ~ bit0    |   bit20 ~ bit13 of ID
          
                    
          Note:1. AccCode[0] means the most significant byte of AccCode and 
                     AccCode[3] means the least significant byte of AccCode.
                  2. AccMask[0] means the most significant byte of AccMask and 
                     AccMask[3] means the least significant byte of AccMask.   
                  3. Bit10 is most significant bit and Bit0 is least significant bit
                  	
	        For example(In the BasicCAN mode):	      
	              AccCode[0]       = 1100 1010  (Binary mode)
   	            AccMask[0]       = 1001 1101  (Binary mode)
   	            bit10~bit3 of ID = x10x xx1x  will be accepted. (x: don't care)
    Return:
    	  CAN_NoError: OK
        CAN_BaudNotSupport: Input baudrate is not support.
        CAN_ResetError: Reseting the CAN controller is fail.
        CAN_ConfigError: Confuguring the CAN controller register is fail.
        CAN_SetACRError: Setting the AccCode register is fail
        CAN_SetAMRError: Setting the AccCode register is fail
        CAN_NotEnoughMemory: Creating a receive buffer for CAN messages is fail.
*************************************************************************************/
int XC100Init(int TypeOf7188,char IntMode,unsigned long CANBaud,
              char BT0, char BT1,unsigned long AccCode,unsigned long AccMask);
              
/*************************************************************************************
XC100Init_Listen:
    Description:
        Initialize the software buffer and XC100 hardware on listen mode, which includes CAN controller, 
        , L1 LED, L2 LED and L3 LED.
    Parameters:
        TypeOf7188: define which type of 7188 you use. 
          
                value  |      TypeOf7188
             ----------+------------------------
                  0    |  for 7188XB or 7188XBD
                  1    |  for 7188EX or 7188EXD

        IntMode: Set the CAN controller interrupt mode. Each bit of IntMode parameters 
                 indicates different function shown as follows.
                 
  	  	           	     Interrupt Type          |  Value of IntMode
  	  		    -----------------------------------+-------------------        
  	  	           Receive Interrupt Enable      |        0x01
   	    	         Transmit Interrupt Enable     |        0x02
  	       	    Error Warning Interrupt Enable   |        0x04
     	      	  Data Overrun Interrupt Enable    |        0x08
    	    	       Wake-up Interrupt Enable      |        0x10
      	  		  Error Passive Interrupt Enable   |        0x20
  	  	    	Arbitration Lost Interrupt Enable  |        0x40
      	          Bus Error Interrupt Enable     |        0x80
      	              
         	Receive Interrupt Enable: 
        		When a message has been received without errors, the receive 
            interrupt will be triggered.
          Transmit Inttrtupt Enable:
	        	When a message has been successfully transmitted or the transmit 
            buffer is accessible again, the transmit interrupt will be triggered. 
          Error Warning Enable:
            If the error or bus status is set or clear, the error interrupt 
            will be triggered. 
          Data Overrun Interrupt Enable: 
            If a message was lost because there was not enough space for that 
            message in the FIFO (FIFO has 64 bytes), the overrun interrupt 
            will be triggered.
          Wake-up Interrupt Enable:
            When the CAN controller is sleeping and bus activity is 
            detected. The Wake-up interrupt will be triggered.
          Error Passive Interrupt Enable:
            If CAN controller has at least one error counter exceeds
    	      the protocol-defined level of 127 or if the CAN controller
    	      is in the error passive status, the Error Passive Interrupt
    	      will be triggered.
          Arbitration Lost Interrupt Enable:
            When the CAN controller lost the arbitration and becomes a
            receiver. The Arbitration Lost Interrupt will be triggered.
          Bus Error Interrupt Enable: 	
            When the CAN controller detects an error on the CAN bus,
            the Bus Error Interrupt will be triggered.

                
   	  	  	Use one-byte value to implement the interrupt. For example, if error 
   	  	  	and overrun interrupt are needed in the BasicCAN(CAN 2.0A) mode. set 
   	  	  	the IntMode value to 0x18(That is 0x08+0x10.).
   	    CANBaud: Use a long int to set this parameter. For example, if users want to set
                  CAN baud to 125K bps. Use the value 125000UL.
        BT0, BT1: Set the special user-defined baudrate. Users can set arbitrary baud
                  with these parameters. But users need to have the background of SJA1000
                  CAN controller and 82C251 CAN transceiver, and calculate the values of 
                  BT0 and BT1 by themselves (The clock frequency of CAN controller 
                  is 16MHz.).
        AccCode,AccMask: The AccCode is used for deciding what kind of ID the
          CAN controller will accept. The AccMask is used for deciding which
          bit of ID will need to check with AccCode. If the bit of AccMask is set 
          to 0, it means that the bit in the same position of ID need to be checked,
          and the bit value ID need to match the bit of AccCode in the same position.
        
          Single Acceptance Filter option: (CANAccFilterMode != 0)
        
             If the CAN message ID is 11-bit ID:
        	
                       Register         | bits of register |     Filter Target
            ----------------------------+------------------+------------------------
             AccCode[0] and AccMask[0]  |   bit7 ~ bit0    |   bit10 ~ bit3 of ID
             AccCode[1] and AccMask[1]  |   bit7 ~ bit5    |   bit2 ~ bit0 of ID
             AccCode[1] and AccMask[1]  |   bit4           |   RTR
             AccCode[1] and AccMask[1]  |   bit3 ~ bit0    |   No use
             AccCode[2] and AccMask[2]  |   bit7 ~ bit0    |   bit7 ~ bit0 of first byte data
             AccCode[3] and AccMask[3]  |   bit7 ~ bit0    |   bit7 ~ bit0 of second byte data
               
             If the CAN message ID is 29-bit ID:
        	
                       Register         | bits of register |     Filter Target
            ----------------------------+------------------+------------------------
             AccCode[0] and AccMask[0]  |   bit7 ~ bit0    |   bit28 ~ bit21 of ID
             AccCode[1] and AccMask[1]  |   bit7 ~ bit0    |   bit20 ~ bit13 of ID
             AccCode[2] and AccMask[2]  |   bit7 ~ bit0    |   bit12 ~ bit5 of ID
             AccCode[3] and AccMask[3]  |   bit7 ~ bit3    |   bit4 ~ bit0 of ID
             AccCode[3] and AccMask[3]  |   bit2           |   RTR
             AccCode[3] and AccMask[3]  |   bit1 ~ bit0    |   No use
          
          --------------------------------------------------------------------------
          
          Dual Acceptance Filter option: (CANAccFilterMode == 0)
          
             If the CAN message ID is 11-bit ID:
        	
        	   filter 1:
                       Register         | bits of register |     Filter Target
            ----------------------------+------------------+------------------------
             AccCode[0] and AccMask[0]  |   bit7 ~ bit0    |   bit10 ~ bit3 of ID
             AccCode[1] and AccMask[1]  |   bit7 ~ bit5    |   bit2 ~ bit0 of ID
             AccCode[1] and AccMask[1]  |   bit4           |   RTR
             AccCode[1] and AccMask[1]  |   bit3 ~ bit0    |   bit7 ~ bit4 of first byte data
             AccCode[3] and AccMask[3]  |   bit3 ~ bit0    |   bit3 ~ bit0 of first byte data
             
             filter 2:
                       Register         | bits of register |     Filter Target
            ----------------------------+------------------+------------------------
             AccCode[2] and AccMask[2]  |   bit7 ~ bit0    |   bit10 ~ bit3 of ID
             AccCode[3] and AccMask[3]  |   bit7 ~ bit5    |   bit2 ~ bit0 of ID
             AccCode[3] and AccMask[3]  |   bit4           |   RTR
             
               
             If the CAN message ID is 29-bit ID:
             
        	   filter 1:
                       Register         | bits of register |     Filter Target
            ----------------------------+------------------+------------------------
             AccCode[0] and AccMask[0]  |   bit7 ~ bit0    |   bit28 ~ bit21 of ID
             AccCode[1] and AccMask[1]  |   bit7 ~ bit0    |   bit20 ~ bit13 of ID
             
             filter 2:
                       Register         | bits of register |     Filter Target
            ----------------------------+------------------+------------------------             
             AccCode[2] and AccMask[2]  |   bit7 ~ bit0    |   bit28 ~ bit21 of ID
             AccCode[3] and AccMask[3]  |   bit7 ~ bit0    |   bit20 ~ bit13 of ID
          
                    
          Note:1. AccCode[0] means the most significant byte of AccCode and 
                     AccCode[3] means the least significant byte of AccCode.
                  2. AccMask[0] means the most significant byte of AccMask and 
                     AccMask[3] means the least significant byte of AccMask.   
                  3. Bit10 is most significant bit and Bit0 is least significant bit
                  	
	        For example(In the BasicCAN mode):	      
	              AccCode[0]       = 1100 1010  (Binary mode)
   	            AccMask[0]       = 1001 1101  (Binary mode)
   	            bit10~bit3 of ID = x10x xx1x  will be accepted. (x: don't care)
    Return:
    	  CAN_NoError: OK
        CAN_BaudNotSupport: Input baudrate is not support.
        CAN_ResetError: Reseting the CAN controller is fail.
        CAN_ConfigError: Confuguring the CAN controller register is fail.
        CAN_SetACRError: Setting the AccCode register is fail
        CAN_SetAMRError: Setting the AccCode register is fail
        CAN_NotEnoughMemory: Creating a receive buffer for CAN messages is fail.
*************************************************************************************/
int XC100Init_Listen(int TypeOf7188,char IntMode,unsigned long CANBaud,
              char BT0, char BT1,unsigned long AccCode,unsigned long AccMask);              
              

/*************************************************************************************
Function SetCANBaud:
    Description:
        This function is used to change the CAN baud after calling XC100init function.
    Parameters:
        CANBaud, BT0, BT1 : Please refer to the parameters description in the 
                            XC100Init function in section 3.1.2.
*************************************************************************************/
int SetCANBaud(unsigned long CANBaud, char BT0, char BT1);

/*************************************************************************************
Function SetCANMask:
    Description:
        This function is used to change the CAN message filter after using XC100init 
        function.
    Parameters:
        AccCode, AccMask : Please refer to the parameters description in the 
                           XC100Init function.
*************************************************************************************/
int SetCANMask(unsigned long AccCode, unsigned long AccMask);

/*************************************************************************************
Function CAN_InstallIrq: 
    Description:
        Set the interrupt function enable. Afterwards, the CPU of I-7188 series 
        embedded controller can reviceive the interrupt signal from CAN controller.
    Return: none
*************************************************************************************/
void CAN_InstallIrq(void);

/*************************************************************************************
Function CAN_RemoveIrq: 
    Description:
        Disable the interrupt function. Afterwards, the CPU of I-7188 series 
        embedded controller can't receive the interrupt signal from CAN controller.
    Return: none
*************************************************************************************/
void CAN_RemoveIrq(void);

/*************************************************************************************
Function CAN_Restore: 
    Description:
        Set the interrupt function disable, release all software buffer, and reset CAN
        chip. This function must be called to release resource before the program is 
        terminated.
    Return: none          
*************************************************************************************/
void CAN_Restore(void);

/*************************************************************************************
Function CAN_CreateBuffer: 
    Description:
	     Call this function for changing the reception and transmission software buffer 
       sizes. If users don't use this function, the default reception and transmission 
       software buffer sizes are both 256 records.
    Parameters:
       BufMode: 0 for changing reception software buffer size.
                others for changing transmission software buffer size.
       BufferSize: the new buffer sizes for software buffer.
    Return:
       CAN_NoError: OK
       CAN_NotEnoughMemory: The buffer size for software buffer will not exceed 65535 bytes.
                            User need to chech the buffer size is over or system don't have
                            enough memory for changing the buffer sizes.
*************************************************************************************/
int CAN_CreateBuffer(int BufMode, unsigned int BufferSize);

/*************************************************************************************
Function SendCANMsg: 
    Description:
	      If the transmit buffer is disable, this function will send a message to the 
        CAN network. However, if the transmit buffer is enable, this function will 
        send all the messages stored in the transmit buffer to the CAN network. 
    Parameters:
        Mode: This parameter is used for CAN ID type
              Mode value  |            Meaning
            --------------+--------------------------------
                  0       |  Send a 11-bit ID CAN message
                other     |  Send a 29-bit ID CAN message

        MsgID: The ID of this CAN message. The ID may be a 11-bit value or 29-bit value.
                        
        RTR: Remote transmit request byte.
     
              RTR value  |                          Meaning
            -------------+--------------------------------------------------------------
                  0      |  This CAN message is not a remote transmit request message.
                  1      |  This CAN message is a remote transmit request message.
          
        DataLen: The pure data length of a CAN message. The range of this value is 0~8.

        *Data: Store the data of CAN message. The numbers of data bytes need to 
               match with the "DataLen".
    Return:
        CAN_NoError: OK
        CAN_DataLengthError: CAN message data length is over 8.
        CAN_TransmitBufferLocked: CAN controller transmit buffer is locked.
        CAN_TransmitIncomplete: CAN controller can't send the message successfully.
*************************************************************************************/
int SendCANMsg(unsigned char Mode,unsigned long MsgID, unsigned char RTR, 
               unsigned char DataLen, unsigned char *Data);    
               
/*************************************************************************************
Function CAN_SendMsg with CAN Bus Off recovery: 
    Description:
	      If the transmit buffer is disable, this function will send a message to the 
        CAN network. However, if the transmit buffer is enable, this function will 
        send all the messages stored in the transmit buffer to the CAN network. 
        
        When CAN Bus Off recovery, this api will actively recover CAN Bus Off status 
        without calling CAN_BusOff_Recovery()
        
    Parameters:
        Mode: This parameter is used for CAN ID type
              Mode value  |            Meaning
            --------------+--------------------------------
                  0       |  Send a 11-bit ID CAN message
                other     |  Send a 29-bit ID CAN message

        MsgID: The ID of this CAN message. The ID may be a 11-bit value or 29-bit value.
                        
        RTR: Remote transmit request byte.
     
              RTR value  |                          Meaning
            -------------+--------------------------------------------------------------
                  0      |  This CAN message is not a remote transmit request message.
                  1      |  This CAN message is a remote transmit request message.
          
        DataLen: The pure data length of a CAN message. The range of this value is 0~8.

        *Data: Store the data of CAN message. The numbers of data bytes need to 
               match with the "DataLen".
    Return:
        CAN_NoError: OK
        CAN_DataLengthError: CAN message data length is over 8.
        CAN_TransmitBufferLocked: CAN controller transmit buffer is locked.
        CAN_TransmitIncomplete: CAN controller can't send the message successfully.
*************************************************************************************/
int SendCANMsg_Recovery(unsigned char Mode,unsigned long MsgID, unsigned char RTR, 
               unsigned char DataLen, unsigned char *Data);           

/*************************************************************************************
Function GetCANMsg: 
    Description:
      	Receive CAN messages from receive buffer or from CAN bus directly. If the 
    		receive interrupt is set to enable in IntMode parameter of XC100Init function.
    		This function will read back the CAN message stored in the software receive 
    		buffer. If the receive interrupt is disable, this function uses the polling 
    		method to check if there is any CAN message in CAN chip buffer. If yes,return 
    		the CAN message.
    Parameters:
        *Mode: This parameter is used for get the ID type (11-bit or 29-bit ID) of 
               a CAN message.
        *MsgID: This is for obtaining the ID of a CAN message.               
        *RTR: This is for obtaining the RTR of a CAN message.
                     
              RTR value  |                          Meaning
            -------------+--------------------------------------------------------------
                  0      |  This CAN message is not a remote transmit request message.
                  1      |  This CAN message is a remote transmit request message.
          
        *DataLen: This is for obtaining the data length of a CAN message.
        *Data: This is for obtaining the Data of a CAN message. The Data buffer size must
               be 8 bytes.
        *UpperTime: Get the time stamp of a CAN message. The time stamp unit is us (micro 
                    second). This parameter only show the upper part of time stamp. 
                             Real time stamp = upper part*0x100000000UL+lower part
        *LowerTime: Get the lower part of time stamp of a CAN message.                             
    Return:
        CAN_NoError: OK
    	  CAN_ReceiveBufferEmpty: No message in the receive buffer.
    	  CAN_DataLengthError: CAN message data length is over 8.
*************************************************************************************/
int GetCANMsg(unsigned char *Mode, unsigned long *MsgID
              , unsigned char *RTR, unsigned char *DataLen, unsigned char *Data
              , unsigned long *UpperTime , unsigned long *LowerTime);

/*************************************************************************************
Function CAN_RxMsgCount: 
    Description:
        If users use CAN_CreateBuffer to create a buffer for received messages, this 
        function will return the message numbers of reception buffer.
    Return: The numbers of messages stored in reception buffer.
*************************************************************************************/
int CAN_RxMsgCount(void);

/*************************************************************************************
Function GetStatus: 
    Description:
        Set the interrupt function enable. Afterwards, the CPU of I-7188 series 
        embedded controller can reviceive the interrupt signal from CAN controller.
    Return: none
*************************************************************************************/
void GetStatus(unsigned char *CANReg, unsigned char *OverflowFlag);

/*************************************************************************************
Function ClearStatus: 
    Description:
        Read the CAN controller status and software buffer overflow flag message.
    Return: none
*************************************************************************************/
void ClearStatus(void);

/*************************************************************************************]
Function L1Off: 
    Description:
	Turn off the first LED
*************************************************************************************/
void L1Off(void);

/*************************************************************************************
Function L1On: 
    Description:
    	Turn on the first LED
*************************************************************************************/
void L1On(void);

/*************************************************************************************
Function L2Off: 
    Description:
    	Turn off the second LED
*************************************************************************************/
void L2Off(void);

/*************************************************************************************
Function L2On: 
    Description:
    	Turn on the second LED
*************************************************************************************/
void L2On(void);

/*************************************************************************************
Function L3Off: 
    Description:
    	Turn off the third LED
*************************************************************************************/
void L3Off(void);

/*************************************************************************************
Function L3On: 
    Description:
    	Turn on the third LED
*************************************************************************************/
void L3On(void);

/*************************************************************************************
Function CAN_SearchBaud: 
    Description:
      CANBaud: Use a long int to set this parameter. For example, if users want to set
               CAN baud to 125K bps. Use the value 125000UL.
      BT0, BT1: Set the special user-defined baudrate. Users can set arbitrary baud
               with these parameters. But users need to have the background of SJA1000
               CAN controller and 82C251 CAN transceiver, and calculate the values of 
               BT0 and BT1 by themselves (The clock frequency of CAN controller 
               is 16MHz.).
      Timeout:
              >0: Use for waiting necessary CAN buadrate
      Return:
        CAN_NoError: OK
        CAN_AutoBaudTimeout: Timeout, not find necessary CAN buadrate
*************************************************************************************/
int CAN_SearchBaud(unsigned long CANBaud, char BT0, char BT1, unsigned int Timeout);

/*************************************************************************************
Function CalcCANBaud: 
    Description:
      BT0, BT1: Set the special user-defined baudrate. Users can set arbitrary baud
               with these parameters. But users need to have the background of SJA1000
               CAN controller and 82C251 CAN transceiver, and calculate the values of 
               BT0 and BT1 by themselves (The clock frequency of CAN controller 
               is 16MHz.).
      Return: CAN Bus baudrate of BTR (bps)
        
*************************************************************************************/
unsigned long CalcCANBaud(unsigned char BT0,unsigned char BT1);

/*************************************************************************************
Function GetXC100LibVer:
    Description:
      Get XC100 Library version.
      
      Return;
        102 means v1.0.2
*************************************************************************************/
int GetXC100LibVer(void); 
               
/*************************************************************************************
Function SendCANMsg with Non Block Mode: 
    Description:
	      If the transmit buffer is disable, this function will send a message to the 
        CAN network. However, if the transmit buffer is enable, this function will 
        send all the messages stored in the transmit buffer to the CAN network. 
    Parameters:
        Mode: This parameter is used for CAN ID type
              Mode value  |            Meaning
            --------------+--------------------------------
                  0       |  Send a 11-bit ID CAN message
                other     |  Send a 29-bit ID CAN message

        MsgID: The ID of this CAN message. The ID may be a 11-bit value or 29-bit value.
                        
        RTR: Remote transmit request byte.
     
              RTR value  |                          Meaning
            -------------+--------------------------------------------------------------
                  0      |  This CAN message is not a remote transmit request message.
                  1      |  This CAN message is a remote transmit request message.
          
        DataLen: The pure data length of a CAN message. The range of this value is 0~8.

        *Data: Store the data of CAN message. The numbers of data bytes need to 
               match with the "DataLen".
    Return:
        CAN_NoError: OK        
        CAN_DataLengthError: CAN message data length is over 8.
        CAN_TransmitBufferLocked: CAN controller transmit buffer is locked.
        CAN_TransmitIncomplete: CAN controller can't send the message successfully.
        CAN_TxBufferNotZero: Use this function on polling mode.
*************************************************************************************/
int SendCANMsg_NonBlock(unsigned char Mode,unsigned long MsgID, unsigned char RTR, 
               unsigned char DataLen, unsigned char *Data);  
                
/*************************************************************************************
Function GetNonBlockTxBufferLockedCount: 
    Description:
	     Get Transmission Buffer Locked Count from this API on Non-Block Mode.
    Parameters:
       
    Return:
       Tx Buffer Lock Error Count on Non-Block Mode
*************************************************************************************/
unsigned long GetNonBlockTxBufferLockedCount(void); 

/*************************************************************************************
Function GetNonBlockTxIncompleteCount: 
    Description:
	     Get Transmission Incomplete Conunt from this API on Non-Block Mode.
    Parameters:
       
    Return:
       Transmission Incomplete Count on Non-Block Mode
*************************************************************************************/
unsigned long GetNonBlockTxIncompleteCount(void);

/*************************************************************************************
Function CAN_BusOff_Recovery: 
    Description:
	     When CAN Bus off, calling this API to recovery CAN Bus.
 
*************************************************************************************/
void CAN_BusOff_Recovery(void); 
void Test(void);