/*******************************************************************************
        Proposta do codigo - Simular uma rotina de um conjunto de um tanque,
valvula, sensor de nivel, LCD e botao, cogumelo para emergencia. O sensor
� simulado pelo potenciometro, nomeado setpoint, a valvula o led, nomeado saida
a bomba o led, nomeado pwm.
        Funcionamento do c�digo - A quantidade de liquido � amostrado no LCD
a bomba alimenta um tanque de capacidade de 300L quando o tanque chega na metade
do armazenamento a valvula � ligada para diminuir o armazenamento usado, a bomba
fica ligada at� o tanque atingir 250L, para evitar risco de transbordamento.
Usa-se delay na valvula e no tanque para n�o ter o problema de fecha e abre de
maneira compulsoria e desgastar o equipamento.
         Foi implementado tamb�m uma interrup��o externa no caso de acidentes,
simulamos um botao cogumelo, com o BT0, quando pressionado a rotina � desligada,
quando pressionado novamente a rotina � restabelecida.
*******************************************************************************/

/*******************************************************************************
*     As diretivas abaixo est�o relacionadas � conex�o do hardware do LCD      *
*******************************************************************************/
#define  Data7     PORTB.B0  //Porto de conex�o D7 do LCD
#define  Data6     PORTB.B1  //Porto de conex�o D6 do LCD
#define  Data5     PORTB.B2  //Porto de conex�o D5 do LCD
#define  Data4     PORTB.B3  //Porto de conex�o D4 do LCD

#define  RS        PORTB.B4  //Bit RW do LCD
#define  EN        PORTB.B5  //Bit EN do LCD
#define  valvula   PORTD.B5
#define  bomba     PORTD.B6

/*******************************************************************************
*     As diretivas abaixo est�o relacionadas ao controle de posicionamento de  *
* caracteres e strings no LCD.                                                 *
*******************************************************************************/
#define  NumCol    16       //N�mero de colunas do LCD utilizado (16x2)
#define  Linha1    0x80     //1� endere�o linha 1 do LCD
#define  Linha2    0xC0     //1� endere�o linha 2 do LCD
#define  Linha3    0x90     //1� endere�o linha 3 do LCD
#define  Linha4    0xD0     //1� endere�o linha 4 do LCD
#define  MSG1      0        //C�digo da 1� Mensagem
#define  MSG2      1        //C�digo da 2� Mensagem
#define  MSG3      2        //C�digo da 3� Mensagem
#define  MSG4      3        //C�digo da 4� Mensagem
#define  MSG5      4        //C�digo da 5� Mensagem
#define  MSG6      5        //C�digo da 6� Mensagem
#define  MSG7      6        //C�digo da 7� Mensagem
#define  MSG8      7        //C�digo da 8� Mensagem
#define  MSG9      8        //C�digo da 9� Mensagem
#define  MSG10     9        //C�digo da 10� Mensagem

/*******************************************************************************
*     As diretivas abaixo est�o relacionadas aos c�digos dos caracteres especi-*
* ais a serem carregados no LCD.                                               *
*******************************************************************************/
#define  Grau_C    0x00     //C�digo Grau C�lsius
#define  Seta_E    0x01     //C�digo Seta para Esquerda
#define  Seta_D    0x02     //C�digo Seta para Direita
#define  Seta_C    0x03     //C�digo Seta para Cima
#define  Seta_B    0x04     //C�digo Seta para Baixo
#define  Grau_F    0x05     //C�digo Grau Fahrenheit

/*******************************************************************************
*     As vari�veis abaixo est�o relacionadas as fun��es a serem executadas pela*
*aplica��o proposta.                                                           *
*******************************************************************************/
unsigned int stop = 0;
//Tabela de valores correspondentes aos n�meros amostrados nos displays de 7 seg
unsigned char Tab_Disp[10] = {0x03, 0x9F, 0x25, 0x0D, 0x99,
                              0x49, 0x41, 0x1F, 0x01, 0x09};

unsigned char Uni, Dez, Cen, Mil, display;

