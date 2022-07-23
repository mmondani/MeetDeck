#include <Arduino.h>
#include <HID-Project.h>
#include <SoftTimers.h>

#include "mpxh.h"

#define DUTY_INT  PIN_A7
#define DUTY_PAL  7
#define LED_MEET  PIN_A3
#define LED_TEAMS PIN_A2
#define LED_ZOOM  PIN_A1


uint8_t t0 = 0, t01 = 0, t1 = 0, t2 = 0;
bool timer_4ms = false;
uint8_t dataH, dataL, dataLayer;

/********************************************/
// FSM para cambiar de aplicación
/********************************************/
enum States {
  WAITING_00E,
  WAITING_00F,
  WAITING_010,
};

States stateSwitchApp = WAITING_00E;
SoftTimer stateTimer;
bool received00E, received00F, received010;

enum Apps {
  MEET,
  TEAMS,
  ZOOM,
};

Apps currentApp = MEET;

void switchAppHandler();
void updateLeds();
/********************************************/


void setup() {
  Keyboard.begin();

  pinMode(DUTY_INT, OUTPUT);
  pinMode(DUTY_PAL, OUTPUT);
  pinMode(LED_MEET, OUTPUT);
  pinMode(LED_TEAMS, OUTPUT);
  pinMode(LED_ZOOM, OUTPUT);

  TX_RX_LED_INIT;
  RXLED1;
  TXLED1;

  mpxh_init();

  updateLeds();

  cli();                      // Deshabilitar interrupciones globales

  TCCR3A = 0;
  TCCR3B = 0;
  TCCR3B |= 0x01;             // Sin prescaler (F = 16 MHz)
  TCCR3B |= (1 << WGM32);     // Habilita el modo CTC (se reinicia el timer al hacer un compare match)

  TCNT3 = 0;

  TIMSK3 |= (1 << OCIE3A);    // Se habilita la interrupción con compare match A

  OCR3A = 2000;               // Interrumpe cada 125 us

  sei();                      // Habilitar interrupciones globales
}

void loop() {
  if (timer_4ms) {
    timer_4ms = false;

    digitalWrite(DUTY_PAL, 1);

    if (mpxh_recibio (MPXH_BITS_17)) {
      mpxh_clearRecFlag(MPXH_BITS_17);
			mpxh_getRecibido(&dataH, &dataL, &dataLayer);

      if (dataH == 0x00) {
        if (dataL == 0xE0) {
          // Encender/apagar micrófono
          received00E = true;

          switch(currentApp) {
            case MEET:
              Keyboard.press(KEY_LEFT_CTRL);
              Keyboard.write(KEY_D);
              Keyboard.release(KEY_LEFT_CTRL);
              break;

            case TEAMS:
              Keyboard.press(KEY_LEFT_CTRL);
              Keyboard.press(KEY_LEFT_SHIFT);
              Keyboard.write(KEY_M);
              Keyboard.release(KEY_LEFT_CTRL);
              Keyboard.release(KEY_LEFT_SHIFT);
              break;

            case ZOOM:
              Keyboard.press(KEY_LEFT_ALT);
              Keyboard.write(KEY_A);
              Keyboard.release(KEY_LEFT_ALT);
              break;
          }
        }
        else if (dataL == 0xF0) {
          // Encender/apagar cámara
          received00F = true;

          switch(currentApp) {
            case MEET:
              Keyboard.press(KEY_LEFT_CTRL);
              Keyboard.write(KEY_E);
              Keyboard.release(KEY_LEFT_CTRL);
              break;

            case TEAMS:
              Keyboard.press(KEY_LEFT_CTRL);
              Keyboard.press(KEY_LEFT_SHIFT);
              Keyboard.write(KEY_O);
              Keyboard.release(KEY_LEFT_CTRL);
              Keyboard.release(KEY_LEFT_SHIFT);
              break;

            case ZOOM:
              Keyboard.press(KEY_LEFT_ALT);
              Keyboard.write(KEY_V);
              Keyboard.release(KEY_LEFT_ALT);
              break;
          }
        }
      }
      else if (dataH == 0x01 && dataL == 0x00) {
        // Levantar la mano
        received010 = true;

        switch(currentApp) {
          case MEET:
            Keyboard.press(KEY_LEFT_CTRL);
            Keyboard.press(KEY_LEFT_ALT);
            Keyboard.write(KEY_H);
            Keyboard.release(KEY_LEFT_CTRL);
            Keyboard.release(KEY_LEFT_ALT);
            break;

          case TEAMS:
            Keyboard.press(KEY_LEFT_CTRL);
            Keyboard.press(KEY_LEFT_SHIFT);
            Keyboard.write(KEY_K);
            Keyboard.release(KEY_LEFT_CTRL);
            Keyboard.release(KEY_LEFT_SHIFT);
            break;

          case ZOOM:
            Keyboard.press(KEY_LEFT_ALT);
            Keyboard.write(KEY_Y);
            Keyboard.release(KEY_LEFT_ALT);
            break;
        }
      }

      switchAppHandler();

      received00E = false;
      received00F = false;
      received010 = false;
    }
  }

  digitalWrite(DUTY_PAL, 0);
}


void switchAppHandler () {
  // Va a rotar entre las posibles apps (Meet, Teams y Zoom) al presionar las tres
  // teclas del T3W en orden, de arriba hacia abajo (la última presionada durante 2 segundos)

  switch(stateSwitchApp) {
    case WAITING_00E:
      if (received00E) {
        stateSwitchApp = WAITING_00F;
        stateTimer.setTimeOutTime(5000);
        stateTimer.reset();
      }
      break;


    case WAITING_00F:
      if (received00F) {
        stateSwitchApp = WAITING_010;
        stateTimer.setTimeOutTime(5000);
        stateTimer.reset();
      }
      else if (received00E || received010 || stateTimer.hasTimedOut())
        stateSwitchApp = WAITING_00E;

      break;


    case WAITING_010:
      if (received010) {
        if (currentApp == MEET)
          currentApp = TEAMS;
        else if (currentApp == TEAMS)
          currentApp = ZOOM;
        else if (currentApp == ZOOM)
          currentApp = MEET;

        updateLeds();

        stateSwitchApp = WAITING_00E;
      }
      else if (received00E || received00F || stateTimer.hasTimedOut())
        stateSwitchApp = WAITING_00E;
      break;
  }
}


void updateLeds () {
  if (currentApp == MEET) {
    digitalWrite(LED_MEET, 1);
    digitalWrite(LED_TEAMS, 0);
    digitalWrite(LED_ZOOM, 0);
  }
  else if (currentApp == TEAMS) {
    digitalWrite(LED_MEET, 0);
    digitalWrite(LED_TEAMS, 1);
    digitalWrite(LED_ZOOM, 0);
  }
  else if (currentApp == ZOOM) {
    digitalWrite(LED_MEET, 0);
    digitalWrite(LED_TEAMS, 0);
    digitalWrite(LED_ZOOM, 1);
  }
}



ISR(TIMER3_COMPA_vect) {
  // Interrumpe cada 125 us
  t0++;

  if (t0 == 2) {
    // Cada 250 us
    t0 = 0;

    digitalWrite(DUTY_INT, 1);

    mpxh_Analizar();

    t01++;

    if (t01 == 4) {
      // Cada 1 ms

      t01 = 0;

      t1++;

      if (t1 == 4) {
        // Cada 4 ms
        t1 = 0;

        timer_4ms = true;
      }
    }
  }

  digitalWrite(DUTY_INT, 0);
}