 
/*-----------------------------------------------

------------------------------------------------*/

#include<reg52.h> //�]�t�Y���A�@�뱡�p���ݭn��ʡA�Y���]�t�S��\��H�s�����w�q 
#include <string.h>                       
#define MAX 10
unsigned char buf[MAX];
unsigned char head = 0;


#define KeyPort P2

#define DataPort P0
sbit LATCH1=P3^2;//�w�q��s�ϯ�ݤf �q��s
sbit LATCH2=P3^3;//                 ����s

unsigned char code dofly_DuanMa[]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,
		                  	         0x77,0x7c,0x39,0x5e,0x79,0x71};// ��ܬq�X��0~F
									 
unsigned char code dofly_WeiMa[]={0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f};//���O�����������ƽX���I�G,�Y��X

unsigned char mode = 1;

unsigned char displayBuf[10];
unsigned char cnt = 0;
unsigned char city_code = 0;

sbit SPK=P1^2;          //�w�q��z���f
unsigned char High,Low; //�w�ɾ��w�˭Ȫ���8��M�C8��
unsigned char code freq[][2]={ 
0x43, 0xF2,		//261Hz 1	C
0xC2, 0xF3,		//293Hz 2
0x18, 0xF5,		//329Hz 3
0xB7, 0xF5,  	//349Hz 4
0xD1, 0xF6,  	//391Hz 5
0xD7, 0xF7,  	//440Hz 6	A
0xBB, 0xF8,  	//494Hz 7
0x22, 0xF9,  	//523Hz 8

0x22, 0xF9,		//523Hz		9
0xE1, 0xF9,		//587Hz		10
0x8C, 0xFA,  	//659Hz 	11
0xDA, 0xFA,  	//698Hz		12
0x6A, 0xFB,  	//784Hz		13
0xEA, 0xFB,  	//880Hz		14
0x5C, 0xFC,  	//987Hz		15
0x90, 0xFC,  	//1046Hz	16

0x90, 0xFC,	 	//1046Hz	17
0xF0, 0xFC,  	//1174Hz 	18
0x46, 0xFD,  	//1318Hz 	19
0x6D, 0xFD,  	//1397Hz 	20
0xB5, 0xFD,  	//1568Hz 	21
0xF5, 0xFD,  	//1760Hz 	22
0x2E, 0xFE,  	//1975Hz 	23
0x48, 0xFE,  	//2093Hz	24

}; 

unsigned char music[64];  // �̦h�x�s 64 ����
unsigned char music_len = 0;
unsigned char music_idx = 0;

