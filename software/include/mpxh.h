#ifndef MPXH_H_
#define MPXH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <Arduino.h>
#include <stdint.h>

// Pines usados por MPXH
#define MPXH_ENT    PIN_A8
#define MPXH_SAL    PIN_A9


// Configurar estos defines de acuerdo al intervalo de tiempo con que se llama a mpxh_Analizar.
// Se lo llama cada 250 usegs.
#define MPXH_MSEG						 4           // unidad de tiempo de t0
#define MPXH_MI_PRIORIDAD                10*MPXH_MSEG     // unidad de tiempo de t0
#define MPXH_T                           5           // unidad de tiempo de tbSend
#define MPXH_T9                          3           // unidad de tiempo de tbSend


typedef enum {
	MPXH_BITS_8 = 6,
	MPXH_BITS_4  = 5,
	MPXH_BITS_17 = 4,
	MPXH_BITS_16 = 3,
	MPXH_BITS_15 = 2,
	MPXH_BITS_12 = 1,
	MPXH_BITS_9 = 0
}MPXH_Bits_t;



void mpxh_init (void);
void mpxh_Analizar ( void );
uint32_t mpxh_Ocupado ( void );
void mpxh_ArmaMensaje ( uint8_t dataH, uint8_t dataL, uint8_t layer, uint8_t nbits );
uint8_t mpxh_recibio ( uint8_t flag );
void mpxh_getRecibido ( uint8_t *dataH, uint8_t *dataL, uint8_t* layer );
uint8_t mpxh_tiempoIdle ( uint8_t tiempo);
uint8_t mpxh_tiempoLow ( uint8_t tiempo);
void mpxh_clearRecFlag ( MPXH_Bits_t flag );
void mpxh_forceMPXHLow ( void );
void mpxh_releaseMPXH ( void );
void mpxh_abortTx ( void );
void mpxh_clearSendFlag ( uint8_t flag );


#ifdef __cplusplus
}
#endif

#endif /* MPXH_H_ */