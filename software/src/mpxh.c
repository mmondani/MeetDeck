#include "mpxh.h"
#include "basicDefinitions.h"


/*****************************************************************************/
//  ESTRUCTURAS PRIVADAS
/*****************************************************************************/
typedef union {
   uint8_t byte;
   struct {
        uint8_t send_9:1;
        uint8_t send_12:1;
        uint8_t send_15:1;
        uint8_t send_16:1;
        uint8_t send_17:1;
        uint8_t send_4:1;
        uint8_t bit6:1;
        uint8_t sending:1;
   } bits;
} _sendActivo;

typedef union {
   uint8_t byte;
   struct {
        uint8_t received_9:1;
        uint8_t received_12:1;
        uint8_t received_15:1;
        uint8_t received_16:1;
        uint8_t received_17:1;
        uint8_t received_4:1;
        uint8_t bit6:1;
        uint8_t receiving:1;
   } bits;
} _received;

typedef union {
   uint8_t byte;
   struct {
        uint8_t bit0:1;
        uint8_t bit1:1;
        uint8_t bit2:1;
        uint8_t bit3:1;
        uint8_t bit4:1;
        uint8_t lbitent:1;
        uint8_t bitent:1;
        uint8_t bit7:1;
   } bits;
} _regent;

typedef union {
        uint8_t bytes[2];
        uint16_t word;
} _wSend;

typedef union {
        uint8_t bytes[2];
        uint16_t word;
} _auxwSend;

typedef union {
        uint8_t bytes[2];
        uint16_t word;
} _wRecAux;

typedef union {
   uint8_t byte;
   struct {
        uint8_t mpxError:1;
        uint8_t es_mpxc:1;
        uint8_t bit2:1;
        uint8_t bit3:1;
        uint8_t bit4:1;
        uint8_t bit5:1;
        uint8_t bit6:1;
        uint8_t bit7:1;
   } bits;
} _mpxFlags;

const volatile uint8_t tablaHamMpx [18] =      {0,                       //entre 0 y 16 (17 entradas)
                                           0b10000000,
                                           0b10001111,
                                           0b10001110,
                                           0b10001101,
                                           0b10001011,
                                           0b10000111,
                                           0b10001100,
                                           0b10001010,
                                           0b10001001,
                                           0b10000110,
                                           0b10000101,
                                           0b10000011,
                                           0b10001000,
                                           0b10000100,
                                           0b10000010,
                                           0b10000001,
                                           0};               // 17 (nbr=18, no puede subir más)



// static para que solo la vea este .c
// volatile porque la usa la ISR y para que no la optimice el compi
static volatile uint8_t tbSend = 0;
static volatile uint8_t tbr0 = 0;
static volatile uint8_t tbr1 = 0;
static volatile _auxwSend auxwSend = {.word = 0};

static volatile uint8_t nbs = 0;
static volatile uint8_t nSent = 0;
static volatile uint8_t nBits = 0;
static volatile uint8_t tMistake = 0;
static volatile uint8_t basurita = 0;
static volatile _sendActivo sendActivo = {.byte = 0};
static volatile _wSend wSend = {.word = 0};
static volatile _received received = {.byte = 0};
static volatile _regent regent = {.byte = 0};

static volatile uint8_t nbr = 0;
static volatile uint8_t regHamR = 0;
static volatile uint8_t wRecL = 0;
static volatile uint8_t wRecH = 0;
static volatile uint8_t wRecLayer = 0;
static volatile _wRecAux wRecAux = {.word = 0};

static volatile uint8_t Taux = 0;
static volatile _mpxFlags mpxFlags = {.byte = 0};

static volatile uint8_t t_alto = 0;
static volatile uint8_t t_bajo = 0;


static void mpxh_txFallido (void);
static void mpxh_verMistake (void);
static void mpxh_armaHamm (uint8_t esParidad);


void mpxh_init (void)
{
    // Se inicializan los pines de entrada y salida
    pinMode(MPXH_ENT, INPUT);
    pinMode(MPXH_SAL, OUTPUT);	
	
	// Inicialización de las variables
	sendActivo.byte = 0;
	received.byte = 0;
	wRecAux.bytes[0] = 0;
	wRecAux.bytes[1] = 0;
	
}



