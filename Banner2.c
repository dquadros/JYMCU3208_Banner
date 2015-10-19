// Mostra mensagens armazenadas na EEProm no display JY-MCU 3208, 
// rolando horizontalmente o texto
// 
// (C) 2012, Daniel Quadros
//
// ----------------------------------------------------------------------------
//   "THE BEER-WARE LICENSE" (Revision 42):
//   <dqsoft.blogspot@gmail.com> wrote this file.  As long as you retain this 
//   notice you can do whatever you want with this stuff. If we meet some day, 
//   and you think this stuff is worth it, you can buy me a beer in return.
//      Daniel Quadros
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <avr/eeprom.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "jymcu3208.h"
#include "ht1632c.h"

#define FALSE 0
#define TRUE  1

// Fonte de caracteres 0x20 a 0x7F 5x8 c/ largura variável
// Adaptada de exemplo de Rick Shear (http://rs-micro.com/?p=185)
const uint8_t font[] PROGMEM =
{
 3, 0x00, 0x00, 0x00, 0x00, 0x00, // " "
 1, 0xFD, 0x00, 0x00, 0x00, 0x00, //  !
 5, 0xE0, 0xC0, 0x00, 0xE0, 0xC0, //  "
 5, 0x24, 0xFF, 0x24, 0xFF, 0x24, //  #
 5, 0x12, 0x2A, 0x7F, 0x2A, 0x24, //  $
 5, 0x63, 0x64, 0x08, 0x13, 0x63, //  %
 5, 0x76, 0x89, 0x95, 0x62, 0x05, //  &
 1, 0xE0, 0x00, 0x00, 0x00, 0x00, //  '
 4, 0x00, 0x3C, 0x42, 0x81, 0x00, //  (
 4, 0x00, 0x81, 0x42, 0x3C, 0x00, //  )
 5, 0x22, 0x2C, 0xF0, 0x2C, 0x22, //  *
 5, 0x08, 0x08, 0x3E, 0x08, 0x08, //  +
 3, 0x07, 0x1C, 0x10, 0x00, 0x00, //  ,
 5, 0x08, 0x08, 0x08, 0x08, 0x08, //  -
 2, 0x03, 0x03, 0x00, 0x00, 0x00, //  .
 5, 0x03, 0x06, 0x18, 0x60, 0xC0, //  /
 5, 0x7E, 0x89, 0x91, 0xA1, 0x7E, //  0
 4, 0x00, 0x41, 0xFF, 0x01, 0x00, //  1
 5, 0x43, 0x85, 0x89, 0x91, 0x61, //  2
 5, 0x42, 0x81, 0x91, 0x91, 0x6E, //  3
 5, 0x18, 0x28, 0x48, 0xFF, 0x08, //  4
 5, 0xF2, 0x91, 0x91, 0x91, 0x8E, //  5
 5, 0x1E, 0x29, 0x49, 0x89, 0x86, //  6
 5, 0x80, 0x8F, 0x90, 0xA0, 0xC0, //  7
 5, 0x6E, 0x91, 0x91, 0x91, 0x6E, //  8
 5, 0x70, 0x89, 0x89, 0x8A, 0x7C, //  9
 2, 0x6C, 0x6C, 0x00, 0x00, 0x00, //  :
 3, 0x03, 0x6E, 0x68, 0x00, 0x00, //  ;
 4, 0x08, 0x14, 0x22, 0x41, 0x00, //  <
 5, 0x14, 0x14, 0x14, 0x14, 0x14, //  =
 4, 0x41, 0x22, 0x14, 0x08, 0x00, //  >
 5, 0x60, 0x80, 0x8D, 0x90, 0x60, //  ?
 5, 0x66, 0x89, 0x8F, 0x81, 0x7E, //  @
 5, 0x3F, 0x48, 0x88, 0x48, 0x3F, //  A
 5, 0xFF, 0x91, 0x91, 0x91, 0x6E, //  B
 5, 0x7E, 0x81, 0x81, 0x81, 0x42, //  C
 5, 0xFF, 0x81, 0x81, 0x42, 0x3C, //  D
 5, 0xFF, 0x91, 0x91, 0x91, 0x81, //  E
 5, 0xFF, 0x90, 0x90, 0x90, 0x80, //  F
 5, 0x7E, 0x81, 0x89, 0x89, 0x4E, //  G
 5, 0xFF, 0x10, 0x10, 0x10, 0xFF, //  H
 5, 0x81, 0x81, 0xFF, 0x81, 0x81, //  I
 5, 0x06, 0x01, 0x01, 0x01, 0xFE, //  J
 5, 0xFF, 0x18, 0x24, 0x42, 0x81, //  K
 5, 0xFF, 0x01, 0x01, 0x01, 0x01, //  L
 5, 0xFF, 0x40, 0x30, 0x40, 0xFF, //  M
 5, 0xFF, 0x40, 0x30, 0x08, 0xFF, //  N
 5, 0x7E, 0x81, 0x81, 0x81, 0x7E, //  O
 5, 0xFF, 0x88, 0x88, 0x88, 0x70, //  P
 5, 0x7E, 0x81, 0x85, 0x82, 0x7D, //  Q
 5, 0xFF, 0x88, 0x8C, 0x8A, 0x71, //  R
 5, 0x61, 0x91, 0x91, 0x91, 0x8E, //  S
 5, 0x80, 0x80, 0xFF, 0x80, 0x80, //  T
 5, 0xFE, 0x01, 0x01, 0x01, 0xFE, //  U
 5, 0xF0, 0x0C, 0x03, 0x0C, 0xF0, //  V
 5, 0xFF, 0x02, 0x0C, 0x02, 0xFF, //  W
 5, 0xC3, 0x24, 0x18, 0x24, 0xC3, //  X
 5, 0xE0, 0x10, 0x0F, 0x10, 0xE0, //  Y
 5, 0x83, 0x85, 0x99, 0xA1, 0xC1, //  Z
 3, 0xFF, 0x81, 0x81, 0x00, 0x00, //  [
 5, 0xC0, 0x60, 0x18, 0x06, 0x03, //  barra reversa
 3, 0x81, 0x81, 0xFF, 0x00, 0x00, //  ]
 5, 0x20, 0x40, 0x80, 0x40, 0x20, //  ^
 5, 0x01, 0x01, 0x01, 0x01, 0x01, //  _
 2, 0xC0, 0x70, 0x00, 0x00, 0x00, //  ´
 5, 0x06, 0x29, 0x29, 0x29, 0x1F, //  a
 5, 0xFF, 0x09, 0x11, 0x11, 0x0E, //  b
 5, 0x1E, 0x21, 0x21, 0x21, 0x12, //  c
 5, 0x0E, 0x11, 0x11, 0x09, 0xFF, //  d
 5, 0x0E, 0x15, 0x15, 0x15, 0x0C, //  e
 5, 0x08, 0x7F, 0x88, 0x80, 0x40, //  f
 5, 0x30, 0x49, 0x49, 0x49, 0x7E, //  g
 5, 0xFF, 0x08, 0x10, 0x10, 0x0F, //  h
 1, 0x5F, 0x00, 0x00, 0x00, 0x00, //  i
 4, 0x02, 0x01, 0x21, 0xBE, 0x00, //  j
 4, 0xFF, 0x04, 0x0A, 0x11, 0x00, //  k
 4, 0x00, 0x81, 0xFF, 0x01, 0x00, //  l
 5, 0x3F, 0x20, 0x18, 0x20, 0x1F, //  m
 5, 0x3F, 0x10, 0x20, 0x20, 0x1F, //  n
 5, 0x0E, 0x11, 0x11, 0x11, 0x0E, //  o
 5, 0x3F, 0x24, 0x24, 0x24, 0x18, //  p
 5, 0x10, 0x28, 0x28, 0x18, 0x3F, //  q
 5, 0x1F, 0x08, 0x10, 0x10, 0x08, //  r
 5, 0x09, 0x15, 0x15, 0x15, 0x02, //  s
 5, 0x20, 0xFE, 0x21, 0x01, 0x02, //  t
 5, 0x1E, 0x01, 0x01, 0x02, 0x1F, //  u
 5, 0x1C, 0x02, 0x01, 0x02, 0x1C, //  v
 5, 0x1E, 0x01, 0x0E, 0x01, 0x1E, //  w
 5, 0x11, 0x0A, 0x04, 0x0A, 0x11, //  x
 5, 0x00, 0x39, 0x05, 0x05, 0x3E, //  y
 5, 0x11, 0x13, 0x15, 0x19, 0x11, //  z
 3, 0x10, 0x6E, 0x81, 0x00, 0x00, //  {
 1, 0xFF, 0x00, 0x00, 0x00, 0x00, //  |
 3, 0x81, 0x6E, 0x10, 0x00, 0x00, //  }
 4, 0x40, 0x80, 0x40, 0x80, 0x00, //  ~
 5, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  //  DEL
};

