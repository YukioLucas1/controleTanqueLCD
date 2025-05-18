/*******************************************************************************
        Proposta do codigo - Simular uma rotina de um conjunto de um tanque,
valvula, sensor de nivel, LCD e botao, cogumelo para emergencia. O sensor
é simulado pelo potenciometro, nomeado setpoint, a valvula o led, nomeado saida
a bomba o led, nomeado pwm.
        Funcionamento do código - A quantidade de liquido é amostrado no LCD
a bomba alimenta um tanque de capacidade de 300L quando o tanque chega na metade
do armazenamento a valvula é ligada para diminuir o armazenamento usado, a bomba
fica ligada até o tanque atingir 250L, para evitar risco de transbordamento.
Usa-se delay na valvula e no tanque para não ter o problema de fecha e abre de
maneira compulsoria e desgastar o equipamento.
         Foi implementado também uma interrupção externa no caso de acidentes,
simulamos um botao cogumelo, com o BT0, quando pressionado a rotina é desligada,
quando pressionado novamente a rotina é restabelecida.
*******************************************************************************/

/*******************************************************************************
*     As diretivas abaixo estão relacionadas à conexão do hardware do LCD      *
*******************************************************************************/
#define  Data7     PORTB.B0  //Porto de conexão D7 do LCD
#define  Data6     PORTB.B1  //Porto de conexão D6 do LCD
#define  Data5     PORTB.B2  //Porto de conexão D5 do LCD
#define  Data4     PORTB.B3  //Porto de conexão D4 do LCD

#define  RS        PORTB.B4  //Bit RW do LCD
#define  EN        PORTB.B5  //Bit EN do LCD
#define  valvula   PORTD.B5
#define  bomba     PORTD.B6

/*******************************************************************************
*     As diretivas abaixo estão relacionadas ao controle de posicionamento de  *
* caracteres e strings no LCD.                                                 *
*******************************************************************************/
#define  NumCol    16       //Número de colunas do LCD utilizado (16x2)
#define  Linha1    0x80     //1º endereço linha 1 do LCD
#define  Linha2    0xC0     //1º endereço linha 2 do LCD
#define  Linha3    0x90     //1º endereço linha 3 do LCD
#define  Linha4    0xD0     //1º endereço linha 4 do LCD
#define  MSG1      0        //Código da 1ª Mensagem
#define  MSG2      1        //Código da 2ª Mensagem
#define  MSG3      2        //Código da 3ª Mensagem
#define  MSG4      3        //Código da 4ª Mensagem
#define  MSG5      4        //Código da 5ª Mensagem
#define  MSG6      5        //Código da 6ª Mensagem
#define  MSG7      6        //Código da 7ª Mensagem
#define  MSG8      7        //Código da 8ª Mensagem
#define  MSG9      8        //Código da 9ª Mensagem
#define  MSG10     9        //Código da 10ª Mensagem

/*******************************************************************************
*     As diretivas abaixo estão relacionadas aos códigos dos caracteres especi-*
* ais a serem carregados no LCD.                                               *
*******************************************************************************/
#define  Grau_C    0x00     //Código Grau Célsius
#define  Seta_E    0x01     //Código Seta para Esquerda
#define  Seta_D    0x02     //Código Seta para Direita
#define  Seta_C    0x03     //Código Seta para Cima
#define  Seta_B    0x04     //Código Seta para Baixo
#define  Grau_F    0x05     //Código Grau Fahrenheit

/*******************************************************************************
*     As variáveis abaixo estão relacionadas as funções a serem executadas pela*
*aplicação proposta.                                                           *
*******************************************************************************/
unsigned int stop = 0;
//Tabela de valores correspondentes aos números amostrados nos displays de 7 seg
unsigned char Tab_Disp[10] = {0x03, 0x9F, 0x25, 0x0D, 0x99,
                              0x49, 0x41, 0x1F, 0x01, 0x09};

unsigned char Uni, Dez, Cen, Mil, display;

unsigned int analog, aux1, aux2;

int measure, ideal_value;