void mpxh_Analizar (void)
{
    tbSend++;

// muestras:
    regent.byte = regent.byte >> 1;
    bit_clear(regent.byte,7);

    ( digitalRead(MPXH_ENT) ) ? (regent.bits.bit7 = 1) : (regent.bits.bit7 = 0);

// corregir:

    if ( (regent.byte & 0b11100000) == 0b10100000)
        bit_set(regent.byte, 6);
    else if ( ( regent.byte & 0b11100000 ) == 0b01000000)
        bit_clear(regent.byte, 6);


// tbr1inc:
    if ( regent.bits.bitent )
    {
        if ( tbr1 < 255 )
        {
            tbr1++;
        }
        mpxFlags.bits.mpxError = 0;
    }
    else
    {
// tbr0inc:
        tbr0++;
    }
// fintbr1inc:

// trasmitir
    if (!sendActivo.bits.sending)
    {


        if ( tbr1 < MPXH_MI_PRIORIDAD )
        {
            mpxh_verMistake();
            goto fintransmi;

        }
        if ( !sendActivo.byte ) // entra si es 0
        {
            mpxh_verMistake();

            goto fintransmi;
        }
		
		
		if (digitalRead(MPXH_ENT)) {
			digitalWrite(MPXH_SAL, 1);
			
			// nbs = 32 - nBits;
			if ( sendActivo.bits.send_17 )
			{
				nbs = 15;
			}
			else if ( sendActivo.bits.send_16 )
			{
				nbs = 16;
			}
			else if ( sendActivo.bits.send_15 )
			{
				nbs = 17;
			}
			else if ( sendActivo.bits.send_12 )
			{
				nbs = 20;
			}
			else if ( sendActivo.bits.send_9 )
			{
				nbs = 23;
			}
			else if ( sendActivo.bits.send_4 )
			{
				nbs = 28;
			}

			sendActivo.bits.sending = 1;
			auxwSend.bytes[1] = wSend.bytes[1];
			auxwSend.bytes[0] = wSend.bytes[0];
			if ((sendActivo.bits.send_9 == 1) || (sendActivo.bits.send_12 == 1))
			{
				tbSend = 2*MPXH_T9;
			}
			else
			{
				tbSend = 2*MPXH_T;
			}
		}
		
        

    }
// sen1:
    // if ( tMistake == 64 )
    if ( bit_test( tMistake,6 ) )   // no hubo error
    {
        if((regent.bits.bit7 == 0) && (digitalRead(MPXH_SAL) == 0) )   // si tengo 00 entro
        {
            if (basurita == 0)
            {
                basurita = 1;

            }
            else
            {
                mpxh_txFallido();
                mpxh_verMistake();
                goto fintransmi;
            }

        }
        else        // si la combinacion es distinto de 00
        {
// sen8:
            basurita = 0;
        }
    }
    else    // error de mistake
    {
        mpxh_txFallido();
        mpxh_verMistake();
        goto fintransmi;
    }

// send81:
    if ( bit_test( nbs,5 ))
    {
        if ( tbSend < 6*MPXH_MSEG )
        {
            goto fintransmi;
        }
        // tbSend >= 6*MSEG
        sendActivo.byte = 0;
        mpxh_verMistake();
        goto fintransmi;
    }

// sen4:
    if ((sendActivo.bits.send_9 == 1) || (sendActivo.bits.send_12 == 1))
    {
        Taux = MPXH_T9;
    }
    else
    {
        Taux = MPXH_T;
    }

    if ( tbSend == Taux )
    {
        if ( !bit_test( auxwSend.bytes[1],7 ) )        // pregunto antes por el carry si es 0(asi no lo pierdo)
        {
            digitalWrite(MPXH_SAL, 1);
        }
        auxwSend.word = auxwSend.word << 1;
    }
// sen5:
    if ((sendActivo.bits.send_9 == 1) || (sendActivo.bits.send_12 == 1))
    {
        Taux = 2*MPXH_T9;
    }
    else
    {
        Taux = 2*MPXH_T;
    }

    if ( tbSend == Taux )
    {
        digitalWrite(MPXH_SAL, 1);
    }
// sen3:
    if ((sendActivo.bits.send_9 == 1) || (sendActivo.bits.send_12 == 1))
    {
        Taux = 3*MPXH_T9;
    }
    else
    {
        Taux = 3*MPXH_T;
    }
    if ( tbSend == Taux )
    {
        digitalWrite(MPXH_SAL, 0);
        nbs++;
        tbSend = 0;
    }
fintransmi:

// recibir:
    if ( received.bits.receiving == 0 )
    {
        if ( regent.bits.bitent == 0)
        {
            if( tbr1 >= 6*MPXH_MSEG )
            {
                received.bits.receiving = 1;
                regHamR = 0;
                nbr = 0;
                tbr0 = 0;
				wRecAux.word = 0;
            }


            tbr1 = 0;
        }
    }
    else
    {
// rec20:
        if ( tbr0 >= 4*MPXH_MSEG )       // Pulso largo en bajo (error)
        {
            tbr1 = 0;
            received.bits.receiving = 0;

            mpxFlags.bits.mpxError = 1;
        }
        else
        {
// rec2:
            if ( tbr1 < 4*MPXH_MSEG )
            {
                // Pregunto si vino flanco positivo para chequear bit MPX)
                if ( regent.bits.bitent == 1 && regent.bits.lbitent == 0 )
                {
                    wRecAux.word = wRecAux.word << 1;
                    if ( tbr1 > tbr0 )
                        bit_set( wRecAux.bytes[0], 0);  // Vino un 1

                    else
                        bit_clear( wRecAux.bytes[0], 0);    // Vino un 0

                    if ( bit_test(wRecAux.bytes[0],0) )
                        regHamR ^= tablaHamMpx[nbr];

                    nbr++;
                    tbr1 = 0;
                    tbr0 = 0;

                    if ( nbr == 17 )
                    {
                        if ( regHamR != 0 )
                        {
                            tMistake = 0;
                            received.bits.receiving = 0;
                        }

                    }
                    else if ( nbr > 17 )
                    {
                        tMistake = 0;
                        received.bits.receiving = 0;
                    }

                }

            }
            else    // tbr1 >= 4*MSEG
            {
				if ( sendActivo.bits.sending )
                {
                    received.bits.receiving = 0;                 // no me recibo a mi mismo
                }
                else if ( nbr == 16 )
                {
                    if ( (regHamR & 0x80) == 0 )  // si no es cero me tengo q ir
                    {
                        if ( tbr1 >= 5*MPXH_MSEG )
                        {
                            wRecAux.word = wRecAux.word << 1;				// justifico a izquierda
                            bit_clear(wRecAux.bytes[1],7);					// clereo la paridad
                            received.bits.received_16 = 1;
                            wRecH = wRecAux.bytes[1];
                            wRecL = wRecAux.bytes[0];
							wRecLayer = (wRecAux.bytes[0] & 0x0F) >> 1;		// Guardo el layer en el que vino
							wRecL = wRecL & 0xF0;							// se borra el layer
                            received.bits.receiving = 0;
                        }
                    }
                    else
                    {
                        tMistake = 0;
                        received.bits.receiving = 0;
                    }

                }
                else // ( nbr != 16 )
                {
// rec31
// rec32
                    switch (nbr)
                    {
                        case 4: received.bits.received_4 = 1; 
								wRecLayer = 0;
								wRecAux.bytes[1] = 0;
								wRecAux.bytes[0] = wRecAux.bytes[0] & 0x0F;
								break;
                        case 9: received.bits.received_9 = 1; 
								wRecLayer = 0;
								wRecAux.bytes[1] = 0;
								break;
                        case 12: received.bits.received_12 = 1;
								wRecLayer = wRecAux.bytes[1] & 0x0F;
								wRecAux.bytes[1] = 0;
								break;
                        case 15: received.bits.received_15 = 1; 
								wRecLayer = 0;
								break;
                        case 17: received.bits.received_17 = 1;
								bit_clear (wRecAux.bytes[1],7);			// clereo bit paridad
								wRecAux.bytes[0] &= 0xF0;				// elimino Hamming
								wRecLayer = 0;
								break;
                    }

                    wRecH = wRecAux.bytes[1];
                    wRecL = wRecAux.bytes[0];

                    received.bits.receiving = 0;

                }
            }
        }
    }
}