#define DLY_CAR 2

// Mapeamento das mensagens na EEProm
// O ATmega8 tem 512 bytes de EEProm
#define T_MSG   40  // tamanho máximo de cada mensagem
#define N_MSG   8   // quantidade de mensagens
uint8_t eepMsgs[N_MSG][T_MSG+1] EEMEM;

// Variaveis
static volatile uint8_t cnt_delay;
static uint8_t msg_atual = 0;

// Fila de recepção da serial
#define T_FILA 16
static volatile uint8_t filaRx[T_FILA];
static volatile uint8_t poe, tira;

// Rotinas
static void    load_msg (uint8_t *pMsg);
static void    init_serial ();
static uint8_t trata_serial ();
static int     le_espera_serial ();
static int     le_serial ();
static void    envia_serial (char *pMsg);
static uint8_t trata_botoes ();
static void    display (uint8_t c, uint8_t dly);
static void    desloca (uint8_t fLimpa);
static void    tempo_init (void);
static void    delay_ticks (uint8_t ticks);

// Programa principal
int main(void)
{
    uint8_t msg[T_MSG+1];
    uint8_t iMsg;
    
    // Liga pullup das teclas
    TEC_DDR &= ~(TEC_KEY1|TEC_KEY2|TEC_KEY3);
    TEC_PORT |= TEC_KEY1|TEC_KEY2|TEC_KEY3;
    
    ht1632c_init();             // inicia o controlador do display
    ht1632c_clear();            // limpa o display
    tempo_init();               // inicia a contagem de tempo
    init_serial();              // inicia a comunicação serial

    // Carregar a primeira mensagem
    load_msg (msg);
    
    // Loop perpétuo
    for (;;)
    {
        if (trata_serial ())
            load_msg (msg);
        if (trata_botoes ())
            load_msg (msg);
        iMsg = 0;
        while (msg[iMsg])
        {
            display (msg[iMsg], DLY_CAR);
            iMsg++;
        }
        delay_ticks (32);
        display (0x20, DLY_CAR);
        display (0x20, DLY_CAR);
        display (0x20, DLY_CAR);
        display (0x20, DLY_CAR);
        display (0x20, DLY_CAR);
        display (0x20, DLY_CAR);
        display (0x20, DLY_CAR);
    }
}