unsigned int analog, aux1, aux2;

int measure, ideal_value;

/*******************************************************************************
*   A tabela abaixo contem caracteres especiais a serem carregados na CGRAM do *
* display LCD, conforme solicita��o via software.                              *
*******************************************************************************/
unsigned char
Tab_CGRam[40] = {0x18, 0x18, 0x06, 0x09, 0x08, 0x09, 0x06, 0x00, //Graus C�lsius
                 0x01, 0x03, 0x07, 0x0F, 0x07, 0x03, 0x01, 0x00, //Seta Esquerda
                 0x08, 0x0C, 0x0E, 0x0F, 0x0E, 0x0C, 0x08, 0x00, //Seta Direita
                 0x00, 0x00, 0x04, 0x0E, 0x1F, 0x00, 0x00, 0x00, //Seta Cima
                 0x00, 0x00, 0x1F, 0x0E, 0x04, 0x00, 0x00, 0x00};//Seta Baixo

/*******************************************************************************
*      A tabela abaixo contem os par�metros de configura��o inicial do display *
* LCD 4 Bits, a qual dever� ser executada ap�s configura��o pr�via em 8 Bits.  *
*******************************************************************************/
unsigned char Tab_Conf[4] = {0x28,
                             0x06,
                             0x01,
                             0x0C};


/*******************************************************************************
*      A tabela abaixo contem os par�metros de configura��o inicial do display *
* LCD em 4 bits, a qual dever� ser executada uma �nica vez no inicio.          *
*******************************************************************************/
unsigned char Tab_4Bit[4] = {0x30,
                             0x30,
                             0x30,
                             0x20};

/*******************************************************************************
*      Procedimento para ajuste inicial do HD44780 considerando que o reset    *
* interno falhou durante a energiza��o. Assim, deve-se enviar tr�s vezes o     *
* comando para configurar a interface para 8 bits (em intervalos conforme o    *
* c�digo abaixo) para ent�o configur�-la para 4 bits.                          *
*******************************************************************************/
void Disp_4bits (void)
{
 unsigned char y;
 display = 0xF0;
 delay_ms (100);

 for (y=0; y<=3; y++)
    {
      display = Tab_4Bit[y];
      Data7 = display.B7;
      Data6 = display.B6;
      Data5 = display.B5;
      Data4 = display.B4;
      delay_ms (2);
      RS = 0;
      EN = 1;
      delay_ms (2);
      EN = 0;
    }

    for (y=0; y<=3; y++)
    {
/*******************************************************************************
*     Fun��o de configura��o do LCD, conectado com via de dados de 8 bits, e   *
* matriz do tipo 7x5 pixels.                                                   *
*******************************************************************************/
/*******************************************************************************
*     Escreve o nibble mais significativo do barramento de dados do LCD.       *
*******************************************************************************/
      display = (Tab_Conf[y] & 0xF0);
      Data7 = display.B7;
      Data6 = display.B6;
      Data5 = display.B5;
      Data4 = display.B4;
      delay_ms (2);
      RS = 0;
      EN = 1;
      delay_ms (2);
      EN = 0;

/*******************************************************************************
*     Escreve o nibble menos significativo do barramento de dados do LCD.      *
*******************************************************************************/
      display = ((Tab_Conf[y]) << 4);
      Data7 = display.B7;
      Data6 = display.B6;
      Data5 = display.B5;
      Data4 = display.B4;
      delay_ms (2);
      RS = 0;
      EN = 1;
      delay_ms (2);
      EN = 0;
    }
}