/*******************************************************************************
*   A tabela abaixo contem caracteres especiais a serem carregados na CGRAM do *
* display LCD, conforme solicitação via software.                              *
*******************************************************************************/
unsigned char
Tab_CGRam[40] = {0x18, 0x18, 0x06, 0x09, 0x08, 0x09, 0x06, 0x00, //Graus Célsius
                 0x01, 0x03, 0x07, 0x0F, 0x07, 0x03, 0x01, 0x00, //Seta Esquerda
                 0x08, 0x0C, 0x0E, 0x0F, 0x0E, 0x0C, 0x08, 0x00, //Seta Direita
                 0x00, 0x00, 0x04, 0x0E, 0x1F, 0x00, 0x00, 0x00, //Seta Cima
                 0x00, 0x00, 0x1F, 0x0E, 0x04, 0x00, 0x00, 0x00};//Seta Baixo

/*******************************************************************************
*      A tabela abaixo contem os parâmetros de configuração inicial do display *
* LCD 4 Bits, a qual deverá ser executada após configuração prévia em 8 Bits.  *
*******************************************************************************/
unsigned char Tab_Conf[4] = {0x28,
                             0x06,
                             0x01,
                             0x0C};


/*******************************************************************************
*      A tabela abaixo contem os parâmetros de configuração inicial do display *
* LCD em 4 bits, a qual deverá ser executada uma única vez no inicio.          *
*******************************************************************************/
unsigned char Tab_4Bit[4] = {0x30,
                             0x30,
                             0x30,
                             0x20};

/*******************************************************************************
*      Procedimento para ajuste inicial do HD44780 considerando que o reset    *
* interno falhou durante a energização. Assim, deve-se enviar três vezes o     *
* comando para configurar a interface para 8 bits (em intervalos conforme o    *
* código abaixo) para então configurá-la para 4 bits.                          *
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
*     Função de configuração do LCD, conectado com via de dados de 8 bits, e   *
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
*     Função que carrega caracteres especiais na CGRAM do LCD, quando solici-  *
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
*     Função que posiciona caractere a ser escrito no LCD, em endereço especi- *
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
*     Função que escreve caractere a ser escrito no LCD, em endereço especi-   *
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
*     Função que escreve string no LCD, em endereço especificado por software. *
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
*   Função que habilita o cursor piscando no LCD para indicar posição de ajuste*
* de horário quando solicitada.                                                *
*******************************************************************************/
void Blink (void)
{
 Posi_Char (0x0F);
}

/*******************************************************************************
*    Função que desabilita o cursor do LCD após uso de indicação de posição de *
* ajuste de horário quando solicitada.                                         *
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

 ADCSRA  = 0x93;            //liga conversão AD, fator de divisão de 8
 //ADMUX   = 0x43;            //tensão de ref. de 5V, canal A3
}


int AD_Conv (unsigned char canalAD)
{
   static int analogH, analogL;    //variáveis locais para valores ADC

   ADMUX   = 0x40 | canalAD;

   ADCSRA |= (1<<ADSC);                    //inicia conversão ad

   while(!(ADCSRA&=~(1<<ADIF)));           //aguarda conversão ad completar

   ADCSRA |= (1<<ADIF);                    //limpa ADIF com transição LOW para HIGH

   analogL = ADCL;                         //armazena o byte menos significativo
   analogH = ADCH;                         //armazena o byte mais  significativo

   analog  = (analogH<<8) | analogL;       //calcula para valor de 10 bits

   return analog;                          //retorna resultado da conversão

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
*     Função de tratamento do programa principal, o qual gerencia a aplicação, *
* solicitando a chamada das funções de forma adequada.                         *
*******************************************************************************/
void InterrupcaoExterna0 (void) iv IVT_ADDR_INT0 ics ICS_OFF
{
     stop++;
     if(stop == 2)stop = 0;

}
void main (void)
{
 Config_Ports ( );          // Configura os portos de comunicação
                            //e direção da informação

 Disp_4bits ( );            //Configura display LCD no modo 4 bits
 Posi_Char (Linha1);
 Escreve_Frase (MSG1);

   DDRD.B5 = 1; //PORTD.B5 definido como saída
   DDRD.B6 = 1;

   //Interrupçao INT0 por Bordo de subida
   EICRA.ISC01 = 1;
   EICRA.ISC00 = 1;

   //Interrupçao INT1 por Bordo de subida
   EICRA.ISC11 = 1;
   EICRA.ISC10 = 1;

   //Habilita interrupções
   EIMSK.INT0 = 1; //habilita interrupcao externa INT0

   EIMSK.INT1 = 1; //habilita interrupcao externa INT1

   SREG_I_Bit = 1; //Habilita interrupção global

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