// Carrega da EEProm a mensagem atual
static void load_msg (uint8_t *pMsg)
{
    int i;
    for (i = 0; i < T_MSG; i++)
        pMsg[i] = eeprom_read_byte (&eepMsgs[msg_atual][i]);
    pMsg[i] = 0;    // garante fim no final
}

// Inicia a comunicação serial
static void init_serial ()
{
    // Inicia a fila
    poe = tira = 0;
    
    // Programa a USART para 9600 8N1
    UCSRA = _BV(U2X);		            // para maior resolução do baud rate
    UCSRB = _BV(RXEN) | _BV(TXEN) | _BV(RXCIE);     // liga recepção e transmissão
                                                    //   com interrupção de Rx
    UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0) ;  // 8bit, 1 stop, sem paridade
    UCSRC = 0;                                      // UBRRH = 0
    UBRRL = (CLK_CPU / (8 * 9600UL)) - 1;           // 9600 bps
}

// Trata a comunicação serial
static uint8_t trata_serial ()
{
    int c = le_serial();
    uint8_t msgEd;
    uint8_t msg[T_MSG+3];
    uint8_t i, j;

    if ((c >= '1') && (c < ('1'+N_MSG)))
    {
        // seleciona a mensagem
        msg_atual = (uint8_t) (c - '1');
        return TRUE;
    }
    if (c == '*')
    {
        // carga de mensagem
        envia_serial("Carga\r\n");
        c = le_espera_serial();
        if ((c < '1') || (c > ('0'+N_MSG)))
            return FALSE;
        msgEd = (uint8_t) (c - '1');
        msg[0] = (uint8_t) c;
        msg[1] = ':';
        msg[2] = 0;
        i = 0;
        for (;;)
        {
            j = 0;
            ht1632c_clear();
            envia_serial ("\r");
            envia_serial ((char *) msg);
            while (j < (i+2))
            {
                display (msg[j], 0);
                j++;
            }
            c = le_espera_serial();
            if (c == 0x0d)
            {
                break;
            } else if (c == 0x1B)
            {
                return FALSE;
            } else if ((c == 0x08) && (i > 0))
            {
                envia_serial ("\x08 ");
                i--;
                msg[i+2] = 0;
            } else if ((c >= 0x20) && (c <= 0x7F) && (i < T_MSG))
            {
                msg[2+i++] = (uint8_t) c;
                msg[i+2] = 0;
            }
        }
        for (j = 0; j <= i; j++)
            eeprom_update_byte (&eepMsgs[msgEd][j], msg[2+j]);
        return TRUE;
    }
    
    return FALSE;
}