unsigned int countdown_sec = 0;
unsigned char alert_cnt = 0;
unsigned char countdown_active = 0;
unsigned int tick_ms = 0;
unsigned char countdown_paused = 0;  // 0=�B�椤, 1=�Ȱ���
/*------------------------------------------------
                   ����n��
------------------------------------------------*/
void SendStr(unsigned char *s);
void SendByte(unsigned char dat);
void delay_ms(unsigned int ms);
void DelayUs2x(unsigned char t);//us�ũ��ɨ���n�� 
void DelayMs(unsigned char t); //ms�ũ���
void InitUART(void);
void UpdateDisplayBuf();
void UpdateDisplay();
unsigned char KeyScan(void);//��L����
unsigned char KeyPro(void);
void Display(unsigned char FirstBit,unsigned char Num);//�ƽX����ܨ��
void Init_Timer0(void);//�w�ɾ���l��
void Init_Timer2(void);//�w�ɾ���l��
unsigned char GetCharCode(char ch);
void CountdownTask();
/*------------------------------------------------
                    ��f��l��
------------------------------------------------*/
void InitUART  (void)
{

    SCON  = 0x50;		        // SCON: �Ҧ� 1, 8-bit UART, �ϯ౵��  
    TMOD |= 0x20;               // TMOD: timer 1, mode 2, 8-bit ����
    TH1   = 0xFD;               // TH1:  ���˭� 9600 �i�S�v ���� 11.0592MHz  
    TR1   = 1;                  // TR1:  timer 1 ���}                         
    EA    = 1;                  //���}�`���_
   // ES    = 1;                  //���}��f���_
}  

       
/*------------------------------------------------
                    �D���
------------------------------------------------*/
void main (void)
{
	unsigned char num;   
	unsigned char i;
	unsigned char spk_num = 0;   
	//unsigned char last_key = 0;
	InitUART();
	Init_Timer0();
	Init_Timer2();
	SPK=0;
	ES    = 1;                  //���}��f���_
	while (1){
		num=KeyPro();
		if(num!=0xff) {
			if(mode == 1){
				switch(num){
					case 0: 	DelayMs(10); SendStr("0\0"); city_code = 1; delay_ms(3000);
								break;  // �x�_
					case 1: 	DelayMs(10); SendStr("1\0"); city_code = 2; delay_ms(3000);  
								break;  // �s�_
					case 2: 	DelayMs(10); SendStr("2\0"); city_code = 3; delay_ms(3000); 
								break;  // ���
					case 3: 	DelayMs(10); SendStr("3\0"); city_code = 4; delay_ms(3000); 
								break;  // �x��
					case 4: 	DelayMs(10); SendStr("4\0"); city_code = 5; delay_ms(3000);
								break;  // �x�n
					case 5: 	DelayMs(10); SendStr("5\0"); city_code = 6; delay_ms(3000); 
								break;  // ����
					case 6: 	DelayMs(10); SendStr("6\0"); city_code = 7; delay_ms(3000); 
								break;  // �̰���
					case 7: 	DelayMs(10); SendStr("7\0");  delay_ms(3000); 
								break;  // �̰���
					default: 	DelayMs(10); SendStr("0\0"); city_code = 1; delay_ms(3000); 
								break;
				}
			}
			else if(mode == 4){
				switch(num){
					case 0: 	DelayMs(10); SendStr("1"); delay_ms(3000);
								break;
					case 1: 	DelayMs(10); SendStr("2"); delay_ms(3000);  
								break;
					case 2: 	DelayMs(10); SendStr("3"); delay_ms(3000); 
								break;
					case 4: 	DelayMs(10); SendStr("4"); delay_ms(3000);
								break;
					case 5: 	DelayMs(10); SendStr("5"); delay_ms(3000); 
								break;
					case 6: 	DelayMs(10); SendStr("6"); delay_ms(3000); 
								break;
					case 8: 	DelayMs(10); SendStr("7"); delay_ms(3000); 
								break;
					case 9: 	DelayMs(10); SendStr("8"); delay_ms(3000); 
								break;
					case 10: 	DelayMs(10); SendStr("9"); delay_ms(3000); 
								break;
					case 12: 	DelayMs(10); SendStr("*"); delay_ms(3000); 
								break;
					case 13: 	DelayMs(10); SendStr("0"); delay_ms(3000); 
								break;
					case 14: 	DelayMs(10); SendStr("#"); delay_ms(3000); 
								break;
					default: 	DelayMs(10); delay_ms(3000); 
								break;
				}
			}
		}
		if(mode == 2 || mode == 3){
			if(spk_num==0){
				TR2=0;
				SPK=0;
			}
		}
		else if(mode == 6){
			CountdownTask();
		}
		if (head == 8) {
			head = 0;
			if(buf[0] =='M'){
				spk_num = 0;
				TR2 = 0;
				SPK = 0;
				music_len = 0;
				music_idx = 0;
				for(i = 0; i < 8; i++){
					displayBuf[i] = GetCharCode(' ');
				}
				if(buf[1]=='1') {
					mode = 1;
					SPK=0;
					continue;
				}
				else if(buf[1]=='2') {
					mode = 2;
					TR2=0;
					SPK=0;
					continue;
				}
				else if(buf[1]=='3') {
					mode = 3;
					music_len = 0;
					music_idx = 0;
					TR2=0;
					SPK=0;
					continue;
				}
				else if(buf[1]=='4') {
					for(i = 0; i < 8; i++){
						displayBuf[i] = GetCharCode(' ');
					}
					mode = 4;
					SPK=0;
					continue;
				}
				else if(buf[1]=='5') {
					mode = 5;
					SPK=0;
					continue;
				}
				else if(buf[1]=='6') {
					mode = 6;
					SPK=0;
					countdown_active = 0;
					countdown_paused = 0;  // 0=�B�椤, 1=�Ȱ���
					// ��s���
					displayBuf[0] = GetCharCode((countdown_sec / 1000) % 10 + '0');
					displayBuf[1] = GetCharCode((countdown_sec / 100) % 10 + '0');
					displayBuf[2] = GetCharCode((countdown_sec / 10) % 10 + '0');
					displayBuf[3] = GetCharCode((countdown_sec % 10) + '0');
					displayBuf[4] = GetCharCode(' ');
					displayBuf[5] = GetCharCode('S');
					displayBuf[6] = GetCharCode('E');
					displayBuf[7] = GetCharCode('C');
					continue;
				}
			}
			if(mode == 1 || mode == 5){
				UpdateDisplay();
			}
			else if(mode == 2){
				if (strncmp(buf, "C4", 2) == 0) spk_num = 1;
				else if (strncmp(buf, "D4", 2) == 0) spk_num = 2;
				else if (strncmp(buf, "E4", 2) == 0) spk_num = 3;
				else if (strncmp(buf, "F4", 2) == 0) spk_num = 4;
				else if (strncmp(buf, "G4", 2) == 0) spk_num = 5;
				else if (strncmp(buf, "A4", 2) == 0) spk_num = 6;
				else if (strncmp(buf, "B4", 2) == 0) spk_num = 7;
				else if (strncmp(buf, "C5", 2) == 0) spk_num = 8;
				//else if (strncmp(buf, "C5", 2) == 0) spk_num = 9;
				else if (strncmp(buf, "D5", 2) == 0) spk_num = 10;
				else if (strncmp(buf, "E5", 2) == 0) spk_num = 11;
				else if (strncmp(buf, "F5", 2) == 0) spk_num = 12;
				else if (strncmp(buf, "G5", 2) == 0) spk_num = 13;
				else if (strncmp(buf, "A5", 2) == 0) spk_num = 14;
				else if (strncmp(buf, "B5", 2) == 0) spk_num = 15;
				else if (strncmp(buf, "C6", 2) == 0) spk_num = 16;
				//else if (strncmp(buf, "C6", 2) == 0) spk_num = 17;
				else if (strncmp(buf, "D6", 2) == 0) spk_num = 18;
				else if (strncmp(buf, "E6", 2) == 0) spk_num = 19;
				else if (strncmp(buf, "F6", 2) == 0) spk_num = 20;
				else if (strncmp(buf, "G6", 2) == 0) spk_num = 21;
				else if (strncmp(buf, "A6", 2) == 0) spk_num = 22;
				else if (strncmp(buf, "B6", 2) == 0) spk_num = 23;
				else if (strncmp(buf, "C7", 2) == 0) spk_num = 24;
				else spk_num = 0;
				
				if(spk_num==0){
					TR2=0;
					SPK=0;
				}
				else {
					displayBuf[0] = GetCharCode('P');
					displayBuf[1] = GetCharCode('L');
					displayBuf[2] = GetCharCode('A');
					displayBuf[3] = GetCharCode('Y');
					displayBuf[4] = GetCharCode(' ');
					displayBuf[5] = GetCharCode(' ');
					displayBuf[6] = GetCharCode(buf[0]);
					displayBuf[7] = GetCharCode(buf[1]);
					
					High=freq[spk_num-1][1];
					Low =freq[spk_num-1][0];
					TR2=1;
					delay_ms(1000);
					spk_num = 0;
					TR2=0;
					SPK=0;
					delay_ms(100);
					for(i = 0; i < 8; i++){
						displayBuf[i] = GetCharCode(' ');
					}
				}
			}
			else if(mode == 3){
				if(buf[7] == '_'){
					while(music_idx < music_len) {
						unsigned char note = music[music_idx++];
						displayBuf[0] = displayBuf[1];
						displayBuf[1] = displayBuf[2];
						displayBuf[2] = displayBuf[3];
						displayBuf[3] = displayBuf[4];
						displayBuf[4] = displayBuf[5];
						displayBuf[5] = displayBuf[6];
						displayBuf[6] = displayBuf[7];
						displayBuf[7] = GetCharCode(note);
						
						if (note == '.') {
							TR2 = 0;
							SPK = 0;
						} 
						else if (note >= '1' && note <= '8') {
							switch(note){
								case '1': spk_num = 9; break;
								case '2': spk_num = 10; break;
								case '3': spk_num = 11; break;
								case '4': spk_num = 12; break;
								case '5': spk_num = 13; break;
								case '6': spk_num = 14; break;
								case '7': spk_num = 15; break;
								case '8': spk_num = 16; break;
								default: spk_num = 0; break;
							}
							if(spk_num==0){
							   TR2=0;
							   SPK=0;
							}
							else {
								High=freq[spk_num-1][1];
								Low =freq[spk_num-1][0];
								TR2=1;
							}
						}
						else if(note == '_'){
							spk_num = 0;
							TR2 = 0;
							SPK = 0;
							break;
						}
						delay_ms(500);
						spk_num = 0;
						TR2 = 0;
						SPK = 0;
						delay_ms(50);
					}
					// �����F
					
					for(i = 0; i < 8; i++){
						displayBuf[i] = GetCharCode(' ');
					}
					spk_num = 0;
					TR2 = 0;
					SPK = 0;
					music_len = 0;
					music_idx = 0;
				}
			}
			else if (mode == 6) {
				// ON ���O (�~��˼�)
				if (strncmp(buf, "ON      ", 8) == 0) {
					countdown_active = 1;
					countdown_paused = 0;
				}
				// OFF ���O (�Ȱ��˼�)
				else if (strncmp(buf, "OFF     ", 8) == 0) {
					countdown_paused = 1;
				}
				// �W�[�ɶ����O (�榡: "+   1234")
				else if (buf[0] == '+') {
					// �ѪR�W�[����� (��4��Ʀr)
					unsigned int add_sec = (buf[4]-'0')*1000 + (buf[5]-'0')*100 
										 + (buf[6]-'0')*10 + (buf[7]-'0');
					
					// �W�[�ɶ� (�W�� 9999)
					if (countdown_sec + add_sec > 9999) {
						countdown_sec = 9999;
					} else {
						countdown_sec += add_sec;
					}
				}
				// ��֮ɶ����O (�榡: "-   1234")
				else if (buf[0] == '-') {
					// �ѪR��֪���� (��4��Ʀr)
					unsigned int sub_sec = (buf[4]-'0')*1000 + (buf[5]-'0')*100 
										 + (buf[6]-'0')*10 + (buf[7]-'0');
					
					// ��֮ɶ� (�U�� 0)
					if (sub_sec >= countdown_sec) {
						countdown_sec = 0;
						countdown_active = 0;
					} else {
						countdown_sec -= sub_sec;
					}
				}
				else{
					countdown_sec = (buf[0] - '0') * 1000 + (buf[1] - '0') * 100 + (buf[2] - '0') * 10 + (buf[3] - '0');
					countdown_active = 1;
					countdown_paused = 0;
					alert_cnt = 0;
					tick_ms = 0;
				}
				// ��s���
				displayBuf[0] = GetCharCode((countdown_sec / 1000) % 10 + '0');
				displayBuf[1] = GetCharCode((countdown_sec / 100) % 10 + '0');
				displayBuf[2] = GetCharCode((countdown_sec / 10) % 10 + '0');
				displayBuf[3] = GetCharCode((countdown_sec % 10) + '0');
				displayBuf[4] = GetCharCode(' ');
				displayBuf[5] = GetCharCode('S');
				displayBuf[6] = GetCharCode('E');
				displayBuf[7] = GetCharCode('C');
			}
		}
	}
}

