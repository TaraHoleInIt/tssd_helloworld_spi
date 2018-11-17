/**
 * Copyright (c) 2018 Tara Keeling
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include <Arduino.h>
#include <SPI.h>
#include "src/tssd1306/ssd1306.h"

void SPIWrapperBeginTransaction( tssd1306* DeviceHandle, bool IsCommand );
size_t SPIWrapperWrite( tssd1306* DeviceHandle, const uint8_t* Data, size_t Length );
void SPIWrapperWriteByte( tssd1306* DeviceHandle, const uint8_t Data );
void SPIWrapperEndTransaction( tssd1306* DeviceHandle );

const int SPIFrequency = 1000000; // 1MHz

#if defined ATTINY_CORE
  // If you use an external reset circuit with a capacitor and a resistor
  // you can free up this pin.
  const int MasterResetPin = PINB3;

  // This is actually the USI DI pin but since these displays are write only
  // we can "safely" reclaim this after USI has been initialized.
  const int MasterDCPin = PINB0;

  const int DisplayCSPin = PINB4;
#else
  const int MasterResetPin = 2;
  const int MasterDCPin = 3;

  const int DisplayCSPin = 10;
#endif

static DisplayInterfaceProcs SPIWrapper = {
    SPIWrapperBeginTransaction,
    SPIWrapperWrite,
    SPIWrapperWriteByte,
    SPIWrapperEndTransaction,
    16  // No hard limit it seems? but be sane
};

void SPIWrapperBeginTransaction( tssd1306* DeviceHandle, bool IsCommand ) {
    if ( DeviceHandle->CSPin > -1 ) {
        digitalWrite( DeviceHandle->CSPin, LOW );
    }

    if ( MasterDCPin > -1 ) {
        digitalWrite( MasterDCPin, ( IsCommand == true ) ? LOW : HIGH );
    }

    SPI.beginTransaction( SPISettings( SPIFrequency, MSBFIRST, SPI_MODE0 ) );
}

size_t SPIWrapperWrite( tssd1306* DeviceHandle, const uint8_t* Data, size_t Length ) {
  SPI.transfer( Data, Length );
    return Length;
}

void SPIWrapperWriteByte( tssd1306* DeviceHandle, const uint8_t Data ) {
    SPI.transfer( Data );
}

void SPIWrapperEndTransaction( tssd1306* DeviceHandle ) {
    if ( DeviceHandle->CSPin > -1 ) {
        digitalWrite( DeviceHandle->CSPin, HIGH );
    }

    if ( MasterDCPin > -1 ) {
        digitalWrite( MasterDCPin, HIGH );
    }

    SPI.endTransaction( );
}

tssd1306 SPIDisplay = {
  &SPIWrapper, // spi interface callback
  128,  // 128px wide
  64, // 64px tall
  0, // spi so no i2c address
  DisplayCSPin, // chip select pin or -1 if permanently tied low
  0,  // text print x coordinate
  0,  // text print y coordinate
  &Font_Droid_Sans_Fallback_17x18 // font
};

void ResetDisplay( void ) {
  if ( MasterResetPin > -1 ) {
    pinMode( MasterResetPin, OUTPUT );

    digitalWrite( MasterResetPin, LOW );
    delay( 10 );

    digitalWrite( MasterResetPin, HIGH );
    delay( 15 );
  } else {
    // In case we're using an external reset circuit or one is
    // built into the display, just make sure it's finished.
    delay( 25 );
  }
}

void DoDisplaySetup( void ) {
  ResetDisplay( );

  pinMode( MasterDCPin, OUTPUT );
  digitalWrite( MasterDCPin, LOW );

  if ( SPIDisplay.CSPin > -1 ) {
    pinMode( SPIDisplay.CSPin, OUTPUT );
    digitalWrite( SPIDisplay.CSPin, HIGH );
  }

  SSD1306_Init( &SPIDisplay );
}

void setup( void ) {
  SPI.begin( );
  DoDisplaySetup( );

  // If your display is upside-down uncomment these below:
  // Note that horizontal flip only takes effect with new data
  SSD1306_SendSingleByteCommand( &SPIDisplay, SSD1306_Op_Horizontal_Flip_On );
  SSD1306_SendSingleByteCommand( &SPIDisplay, SSD1306_Op_Vertical_Flip_On );
  
  SSD1306_PrintString( &SPIDisplay, "Hello, World!" );
}

void loop( void ) {
}