static void mpxh_txFallido (void)
{
// sen10:
    nSent++;
    if ( bit_test(nSent,4))
    {
        sendActivo.byte = 0;
    }
    sendActivo.bits.sending = 0;

}

uint32_t mpxh_Ocupado (void)
{
    return( sendActivo.byte != 0 );
}

void mpxh_ArmaMensaje( uint8_t dataH, uint8_t dataL, uint8_t layer, uint8_t nbits )
{
    wSend.bytes[1] = dataH;
    wSend.bytes[0] = dataL;
    nSent = 0;

    if ( nbits == MPXH_BITS_17)
    {
        mpxh_armaHamm( 0 );
    }
    else if (nbits == MPXH_BITS_16)
    {
        wSend.bytes[0] = wSend.bytes[0] & 0xF0;
        wSend.bytes[0] = wSend.bytes[0] | (layer << 1);

        mpxh_armaHamm( 1 );
    }
    else if (nbits == MPXH_BITS_12)
    {
        wSend.word = wSend.word >> 3;
        wSend.bytes[1] = wSend.bytes[1] & 0b00011111;
        wSend.bytes[1] = wSend.bytes[1] | (layer & 0x07) << 5;
/*      
#asm
        RRCF     _wSendH,f
        RRCF     _wSendL,f

        RRCF    _wSendH,f
        RRCF    _wSendL,f

        RRCF    _wSendH,f
        RRCF    _wSendL,f
#endasm
 */

        
    }
    else if (nbits == MPXH_BITS_15)
    {
        wSend.word = wSend.word << 2;
        wSend.bytes[0] = wSend.bytes[0] & 0b11111100;
/*
#asm
        RLCF    _wSendL,f
        BCF     _wSendL,0
        LSLF    _wSendH,f

        LSLF    _wSendL,f
        LSLF    _wSendH,f
#endasm
 */
    }

    bit_set(sendActivo.byte, nbits);
}