/*------------------------------------------------
                    �o�e�@�Ӧr�`
------------------------------------------------*/
void SendByte(unsigned char dat)
{
 SBUF = dat;
 while(!TI);
      TI = 0;
}
/*------------------------------------------------
                    �o�e�@�Ӧr�Ŧ�
------------------------------------------------*/
void SendStr(unsigned char *s)
{
 while(*s!='\0')// \0 ��ܦr�Ŧ굲���лx�A�q�L�˴��O�_�r�Ŧ꥽��
  {
  SendByte(*s);
  s++;
  }
//  SendByte(0x0d);
//  SendByte(0x0a);
}
/*------------------------------------------------
                     ��f���_�{��
------------------------------------------------*/
void UART_SER (void) interrupt 4 //��椤�_�A�ȵ{��
{
    unsigned char Temp;          //�w�q�{���ܶq 
   
   if(RI)                        //�P�_�O�������_����
     {
	  	RI=0;                      //�лx��M�s
	  	Temp=SBUF;                 //Ū�J�w�İϪ���
	  	buf[head] = Temp;
		head++;
		if (mode == 3) {
            if (music_len < 64) {
                music[music_len++] = Temp;
            }
        }
		if (head == MAX) head = 0;
	 }
} 

void delay_ms(unsigned int ms)
{
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 120; j++);
}
 