// Aguarda receber um caracter e o retorna
static int le_espera_serial ()
{
    int c;
    
    for (;;)
    {
        c = le_serial();
        if (c != -1)
            return c;
    }
}

// Pega o próximo caracter da fila de recepção da serial
// Retorna -1 se fila vazia
static int le_serial ()
{
    int c;
    cli ();
    if (poe == tira)
        c = -1;   // fila vazia
    else
    {
        c = filaRx[tira];
        if (++tira == T_FILA)
            tira = 0;
    }
    sei ();
    return c;
}

// Envia mensagem pela serial
static void envia_serial (char *pMsg)
{
    while (*pMsg != 0)
    {
        while ((UCSRA & _BV(UDRE)) == 0)
            ;
        UDR = *pMsg++;
    }
}

// Interrupção de recepção da USART
ISR(USART_RXC_vect)
{
    uint8_t prox;
    
    filaRx[poe] = UDR;      // pega o caracter
    prox = poe + 1;         // avança o ponteiro de entrada
    if (prox == T_FILA)
        prox = 0;
    if (prox != tira)       // não atualizar se fila cheia
        poe = prox;
}

// Trata os botões
static uint8_t trata_botoes ()
{
    static uint8_t teclas = TEC_KEY1|TEC_KEY2|TEC_KEY3;
    uint8_t ret = FALSE;
    
    if (((teclas & TEC_KEY1) == 0) && ((TEC_PIN & TEC_KEY1) != 0))
    {
        // soltou tecla 1 - avança para a próxima mensagem
        if (++msg_atual == N_MSG)
            msg_atual = 0;
        ret = TRUE;
    }
    if (((teclas & TEC_KEY2) == 0) && ((TEC_PIN & TEC_KEY2) != 0))
    {
        // soltou tecla 2 - recua para a mensagem anterior
        msg_atual = (msg_atual == 0) ? N_MSG-1 : msg_atual-1;
        ret = TRUE;
    }
    teclas = TEC_PIN;
    return ret;
}


// Mostra o caracter c
static void display (uint8_t c, uint8_t dly)
{
    uint8_t x, y, mask, gc, tam;
    int iFont;
    
    if ((c < 0x20) || (c > 0x7F))
        c = 0x7F;
    iFont = (c - 0x20) * 6;
    tam = pgm_read_byte(&(font[iFont]));
    
    for (x = 0; x < tam; x++)
    {
        desloca(FALSE);
        iFont++;
        gc = pgm_read_byte(&(font[iFont]));
        mask = 0x80;
        for (y = 0; y < 8; y++)
        {
            ht1632c_setLED (31, y, gc & mask);
            mask = mask >> 1;
        }
        delay_ticks (dly);
    }
    desloca (TRUE);
    delay_ticks (2);
}

// Desloca o conteudo display uma coluna para a esquerda
static void desloca (uint8_t fLimpa)
{
    uint8_t x, y;
    
    for (x = 1; x < 32; x++)
        for (y = 0; y < 8; y++)
            ht1632c_setLED(x-1, y, ht1632c_getLED(x, y));
    if (fLimpa)
        for (y = 0; y < 8; y++)
            ht1632c_setLED(31, y, 0);
}

// Inicia a contagem de tempo
static void tempo_init (void)
{
  ASSR |= (1<<AS2);     // timer2 async from external quartz
  TCCR2 = 0b00000010;   // normal,off,/8; 32768Hz/256/8 = 16 Hz
  TIMSK |= (1<<TOIE2);  // enable timer2 overflow int
  sei();                // enable interrupts
}

// Aguarda um certo número de ticks (1 tick = 1/16 segundo)
static void delay_ticks (uint8_t ticks)
{
    cnt_delay = ticks;
    while (cnt_delay)
        ;
}

// Interrupção do Timer2
ISR(TIMER2_OVF_vect) 
{
  if (cnt_delay)
      cnt_delay--;
}