/*******************************************************************************
*     Fun��o que carrega caracteres especiais na CGRAM do LCD, quando solici-  *
* tadas por software.                                                          *
*******************************************************************************/
void Load_GraphChar (unsigned char codigo)
{
 unsigned char z;
 display = ((0x40 + (codigo * 8)) & 0xF0);
 Data7 = display.B7;
 Data6 = display.B6;
 Data5 = display.B5;
 Data4 = display.B4;
 delay_ms (2);
 RS = 0;
 EN = 1;
 delay_ms (2);
 EN = 0;

 display = ((0x40 + (codigo * 8)) << 4);
 Data7 = display.B7;
 Data6 = display.B6;
 Data5 = display.B5;
 Data4 = display.B4;
 delay_ms (2);
 RS = 0;
 EN = 1;
 delay_ms (2);
 EN = 0;

 for (z=(codigo * 8); z<=(7 + (codigo * 8)); z++)
  {
   display = (Tab_CGRam[z] & 0xF0);
   Data7 = display.B7;
   Data6 = display.B6;
   Data5 = display.B5;
   Data4 = display.B4;
   delay_ms (2);
   RS = 1;
   EN = 1;
   delay_ms (2);
   EN = 0;

   display = ((Tab_CGRam[z]) << 4);
   Data7 = display.B7;
   Data6 = display.B6;
   Data5 = display.B5;
   Data4 = display.B4;
   delay_ms (2);
   RS = 1;
   EN = 1;
   delay_ms (2);
   EN = 0;
  }
}

/*******************************************************************************
*     Fun��o que posiciona caractere a ser escrito no LCD, em endere�o especi- *
* ficado por software.                                                         *
*******************************************************************************/
void Posi_Char (unsigned char Position)
{
 display = (Position & 0xF0);
 Data7 = display.B7;
 Data6 = display.B6;
 Data5 = display.B5;
 Data4 = display.B4;
 delay_ms (2);
 RS = 0;
 EN = 1;
 delay_ms (2);
 EN = 0;

 display = ((Position) << 4);
 Data7 = display.B7;
 Data6 = display.B6;
 Data5 = display.B5;
 Data4 = display.B4;
 delay_ms (2);
 RS = 0;
 EN = 1;
 delay_ms (2);
 EN = 0;
}

/*******************************************************************************
*     Fun��o que escreve caractere a ser escrito no LCD, em endere�o especi-   *
* ficado por software.                                                         *
*******************************************************************************/
void Escreve_Char (unsigned char Num_CGRAM)
{
  display = (Num_CGRAM & 0xF0);
  Data7 = display.B7;
  Data6 = display.B6;
  Data5 = display.B5;
  Data4 = display.B4;
  delay_ms (2);
  RS = 1;
  EN = 1;
  delay_ms (2);
  EN = 0;

  display = ((Num_CGRAM) << 4);
  Data7 = display.B7;
  Data6 = display.B6;
  Data5 = display.B5;
  Data4 = display.B4;
  delay_ms (2);
  RS = 1;
  EN = 1;
  delay_ms (2);
  EN = 0;
}

/*******************************************************************************
*     Fun��o que escreve string no LCD, em endere�o especificado por software. *
*******************************************************************************/
void Escreve_Frase (unsigned char Local)
{
 unsigned char i;
//                                      0123456789ABCDEF
 code unsigned char Message[10][17] = {"Status do Tanque",
                                       "Rotina desligada"};
 for (i=0; i<=(NumCol-1); i++)
  {
   display = ((Message[Local][i]) & 0xF0);
   Data7 = display.B7;
   Data6 = display.B6;
   Data5 = display.B5;
   Data4 = display.B4;
   delay_ms (2);
   RS = 1;
   EN = 1;
   delay_ms (2);
   EN = 0;

   display = ((Message[Local][i]) << 4);
   Data7 = display.B7;
   Data6 = display.B6;
   Data5 = display.B5;
   Data4 = display.B4;
   delay_ms (2);
   RS = 1;
   EN = 1;
   delay_ms (2);
   EN = 0;
  }
}

/*******************************************************************************
*   Fun��o que habilita o cursor piscando no LCD para indicar posi��o de ajuste*
* de hor�rio quando solicitada.                                                *
*******************************************************************************/
void Blink (void)
{
 Posi_Char (0x0F);
}