void UpdateDisplay()
{
	/*
    char city_name[3];
    switch(city_code){
        case 1: 	city_name[0] = 'T'; city_name[1] = 'P'; city_name[2] = 'E';
					break;  // �x�_
        case 2: 	city_name[0] = 'N'; city_name[1] = 'T'; city_name[2] = 'P';  
					break;  // �s�_
        case 3: 	city_name[0] = 'T'; city_name[1] = 'Y'; city_name[2] = 'N'; 
					break;  // ���
        case 4: 	city_name[0] = 'T'; city_name[1] = 'X'; city_name[2] = 'G'; 
					break;  // �x��
        case 5: 	city_name[0] = 'T'; city_name[1] = 'N'; city_name[2] = 'N';
					break;  // �x�n
        case 6: 	city_name[0] = 'K'; city_name[1] = 'H'; city_name[2] = 'H'; 
					break;  // ����
        case 7: 	city_name[0] = 'H'; city_name[1] = 'O'; city_name[2] = 'T'; 
					break;  // �̰���
        default: 	city_name[0] = ' '; city_name[1] = ' '; city_name[2] = ' '; 
					break;
    }
	
    displayBuf[0] = GetCharCode(city_name[0]);
    displayBuf[1] = GetCharCode(city_name[1]);
    displayBuf[2] = GetCharCode(city_name[2]);
    displayBuf[3] = GetCharCode(' '); // �Ů�

    displayBuf[4] = GetCharCode(temp1);
    displayBuf[5] = GetCharCode(temp2);

    displayBuf[6] = GetCharCode('%');
    displayBuf[7] = GetCharCode('C');
	*/
	unsigned char i;
	for(i = 0; i < 8; i++){
		displayBuf[i] = GetCharCode(buf[i]);
	}
	if(mode == 5 && displayBuf[5] != 0x00) displayBuf[5] = displayBuf[5] + 0x80;
}
void DelayUs2x(unsigned char t)
{   
 while(--t);
}
/*------------------------------------------------
 mS���ɨ�ơA�t����J�Ѽ� unsigned char t�A�L��^��
 unsigned char �O�w�q�L�Ÿ��r���ܶq�A��Ȫ��d��O
 0~255 �o�̨ϥδ���12M�A��T���ɽШϥηJ�s
------------------------------------------------*/
void DelayMs(unsigned char t)
{
     
 while(t--)
 {
     //�j�P����1mS
     DelayUs2x(245);
	 DelayUs2x(245);
 }
}
/*------------------------------------------------
 ��ܨ�ơA�Ω�ʺA���˼ƽX��
 ��J�Ѽ� FirstBit ��ܻݭn��ܪ��Ĥ@��A�p���2��ܱq�ĤT�ӼƽX�޶}�l���
 �p��J0��ܱq�Ĥ@����ܡC
 Num��ܻݭn��ܪ���ơA�p�ݭn���99���ƭȫh�ӭȿ�J2
------------------------------------------------*/
void Display(unsigned char FirstBit,unsigned char Num)
{
      static unsigned char i=0;
	  

	   DataPort=0;   //�M�żƾڡA���������v
       LATCH1=1;     //�q��s
       LATCH1=0;

       DataPort=dofly_WeiMa[i+FirstBit]; //����X 
       LATCH2=1;     //����s
       LATCH2=0;

       DataPort=displayBuf[i]; //����ܼƾڡA�q�X
       LATCH1=1;     //�q��s
       LATCH1=0;
       
	   i++;
       if(i==Num)
	      i=0;
       


}
void Init_Timer0(void)
{
 TMOD |= 0x01;	  //�ϥμҦ�1�A16��w�ɾ��A�ϥ�"|"�Ÿ��i�H�b�ϥΦh�өw�ɾ��ɤ����v�T		     
 //TH0=0x00;	      //���w���
 //TL0=0x00;
 EA=1;            //�`���_���}
 ET0=1;           //�w�ɾ����_���}
 TR0=1;           //�w�ɾ��}�����}
}