static void mpxh_verMistake (void)
{
// ver_mistake:
    if ( bit_test(tMistake,6) )     // entro si esta OK
    {

        if ( !sendActivo.bits.sending )
        {
            digitalWrite(MPXH_SAL, 0);
        }
    }
// hay_mistake:
    else        // entro si hay mistake
    {

        digitalWrite(MPXH_SAL, 1);
        tMistake++;
        if ( bit_test(tMistake,6) )
        {
            digitalWrite(MPXH_SAL, 0);
        }
    }
// fin_mistake:
}

static void mpxh_armaHamm (uint8_t esParidad)
{
    register uint8_t wAux = 0;

    if (!esParidad)
        wSend.bytes[0] = wSend.bytes[0] & 0xF0;

    if ( bit_test( wSend.bytes[1],6 ) )
        wAux ^= 0b10001111 ;
    if ( bit_test( wSend.bytes[1],5 ) )
        wAux ^= 0b10001110 ;
    if ( bit_test( wSend.bytes[1],4 ) )
        wAux ^= 0b10001101 ;
    if ( bit_test( wSend.bytes[1],3 ) )
        wAux ^= 0b10001011 ;
    if ( bit_test( wSend.bytes[1],2 ) )
        wAux ^= 0b10000111 ;
    if ( bit_test( wSend.bytes[1],1 ) )
        wAux ^= 0b10001100 ;
    if ( bit_test( wSend.bytes[1],0 ) )
        wAux ^= 0b10001010 ;
    if ( bit_test( wSend.bytes[0],7 ) )
        wAux ^= 0b10001001 ;
    if ( bit_test( wSend.bytes[0],6 ) )
        wAux ^= 0b10000110 ;
    if ( bit_test( wSend.bytes[0],5 ) )
        wAux ^= 0b10000101 ;
    if ( bit_test( wSend.bytes[0],4 ) )
        wAux ^= 0b10000011 ;

    if (!esParidad)
        wSend.bytes[0] = wSend.bytes[0] | ( wAux & 0x0F );
    
    wAux = wAux & 0x80;

    if ( bit_test( wSend.bytes[0],3 ) )
        wAux ^= 0b10000000 ;
    if ( bit_test( wSend.bytes[0],2 ) )
        wAux ^= 0b10000000 ;
    if ( bit_test( wSend.bytes[0],1 ) )
        wAux ^= 0b10000000 ;
    if ( bit_test( wSend.bytes[0],0 ) )
        wAux ^= 0b10000000 ;

    wSend.bytes[1] = wSend.bytes[1] | wAux ;

}

uint8_t mpxh_recibio ( uint8_t flag )
{
    return ( bit_test (received.byte, flag) );
}

void mpxh_clearRecFlag ( MPXH_Bits_t flag )
{
    bit_clear ( received.byte, flag );
}

void mpxh_getRecibido ( uint8_t *dataH, uint8_t *dataL, uint8_t* layer )
{
    *dataH = wRecH;
    *dataL = wRecL;
	*layer = wRecLayer;
}

uint8_t mpxh_tiempoIdle ( uint8_t tiempo)
{
    return ( tbr1 > tiempo );
}


uint8_t mpxh_tiempoLow ( uint8_t tiempo)
{
	return ( tbr0 > tiempo );
}


void mpxh_forceMPXHLow (void)
{
    digitalWrite(MPXH_SAL, 1);
}

void mpxh_releaseMPXH (void)
{
    digitalWrite(MPXH_SAL, 0);

    tbr1 = 0;
    tbr0 = 0;
}

void mpxh_abortTx (void)
{
    sendActivo.byte = 0x00;
}


uint8_t mpxh_getError (void)
{
    return mpxFlags.bits.mpxError == 1;
}


void mpxh_clearSendFlag ( uint8_t flag )
{
	bit_clear (sendActivo.byte, flag);
}