/*******************************************************************************
*    Fun��o que desabilita o cursor do LCD ap�s uso de indica��o de posi��o de *
* ajuste de hor�rio quando solicitada.                                         *
*******************************************************************************/
void Cursor_Off (void)
{
 Posi_Char (0x0C);
}

void Config_Ports (void)
{
 DDRB.B0 = 1;
 DDRB.B1 = 1;
 DDRB.B2 = 1;
 DDRB.B3 = 1;

 DDRB.B4 = 1;
 DDRB.B5 = 1;

 ADCSRA  = 0x93;            //liga convers�o AD, fator de divis�o de 8
 //ADMUX   = 0x43;            //tens�o de ref. de 5V, canal A3
}


int AD_Conv (unsigned char canalAD)
{
   static int analogH, analogL;    //vari�veis locais para valores ADC

   ADMUX   = 0x40 | canalAD;

   ADCSRA |= (1<<ADSC);                    //inicia convers�o ad

   while(!(ADCSRA&=~(1<<ADIF)));           //aguarda convers�o ad completar

   ADCSRA |= (1<<ADIF);                    //limpa ADIF com transi��o LOW para HIGH

   analogL = ADCL;                         //armazena o byte menos significativo
   analogH = ADCH;                         //armazena o byte mais  significativo

   analog  = (analogH<<8) | analogL;       //calcula para valor de 10 bits

   return analog;                          //retorna resultado da convers�o

} //end ad_conv


void converte (unsigned int valor)
{
 aux1 = valor/100;
 aux2 = valor%100;

 Uni = aux2%10;
 Dez = aux2/10;
 Cen = aux1%10;
 Mil = aux1/10;
}

void mostra (void)
{
 Posi_Char(Linha2 + 5);
 Escreve_Char(Mil + '0');
 Posi_Char(Linha2 + 6);
 Escreve_Char(Cen + '0');
 Posi_Char(Linha2 + 7);
 Escreve_Char(Dez + '0');
 Posi_Char(Linha2 + 8);
 Escreve_Char(0x2E);
 Posi_Char(Linha2 + 9);
 Escreve_Char(Uni + '0');
 Posi_Char(Linha2 + 10);
 Escreve_Char(0x4C);
}

/*******************************************************************************
*     Fun��o de tratamento do programa principal, o qual gerencia a aplica��o, *
* solicitando a chamada das fun��es de forma adequada.                         *
*******************************************************************************/
void InterrupcaoExterna0 (void) iv IVT_ADDR_INT0 ics ICS_OFF
{
     stop++;
     if(stop == 2)stop = 0;

}
void main (void)
{
 Config_Ports ( );          // Configura os portos de comunica��o
                            //e dire��o da informa��o

 Disp_4bits ( );            //Configura display LCD no modo 4 bits
 Posi_Char (Linha1);
 Escreve_Frase (MSG1);

   DDRD.B5 = 1; //PORTD.B5 definido como sa�da
   DDRD.B6 = 1;

   //Interrup�ao INT0 por Bordo de subida
   EICRA.ISC01 = 1;
   EICRA.ISC00 = 1;

   //Interrup�ao INT1 por Bordo de subida
   EICRA.ISC11 = 1;
   EICRA.ISC10 = 1;

   //Habilita interrup��es
   EIMSK.INT0 = 1; //habilita interrupcao externa INT0

   EIMSK.INT1 = 1; //habilita interrupcao externa INT1

   SREG_I_Bit = 1; //Habilita interrup��o global

 while (1)
 {
 if(stop==1){
   Posi_Char (Linha1);
   Escreve_Frase (MSG2);
   }
 if(stop != 1){
  Posi_Char (Linha1);
  Escreve_Frase (MSG1);

  AD_Conv(5);
  ideal_value = analog*3000.0/1024.0;
  converte (ideal_value);
  mostra( );

  if (ideal_value >= 1500) {
            valvula = 1;
        } else {
            delay_ms(200);
            valvula = 0;
        }
        if (ideal_value <= 2500) {
                  bomba = 1;
              } else {
                  delay_ms(200);
                  bomba = 0;
              }
  }
 }
}