void Init_Timer2(void) {
	TMOD |= 0x01;
    RCAP2H = High;
    RCAP2L = Low;
	EA=1;
    ET2 = 1;
}

void Timer0_isr(void) interrupt 1 {
	TH0=(65536-2000)/256;  // 2ms
	TL0=(65536-2000)%256;
	Display(0, 8);
}

void Timer2_isr(void) interrupt 5 {
	TF2=0;
	if (mode == 2 || mode == 3 || mode == 6) {
		RCAP2H = High;
		RCAP2L = Low;
		SPK = !SPK;
	}
}

unsigned char KeyScan(void)  //��L���˨�ơA�ϥΦ�C���౽�˪k
{
 unsigned char cord_h,cord_l;//��C�Ȥ����ܶq
 KeyPort=0x0f;            //��u��X����0
 cord_h=KeyPort&0x0f;     //Ū�J�C�u��
 if(cord_h!=0x0f)    //���˴����L������U
 {
  DelayMs(10);        //�h��
  if((KeyPort&0x0f)!=0x0f)
  {
    cord_h=KeyPort&0x0f;  //Ū�J�C�u��
    KeyPort=cord_h|0xf0;  //��X��e�C�u��
    cord_l=KeyPort&0xf0;  //Ū�J��u��

    while((KeyPort&0xf0)!=0xf0);//�����P�}�ÿ�X

    return(cord_h+cord_l);//��L�̫�զX�X��
   }
  }return(0xff);     //��^�ӭ�
}

unsigned char KeyPro(void)
{
 switch(KeyScan())
 {
  case 0x7e:return 0;break;//0 ���U����������ܬ۹������X��
  case 0x7d:return 1;break;//1
  case 0x7b:return 2;break;//2
  case 0x77:return 3;break;//3
  case 0xbe:return 4;break;//4
  case 0xbd:return 5;break;//5
  case 0xbb:return 6;break;//6
  case 0xb7:return 7;break;//7
  case 0xde:return 8;break;//8
  case 0xdd:return 9;break;//9
  case 0xdb:return 10;break;//a
  case 0xd7:return 11;break;//b
  case 0xee:return 12;break;//c
  case 0xed:return 13;break;//d
  case 0xeb:return 14;break;//e
  case 0xe7:return 15;break;//f
  default:return 0xff;break;
 }
}
unsigned char GetCharCode(unsigned char ch) {
    switch (ch) {
        case '0': return 0x3f;
        case '1': return 0x06;
        case '2': return 0x5b;
        case '3': return 0x4f;
        case '4': return 0x66;
        case '5': return 0x6d;
        case '6': return 0x7d;
        case '7': return 0x07;
        case '8': return 0x7f;
        case '9': return 0x6f;

        case 'A': return 0x77;
        case 'B': return 0x7c;
        case 'b': return 0x7c;
        case 'C': return 0x39;
        case 'D': return 0x5e;
        case 'd': return 0x5e;
        case 'E': return 0x79;
        case 'F': return 0x71;
        case 'G': return 0x3d;
        case 'H': return 0x76;
        case 'I': return 0x06;
        case 'J': return 0x1e;
        case 'K': return 0x75;
        case 'L': return 0x38;
        case 'M': return 0x15;
        case 'N': return 0x54;
        case 'O': return 0x3f;
        case 'P': return 0x73;
        case 'Q': return 0x6b;
        case 'R': return 0x50;
        case 'r': return 0x50;
        case 'S': return 0x6d;
        case 'T': return 0x78;
        case 'U': return 0x3e;
        case 'V': return 0x3e;
        case 'W': return 0x2a;
        case 'X': return 0x76;
        case 'Y': return 0x6e;
        case 'Z': return 0x5b;

        case ' ': return 0x00;
        case '.': return 0x80;
        case '%': return 0x63; // ���
        default: return 0x00;  // �����r����ܪť�
    }
}

void CountdownTask() {
    if (mode != 6 || countdown_active == 0 || countdown_paused) return;

    tick_ms += 1;
	if(countdown_sec == 0) tick_ms += 1;
    if (tick_ms >= 2500) {
        tick_ms = 0;

        if (countdown_sec > 0) {
			alert_cnt = 0;
            countdown_sec--;

            // ��s���
            displayBuf[0] = GetCharCode((countdown_sec / 1000) % 10 + '0');
            displayBuf[1] = GetCharCode((countdown_sec / 100) % 10 + '0');
            displayBuf[2] = GetCharCode((countdown_sec / 10) % 10 + '0');
            displayBuf[3] = GetCharCode((countdown_sec % 10) + '0');
            displayBuf[4] = GetCharCode(' ');
            displayBuf[5] = GetCharCode('S');
            displayBuf[6] = GetCharCode('E');
            displayBuf[7] = GetCharCode('C');

            if (countdown_sec <= 10) {
                High = freq[0][1];  // C tone
                Low = freq[0][0];
                TR2 = 1;
                SPK = 1;
                delay_ms(100);  // �u�T
                TR2 = 0;
                SPK = 0;
            }

        } 
		else {
            if (alert_cnt < 5) {
                High = freq[23][1];
                Low = freq[23][0];
                TR2 = 1;
                SPK = 1;
                delay_ms(25);  // B�n����ɶ�
                TR2 = 0;
                SPK = 0;
                delay_ms(25);  // ���j�ɶ�
                High = freq[23][1];
                Low = freq[23][0];
                TR2 = 1;
                SPK = 1;
                delay_ms(25);  // B�n����ɶ�
                TR2 = 0;
                SPK = 0;
                delay_ms(25);  // ���j�ɶ�
                High = freq[23][1];
                Low = freq[23][0];
                TR2 = 1;
                SPK = 1;
                delay_ms(25);  // B�n����ɶ�
                TR2 = 0;
                SPK = 0;
                delay_ms(25);  // ���j�ɶ�
                High = freq[23][1];
                Low = freq[23][0];
                TR2 = 1;
                SPK = 1;
                delay_ms(25);  // B�n����ɶ�
                TR2 = 0;
                SPK = 0;
                delay_ms(25);  // ���j�ɶ�
                alert_cnt++;
				tick_ms += 1;
            } 
			else {
                countdown_active = 0;
            }
        }
    }
}